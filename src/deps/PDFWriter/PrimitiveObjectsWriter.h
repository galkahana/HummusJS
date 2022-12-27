/*
   Source File : PrimitiveObjectsWriter.h


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

#include "ETokenSeparator.h"
#include <string.h>
#include <string>



class IByteWriter;

class PrimitiveObjectsWriter
{
public:
	PrimitiveObjectsWriter(IByteWriter* inStreamForWriting = NULL);
	~PrimitiveObjectsWriter(void);

	void SetStreamForWriting(IByteWriter* inStreamForWriting);

	// Token Writing
	void WriteTokenSeparator(ETokenSeparator inSeparate);
	void EndLine();

	void WriteKeyword(const std::string& inKeyword);
	void WriteName(const std::string& inName,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteInteger(long long inIntegerToken,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteUnsafeLiteralString(const std::string& inString,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteLiteralString(const std::string& inString,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteHexString(const std::string& inString,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteEncodedHexString(const std::string& inString, ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteDouble(double inDoubleToken,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteBoolean(bool inBoolean,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteNull(ETokenSeparator inSeparate = eTokenSeparatorSpace);

	void StartArray();
	void EndArray(ETokenSeparator inSeparate = eTokenSeparatorNone);
    
    IByteWriter* GetWritingStream();

private:
	IByteWriter* mStreamForWriting;

	size_t DetermineDoubleTrimmedLength(const std::string& inString);
};
