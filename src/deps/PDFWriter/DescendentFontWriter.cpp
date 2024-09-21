/*
   Source File : DescendentFontWriter.cpp


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
#include "DescendentFontWriter.h"
#include "FreeTypeFaceWrapper.h"
#include "ObjectsContext.h"
#include "Trace.h"
#include "FontDescriptorWriter.h"
#include "PDFStream.h"
#include "IByteWriter.h"
#include "DictionaryContext.h"

#include FT_CID_H 


using namespace PDFHummus;

DescendentFontWriter::DescendentFontWriter(void)
{
}

DescendentFontWriter::~DescendentFontWriter(void)
{
}

static const std::string scType = "Type";
static const std::string scFont = "Font";
static const std::string scSubtype = "Subtype";
static const std::string scBaseFont = "BaseFont";
static const std::string scFontDescriptor = "FontDescriptor";
static const std::string scCIDSystemInfo = "CIDSystemInfo";

EStatusCode DescendentFontWriter::WriteFont(	ObjectIDType inDecendentObjectID, 
												const std::string& inFontName,
												FreeTypeFaceWrapper& inFontInfo,
												const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs,
												ObjectsContext* inObjectsContext,
												IDescendentFontWriter* inDescendentFontWriterHelper,
												unsigned int inMaxCIDGlyph)
{
	EStatusCode status = PDFHummus::eSuccess;
	FontDescriptorWriter fontDescriptorWriter;
	inObjectsContext->StartNewIndirectObject(inDecendentObjectID);
	
	mFontInfo = &inFontInfo;
	mObjectsContext = inObjectsContext;
	mCIDSetObjectID = 0;

	do
	{
		DictionaryContext* fontContext = inObjectsContext->StartDictionary();

		// Type
		fontContext->WriteKey(scType);
		fontContext->WriteNameValue(scFont);

		// SubType
		fontContext->WriteKey(scSubtype);
		inDescendentFontWriterHelper->WriteSubTypeValue(fontContext);

		// BaseFont
		fontContext->WriteKey(scBaseFont);
		fontContext->WriteNameValue(inFontName);

		WriteWidths(inEncodedGlyphs,fontContext);

		// CIDSystemInfo
		fontContext->WriteKey(scCIDSystemInfo);
		ObjectIDType cidSystemInfoObjectID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
		fontContext->WriteNewObjectReferenceValue(cidSystemInfoObjectID);

		// FontDescriptor
		fontContext->WriteKey(scFontDescriptor);
		ObjectIDType fontDescriptorObjectID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
		fontContext->WriteNewObjectReferenceValue(fontDescriptorObjectID);

		// free dictionary end writing
		inDescendentFontWriterHelper->WriteAdditionalKeys(fontContext);

		status = inObjectsContext->EndDictionary(fontContext);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFANSIFontWriter::WriteFont, unexpected failure. Failed to end dictionary in font write.");
			break;
		}

		inObjectsContext->EndIndirectObject();	

		WriteCIDSystemInfo(cidSystemInfoObjectID); 
		mWriterHelper = inDescendentFontWriterHelper; // save the helper pointer, to write the font program reference in the descriptor
		fontDescriptorWriter.WriteFontDescriptor(fontDescriptorObjectID,inFontName,&inFontInfo,inEncodedGlyphs,inObjectsContext,this);

		if(mCIDSetObjectID) // set by descriptor writer callback
			WriteCIDSet(inMaxCIDGlyph);
	}while(false);
	return status;	
}

static const std::string scDW = "DW";
static const std::string scW = "W";

void DescendentFontWriter::WriteWidths(const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs,
										  DictionaryContext* inFontContext)
{
	UIntAndGlyphEncodingInfoVector::const_iterator it = inEncodedGlyphs.begin(); // will be the 0 glyph
	FT_Pos defaultWidth;
	FTPosList widthsList;
	bool allWidthsSame = true;
	FT_Pos currentWidth;
	unsigned short firstCIDInList = 0;
	unsigned short previousCIDInList = 0;

	// DW
	inFontContext->WriteKey(scDW);
	defaultWidth = mFontInfo->GetGlyphWidth(it->first);
	inFontContext->WriteIntegerValue(defaultWidth);

	++it;

	// find the first which is not default width
	for(; it != inEncodedGlyphs.end();++it)
	{
		currentWidth =  mFontInfo->GetGlyphWidth(it->first);
		if(currentWidth != defaultWidth)
		{
			widthsList.push_back(currentWidth);
			firstCIDInList = it->second.mEncodedCharacter;
			previousCIDInList = it->second.mEncodedCharacter;
			break;
		}
	}

	if(widthsList.size() != 0)
	{
		++it; // promote the iterator, from the already recorded glyph

		// W
		inFontContext->WriteKey(scW);
		mObjectsContext->StartArray();

		for(; it != inEncodedGlyphs.end();++it)
		{
			currentWidth = mFontInfo->GetGlyphWidth(it->first);
            if(currentWidth != defaultWidth)
			{
				if(it->second.mEncodedCharacter == previousCIDInList + 1)
				{
					++previousCIDInList;
					allWidthsSame &= (widthsList.front() == currentWidth);
					widthsList.push_back(currentWidth);
				}
				else
				{
					WriteWidthsItem(allWidthsSame,widthsList,firstCIDInList,previousCIDInList);
					widthsList.clear();
					widthsList.push_back(currentWidth);
					allWidthsSame = true;
					firstCIDInList = it->second.mEncodedCharacter;
					previousCIDInList = it->second.mEncodedCharacter;
				}

			}
			
		}
		if(widthsList.size() > 0)
			WriteWidthsItem(allWidthsSame,widthsList,firstCIDInList,previousCIDInList);
		mObjectsContext->EndArray(eTokenSeparatorEndLine);
	}
}

void DescendentFontWriter::WriteWidthsItem(bool inAllWidthsSame,const FTPosList& inWidths,unsigned short inFirstCID, unsigned short inLastCID)
{
	mObjectsContext->WriteInteger(inFirstCID);
	if(inAllWidthsSame)
	{
		mObjectsContext->WriteInteger(inLastCID);
		mObjectsContext->WriteInteger(inWidths.front());
	}
	else
	{
		FTPosList::const_iterator it = inWidths.begin();

		mObjectsContext->StartArray();
		for(; it != inWidths.end(); ++it)
			mObjectsContext->WriteInteger(*it);
		mObjectsContext->EndArray(eTokenSeparatorSpace);
	}
}

static const char* scIdentity = "Identity";
static const char* scAdobe = "Adobe";
static const std::string scOrdering = "Ordering";
static const std::string scRegistry = "Registry";
static const std::string scSupplement = "Supplement";

void DescendentFontWriter::WriteCIDSystemInfo(ObjectIDType inCIDSystemInfoObjectID)
{
	FT_Bool isCID;
	const char* registry;
	const char* ordering;
	FT_Int supplement;
	
	if(FT_Get_CID_Is_Internally_CID_Keyed(*mFontInfo,&isCID) != 0)
		isCID = false;	
	if(isCID && FT_Get_CID_Registry_Ordering_Supplement(*mFontInfo,&registry,&ordering,&supplement) != 0)
		isCID = false;

	if(!isCID)
	{
		registry = scAdobe;
		ordering = scIdentity;
		supplement = 0;
	}

	mObjectsContext->StartNewIndirectObject(inCIDSystemInfoObjectID);
	DictionaryContext* systemInfoContext = mObjectsContext->StartDictionary();

	// Registry
	systemInfoContext->WriteKey(scRegistry);
	systemInfoContext->WriteLiteralStringValue(registry);

	// Ordering
	systemInfoContext->WriteKey(scOrdering);
	systemInfoContext->WriteLiteralStringValue(ordering);

	// Supplement
	systemInfoContext->WriteKey(scSupplement);
	systemInfoContext->WriteIntegerValue(supplement);

	mObjectsContext->EndDictionary(systemInfoContext);
	mObjectsContext->EndIndirectObject();
}

static const std::string scCIDSet = "CIDSet";

void DescendentFontWriter::WriteCharSet(	DictionaryContext* inDescriptorContext,
											ObjectsContext* inObjectsContext,
											FreeTypeFaceWrapper* inFontInfo,
											const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs)
{
	// use this hook to Write the CIDSet key, and later write the CIDSet stream. 
	// ignoring input values for now, assuming they are the same as the ones 
	// used in the font. 

	// CIDSet
	inDescriptorContext->WriteKey(scCIDSet);
	mCIDSetObjectID = inObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
	inDescriptorContext->WriteNewObjectReferenceValue(mCIDSetObjectID);
}

void DescendentFontWriter::WriteCIDSet(unsigned int cidSetMaxGlyph)
{
	mObjectsContext->StartNewIndirectObject(mCIDSetObjectID);
	PDFStream* pdfStream = mObjectsContext->StartPDFStream();	
	IByteWriter* cidSetWritingContext = pdfStream->GetWriteStream();
	Byte buffer;

	// this should be the exact same ids as in the font itself. so should match what's in the font programs:
	// for true type the CIDs are 0...max glyph including, where the font program fills the betweens with blanks
	// for open type the CIDs are 0..size-1 glyphs, where the font program just maps glyphs to 0..count
	
	buffer = 255;
	for(unsigned int i=0;i < (cidSetMaxGlyph/8);++i) {
		cidSetWritingContext->Write(&buffer,1);
	}
	
	if(cidSetMaxGlyph % 8 != 0) {
		buffer = ~(255 >> (cidSetMaxGlyph % 8));
		cidSetWritingContext->Write(&buffer,1);
	}

	mObjectsContext->EndPDFStream(pdfStream);
	delete pdfStream;
}

void DescendentFontWriter::WriteFontFileReference(
										DictionaryContext* inDescriptorContext,
										ObjectsContext* inObjectsContext)
{
	// may not write font reference if font is not embedded, which the helper should know about.
	mWriterHelper->WriteFontFileReference(inDescriptorContext,inObjectsContext);
}