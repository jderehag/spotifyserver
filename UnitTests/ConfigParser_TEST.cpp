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

#include "ConfigParser_TEST.h"
#include "unittest.h"

#include "ConfigParser.h"
#include <string>
#include <iostream>
#include <assert.h>

namespace Test
{

static std::string validConfig(
"###############################\n"
"# this is a valid config       \n"
"###############################\n"
"Section \"section1\"		    \n"
"Attribute1 	\"value1\"		\n"
"###############################\n"
"# this is a random comment		\n"
"###############################\n"
"Attribute2		\"value2\"		\n"
"SubSection   \"subsection1\"   \n"
"	Attribute1 \"value3\"		\n"
"EndSubSection 					\n"
"EndSection 					\n"
"Section \"section2\"		    \n"
"Attribute4 	\"value4\"		\n"
"Attribute5		\"value5\"		\n"
"EndSection 					\n");

static bool ut_testValidConfig()
{
	std::string attributeValue1;
	std::string attributeValue2;
	std::string attributeValue3;
	std::string attributeValue4;
	std::string attributeValue5;

	SectionAttributes parseDefintion[] =
	{
			 /* section 1 */
			 {0, TYPE_SECTION, 		"section1", 		NULL					},
			 {1, TYPE_ATTRIBUTE, 	"Attribute1", 		&attributeValue1   		},
			 {1, TYPE_ATTRIBUTE, 	"Attribute2", 		&attributeValue2   		},
			 {1, TYPE_SUBSECTION, 	"subsection1", 		NULL	   				},
			 {2, TYPE_ATTRIBUTE, 	"Attribute1", 		&attributeValue3		},
			 /* section 2 */
			 {0, TYPE_SECTION, 		"section2", 		NULL					},
			 {1, TYPE_ATTRIBUTE, 	"Attribute4", 		&attributeValue4   		},
			 {1, TYPE_ATTRIBUTE, 	"Attribute5", 		&attributeValue5   		}
	};
	parseConfig(validConfig, parseDefintion);

	assert(attributeValue1 == "value1");
	assert(attributeValue2 == "value2");
	assert(attributeValue3 == "value3");
	assert(attributeValue4 == "value4");
	assert(attributeValue5 == "value5");

	return true;
}



static const char* invalidConfigNoEndSection(
"###############################\n"
"# this is an invalid config    \n"
"# (No EndSubsection)			\n"
"###############################\n"
"Section \"section1\"		    \n"
"Attribute1 	\"value1\"		\n"
"Attribute2		\"value2\"		\n"
"Attribute3 \"value3\"			\n");

static bool ut_testInvalidConfigNoEndSection()
{

	std::string attributeValue1;
	std::string attributeValue2;
	std::string attributeValue3;

	SectionAttributes parseDefintion[] =
	{

			 {0, TYPE_SECTION, 		"section1", 		NULL					},
			 {1, TYPE_ATTRIBUTE, 	"Attribute1", 		&attributeValue1   		},
			 {1, TYPE_ATTRIBUTE, 	"Attribute2", 		&attributeValue2   		},
			 {1, TYPE_ATTRIBUTE, 	"Attribute3", 		&attributeValue3		}
	};
	parseConfig(invalidConfigNoEndSection, parseDefintion);

	assert(attributeValue1 == "");
	assert(attributeValue2 == "");
	assert(attributeValue3 == "");
	return true;

}


bool ConfigParser_SUITE::run_unittests()
{
	ut_testValidConfig();
	ut_testInvalidConfigNoEndSection();
	return true;
}

}

