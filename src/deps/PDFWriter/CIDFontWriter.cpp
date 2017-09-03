/*
   Source File : CIDFontWriter.cpp


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
#include "CIDFontWriter.h"
#include "DictionaryContext.h"
#include "ObjectsContext.h"
#include "FreeTypeFaceWrapper.h"
#include "Trace.h"
#include "PDFStream.h"
#include "IByteWriter.h"
#include "SafeBufferMacrosDefs.h"
#include "CFFDescendentFontWriter.h"
#include "IDescendentFontWriter.h"
#include "UnicodeString.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>


using namespace PDFHummus;

CIDFontWriter::CIDFontWriter(void)
{
}

CIDFontWriter::~CIDFontWriter(void)
{
}

static const std::string scType = "Type";
static const std::string scFont = "Font";
static const std::string scSubtype = "Subtype";
static const std::string scType0 = "Type0";
static const std::string scBaseFont = "BaseFont";
static const std::string scPlus = "+";
static const std::string scDescendantFonts = "DescendantFonts";
static const std::string scToUnicode = "ToUnicode";

EStatusCode CIDFontWriter::WriteFont(FreeTypeFaceWrapper& inFontInfo,
										WrittenFontRepresentation* inFontOccurrence,
										ObjectsContext* inObjectsContext,
										IDescendentFontWriter* inDescendentFontWriter,
										bool inEmbedFont)
{

	EStatusCode status = PDFHummus::eSuccess;
	inObjectsContext->StartNewIndirectObject(inFontOccurrence->mWrittenObjectID);

	mFontInfo = &inFontInfo;
	mFontOccurrence = inFontOccurrence;
	mObjectsContext = inObjectsContext;

	do
	{
		DictionaryContext* fontContext = inObjectsContext->StartDictionary();

		// Type
		fontContext->WriteKey(scType);
		fontContext->WriteNameValue(scFont);

		// SubType
		fontContext->WriteKey(scSubtype);
		fontContext->WriteNameValue(scType0);

		// BaseFont
		fontContext->WriteKey(scBaseFont);
		std::string postscriptFontName = inFontInfo.GetPostscriptName();
		if(postscriptFontName.length() == 0)
		{
			TRACE_LOG("CIDFontWriter::WriteFont, unexpected failure. no postscript font name for font");
			status = PDFHummus::eFailure;
			break;
		}
		std::string fontName = inEmbedFont ? (inObjectsContext->GenerateSubsetFontPrefix() + scPlus + postscriptFontName) : postscriptFontName;
		fontContext->WriteNameValue(fontName);

		WriteEncoding(fontContext);

		// DescendantFonts 
		ObjectIDType descendantFontID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();

		fontContext->WriteKey(scDescendantFonts);
		mObjectsContext->StartArray();
		mObjectsContext->WriteNewIndirectObjectReference(descendantFontID);
		mObjectsContext->EndArray(eTokenSeparatorEndLine);

		CalculateCharacterEncodingArray(); // put the charachter in the order of encoding, for the ToUnicode map

		// ToUnicode
		if (mCharactersVector.size() > 1) {
			// make sure there's more than just the 0 char (which would make this an array of size of one
			fontContext->WriteKey(scToUnicode);
			ObjectIDType toUnicodeMapObjectID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
			fontContext->WriteNewObjectReferenceValue(toUnicodeMapObjectID);

			status = inObjectsContext->EndDictionary(fontContext);
			if (status != PDFHummus::eSuccess)
			{
				TRACE_LOG("CIDFontWriter::WriteFont, unexpected failure. Failed to end dictionary in font write.");
				break;
			}
			inObjectsContext->EndIndirectObject();
			WriteToUnicodeMap(toUnicodeMapObjectID);
		}
		else {
			// else just finish font writing (a bit of an edge case here...but should take care of, for cleanliness)
			status = inObjectsContext->EndDictionary(fontContext);
			if (status != PDFHummus::eSuccess)
			{
				TRACE_LOG("CIDFontWriter::WriteFont, unexpected failure. Failed to end dictionary in font write.");
				break;
			}
			inObjectsContext->EndIndirectObject();
		}

		// Write the descendant font
		status = inDescendentFontWriter->WriteFont(descendantFontID, fontName, *mFontInfo, mCharactersVector, mObjectsContext, inEmbedFont);

	} while(false);

	return status;
}

static const std::string scEncoding = "Encoding";
static const std::string scIdentityH = "Identity-H";
void CIDFontWriter::WriteEncoding(DictionaryContext* inFontContext)
{
	// Encoding
	inFontContext->WriteKey(scEncoding);
	inFontContext->WriteNameValue(scIdentityH);
}

static bool sUShortSort(const UIntAndGlyphEncodingInfo& inLeft, const UIntAndGlyphEncodingInfo& inRight)
{
	return inLeft.second.mEncodedCharacter < inRight.second.mEncodedCharacter;
}

void CIDFontWriter::CalculateCharacterEncodingArray()
{
	// first we need to sort the fonts charachters by character code
	UIntToGlyphEncodingInfoMap::iterator it = mFontOccurrence->mGlyphIDToEncodedChar.begin();

	for(; it != mFontOccurrence->mGlyphIDToEncodedChar.end();++it)
		mCharactersVector.push_back(UIntAndGlyphEncodingInfo(it->first,it->second));

	std::sort(mCharactersVector.begin(),mCharactersVector.end(),sUShortSort);
}


static const char* scCmapHeader =
"/CIDInit /ProcSet findresource begin\n\
12 dict begin\n\
begincmap\n\
/CIDSystemInfo\n\
<< /Registry (Adobe)\n\
/Ordering (UCS) /Supplement 0 >> def\n\
/CMapName /Adobe-Identity-UCS def\n\
/CMapType 2 def\n\
1 begincodespacerange\n";
static const char* scFourByteRangeStart = "0000";
static const char* scFourByteRangeEnd = "FFFF";
static const char* scEndCodeSpaceRange = "endcodespacerange\n";
static const std::string scBeginBFChar = "beginbfchar";
static const std::string scEndBFChar = "endbfchar";
static const char* scCmapFooter = "endcmap CMapName currentdict /CMap defineresource pop end end\n";

void CIDFontWriter::WriteToUnicodeMap(ObjectIDType inToUnicodeMap)
{
	mObjectsContext->StartNewIndirectObject(inToUnicodeMap);
	PDFStream* pdfStream = mObjectsContext->StartPDFStream();
	IByteWriter* cmapWriteContext = pdfStream->GetWriteStream();
	PrimitiveObjectsWriter primitiveWriter(cmapWriteContext);
	unsigned long i = 1;
	UIntAndGlyphEncodingInfoVector::iterator it = mCharactersVector.begin() + 1; // skip 0 glyph
	unsigned long vectorSize = (unsigned long)mCharactersVector.size() - 1; // cause 0 is not there

	cmapWriteContext->Write((const Byte*)scCmapHeader,strlen(scCmapHeader));
	primitiveWriter.WriteEncodedHexString(scFourByteRangeStart);
	primitiveWriter.WriteEncodedHexString(scFourByteRangeEnd,eTokenSeparatorEndLine);
	cmapWriteContext->Write((const Byte*)scEndCodeSpaceRange,strlen(scEndCodeSpaceRange));

	if(vectorSize < 100)
		primitiveWriter.WriteInteger(vectorSize);
	else
		primitiveWriter.WriteInteger(100);
	primitiveWriter.WriteKeyword(scBeginBFChar);
	
	WriteGlyphEntry(cmapWriteContext,it->second.mEncodedCharacter,it->second.mUnicodeCharacters);
	++it;
	for(; it != mCharactersVector.end(); ++it,++i)
	{
		if(i % 100 == 0)
		{
			primitiveWriter.WriteKeyword(scEndBFChar);
			if(vectorSize - i < 100)
				primitiveWriter.WriteInteger(vectorSize - i);
			else
				primitiveWriter.WriteInteger(100);
			primitiveWriter.WriteKeyword(scBeginBFChar);
		}
		WriteGlyphEntry(cmapWriteContext,it->second.mEncodedCharacter,it->second.mUnicodeCharacters);
	}
	primitiveWriter.WriteKeyword(scEndBFChar);
	cmapWriteContext->Write((const Byte*)scCmapFooter,strlen(scCmapFooter));
	mObjectsContext->EndPDFStream(pdfStream);
	delete pdfStream;
}

static const Byte scEntryEnding[2] = {'>','\n'};
static const Byte scAllZeros[4] = {'0','0','0','0'};
void CIDFontWriter::WriteGlyphEntry(IByteWriter* inWriter,unsigned short inEncodedCharacter,const ULongVector& inUnicodeValues)
{
	UnicodeString unicode;
	char formattingBuffer[17];
	ULongVector::const_iterator it = inUnicodeValues.begin();

	SAFE_SPRINTF_1(formattingBuffer,17,"<%04x> <",inEncodedCharacter);
	inWriter->Write((const Byte*)formattingBuffer,8);
	
	if(inUnicodeValues.size() == 0)
	{
		inWriter->Write(scAllZeros,4);
	}
	else
	{
		for(; it != inUnicodeValues.end(); ++it)
		{
			unicode.GetUnicodeList().push_back(*it);
			EStatusCodeAndUShortList utf16Result = unicode.ToUTF16UShort();
			unicode.GetUnicodeList().clear();

			if (utf16Result.first == eFailure || utf16Result.second.size() == 0) {
				TRACE_LOG1("CIDFontWriter::WriteGlyphEntry, got invalid glyph value. saving as 0. value = ", *it);
				utf16Result.second.clear();
				utf16Result.second.push_back(0);
			}

			if(utf16Result.second.size() == 2)
			{
				SAFE_SPRINTF_2(formattingBuffer,17,"%04x%04x",
																utf16Result.second.front(),
																utf16Result.second.back());
				inWriter->Write((const Byte*)formattingBuffer,8);
			}
			else // 1
			{
				SAFE_SPRINTF_1(formattingBuffer,17,"%04x",utf16Result.second.front());
				inWriter->Write((const Byte*)formattingBuffer,4);
			}
		}
	}
	inWriter->Write(scEntryEnding,2);
}
