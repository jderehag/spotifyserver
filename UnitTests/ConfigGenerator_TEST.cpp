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

#include "ConfigGenerator_TEST.h"
#include "unittest.h"

#include "ConfigGenerator.h"
#include <string>
#include <iostream>
#include <assert.h>

namespace Test
{

static bool ut_testValidConfig()
{
	std::string attributeValue1 = "newValue1";
	std::string attributeValue2 = "newValue2";
	std::string attributeValue3 = "newValue3";
	std::string attributeValue4 = "newValue4";
	std::string attributeValue5 = "";
    std::string attributeValue6 = "newValue6";
    std::string attributeValue7 = "newValue7";
    std::string attributeValue8 = "newValue8";
    std::list<std::string> outConfig;

    std::list<std::string> validConfig;
    std::list<std::string> correctOutConfig;
    validConfig.push_back("################################");
    validConfig.push_back( "# this is a valid config        ");
    validConfig.push_back( "################################");
    validConfig.push_back( "Section \"section1\"            ");
    correctOutConfig = validConfig;
    validConfig.push_back( "Attribute1  \"value1\"          ");
    correctOutConfig.push_back("Attribute1  \"newValue1\"          ");
    validConfig.push_back( "################################");
    correctOutConfig.push_back(validConfig.back());
    validConfig.push_back( "# this is a random comment      ");
    correctOutConfig.push_back(validConfig.back());
    validConfig.push_back( "################################");
    correctOutConfig.push_back(validConfig.back());
    validConfig.push_back( "WrongAttribute2 \"wrongvalue\"  ");
    correctOutConfig.push_back(validConfig.back());
    validConfig.push_back( "Attribute2      \"value2\"      ");
    correctOutConfig.push_back("Attribute2      \"newValue2\"      ");
    validConfig.push_back( "SubSection   \"subsection1\"    ");
    correctOutConfig.push_back(validConfig.back());
    validConfig.push_back( "\t Attribute1 \"value3\"      ");
    correctOutConfig.push_back("\t Attribute1 \"newValue3\"      ");
    validConfig.push_back( " EndSubSection                  ");
    correctOutConfig.push_back(validConfig.back());
    validConfig.push_back( "EndSection                      ");
    correctOutConfig.push_back(validConfig.back());
    validConfig.push_back( "Section \"section2\"            ");
    correctOutConfig.push_back(validConfig.back());
    validConfig.push_back( "#Attribute4 \"wrongvalue\"      ");
    correctOutConfig.push_back(validConfig.back());
    validConfig.push_back( "Attribute4  \"value4\"          ");
    correctOutConfig.push_back("Attribute4  \"newValue4\"          ");
    validConfig.push_back( "Attribute5      \"value5\"      ");
    correctOutConfig.push_back("Attribute5      \"\"      ");
    validConfig.push_back( "#Attribute6  \"notconfigured\"  ");
    correctOutConfig.push_back(validConfig.back());
    correctOutConfig.push_back( "Attribute6\t\t\"newValue6\"");
    validConfig.push_back( "EndSection                      ");
    correctOutConfig.push_back(validConfig.back());

    correctOutConfig.push_back( "" );
    correctOutConfig.push_back( "Section  \"new section\"" );
    correctOutConfig.push_back( "Attribute1\t\t\"newValue7\"" );
    correctOutConfig.push_back( "SubSection   \"new subsection\"" );
    correctOutConfig.push_back( "Attribute1\t\t\"newValue8\"" );
    correctOutConfig.push_back( "EndSubSection" );
    correctOutConfig.push_back( "EndSection" );
    correctOutConfig.push_back( "" );


	std::list<SectionAttributes> parseDefintion;
	/* section 1 */
	parseDefintion.push_back( SectionAttributes(0, TYPE_SECTION, 	"section1" ));
	parseDefintion.push_back( SectionAttributes(1, TYPE_ATTRIBUTE, 	"Attribute1", 		&attributeValue1 ));
	parseDefintion.push_back( SectionAttributes(1, TYPE_ATTRIBUTE, 	"Attribute2", 		&attributeValue2 ));
	parseDefintion.push_back( SectionAttributes(1, TYPE_SUBSECTION, "subsection1", 		NULL ));
	parseDefintion.push_back( SectionAttributes(2, TYPE_ATTRIBUTE, 	"Attribute1", 		&attributeValue3 ));
	/* section 2 */
	parseDefintion.push_back( SectionAttributes(0, TYPE_SECTION, 	"section2", 		NULL ));
	parseDefintion.push_back( SectionAttributes(1, TYPE_ATTRIBUTE, 	"Attribute4", 		&attributeValue4 ));
	parseDefintion.push_back( SectionAttributes(1, TYPE_ATTRIBUTE, 	"Attribute5", 		&attributeValue5 ));
    parseDefintion.push_back( SectionAttributes(1, TYPE_ATTRIBUTE,  "Attribute6",       &attributeValue6 ));
    /* new section */
    parseDefintion.push_back( SectionAttributes(0, TYPE_SECTION,    "new section" ));
    parseDefintion.push_back( SectionAttributes(1, TYPE_ATTRIBUTE,  "Attribute1",       &attributeValue7 ));
    parseDefintion.push_back( SectionAttributes(1, TYPE_SUBSECTION, "new subsection",   NULL ));
    parseDefintion.push_back( SectionAttributes(2, TYPE_ATTRIBUTE,  "Attribute1",       &attributeValue8 ));

	generateConfig(validConfig, outConfig, parseDefintion);

	std::list<std::string>::iterator it1 = outConfig.begin();
    std::list<std::string>::iterator it2 = correctOutConfig.begin();
	while ( it1 != outConfig.end() && it2 != correctOutConfig.end() )
	{
	    UT_ASSERT_STRING_EQUAL( *it1, *it2 );
	    it1++;
	    it2++;
	}

    assert( it1 == outConfig.end() );
	assert( it2 == correctOutConfig.end() );

	return true;
}


static bool ut_testInvalidConfigNoEndSection()
{

	std::string attributeValue1 = "value1";
	std::string attributeValue2 = "value2";
	std::string attributeValue3 = "newValue3";

    std::list<std::string> config;
    std::list<std::string> correctOutConfig;
    config.push_back( "###############################" );
    config.push_back( "# this is an invalid config    " );
    config.push_back( "# (No EndSubsection)         " );
    config.push_back( "###############################" );
    config.push_back( "Section \"section1\"         " );
    config.push_back( "Attribute1   \"value1\"      " );
    config.push_back( "Attribute2\t     \"value2\"      " );
    config.push_back( "SubSection   \"subsection1\"    ");
    correctOutConfig = config;
    config.push_back( " Attribute3 \"value3\"            ");
    correctOutConfig.push_back( " Attribute3 \"newValue3\"            ");
    config.push_back( "#EndSubSection");
    correctOutConfig.push_back(config.back());
    correctOutConfig.push_back( "EndSubSection" );
    correctOutConfig.push_back( "EndSection" );

    std::list<SectionAttributes> parseDefintion;
	parseDefintion.push_back( SectionAttributes( 0, TYPE_SECTION, 		"section1", 		NULL ));
	parseDefintion.push_back( SectionAttributes( 1, TYPE_ATTRIBUTE, 	"Attribute1", 		&attributeValue1 ));
	parseDefintion.push_back( SectionAttributes( 1, TYPE_ATTRIBUTE, 	"Attribute2", 		&attributeValue2 ));
    parseDefintion.push_back( SectionAttributes( 1, TYPE_SUBSECTION,    "subsection1",         NULL ));
	parseDefintion.push_back( SectionAttributes( 2, TYPE_ATTRIBUTE, 	"Attribute3", 		&attributeValue3 ));

    std::list<std::string> outConfig;
    generateConfig(config, outConfig, parseDefintion);

    std::list<std::string>::iterator it1 = outConfig.begin();
    std::list<std::string>::iterator it2 = correctOutConfig.begin();
    while ( it1 != outConfig.end() && it2 != correctOutConfig.end() )
    {
        UT_ASSERT_STRING_EQUAL( *it1, *it2 );
        it1++;
        it2++;
    }

    assert( it1 == outConfig.end() );
    assert( it2 == correctOutConfig.end() );
	return true;

}


bool ConfigGenerator_SUITE::run_unittests()
{
	bool success = true;
	success &= ut_testValidConfig();
	success &= ut_testInvalidConfigNoEndSection();
	return success;
}

}

