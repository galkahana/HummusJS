/*
   Source File : CFFANSIFontWriter.cpp


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
#include "CFFANSIFontWriter.h"
#include "ANSIFontWriter.h"
#include "DictionaryContext.h"
#include "OutputStringBufferStream.h"
#include "PrimitiveObjectsWriter.h"
#include "PDFStream.h"
#include "SafeBufferMacrosDefs.h"
#include "ObjectsContext.h"
#include "FreeTypeFaceWrapper.h"
#include "CFFEmbeddedFontWriter.h"
#include "Type1ToCFFEmbeddedFontWriter.h"
#include "Trace.h"

#include <ft2build.h>
#include FT_FREETYPE_H


using namespace PDFHummus;

CFFANSIFontWriter::CFFANSIFontWriter(void)
{
}

CFFANSIFontWriter::~CFFANSIFontWriter(void)
{
}

static const std::string scType1C = "Type1C";
static const char* scType1Type = "Type 1";
static const char* scCFF = "CFF";
static const std::string scPlus = "+";
EStatusCode CFFANSIFontWriter::WriteFont(	FreeTypeFaceWrapper& inFontInfo,
											WrittenFontRepresentation* inFontOccurrence,
											ObjectsContext* inObjectsContext,
											bool inEmbedFont)
{
	std::string postscriptFontName = inFontInfo.GetPostscriptName();
	if(postscriptFontName.length() == 0)
	{
		TRACE_LOG("CFFANSIFontWriter::WriteFont, unexpected failure. no postscript font name for font");
		return PDFHummus::eFailure;
	}
	std::string fontName;
	
	// reset embedded font object ID (and flag...to whether it was actually embedded or not, which may 
	// happen due to font embedding restrictions)
	mEmbeddedFontFileObjectID = 0;

	if (inEmbedFont)
	{
		fontName = inObjectsContext->GenerateSubsetFontPrefix() + scPlus + postscriptFontName;
		const char* fontType = inFontInfo.GetTypeString();

		EStatusCode status;
		if (strcmp(scType1Type, fontType) == 0)
		{
			Type1ToCFFEmbeddedFontWriter embeddedFontWriter;

			status = embeddedFontWriter.WriteEmbeddedFont(inFontInfo,
				inFontOccurrence->GetGlyphIDsAsOrderedVector(),
				scType1C,
				fontName,
				inObjectsContext,
				mEmbeddedFontFileObjectID);
		}
		else if (strcmp(scCFF, fontType) == 0)
		{
			CFFEmbeddedFontWriter embeddedFontWriter;

			status = embeddedFontWriter.WriteEmbeddedFont(inFontInfo,
				inFontOccurrence->GetGlyphIDsAsOrderedVector(),
				scType1C,
				fontName,
				inObjectsContext,
				mEmbeddedFontFileObjectID);
		}
		else
		{

			TRACE_LOG("CFFANSIFontWriter::WriteFont, Exception, unfamilar font type for embedding representation");
			status = PDFHummus::eFailure;
		}
		if (status != PDFHummus::eSuccess)
			return status;
	}
	else
		fontName = postscriptFontName;

	ANSIFontWriter fontWriter;

	return fontWriter.WriteFont(inFontInfo, inFontOccurrence, inObjectsContext, this, fontName);
}

static const char* scType1 = "Type1";
void CFFANSIFontWriter::WriteSubTypeValue(DictionaryContext* inDictionary)
{
	inDictionary->WriteNameValue(scType1);
}

IFontDescriptorHelper* CFFANSIFontWriter::GetCharsetWriter()
{
	return this;
}

bool CFFANSIFontWriter::CanWriteDifferencesFromWinAnsi()
{
	return true;
}



static const std::string scCharSet = "CharSet";
//static const Byte scLeftParanthesis[] = {'('};
//static const Byte scRightParanthesis[] = {')'};

void CFFANSIFontWriter::WriteCharSet(	DictionaryContext* inDescriptorContext,
										ObjectsContext* inObjectsContext,
										FreeTypeFaceWrapper* inFontInfo,
										const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs)
{
	// constructing glyph names in a string as names. only when done - submit to output
	OutputStringBufferStream aStream;
	PrimitiveObjectsWriter primitiveWriter(&aStream);

	UIntAndGlyphEncodingInfoVector::const_iterator it = inEncodedGlyphs.begin() + 1; // skip 0 character

	for (; it != inEncodedGlyphs.end(); ++it)
	{
		std::string glyphName = inFontInfo->GetGlyphName(it->first);
		primitiveWriter.WriteName(glyphName.c_str(), eTokenSeparatorNone);
	}


	// ChartSet
	inDescriptorContext->WriteKey(scCharSet);
	inDescriptorContext->WriteLiteralStringValue(aStream.ToString());
}

static const std::string scFontFile3 = "FontFile3";
void CFFANSIFontWriter::WriteFontFileReference(	
										DictionaryContext* inDescriptorContext,
										ObjectsContext* inObjectsContext)
{
	// write font reference only if there's what to write....
	if(mEmbeddedFontFileObjectID != 0)
	{
		// FontFile3
		inDescriptorContext->WriteKey(scFontFile3);
		inDescriptorContext->WriteNewObjectReferenceValue(mEmbeddedFontFileObjectID);
	}
}
