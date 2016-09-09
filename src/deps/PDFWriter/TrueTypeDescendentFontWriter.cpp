/*
   Source File : TrueTypeDescendentFontWriter.cpp


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
#include "TrueTypeDescendentFontWriter.h"
#include "DescendentFontWriter.h"
#include "DictionaryContext.h"
#include "TrueTypeEmbeddedFontWriter.h"
#include "ObjectsContext.h"
#include "IndirectObjectsReferenceRegistry.h"

using namespace PDFHummus;

TrueTypeDescendentFontWriter::TrueTypeDescendentFontWriter(void)
{
}

TrueTypeDescendentFontWriter::~TrueTypeDescendentFontWriter(void)
{
}

static UIntVector GetOrderedKeys(const UIntAndGlyphEncodingInfoVector& inMap)
{
	UIntVector result;
	for(UIntAndGlyphEncodingInfoVector::const_iterator it = inMap.begin(); it != inMap.end(); ++it)
		result.push_back(it->first);
	sort(result.begin(),result.end());
	return result;
}


EStatusCode TrueTypeDescendentFontWriter::WriteFont(	ObjectIDType inDecendentObjectID, 
														const std::string& inFontName,
														FreeTypeFaceWrapper& inFontInfo,
														const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs,
														ObjectsContext* inObjectsContext,
														bool inEmbedFont)
{
	// reset embedded font object ID (and flag...to whether it was actually embedded or not, which may 
	// happen due to font embedding restrictions)
	mEmbeddedFontFileObjectID = 0;

	if (inEmbedFont)
	{
		TrueTypeEmbeddedFontWriter embeddedFontWriter;
		EStatusCode status = embeddedFontWriter.WriteEmbeddedFont(inFontInfo, GetOrderedKeys(inEncodedGlyphs), inObjectsContext, mEmbeddedFontFileObjectID);

		if (PDFHummus::eFailure == status)
			return status;
	}

	DescendentFontWriter descendentFontWriter;
	return descendentFontWriter.WriteFont(inDecendentObjectID,inFontName,inFontInfo,inEncodedGlyphs,inObjectsContext,this);
}

static const std::string scCIDFontType2 = "CIDFontType2";

void TrueTypeDescendentFontWriter::WriteSubTypeValue(DictionaryContext* inDescendentFontContext)
{
	inDescendentFontContext->WriteNameValue(scCIDFontType2);
}

static const std::string scCIDToGIDMap = "CIDToGIDMap";
static const std::string scIdentity = "Identity";

void TrueTypeDescendentFontWriter::WriteAdditionalKeys(DictionaryContext* inDescendentFontContext)
{
	// CIDToGIDMap 
	inDescendentFontContext->WriteKey(scCIDToGIDMap);
	inDescendentFontContext->WriteNameValue(scIdentity);
}

static const std::string scFontFile2 = "FontFile2";
void TrueTypeDescendentFontWriter::WriteFontFileReference(	
										DictionaryContext* inDescriptorContext,
										ObjectsContext* inObjectsContext)
{
	// write font reference only if there's what to write....
	if(mEmbeddedFontFileObjectID != 0)
	{
		// FontFile2
		inDescriptorContext->WriteKey(scFontFile2);
		inDescriptorContext->WriteNewObjectReferenceValue(mEmbeddedFontFileObjectID);
	}
}