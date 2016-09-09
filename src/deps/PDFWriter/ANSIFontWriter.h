/*
   Source File : ANSIFontWriter.h


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

#include "EStatusCode.h"
#include "ObjectsBasicTypes.h"
#include "WrittenFontRepresentation.h"

#include <utility>
#include <vector>
#include <string>
#include <list>

class FreeTypeFaceWrapper;
class ObjectsContext;
class DictionaryContext;
class IANSIFontWriterHelper;
class IByteWriter;



typedef std::pair<unsigned short,std::string>  UShortAndString;
typedef std::list<UShortAndString> UShortAndStringList;

typedef std::pair<unsigned int, GlyphEncodingInfo> UIntAndGlyphEncodingInfo;
typedef std::vector<UIntAndGlyphEncodingInfo> UIntAndGlyphEncodingInfoVector;

class ANSIFontWriter 
{
public:
	ANSIFontWriter(void);
	~ANSIFontWriter(void);


	PDFHummus::EStatusCode WriteFont(	FreeTypeFaceWrapper& inFontInfo,
							WrittenFontRepresentation* inFontOccurrence,
							ObjectsContext* inObjectsContext,
							IANSIFontWriterHelper* inANSIFontWriterHelper,
							const std::string& inWrittenFontName);

private:

	void CalculateCharacterEncodingArray();
	void WriteWidths(DictionaryContext* inFontContext);
	void CalculateDifferences();
	void WriteEncoding(DictionaryContext* inFontContext);
	void WriteEncodingDictionary();
	void WriteToUnicodeMap(ObjectIDType inToUnicodeMap);
	void WriteGlyphEntry(IByteWriter* inWriter,unsigned short inEncodedCharacter,const ULongVector& inUnicodeValues);

	FreeTypeFaceWrapper* mFontInfo;
	WrittenFontRepresentation* mFontOccurrence;
	ObjectsContext* mObjectsContext;

	UIntAndGlyphEncodingInfoVector mCharactersVector;
	UShortAndStringList mDifferences;
	ObjectIDType mEncodingDictionaryID;
};
