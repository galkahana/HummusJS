/*
   Source File : PDFName.h


   Copyright 2011 Gal Kahana PDFWriter

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   
*/
#pragma once
#include "PDFObject.h"

#include <string>



class PDFName : public PDFObject
{
public:

	enum EType
	{
		eType = ePDFObjectName
	};

	// value must be the already interpreted name - no initial slash, and all special charachters (with # definition) interpreted
	PDFName(const std::string& inValue);
	virtual ~PDFName(void);

	const std::string& GetValue() const;
	operator std::string() const;

private:

	std::string mValue;
};
