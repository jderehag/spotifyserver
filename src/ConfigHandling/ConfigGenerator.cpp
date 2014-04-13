/*
 * Copyright (c) 2014, Jens Nielsen
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL JENS NIELSEN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "ConfigDefs.h"
#include "ConfigGenerator.h"
#include "ConfigParser.h"
//#include "applog.h"

#include <string.h>
#include <assert.h>


// write items from a list of SectionAttributes to an output config
static void writeAttributes( std::list<std::string>& outconfig,
                             std::list<SectionAttributes>& listOfAttributes,
                             std::list<SectionAttributes>::iterator& listOfAttributesIterator)
{
    while(true)
    {
        const SectionAttributes& attribute = *listOfAttributesIterator;

        switch ( attribute.type )
        {
            case TYPE_SECTION:
            {
                std::list<std::string> subScope;

                // write start tag
                outconfig.push_back("");
                outconfig.push_back("Section  \"" + std::string(attribute.attributeName) + "\"");

                /* check if we should go deeper into the tree */
                listOfAttributesIterator++;
                if ( listOfAttributesIterator != listOfAttributes.end() &&
                     listOfAttributesIterator->level >= attribute.level )
                {
                    // next entry is deeper, it belongs to the Section
                    // recursive call to write all sub-entries
                    writeAttributes( outconfig, listOfAttributes, listOfAttributesIterator );
                }

                // write end tag
                outconfig.push_back("EndSection");
                outconfig.push_back("");
            }
            break;

            case TYPE_SUBSECTION:
            {
                std::list<std::string> subScope;

                // write start tag
                outconfig.push_back("SubSection   \"" + std::string(attribute.attributeName) + "\"");

                /* check if we should go deeper into the tree */
                listOfAttributesIterator++;
                if ( listOfAttributesIterator != listOfAttributes.end() &&
                     listOfAttributesIterator->level >= attribute.level )
                {
                    // next entry is deeper, it belongs to the SubSection
                    // recursive call to write all sub-entries
                    writeAttributes( outconfig, listOfAttributes, listOfAttributesIterator );
                }
                // and finally end tag
                outconfig.push_back("EndSubSection");
            }
            break;

            case TYPE_ATTRIBUTE:
            {
                //write an attribute line
                outconfig.push_back( std::string(attribute.attributeName) + "\t\t\"" + *attribute.attributeValue + "\"" );

                listOfAttributesIterator++;
                // sanity check that next entry isn't deeper
                if ( listOfAttributesIterator != listOfAttributes.end() )
                    assert(listOfAttributesIterator->level <= attribute.level);
            }
            break;
        }

        // next entry has lower depth, we're done with this scope
        if ( listOfAttributesIterator == listOfAttributes.end() ||
             listOfAttributesIterator->level < attribute.level )
            break;
    }
}

// extract a subset from a list of SectionAttributes
static bool findSection( std::list<SectionAttributes>& scope, std::string name, unsigned int level, ElementType type, std::list<SectionAttributes>& outputScope )
{
    std::list<SectionAttributes>::iterator it = scope.begin();
    for ( ; it != scope.end() ; it++ )
    {
        if ( (*it).level == level && (*it).type == type && name.compare((*it).attributeName) == 0 )
        {
            /* we have a match, move all related items to output list */
            do
            {
                outputScope.push_back(*it);
                scope.erase(it++);
            } while ( it != scope.end() && (*it).level > level );
            return true;
        }
    }
    return false;
}

static void appendLine( const std::string linein, std::list<std::string>& outconfig, unsigned int level, std::list<SectionAttributes>& scope )
{
    std::list<SectionAttributes> subScope;
    std::string lineout = linein;

    // if this is an attribute, the name starts with first non-whitespace and ends with following whitespace
    size_t startName = linein.find_first_not_of(" \t");
    size_t endName = linein.find_first_of(" \t", startName);
    if ( startName != std::string::npos && endName != std::string::npos )
    {
        std::string name = linein.substr(startName, endName-startName);
        findSection( scope, name, level, TYPE_ATTRIBUTE, subScope );
        assert( subScope.size() <= 1 ); // this is an attribute, should get one or zero SectionAttributes

        if ( subScope.size() == 1 )
        {
            // extract attribute value from within quotes
            size_t startVal = linein.find_first_of('"', endName);
            size_t endVal = std::string::npos;

            if ( startVal != std::string::npos )
                endVal = linein.find_first_of('"', startVal+1);

            if ( startVal != std::string::npos && endVal != std::string::npos )
            {
                // found attribute entry, write new string
                lineout = linein.substr(0, startVal+1); // everything from old string up until first quote
                lineout.append( *(subScope.front().attributeValue) ); // new value
                lineout.append( linein.substr(endVal, std::string::npos)); // everything from old string from last quote
            }
        }
    }

    outconfig.push_back(lineout);
}

//iterate a section or subsection scope of a config file
static void iterateOldConfigSection( const std::list<std::string>& oldConfig, std::list<std::string>::const_iterator& oldConfigIterator, 
                                     std::list<std::string>& outconfig, unsigned int level, ElementType type, std::list<SectionAttributes>& scope )
{
    // find which tag will close this scope
    std::string endTag;
    if ( type == TYPE_SECTION ) endTag = "EndSection";
    else if ( type == TYPE_SUBSECTION ) endTag = "EndSubSection";
    else assert(0);

    // copy start tag to output
    outconfig.push_back(*oldConfigIterator);
    oldConfigIterator++;

    while ( oldConfigIterator != oldConfig.end() )
    {
        std::list<SectionAttributes> subScope;
        //ignore whitespace at beginning of line
        size_t start = (*oldConfigIterator).find_first_not_of(" \t");

        if ( start != std::string::npos && (*oldConfigIterator).compare(start, strlen("SubSection"), "SubSection") == 0 )
        {
            //found a new subsection
            size_t startMark = (*oldConfigIterator).find_first_of('"');
            size_t endMark = std::string::npos;
            if(startMark != std::string::npos)
                endMark = (*oldConfigIterator).find_first_of('"', startMark + 1);
            //if(endMark == std::string::npos) return;

            if ( startMark != std::string::npos && endMark != std::string::npos )
            {
                std::string sectionName = (*oldConfigIterator).substr( startMark + 1, endMark - startMark - 1 );

                // extract corresponding SectionAttributes for this subscope from scope
                if ( findSection( scope, sectionName, level, TYPE_SUBSECTION, subScope ) )
                    subScope.erase(subScope.begin()); // if found, remove the TYPE_SUBSECTION entry
            }
            //else remove scope?

            // recursive call to handle new subscope
            iterateOldConfigSection( oldConfig, oldConfigIterator, outconfig, level+1, TYPE_SUBSECTION, subScope);
        }
        else if ( start != std::string::npos && (*oldConfigIterator).compare(start, endTag.length(), endTag) == 0 )
        {
            // Found end tag for this scope, write any remaining attributes to output
            std::list<SectionAttributes>::iterator it = scope.begin();
            if ( it != scope.end() )
                writeAttributes( outconfig, scope, it);

            // copy end tag to output
            outconfig.push_back(*oldConfigIterator);
            oldConfigIterator++;

            return;
        }
        else
        {
            // comment or attribute, find and write to output
            appendLine( *oldConfigIterator, outconfig, level, scope );
            oldConfigIterator++;
        }
    }

    //log(LOG_WARN) << "Reached end of inconfig but missing end tag " << endTag;

    std::list<SectionAttributes>::iterator it = scope.begin();
    if ( it != scope.end() )
        writeAttributes( outconfig, scope, it);

    outconfig.push_back( endTag );
}

// iterates a config file at root level, here we should only find "Section" and comments
static void iterateOldConfig( const std::list<std::string>& oldConfig, std::list<std::string>::const_iterator& oldConfigIterator,
                              std::list<std::string>& outconfig, std::list<SectionAttributes>& scope)
{
    while ( oldConfigIterator != oldConfig.end() )
    {
        std::list<SectionAttributes> subScope;
        size_t start = (*oldConfigIterator).find_first_not_of(" \t");

        if ( start != std::string::npos && (*oldConfigIterator).compare(start, strlen("Section"), "Section") == 0 )
        {
            size_t startMark = (*oldConfigIterator).find_first_of('"');
            if(startMark == std::string::npos) return;
            size_t endMark = (*oldConfigIterator).find_first_of('"', startMark + 1);
            if(endMark == std::string::npos) return;

            std::string sectionName = (*oldConfigIterator).substr(startMark + 1, endMark - startMark - 1);

            if ( findSection( scope, sectionName, 0, TYPE_SECTION, subScope ) )
                subScope.erase(subScope.begin()); // get rid of "TYPE_SECTION" tag

            iterateOldConfigSection( oldConfig, oldConfigIterator, outconfig, 1, TYPE_SECTION, subScope);
        }
        else
        {
            // todo: verify this is empty or comment, add '#' otherwise
            outconfig.push_back((*oldConfigIterator));
            oldConfigIterator++;
        }
    }
}

void generateConfig( const std::list<std::string>& inconfig, std::list<std::string>& outconfig,
                     std::list<SectionAttributes>& listOfSubSections )
{
    // first go through old config file and match sections/subsections/attributes and corresponding values from listOfSubSections
    // items will be removed from listOfSubSections if they are found in old config and written to new config
    std::list<std::string>::const_iterator iterator = inconfig.begin();
    iterateOldConfig( inconfig, iterator, outconfig, listOfSubSections );

    // Finally, write everything (if anything) that wasn't in the old config but should be there
    std::list<SectionAttributes>::iterator it = listOfSubSections.begin();
    if ( it != listOfSubSections.end() )
        writeAttributes( outconfig, listOfSubSections, it);
}

