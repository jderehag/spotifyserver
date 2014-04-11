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
#include "applog.h"

#include <string.h>
#include <assert.h>


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

                outconfig.push_back("");
                std::string line = "Section  \"";
                line.append(attribute.attributeName);
                line.append("\"");
                outconfig.push_back(line);

                /* check if we should go deeper into the tree */
                listOfAttributesIterator++;
                if ( listOfAttributesIterator != listOfAttributes.end() &&
                     listOfAttributesIterator->level >= attribute.level )
                {
                    writeAttributes( outconfig, listOfAttributes, listOfAttributesIterator );
                }
                outconfig.push_back("EndSection");
                outconfig.push_back("");
            }
            break;

        case TYPE_SUBSECTION:
            {
                std::list<std::string> subScope;

                std::string line = "SubSection   \"";
                line.append(attribute.attributeName);
                line.append("\"");
                outconfig.push_back(line);

                /* check if we should go deeper into the tree */
                listOfAttributesIterator++;
                if ( listOfAttributesIterator != listOfAttributes.end() &&
                     listOfAttributesIterator->level >= attribute.level )
                {
                    writeAttributes( outconfig, listOfAttributes, listOfAttributesIterator );
                }
                outconfig.push_back("EndSubSection");
            }
            break;

        case TYPE_ATTRIBUTE:
            std::string line = attribute.attributeName;
            line.append( "\t\t\"" + *attribute.attributeValue + "\"" );
            outconfig.push_back(line);

            listOfAttributesIterator++;
            if ( listOfAttributesIterator != listOfAttributes.end() )
                assert(listOfAttributesIterator->level <= attribute.level);
            break;
        }

        if ( listOfAttributesIterator == listOfAttributes.end() ||
             listOfAttributesIterator->level < attribute.level )
            break;
    }
}

static bool findSection( std::list<SectionAttributes>& scope, std::string name, unsigned int level, ElementType type, std::list<SectionAttributes>& outputScope )
{
    std::list<SectionAttributes>::iterator it = scope.begin();
    for ( ; it != scope.end() ; it++ )
    {
        if ( (*it).level == level && (*it).type == type && name.compare((*it).attributeName) == 0 )
        {
            /**/
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

    size_t startName = linein.find_first_not_of(" \t");
    size_t endName = linein.find_first_of(" \t", startName);
    if ( startName != std::string::npos && endName != std::string::npos )
    {
        std::string name = linein.substr(startName, endName-startName);
        findSection( scope, name, level, TYPE_ATTRIBUTE, subScope );
        assert( subScope.size() <= 1 );

        if ( subScope.size() == 1 )
        {
            size_t startVal = linein.find_first_of('"', endName);
            size_t endVal = std::string::npos;

            if ( startVal != std::string::npos )
                endVal = linein.find_first_of('"', startVal+1);

            if ( startVal != std::string::npos && endVal != std::string::npos )
            {
                lineout = linein.substr(0, startVal+1);
                lineout.append( *(subScope.front().attributeValue) );
                lineout.append( linein.substr(endVal, std::string::npos));
            }
        }
    }

    outconfig.push_back(lineout);
}

static void iterateOldConfigSection( const std::list<std::string>& oldConfig, std::list<std::string>::const_iterator& oldConfigIterator, 
                                     std::list<std::string>& outconfig, unsigned int level, ElementType type, std::list<SectionAttributes>& scope )
{
    std::string endTag;
    if ( type == TYPE_SECTION ) endTag = "EndSection";
    else if ( type == TYPE_SUBSECTION ) endTag = "EndSubSection";
    else assert(0);

    while ( oldConfigIterator != oldConfig.end() )
    {
        std::list<SectionAttributes> subScope;
        size_t start = (*oldConfigIterator).find_first_not_of(" \t");

        if ( start != std::string::npos && (*oldConfigIterator).compare(start, strlen("SubSection"), "SubSection") == 0 )
        {
            size_t startMark = (*oldConfigIterator).find_first_of('"');
            if(startMark == std::string::npos) return;
            size_t endMark = (*oldConfigIterator).find_first_of('"', startMark + 1);
            if(endMark == std::string::npos) return;

            std::string sectionName = (*oldConfigIterator).substr(startMark + 1, endMark - startMark - 1);

            if ( findSection( scope, sectionName, level, TYPE_SUBSECTION, subScope ) )
                subScope.erase(subScope.begin());
            outconfig.push_back(*oldConfigIterator);
            oldConfigIterator++;
            iterateOldConfigSection( oldConfig, oldConfigIterator, outconfig, level+1, TYPE_SUBSECTION, subScope);
        }
        else if ( start != std::string::npos && (*oldConfigIterator).compare(start, endTag.length(), endTag) == 0 )
        {
            std::list<SectionAttributes>::iterator it = scope.begin();
            if ( it != scope.end() )
                writeAttributes( outconfig, scope, it);
            outconfig.push_back(*oldConfigIterator);
            oldConfigIterator++;
            return;
        }
        else
        {
            appendLine(*oldConfigIterator, outconfig, level, scope);
            oldConfigIterator++;
        }
    }

    log(LOG_WARN) << "Reached end of inconfig but missing end tag " << endTag;
    outconfig.push_back( endTag );
}

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
            outconfig.push_back((*oldConfigIterator));
            oldConfigIterator++;
            iterateOldConfigSection( oldConfig, oldConfigIterator, outconfig, 1, TYPE_SECTION, subScope);
        }
        else
        {
            outconfig.push_back((*oldConfigIterator));
            oldConfigIterator++;
        }
    }
    // Finally, write everything (if anything) that wasn't in the old config but should be there
    std::list<SectionAttributes>::iterator it = scope.begin();
    if ( it != scope.end() )
        writeAttributes( outconfig, scope, it);
}

void generateConfig( const std::list<std::string>& inconfig, std::list<std::string>& outconfig,
                     std::list<SectionAttributes>& listOfSubSections )
{
    std::list<std::string>::const_iterator iterator = inconfig.begin();
    iterateOldConfig( inconfig, iterator, outconfig, listOfSubSections );
}

