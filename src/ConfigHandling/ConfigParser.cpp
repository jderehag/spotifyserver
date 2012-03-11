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

#include <assert.h>

bool findSection(const std::string& scope, const char* sectionName, std::string& outputScope)
{
	size_t startSection = 0;
	while(true)
	{
	    size_t sectionIndex = scope.find("Section", startSection);
		if(sectionIndex == std::string::npos)goto not_found;
		startSection = sectionIndex;

		// Found 'A' section, lets find out if its the correct one..
		size_t startMark = scope.find_first_of('"', startSection);
		if(startMark == std::string::npos)goto not_found;
		size_t endMark = scope.find_first_of('"', startMark+1);
		if(endMark == std::string::npos)goto not_found;
		startSection += (endMark+1) - startSection; //Forward the lookup pointer past this section name

		std::string sectionNameString = scope.substr(startMark + 1, endMark - startMark - 1);
		if(sectionNameString == sectionName)
		{
			// Found the correct section!
			size_t endSection = scope.find("EndSection", startSection);
			if(endSection == std::string::npos)goto not_found;
			outputScope = scope.substr(startSection, endSection - startSection);
			break;
		}
	}
	return true;

not_found:
	return false;
}

bool findSubSection(const std::string& scope, const char* sectionName, std::string& outputScope)
{
	size_t startSection = 0;
	while(true)
	{
	    size_t sectionIndex = scope.find("SubSection", startSection);
		if(sectionIndex == std::string::npos)goto not_found;
        startSection = sectionIndex;

		size_t startMark = scope.find_first_of('"', startSection);
		if(startMark == std::string::npos)goto not_found;
		size_t endMark = scope.find_first_of('"', startMark + 1);
		if(endMark == std::string::npos)goto not_found;
		startSection += (endMark + 1) - startSection; //Forward the lookup pointer past this section name

		std::string sectionNameString = scope.substr(startMark + 1, endMark - startMark - 1);
		if(sectionNameString == sectionName)
		{
			// Found the correct section!
			size_t endSection = scope.find("EndSubSection", startSection);
			if(endSection != std::string::npos)
			{
				// Found the endsection
				outputScope = scope.substr(startSection, endSection - startSection);
				break;
			}
		}
	}
	return true;

not_found:
	return false;
}

void findAttribute(const std::string& scope, const char* attributeName, std::string& outputScope)
{
	size_t startAttribute = scope.find(attributeName);
	if(startAttribute == std::string::npos)goto not_found;

	/* found the attribute, now store it.. */
	{
		size_t startMark = scope.find_first_of('"', startAttribute);
		if(startMark == std::string::npos)goto not_found;
		size_t endMark = scope.find_first_of('"', startMark + 1);
		if(endMark == std::string::npos)goto not_found;
		outputScope = scope.substr(startMark + 1, endMark - startMark - 1);
	}
	return;

not_found:
	return;
}

void findNextElementScope(const std::string& scope, SectionAttributes listOfSubSections[],
																	  unsigned int length,
																	  unsigned int& listOfSubsectionIndex)
{
	/* should never happen*/
	assert(listOfSubsectionIndex < length);

	while(true)
	{
		SectionAttributes* attribute = &listOfSubSections[listOfSubsectionIndex];

		std::string subScope;
		const std::string* nextScope = NULL;

		switch(attribute->type)
		{
		case TYPE_SECTION:
			findSection(scope, attribute->attributeName, subScope);
			nextScope = &subScope;
			break;
		case TYPE_SUBSECTION:
			findSubSection(scope, attribute->attributeName, subScope);
			nextScope = &subScope;
			break;
		case TYPE_ATTRIBUTE:
			findAttribute(scope, attribute->attributeName, subScope);
			nextScope = &scope;
			break;

		}

		/* if user has set the attribute, then store it */
		if(attribute->attributeValue != NULL)*attribute->attributeValue = subScope;

		/* check if we should go deeper into the tree */
		listOfSubsectionIndex++;

        if(listOfSubsectionIndex < length &&
           listOfSubSections[listOfSubsectionIndex].level >= attribute->level)
        {
            findNextElementScope(*nextScope, listOfSubSections, length, listOfSubsectionIndex);
        }
		/* could be changed in the recursive findNextElementScope(..), therefore check it again */
		if(listOfSubsectionIndex >= length)break;
		if(listOfSubSections[listOfSubsectionIndex].level < attribute->level)break;
	}
}

void parseConfigInt(const std::string& config, SectionAttributes listOfSubSections[], unsigned int length)
{
	unsigned int iteratorIndex = 0;
	findNextElementScope(config, listOfSubSections, length, iteratorIndex);
}

