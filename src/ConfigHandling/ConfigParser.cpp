/*
 * Copyright (c) 2012, Jesper Derehag
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
 * DISCLAIMED. IN NO EVENT SHALL JESPER DEREHAG BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *      Improvements:
 *      Is not 100% secure yet.
 *      If an attribute with the same name exists within a SubSection as one that exists below a
 *      subsection, they might get confused. To fix this I need to filter out subsections and sections
 *      from the scope when looking for specific attributes.
 *
 *      Should also optimize and remove all std::strings, they do alot of unnecessary reallocs in the background.
 *      char* should work just as well for this case (might clog up the implementation with alot of c-string handling
 *      but hey, optimization beats beauty every time right?
 *
 *      Also, config verification should also be built in here.. somehow..
 */

#include "ConfigParser.h"
#include <string>
#include <iostream>

#include <string.h>
#include <assert.h>

static void iterateTo( const std::list<std::string>& scope, const char* tag, std::list<std::string>::const_iterator& it )
{
    for(;it != scope.end(); it++)
    {
        size_t startTag = (*it).find_first_not_of(" \t");
        if ( startTag != std::string::npos &&
             (*it)[startTag] != '#' &&
             (*it).compare(startTag, strlen(tag), tag ) == 0 )
        {
            break;
        }
    }
}

static std::list<std::string>::const_iterator findSectionStart(const std::list<std::string>& scope, const char* sectionName)
{
    std::list<std::string>::const_iterator startSection = scope.begin();

    while(true)
    {
        std::list<std::string>::const_iterator it = startSection;

        iterateTo( scope, "Section", it );

        if ( it == scope.end() ) return scope.end();

        size_t startMark = (*it).find_first_of('"');
        if(startMark == std::string::npos) return scope.end();
        size_t endMark = (*it).find_first_of('"', startMark + 1);
        if(endMark == std::string::npos) return scope.end();

        std::string sectionNameString = (*it).substr(startMark + 1, endMark - startMark - 1);

        if(sectionNameString == sectionName)
        {
            return it;
        }

        it++;
        startSection = it;
    }

    return scope.end();
}

static bool findSection(const std::list<std::string>& scope, const char* sectionName, std::list<std::string>& outputScope)
{
    outputScope.clear();
    std::list<std::string>::const_iterator startSection = findSectionStart(scope, sectionName);

    if ( startSection != scope.end() )
    {
        startSection++;
        std::list<std::string>::const_iterator it = startSection;
    
        // Found the correct section!
        iterateTo( scope, "EndSection", it );

        if ( it != scope.end() )
        {
            // Found the endsection
            std::list<std::string>::const_iterator endSection = it;

            if ( startSection != endSection )
                outputScope = std::list<std::string>(startSection, endSection);

            return true;
        }
    }

    return false;
}

static std::list<std::string>::const_iterator findSubSectionStart(const std::list<std::string>& scope, const char* sectionName)
{
    std::list<std::string>::const_iterator startSection = scope.begin();

    while(true)
    {
        std::list<std::string>::const_iterator it = startSection;

        iterateTo( scope, "SubSection", it );

        if ( it == scope.end() ) return scope.end();

        size_t startMark = (*it).find_first_of('"');
        if(startMark == std::string::npos) return scope.end();
        size_t endMark = (*it).find_first_of('"', startMark + 1);
        if(endMark == std::string::npos) return scope.end();

        std::string sectionNameString = (*it).substr(startMark + 1, endMark - startMark - 1);

        if(sectionNameString == sectionName)
        {
            return it;
        }

        it++;
        startSection = it;
    }

    return scope.end();
}


static bool findSubSection(const std::list<std::string>& scope, const char* sectionName, std::list<std::string>& outputScope)
{
    outputScope.clear();
    std::list<std::string>::const_iterator startSection = findSubSectionStart(scope, sectionName);

    if ( startSection != scope.end() )
    {
        startSection++;
        std::list<std::string>::const_iterator it = startSection;
    
        // Found the correct section!
        iterateTo( scope, "EndSubSection", it );

        if ( it != scope.end() )
        {
            // Found the endsection
            std::list<std::string>::const_iterator endSection = it;

            if ( startSection != endSection )
                outputScope = std::list<std::string>(startSection, endSection);

            return true;
        }
    }

    return false;
}

static void findAttribute(const std::list<std::string>& scope, const char* attributeName, std::string& outputScope)
{
    outputScope = "";

    std::list<std::string>::const_iterator it = scope.begin();
    iterateTo( scope, attributeName, it );

    if ( it != scope.end() )
    {
        /* found the attribute, now store it.. */
        size_t startMark = (*it).find_first_of('"');
        if(startMark == std::string::npos)
            return;
        size_t endMark = (*it).find_first_of('"', startMark + 1);
        if(endMark == std::string::npos)
            return;
        outputScope = (*it).substr(startMark + 1, endMark - startMark - 1);
    }

    return;
}

static void findNextElementScope(const std::list<std::string>& scope, std::list<SectionAttributes>& listOfSubSections,
                                                    std::list<SectionAttributes>::iterator& listOfSubsectionIterator)
{
    while(true)
    {
        SectionAttributes& attribute = *listOfSubsectionIterator;

        std::list<std::string> subScope;
        std::string attributeValue;

        switch(attribute.type)
        {
        case TYPE_SECTION:
            findSection(scope, attribute.attributeName, subScope);
            break;
        case TYPE_SUBSECTION:
            findSubSection(scope, attribute.attributeName, subScope);
            break;
        case TYPE_ATTRIBUTE:
            findAttribute(scope, attribute.attributeName, attributeValue);
            /* if user has set the attribute, then store it */
            if(attribute.attributeValue != NULL) 
                *attribute.attributeValue = attributeValue;
            break;
        }

        /* check if we should go deeper into the tree */
        listOfSubsectionIterator++;

        if ( listOfSubsectionIterator != listOfSubSections.end() &&
             listOfSubsectionIterator->level > attribute.level)
        {
            assert( attribute.type != TYPE_ATTRIBUTE );
            findNextElementScope(subScope, listOfSubSections, listOfSubsectionIterator);
        }

        /* could be changed in the recursive findNextElementScope(..), therefore check it again */
        if ( listOfSubsectionIterator == listOfSubSections.end() ||
             listOfSubsectionIterator->level < attribute.level )
            break;
    }
}

void parseConfig( const std::list<std::string>& config, std::list<SectionAttributes>& listOfSubSections )
{
    std::list<SectionAttributes>::iterator listOfSubsectionIterator = listOfSubSections.begin();
    findNextElementScope(config, listOfSubSections, listOfSubsectionIterator);
}

