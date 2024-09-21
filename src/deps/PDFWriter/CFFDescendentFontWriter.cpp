/*
   Source File : CFFDescendentFontWriter.cpp


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
#include "CFFDescendentFontWriter.h"
#include "DescendentFontWriter.h"
#include "DictionaryContext.h"
#include "ObjectsContext.h"
#include "CFFEmbeddedFontWriter.h"
#include "FreeTypeFaceWrapper.h"
#include "Trace.h"


using namespace PDFHummus;

CFFDescendentFontWriter::CFFDescendentFontWriter(void)
{
}

CFFDescendentFontWriter::~CFFDescendentFontWriter(void)
{
}

/*static bool sEncodedGlypsSort(const UIntAndGlyphEncodingInfo& inLeft, const UIntAndGlyphEncodingInfo& inRight)
{
	return inLeft.first < inRight.first;
}*/

static const std::string scCIDFontType0C = "CIDFontType0C";
static const char* scType1 = "Type 1";
EStatusCode CFFDescendentFontWriter::WriteFont(	ObjectIDType inDecendentObjectID, 
														const std::string& inFontName,
														FreeTypeFaceWrapper& inFontInfo,
														const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs,
														ObjectsContext* inObjectsContext,
														bool inEmbedFont)
{
	// reset embedded font object ID (and flag...to whether it was actually embedded or not, which may 
	// happen due to font embedding restrictions)
	mEmbeddedFontFileObjectID = 0;

	// Logically speaking, i shouldn't be getting to CID writing
	// if in type 1. at least, this is the current assumption, since
	// i don't intend to support type 1 CIDs, but just regular type 1s.
	// as such - fail if got here for type 1
	const char* fontType = inFontInfo.GetTypeString();
	if(strcmp(scType1,fontType) == 0)
	{
		TRACE_LOG1("CFFDescendentFontWriter::WriteFont, Exception. identified type1 font when writing CFF CID font, font name - %s. type 1 CIDs are not supported.",inFontName.substr(0, MAX_TRACE_SIZE - 200).c_str());
		return PDFHummus::eFailure;
	}

	unsigned int subsetFontSize = inEncodedGlyphs.size();

	if (inEmbedFont)
	{
		CFFEmbeddedFontWriter embeddedFontWriter;
		UIntAndGlyphEncodingInfoVector encodedGlyphs = inEncodedGlyphs;
		UIntVector orderedGlyphs;
		UShortVector cidMapping;

		// Gal: the following sort completely ruins everything.
		// the order of the glyphs should be maintained per the ENCODED characthers
		// which is how the input is recieved. IMPORTANT - the order is critical
		// for the success of the embedding, as the order determines the order of the glyphs
		// in the subset font and so their GID which MUST match the encoded char.
		//sort(encodedGlyphs.begin(), encodedGlyphs.end(), sEncodedGlypsSort);

		for (UIntAndGlyphEncodingInfoVector::const_iterator it = encodedGlyphs.begin();
			it != encodedGlyphs.end();
			++it)
		{
			orderedGlyphs.push_back(it->first);
			cidMapping.push_back(it->second.mEncodedCharacter);
		}
		EStatusCode status = embeddedFontWriter.WriteEmbeddedFont(inFontInfo,
			orderedGlyphs,
			scCIDFontType0C,
			inFontName,
			inObjectsContext,
			&cidMapping,
			mEmbeddedFontFileObjectID);
		if (status != PDFHummus::eSuccess)
			return status;

		subsetFontSize = embeddedFontWriter.GetSubsetFontGlyphsCount();
	}

	DescendentFontWriter descendentFontWriter;

	return descendentFontWriter.WriteFont(
		inDecendentObjectID,
		inFontName,
		inFontInfo,
		inEncodedGlyphs,
		inObjectsContext,
		this,
		subsetFontSize // the font program includes the glyphs 0...glyps.size using cid=sid. so cidset should be the same. 0..size.
	);
}

static const std::string scCIDFontType0 = "CIDFontType0";

void CFFDescendentFontWriter::WriteSubTypeValue(DictionaryContext* inDescendentFontContext)
{
	inDescendentFontContext->WriteNameValue(scCIDFontType0);
}

void CFFDescendentFontWriter::WriteAdditionalKeys(DictionaryContext* inDescendentFontContext)
{
	// do nothing
}

static const std::string scFontFile3 = "FontFile3";
void CFFDescendentFontWriter::WriteFontFileReference(DictionaryContext* inDescriptorContext,
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
