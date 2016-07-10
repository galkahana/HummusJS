/*
   Source File : FontDescriptorWriter.cpp


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
#include "FontDescriptorWriter.h"
#include "FreeTypeFaceWrapper.h"
#include "ObjectsContext.h"
#include "EFontStretch.h"
#include "IFontDescriptorHelper.h"
#include "DictionaryContext.h"




FontDescriptorWriter::FontDescriptorWriter(void)
{
}

FontDescriptorWriter::~FontDescriptorWriter(void)
{
}

static const std::string scType = "Type";
static const std::string scFontDescriptor = "FontDescriptor";
static const std::string scFontName = "FontName";
static const std::string scFontFamily = "FontFamily";
static const std::string scFontStretch = "FontStretch";
static const std::string scFontWeight = "FontWeight";
static const std::string scFlags = "Flags";
static const std::string scFontBBox = "FontBBox";
static const std::string scItalicAngle = "ItalicAngle";
static const std::string scAscent = "Ascent";
static const std::string scDescent = "Descent";
static const std::string scCapHeight = "CapHeight";
static const std::string scXHeight = "XHeight";
static const std::string scStemV = "StemV";
static const std::string scCharSet = "CharSet";


static const char* scFontStretchLabels[eFontStretchMax] =
{
	"",
	"UltraCondensed", 
	"ExtraCondensed", 
	"Condensed", 
	"SemiCondensed", 
	"Normal", 
	"SemiExpanded", 
	"Expanded", 
	"ExtraExpanded",
	"UltraExpanded"
};

void FontDescriptorWriter::WriteFontDescriptor(	ObjectIDType inFontDescriptorObjectID,
												const std::string& inFontPostscriptName,
												FreeTypeFaceWrapper* inFontInfo,
												const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs,
												ObjectsContext* inObjectsContext,
												IFontDescriptorHelper* inDescriptorHelper)
{
	DictionaryContext* fontDescriptorDictionary;

	inObjectsContext->StartNewIndirectObject(inFontDescriptorObjectID);
	fontDescriptorDictionary = inObjectsContext->StartDictionary();
	
	// Type
	fontDescriptorDictionary->WriteKey(scType);
	fontDescriptorDictionary->WriteNameValue(scFontDescriptor);

	// FontName
	fontDescriptorDictionary->WriteKey(scFontName);
	fontDescriptorDictionary->WriteNameValue(inFontPostscriptName);

	// FontFamily
	fontDescriptorDictionary->WriteKey(scFontFamily);
	fontDescriptorDictionary->WriteLiteralStringValue((*inFontInfo)->family_name);

	// FontStretch
	fontDescriptorDictionary->WriteKey(scFontStretch);
	fontDescriptorDictionary->WriteNameValue(scFontStretchLabels[inFontInfo->GetFontStretch()]);

	// FontWeight
	fontDescriptorDictionary->WriteKey(scFontWeight);
	fontDescriptorDictionary->WriteIntegerValue(inFontInfo->GetFontWeight());

	// FontBBox
	fontDescriptorDictionary->WriteKey(scFontBBox);
	fontDescriptorDictionary->WriteRectangleValue(
											PDFRectangle(
												inFontInfo->GetInPDFMeasurements((*inFontInfo)->bbox.xMin),
												inFontInfo->GetInPDFMeasurements((*inFontInfo)->bbox.yMin),
												inFontInfo->GetInPDFMeasurements((*inFontInfo)->bbox.xMax),
												inFontInfo->GetInPDFMeasurements((*inFontInfo)->bbox.yMax)));

	// ItalicAngle
	fontDescriptorDictionary->WriteKey(scItalicAngle);
	fontDescriptorDictionary->WriteDoubleValue(inFontInfo->GetItalicAngle());

	// Ascent
	fontDescriptorDictionary->WriteKey(scAscent);
	fontDescriptorDictionary->WriteIntegerValue(inFontInfo->GetInPDFMeasurements((*inFontInfo)->ascender));

	// Descent
	fontDescriptorDictionary->WriteKey(scDescent);
	fontDescriptorDictionary->WriteIntegerValue(inFontInfo->GetInPDFMeasurements((*inFontInfo)->descender));

	// CapHeight
	BoolAndFTShort result =  inFontInfo->GetCapHeight();
	if(result.first)
	{
		fontDescriptorDictionary->WriteKey(scCapHeight);
		fontDescriptorDictionary->WriteIntegerValue(result.second);
	}

	// XHeight
	result = inFontInfo->GetxHeight();
	if(result.first)
	{
		fontDescriptorDictionary->WriteKey(scXHeight);
		fontDescriptorDictionary->WriteIntegerValue(result.second);
	}


	// StemV
	fontDescriptorDictionary->WriteKey(scStemV);
	fontDescriptorDictionary->WriteIntegerValue(inFontInfo->GetStemV());

	// ChartSet writing (variants according to ANSI/CID)
	inDescriptorHelper->WriteCharSet(fontDescriptorDictionary,inObjectsContext,inFontInfo,inEncodedGlyphs);

	// Flags
	fontDescriptorDictionary->WriteKey(scFlags);
	fontDescriptorDictionary->WriteIntegerValue(CalculateFlags(inFontInfo,inEncodedGlyphs));

	// font embedding [may not happen due to font embedding restrictions. helper is supposed to avoid reference as well]
	inDescriptorHelper->WriteFontFileReference(fontDescriptorDictionary,inObjectsContext);

	inObjectsContext->EndDictionary(fontDescriptorDictionary);
	inObjectsContext->EndIndirectObject();
}

unsigned int FontDescriptorWriter::CalculateFlags(	FreeTypeFaceWrapper* inFontInfo,
													const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs)
{
	unsigned int flags = 0;

	// see FreeTypeFaceWrapper::GetFontFlags for explanation, if you must

	if(inFontInfo->IsFixedPitch())
		flags |= 1;
	if(inFontInfo->IsSerif())
		flags |= 2;
	if(IsSymbolic(inFontInfo,inEncodedGlyphs))
		flags |= 4;
	else
		flags |= 32;
	if(inFontInfo->IsScript())
		flags |= 8;
	if(inFontInfo->IsItalic())
		flags |= 64;
	if(inFontInfo->IsForceBold())
		flags |= (1<<18);

	return flags;
	
}

bool FontDescriptorWriter::IsSymbolic(	FreeTypeFaceWrapper* inFontInfo,
										const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs)
{
	bool hasOnlyAdobeStandard = true;

	UIntAndGlyphEncodingInfoVector::const_iterator it = inEncodedGlyphs.begin()+1; // skip 0 character
	
	for(; it != inEncodedGlyphs.end() && hasOnlyAdobeStandard; ++it)
	{
		ULongVector::const_iterator itCharacters = it->second.mUnicodeCharacters.begin();
		for(; itCharacters != it->second.mUnicodeCharacters.end() && hasOnlyAdobeStandard;++itCharacters)
			hasOnlyAdobeStandard = inFontInfo->IsCharachterCodeAdobeStandard(*itCharacters);
	}
	return !hasOnlyAdobeStandard;
}
