/*
   Source File : TrueTypeANSIFontWriter.cpp


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
#include "TrueTypeANSIFontWriter.h"
#include "ANSIFontWriter.h"
#include "DictionaryContext.h"
#include "ObjectsContext.h"
#include "IndirectObjectsReferenceRegistry.h"
#include "TrueTypeEmbeddedFontWriter.h"
#include "FreeTypeFaceWrapper.h"
#include "Trace.h"

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace PDFHummus;

TrueTypeANSIFontWriter::TrueTypeANSIFontWriter(void)
{
}

TrueTypeANSIFontWriter::~TrueTypeANSIFontWriter(void)
{
}

static const std::string scPlus = "+";
EStatusCode TrueTypeANSIFontWriter::WriteFont(	FreeTypeFaceWrapper& inFontInfo,
											WrittenFontRepresentation* inFontOccurrence,
											ObjectsContext* inObjectsContext,
											bool inEmbedFont)
{
	std::string postscriptFontName = inFontInfo.GetPostscriptName();
	if(postscriptFontName.length() == 0)
	{
		TRACE_LOG("TrueTypeANSIFontWriter::WriteFont, unexpected failure. no postscript font name for font");
		return PDFHummus::eFailure;
	}
	std::string fontName;

	// reset embedded font object ID (and flag...to whether it was actually embedded or not, which may 
	// happen due to font embedding restrictions, or due to users choice)
	mEmbeddedFontFileObjectID = 0;

	TrueTypeEmbeddedFontWriter embeddedFontWriter;

	if (inEmbedFont)
	{
		fontName = inObjectsContext->GenerateSubsetFontPrefix() + scPlus + postscriptFontName;
		EStatusCode status = embeddedFontWriter.WriteEmbeddedFont(inFontInfo,
																	inFontOccurrence->GetGlyphIDsAsOrderedVector(),
																	inObjectsContext,
																	mEmbeddedFontFileObjectID);
		if (PDFHummus::eFailure == status)
			return status;
	}
	else
		fontName  = postscriptFontName;

	ANSIFontWriter fontWriter;

	return fontWriter.WriteFont(inFontInfo, inFontOccurrence, inObjectsContext, this, fontName);
}

static const std::string scTrueType = "TrueType";

void TrueTypeANSIFontWriter::WriteSubTypeValue(DictionaryContext* inDictionary)
{
	inDictionary->WriteNameValue(scTrueType);
}

IFontDescriptorHelper* TrueTypeANSIFontWriter::GetCharsetWriter()
{
	// note that there's no charset writing for true types
	return this;
}

bool TrueTypeANSIFontWriter::CanWriteDifferencesFromWinAnsi()
{
	return false;
}


void TrueTypeANSIFontWriter::WriteCharSet(	DictionaryContext* inDescriptorContext,
										ObjectsContext* inObjectsContext,
										FreeTypeFaceWrapper* inFontInfo,
										const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs)
{
	// do nothing. no charset writing for true types
}

static const std::string scFontFile2 = "FontFile2";

void TrueTypeANSIFontWriter::WriteFontFileReference(	
										DictionaryContext* inDescriptorContext,
										ObjectsContext* inObjectsContext)
{
	// will be 0 in case embedding didn't occur due to font embedding restrictions
	if(mEmbeddedFontFileObjectID != 0)
	{
		// FontFile2
		inDescriptorContext->WriteKey(scFontFile2);
		inDescriptorContext->WriteNewObjectReferenceValue(mEmbeddedFontFileObjectID);
	}
}
