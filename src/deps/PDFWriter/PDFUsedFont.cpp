/*
   Source File : PDFUsedFont.cpp


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
#include "PDFUsedFont.h"
#include "IWrittenFont.h"
#include "UnicodeString.h"
#include "ObjectsContext.h"
#include "DictionaryContext.h"
#include "PDFParser.h"
#include "PDFObjectCast.h"
#include "PDFDictionary.h"
#include "PDFIndirectObjectReference.h"

using namespace PDFHummus;

PDFUsedFont::PDFUsedFont(FT_Face inInputFace,
						 const std::string& inFontFilePath,
						 const std::string& inAdditionalMetricsFontFilePath,
                         long inFontIndex,
						 ObjectsContext* inObjectsContext):mFaceWrapper(inInputFace,inFontFilePath,inAdditionalMetricsFontFilePath,inFontIndex)
{
	mObjectsContext = inObjectsContext;
	mWrittenFont = NULL;
}

PDFUsedFont::~PDFUsedFont(void)
{
	delete mWrittenFont;
}

bool PDFUsedFont::IsValid()
{
	return mFaceWrapper.IsValid();
}

EStatusCode PDFUsedFont::EncodeStringForShowing(const GlyphUnicodeMappingList& inText,
												ObjectIDType &outFontObjectToUse,
												UShortList& outCharactersToUse,
												bool& outTreatCharactersAsCID)
{
	if(!mWrittenFont)
		mWrittenFont = mFaceWrapper.CreateWrittenFontObject(mObjectsContext);

	mWrittenFont->AppendGlyphs(inText,outCharactersToUse,outTreatCharactersAsCID,outFontObjectToUse);

	return PDFHummus::eSuccess;
}

EStatusCode PDFUsedFont::TranslateStringToGlyphs(const std::string& inText,GlyphUnicodeMappingList& outGlyphsUnicodeMapping)
{
	UIntList glyphs;
	UnicodeString unicode;

	EStatusCode status = unicode.FromUTF8(inText);
	if(status != PDFHummus::eSuccess)
		return status;


	status = mFaceWrapper.GetGlyphsForUnicodeText(unicode.GetUnicodeList(),glyphs);

	ULongList::const_iterator itUnicode = unicode.GetUnicodeList().begin();
	UIntList::iterator itGlyphs = glyphs.begin();

	for(; itUnicode != unicode.GetUnicodeList().end(); ++itUnicode,++itGlyphs)
		outGlyphsUnicodeMapping.push_back(GlyphUnicodeMapping(*itGlyphs,*itUnicode));

	return status;
}

EStatusCode PDFUsedFont::EncodeStringsForShowing(const GlyphUnicodeMappingListList& inText,
												ObjectIDType &outFontObjectToUse,
												UShortListList& outCharactersToUse,
												bool& outTreatCharactersAsCID)
{
	if(!mWrittenFont)
		mWrittenFont = mFaceWrapper.CreateWrittenFontObject(mObjectsContext);

	mWrittenFont->AppendGlyphs(inText,outCharactersToUse,outTreatCharactersAsCID,outFontObjectToUse);

	return PDFHummus::eSuccess;
}

EStatusCode PDFUsedFont::WriteFontDefinition()
{
    // note that written font may be empty, in case no glyphs were used for this font! in the empty case, just dont write the def
    if(!mWrittenFont)
        return eSuccess;
    else
        return mWrittenFont->WriteFontDefinition(mFaceWrapper);
}

EStatusCode PDFUsedFont::WriteState(ObjectsContext* inStateWriter,ObjectIDType inObjectID)
{
	inStateWriter->StartNewIndirectObject(inObjectID);
	DictionaryContext* pdfUsedFontObject = inStateWriter->StartDictionary();

	pdfUsedFontObject->WriteKey("Type");
	pdfUsedFontObject->WriteNameValue("PDFUsedFont");

	ObjectIDType writtenFontObject = 0;

	if(mWrittenFont)
	{
		writtenFontObject = inStateWriter->GetInDirectObjectsRegistry().AllocateNewObjectID();

		pdfUsedFontObject->WriteKey("mWrittenFont");
		pdfUsedFontObject->WriteNewObjectReferenceValue(writtenFontObject);
	}

	inStateWriter->EndDictionary(pdfUsedFontObject);
	inStateWriter->EndIndirectObject();

	if(mWrittenFont)
		mWrittenFont->WriteState(inStateWriter,writtenFontObject);

	return PDFHummus::eSuccess;
}

EStatusCode PDFUsedFont::ReadState(PDFParser* inStateReader,ObjectIDType inObjectID)
{
	PDFObjectCastPtr<PDFDictionary> pdfUsedFontState(inStateReader->ParseNewObject(inObjectID));

	PDFObjectCastPtr<PDFIndirectObjectReference> writtenFontReference(pdfUsedFontState->QueryDirectObject("mWrittenFont"));

	if(!writtenFontReference)
		return PDFHummus::eSuccess;

	if(mWrittenFont)
		delete mWrittenFont;

	mWrittenFont = mFaceWrapper.CreateWrittenFontObject(mObjectsContext);
	if(!mWrittenFont)
		return PDFHummus::eFailure;

	return mWrittenFont->ReadState(inStateReader,writtenFontReference->mObjectID);
}

FreeTypeFaceWrapper* PDFUsedFont::GetFreeTypeFont()
{
    return &mFaceWrapper;
}
