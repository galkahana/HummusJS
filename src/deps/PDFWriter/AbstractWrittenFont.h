/*
   Source File : AbstractWrittenFont.h


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

#include "IWrittenFont.h"
#include "WrittenFontRepresentation.h"
#include "GlyphUnicodeMapping.h"

class ObjectsContext;
class DictionaryContext;
class PDFDictionary;
class PDFParser;

class AbstractWrittenFont : public IWrittenFont
{
public:
	AbstractWrittenFont(ObjectsContext* inObjectsContext);
	virtual ~AbstractWrittenFont(void);

	virtual void AppendGlyphs(const GlyphUnicodeMappingList& inGlyphsList,
							  UShortList& outEncodedCharacters,
							  bool& outEncodingIsMultiByte,
							  ObjectIDType &outFontObjectID);
	virtual void AppendGlyphs(const GlyphUnicodeMappingListList& inGlyphsList,
							  UShortListList& outEncodedCharacters,
							  bool& outEncodingIsMultiByte,
							  ObjectIDType &outFontObjectID);
protected:
	WrittenFontRepresentation* mCIDRepresentation;
	WrittenFontRepresentation* mANSIRepresentation;
	ObjectsContext* mObjectsContext;

	PDFHummus::EStatusCode WriteStateInDictionary(ObjectsContext* inStateWriter,DictionaryContext* inDerivedObjectDictionary);
	PDFHummus::EStatusCode WriteStateAfterDictionary(ObjectsContext* inStateWriter);
	PDFHummus::EStatusCode ReadStateFromObject(PDFParser* inStateReader,PDFDictionary* inState);

private:
	ObjectIDType mCidRepresentationObjectStateID;
	ObjectIDType mAnsiRepresentationObjectStateID;


	bool CanEncodeWithIncludedChars(WrittenFontRepresentation* inRepresentation, 
									const GlyphUnicodeMappingList& inGlyphsList,
									UShortList& outEncodedCharacters);
	bool CanEncodeWithIncludedChars(WrittenFontRepresentation* inRepresentation, 
									const GlyphUnicodeMappingListList& inGlyphsList,
									UShortListList& outEncodedCharacters);

	void AddToCIDRepresentation(const GlyphUnicodeMappingList& inGlyphsList,UShortList& outEncodedCharacters);
	void AddToCIDRepresentation(const GlyphUnicodeMappingListList& inGlyphsList,UShortListList& outEncodedCharacters);
	
	// Aha! This method remains virtual for sub implementations to 
	// override. Adding to an ANSI representation is dependent on the output format,
	// where True Type has some different ruling from OpenType(CFF)/Type1
	virtual bool AddToANSIRepresentation(
									const GlyphUnicodeMappingList& inGlyphsList,
									UShortList& outEncodedCharacters) = 0;
	virtual bool AddToANSIRepresentation(
									const GlyphUnicodeMappingListList& inGlyphsList,
									UShortListList& outEncodedCharacters) = 0;

	PDFHummus::EStatusCode WriteWrittenFontState(WrittenFontRepresentation* inRepresentation,ObjectsContext* inStateWriter,ObjectIDType inObjectID);
	void WriteGlyphEncodingInfoState(ObjectsContext* inStateWriter,
									 ObjectIDType inObjectId,
									 const GlyphEncodingInfo& inGlyphEncodingInfo);
	void ReadWrittenFontState(PDFParser* inStateReader,PDFDictionary* inState,WrittenFontRepresentation* inRepresentation);
	void ReadGlyphEncodingInfoState(PDFParser* inStateReader,ObjectIDType inObjectID,GlyphEncodingInfo& inGlyphEncodingInfo);

};
