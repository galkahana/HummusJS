/*
   Source File : ANSIFontWriter.cpp


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
#include "ANSIFontWriter.h"
#include "ObjectsContext.h"
#include "FreeTypeFaceWrapper.h"
#include "DictionaryContext.h"
#include "Trace.h"
#include "WinAnsiEncoding.h"
#include "IByteWriter.h"
#include "PrimitiveObjectsWriter.h"
#include "PDFStream.h"
#include "SafeBufferMacrosDefs.h"
#include "FontDescriptorWriter.h"
#include "IANSIFontWriterHelper.h"
#include "UnicodeString.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>



using namespace PDFHummus;

ANSIFontWriter::ANSIFontWriter(void)
{
}

ANSIFontWriter::~ANSIFontWriter(void)
{
}

static const std::string scType = "Type";
static const std::string scFont = "Font";
static const std::string scSubtype = "Subtype";
static const std::string scBaseFont = "BaseFont";
static const std::string scToUnicode = "ToUnicode";
static const std::string scFontDescriptor = "FontDescriptor";

EStatusCode ANSIFontWriter::WriteFont(	FreeTypeFaceWrapper& inFontInfo,
											WrittenFontRepresentation* inFontOccurrence,
											ObjectsContext* inObjectsContext,
											IANSIFontWriterHelper* inANSIFontWriterHelper,
											const std::string& inSubsetFontName)
{
	EStatusCode status = PDFHummus::eSuccess;
	FontDescriptorWriter fontDescriptorWriter;

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
		inANSIFontWriterHelper->WriteSubTypeValue(fontContext); // using the font particular type

		// BaseFont
		fontContext->WriteKey(scBaseFont);
		fontContext->WriteNameValue(inSubsetFontName);


		/*
			as for widths.
			i have to create a list/array of the characters ordered from lowest encoded value to highest, and fill it up with the charachters mapped
			to glyphs. this will allow me to later write the highest and lowest char codes, as well as loop the list and 
			write the widths.
		*/
		CalculateCharacterEncodingArray();

		WriteWidths(fontContext);

		if(inANSIFontWriterHelper->CanWriteDifferencesFromWinAnsi())
			CalculateDifferences();
		WriteEncoding(fontContext);

		// ToUnicode
		fontContext->WriteKey(scToUnicode);
		ObjectIDType toUnicodeMapObjectID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
		fontContext->WriteNewObjectReferenceValue(toUnicodeMapObjectID);


		// FontDescriptor
		fontContext->WriteKey(scFontDescriptor);
		ObjectIDType fontDescriptorObjectID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
		fontContext->WriteNewObjectReferenceValue(fontDescriptorObjectID);

		status = inObjectsContext->EndDictionary(fontContext);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("ANSIFontWriter::WriteFont, unexpected failure. Failed to end dictionary in font write.");
			break;
		}

		inObjectsContext->EndIndirectObject();	

		// if necessary, write a dictionary encoding
		if(mDifferences.size() > 0)
			WriteEncodingDictionary();

		WriteToUnicodeMap(toUnicodeMapObjectID);
		fontDescriptorWriter.WriteFontDescriptor(fontDescriptorObjectID,
												inSubsetFontName,
												&inFontInfo,
												mCharactersVector,
												inObjectsContext,
												inANSIFontWriterHelper->GetCharsetWriter());
	}while(false);
	return status;
}

static bool sUShortSort(const UIntAndGlyphEncodingInfo& inLeft, const UIntAndGlyphEncodingInfo& inRight)
{
	return inLeft.second.mEncodedCharacter < inRight.second.mEncodedCharacter;
}

void ANSIFontWriter::CalculateCharacterEncodingArray()
{
	// first we need to sort the fonts charachters by character code
	UIntToGlyphEncodingInfoMap::iterator it = mFontOccurrence->mGlyphIDToEncodedChar.begin();

	for(; it != mFontOccurrence->mGlyphIDToEncodedChar.end();++it)
		mCharactersVector.push_back(UIntAndGlyphEncodingInfo(it->first,it->second));

	std::sort(mCharactersVector.begin(),mCharactersVector.end(),sUShortSort);
}

static const std::string scFirstChar = "FirstChar";
static const std::string scLastChar = "LastChar";
static const std::string scWidths = "Widths";

void ANSIFontWriter::WriteWidths(DictionaryContext* inFontContext)
{

	// FirstChar
	inFontContext->WriteKey(scFirstChar);
	inFontContext->WriteIntegerValue((mCharactersVector.begin())->second.mEncodedCharacter);
	
    // LastChar
	inFontContext->WriteKey(scLastChar);
	inFontContext->WriteIntegerValue(mCharactersVector.back().second.mEncodedCharacter);

	// Widths
	inFontContext->WriteKey(scWidths);

	mObjectsContext->StartArray();
	
	UIntAndGlyphEncodingInfoVector::iterator itCharacters = mCharactersVector.begin();
	for(unsigned short i = itCharacters->second.mEncodedCharacter; i <= mCharactersVector.back().second.mEncodedCharacter; ++i)
	{
		if(itCharacters->second.mEncodedCharacter == i)
		{
			mObjectsContext->WriteInteger(mFontInfo->GetGlyphWidth(itCharacters->first));
			++itCharacters;
		}
		else
		{
			mObjectsContext->WriteInteger(0);
		}
	}
	mObjectsContext->EndArray();
	mObjectsContext->EndLine();

}

void ANSIFontWriter::CalculateDifferences()
{
	// go over the encoded charachters. find differences from WinAnsiEncoding.
	// whenever glyph name is different, add to differences array
	WinAnsiEncoding winAnsiEncoding;

	UIntAndGlyphEncodingInfoVector::iterator it = mCharactersVector.begin();

	for(; it != mCharactersVector.end(); ++it)
	{
		// hmm. there should always be a glyph name for a CFF or Type1 glyph. so it shouldn't be a problem
		// to ask them here. should be sufficient length as well.
        std::string glyphName = mFontInfo->GetGlyphName(it->first);
		if(strcmp(glyphName.c_str(),winAnsiEncoding.GetEncodedGlyphName((IOBasicTypes::Byte)it->second.mEncodedCharacter)) != 0)
			mDifferences.push_back(UShortAndString(it->second.mEncodedCharacter,glyphName));
	}
}

static const std::string scEncoding = "Encoding";
static const std::string scWinAnsiEncoding = "WinAnsiEncoding";
void ANSIFontWriter::WriteEncoding(DictionaryContext* inFontContext)
{
	// Encoding

	inFontContext->WriteKey(scEncoding);
	if(mDifferences.size() == 0)
	{
		inFontContext->WriteNameValue(scWinAnsiEncoding);
	}
	else
	{
		mEncodingDictionaryID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
		inFontContext->WriteNewObjectReferenceValue(mEncodingDictionaryID);
	}
}

static const std::string scBaseEncoding = "BaseEncoding";
static const std::string scDifferences = "Differences";
void ANSIFontWriter::WriteEncodingDictionary()
{
	DictionaryContext* encodingDictionary;

	mObjectsContext->StartNewIndirectObject(mEncodingDictionaryID);
	encodingDictionary = mObjectsContext->StartDictionary();

	// Type
	encodingDictionary->WriteKey(scType);
	encodingDictionary->WriteNameValue(scEncoding);

	// BaseEncoding
	encodingDictionary->WriteKey(scBaseEncoding);
	encodingDictionary->WriteNameValue(scWinAnsiEncoding);

	// Differences
	encodingDictionary->WriteKey(scDifferences);
	mObjectsContext->StartArray();

	UShortAndStringList::iterator it = mDifferences.begin();
	unsigned short previousEncoding;
	
	encodingDictionary->WriteIntegerValue(it->first);
	encodingDictionary->WriteNameValue(it->second);
	previousEncoding = it->first;
	++it;
	for(; it != mDifferences.end();++it)
	{
		if(previousEncoding + 1 != it->first)
		{
			mObjectsContext->EndLine();
			encodingDictionary->WriteIndents();
			encodingDictionary->WriteIntegerValue(it->first);
			encodingDictionary->WriteNameValue(it->second);

			// put two spaces to account for the starting bracket and following spaces. y'know, just for the sake of proper formatting
			mObjectsContext->WriteTokenSeparator(eTokenSeparatorSpace);
			mObjectsContext->WriteTokenSeparator(eTokenSeparatorSpace);
		}
    else
    {
      encodingDictionary->WriteNameValue(it->second);
    }
		previousEncoding = it->first;
	}

	mObjectsContext->EndArray();
	mObjectsContext->EndLine();

	mObjectsContext->EndDictionary(encodingDictionary);
	mObjectsContext->EndIndirectObject();
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
static const char* scTwoByteRangeStart = "00";
static const char* scTwoByteRangeEnd = "FF";
static const char* scEndCodeSpaceRange = "endcodespacerange\n";
static const std::string scBeginBFChar = "beginbfchar";
static const std::string scEndBFChar = "endbfchar";
static const char* scCmapFooter = "endcmap CMapName currentdict /CMap defineresource pop end end\n";


void ANSIFontWriter::WriteToUnicodeMap(ObjectIDType inToUnicodeMap)
{
	mObjectsContext->StartNewIndirectObject(inToUnicodeMap);
	PDFStream* pdfStream = mObjectsContext->StartPDFStream();
	IByteWriter* cmapWriteContext = pdfStream->GetWriteStream();
	PrimitiveObjectsWriter primitiveWriter(cmapWriteContext);
	unsigned long i = 1;
	UIntAndGlyphEncodingInfoVector::iterator it = mCharactersVector.begin() + 1; // skip 0 glyph
	unsigned long vectorSize = (unsigned long)mCharactersVector.size() - 1; // cause 0 is not there

	cmapWriteContext->Write((const Byte*)scCmapHeader,strlen(scCmapHeader));
	primitiveWriter.WriteEncodedHexString(scTwoByteRangeStart);
	primitiveWriter.WriteEncodedHexString(scTwoByteRangeEnd,eTokenSeparatorEndLine);
	cmapWriteContext->Write((const Byte*)scEndCodeSpaceRange,strlen(scEndCodeSpaceRange));

	if(vectorSize < 100)
		primitiveWriter.WriteInteger(vectorSize);
	else
		primitiveWriter.WriteInteger(100);
	primitiveWriter.WriteKeyword(scBeginBFChar);
	// vectorSize can be zero in a font with custom encoding with a meaningful zero glyph
	if (vectorSize>0)
    {
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
	}
	primitiveWriter.WriteKeyword(scEndBFChar);
	cmapWriteContext->Write((const Byte*)scCmapFooter,strlen(scCmapFooter));
	mObjectsContext->EndPDFStream(pdfStream);
	delete pdfStream;
}

static const Byte scEntryEnding[2] = {'>','\n'};
static const Byte scAllZeros[4] = {'0','0','0','0'};
void ANSIFontWriter::WriteGlyphEntry(IByteWriter* inWriter,unsigned short inEncodedCharacter,const ULongVector& inUnicodeValues)
{
	UnicodeString unicode;
	char formattingBuffer[17];
	ULongVector::const_iterator it = inUnicodeValues.begin();

	SAFE_SPRINTF_1(formattingBuffer,17,"<%02x> <",inEncodedCharacter);
	inWriter->Write((const Byte*)formattingBuffer,6);
	
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
				TRACE_LOG1("ANSIFontWriter::WriteGlyphEntry, got invalid glyph value. saving as 0. value = ", *it);
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