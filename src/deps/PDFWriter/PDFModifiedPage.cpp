/*
   Source File : PDFModifiedPage.cpp


   Copyright 2013 Gal Kahana PDFWriter

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
#include "PDFModifiedPage.h"
#include "AbstractContentContext.h"
#include "PDFFormXObject.h"
#include "XObjectContentContext.h"
#include "PDFWriter.h"
#include "PDFParser.h"
#include "PDFPageInput.h"
#include "PDFDictionary.h"
#include "PDFDocumentCopyingContext.h"
#include "PDFObjectCast.h"
#include "RefCountPtr.h"
#include "DictionaryContext.h"
#include "PDFIndirectObjectReference.h"
#include "PDFArray.h"
#include "BoxingBase.h"
#include "PDFStream.h"
#include "PDFObject.h"
#include "Trace.h"

#include <string>

using namespace std;

PDFModifiedPage::PDFModifiedPage(PDFWriter* inWriter,unsigned long inPageIndex,bool inEnsureContentEncapsulation)
{
	mWriter = inWriter;
	mPageIndex = inPageIndex;
	mCurrentContext = NULL;
	mEnsureContentEncapsulation = inEnsureContentEncapsulation;
	mIsDirty = false;
}

PDFModifiedPage::~PDFModifiedPage(void)
{
	for (PDFFormXObjectVector::iterator it = mContenxts.begin(); it != mContenxts.end(); ++it) {
		delete *it;
	}
}

AbstractContentContext* PDFModifiedPage::StartContentContext()
{
	if(!mCurrentContext)
	{
		PDFObject* page = mWriter->GetModifiedFileParser().ParsePage(mPageIndex);
		if (!page) {
			TRACE_LOG("AbstractContentContext::PDFModifiedPage, null page object");
			return NULL;
		}
		PDFRectangle mediaBox = PDFPageInput(&mWriter->GetModifiedFileParser(), page).GetMediaBox();
		mCurrentContext = mWriter->StartFormXObject(mediaBox);
	}
	return mCurrentContext->GetContentContext();
}

PDFHummus::EStatusCode PDFModifiedPage::PauseContentContext()
{
	// does the same
	return EndContentContext();
}

PDFHummus::EStatusCode PDFModifiedPage::EndContentContext()
{
	if (mCurrentContext) {
		mIsDirty = true;
		EStatusCode status = mWriter->EndFormXObject(mCurrentContext);
		mContenxts.push_back(mCurrentContext);
		mCurrentContext = NULL;
		return status;
	}
	else
		return eSuccess;
}

AbstractContentContext* PDFModifiedPage::GetContentContext()
{
	return mCurrentContext ? mCurrentContext->GetContentContext():NULL;
}

PDFHummus::EStatusCode PDFModifiedPage::AttachURLLinktoCurrentPage(const std::string& inURL, const PDFRectangle& inLinkClickArea)
{
	mIsDirty = true;
	return mWriter->GetDocumentContext().AttachURLLinktoCurrentPage(inURL, inLinkClickArea);
}

vector<string> PDFModifiedPage::WriteNewResourcesDictionary(ObjectsContext& inObjectContext) {
	vector<string> formResourcesNames;

	// no existing resource dictionary, so write a new one
	DictionaryContext*  dict = inObjectContext.StartDictionary();
	dict->WriteKey("XObject");
	DictionaryContext* xobjectDict = inObjectContext.StartDictionary();
	for (unsigned long i = 0; i<mContenxts.size(); ++i)
	{
		string formObjectName = string("myForm_") + Int(i).ToString();
		dict->WriteKey(formObjectName);
		dict->WriteObjectReferenceValue(mContenxts[i]->GetObjectID());
		formResourcesNames.push_back(formObjectName);
	}
	inObjectContext.EndDictionary(xobjectDict);
	inObjectContext.EndDictionary(dict);
	return formResourcesNames;	
}

PDFObject* PDFModifiedPage::findInheritedResources(PDFParser* inParser,PDFDictionary* inDictionary) {
	if(inDictionary->Exists("Resources")) {
		return inParser->QueryDictionaryObject(inDictionary, "Resources");
	}
	else {
		PDFObjectCastPtr<PDFDictionary> parentDict(
			inDictionary->Exists("Parent") ? 
				inParser->QueryDictionaryObject(inDictionary, "Parent"): 
				NULL);
		if(!parentDict) {
			return NULL;
		}
		else {
			return findInheritedResources(inParser,parentDict.GetPtr());
		}
		
	}
}

PDFHummus::EStatusCode PDFModifiedPage::WritePage()
{
	EStatusCode status = EndContentContext(); // just in case someone forgot to close the latest content context

	do {
		if (status != eSuccess || !mIsDirty) {
			break;
		}

		// allocate an object ID for the new contents stream (for placing the form)
		// we first create the modified page object, so that we can define a name for the new form xobject
		// that is unique
		ObjectsContext& objectContext = mWriter->GetObjectsContext();
		ObjectIDType newContentObjectID = objectContext.GetInDirectObjectsRegistry().AllocateNewObjectID();
		ObjectIDType newEncapsulatingObjectID = 0;


		// create a copying context, so we can copy the page dictionary, and modify its contents + resources dict
		PDFDocumentCopyingContext* copyingContext = mWriter->CreatePDFCopyingContextForModifiedFile();

		// get the page object
		ObjectIDType pageObjectID = copyingContext->GetSourceDocumentParser()->GetPageObjectID(mPageIndex);
		PDFObjectCastPtr<PDFDictionary> pageDictionaryObject = copyingContext->GetSourceDocumentParser()->ParsePage(mPageIndex);
		if (!pageDictionaryObject)
		    return eFailure;
		MapIterator<PDFNameToPDFObjectMap>  pageDictionaryObjectIt = pageDictionaryObject->GetIterator();

		// create modified page object
		objectContext.StartModifiedIndirectObject(pageObjectID);
		DictionaryContext* modifiedPageObject = mWriter->GetObjectsContext().StartDictionary();

		// copy all elements of the page to the new page object, but the "Contents", "Resources" and "Annots" elements
		while (pageDictionaryObjectIt.MoveNext())
		{
			if (pageDictionaryObjectIt.GetKey()->GetValue() != "Resources" &&
				pageDictionaryObjectIt.GetKey()->GetValue() != "Contents" &&
				pageDictionaryObjectIt.GetKey()->GetValue() != "Annots")
			{
				modifiedPageObject->WriteKey(pageDictionaryObjectIt.GetKey()->GetValue());
				copyingContext->CopyDirectObjectAsIs(pageDictionaryObjectIt.GetValue());
			}
		}

		// Write new annotations entry, joining existing annotations, and new ones (from links attaching or what not)
		if (pageDictionaryObject->Exists("Annots") || mWriter->GetDocumentContext().GetAnnotations().size() > 0)
		{
			modifiedPageObject->WriteKey("Annots");
			objectContext.StartArray();

			// write old annots, if any exist
			if(pageDictionaryObject->Exists("Annots")) {
				PDFObjectCastPtr<PDFArray> anArray(copyingContext->GetSourceDocumentParser()->QueryDictionaryObject(pageDictionaryObject.GetPtr(), "Annots"));
				SingleValueContainerIterator<PDFObjectVector> refs = anArray->GetIterator();
				while (refs.MoveNext())
					copyingContext->CopyDirectObjectAsIs(refs.GetItem());

			}

			// write new annots from links
			ObjectIDTypeSet& annotations = mWriter->GetDocumentContext().GetAnnotations();
			if (annotations.size() > 0)
			{
				ObjectIDTypeSet::iterator it = annotations.begin();
				for (; it != annotations.end(); ++it)
					objectContext.WriteNewIndirectObjectReference(*it);
			}
			annotations.clear();
			objectContext.EndArray(eTokenSeparatorEndLine);

		}

		// Write new contents entry, joining the existing contents with the new one. take care of various scenarios of the existing Contents
		modifiedPageObject->WriteKey("Contents");
		if (!pageDictionaryObject->Exists("Contents"))
		{	// no contents
			objectContext.WriteIndirectObjectReference(newContentObjectID);
		}
		else
		{
			objectContext.StartArray();
			if (mEnsureContentEncapsulation)
			{
				newEncapsulatingObjectID = objectContext.GetInDirectObjectsRegistry().AllocateNewObjectID();
				objectContext.WriteNewIndirectObjectReference(newEncapsulatingObjectID);
			}

			RefCountPtr<PDFObject> pageContent(copyingContext->GetSourceDocumentParser()->QueryDictionaryObject(pageDictionaryObject.GetPtr(), "Contents"));
			if (pageContent->GetType() == PDFObject::ePDFObjectStream)
			{
				// single content stream. must be a refrence which points to it
				PDFObjectCastPtr<PDFIndirectObjectReference> ref(pageDictionaryObject->QueryDirectObject("Contents"));
				objectContext.WriteIndirectObjectReference(ref->mObjectID, ref->mVersion);
			}
			else if (pageContent->GetType() == PDFObject::ePDFObjectArray)
			{
				PDFArray* anArray = (PDFArray*)pageContent.GetPtr();

				// multiple content streams
				SingleValueContainerIterator<PDFObjectVector> refs = anArray->GetIterator();
				PDFObjectCastPtr<PDFIndirectObjectReference> ref;
				while (refs.MoveNext())
				{
					ref = refs.GetItem();
					objectContext.WriteIndirectObjectReference(ref->mObjectID, ref->mVersion);
				}

			}
			else {
				// this basically means no content...or whatever. just ignore.
			}

			objectContext.WriteNewIndirectObjectReference(newContentObjectID);
			objectContext.EndArray();
			objectContext.EndLine();
		}

		// Write a new resource entry. copy all but the "XObject" entry, which needs to be modified. Just for kicks i'm keeping the original 
		// form (either direct dictionary, or indirect object)
		ObjectIDType resourcesIndirect = 0;
		ObjectIDType newResourcesIndirect = 0;
		vector<string> formResourcesNames;

		modifiedPageObject->WriteKey("Resources");
		if (!pageDictionaryObject->Exists("Resources"))
		{
			// check if there's inherited dict. if so - write directly as a modified version
			PDFObjectCastPtr<PDFDictionary> parentDict(
				pageDictionaryObject->Exists("Parent") ? 
					copyingContext->GetSourceDocumentParser()->QueryDictionaryObject(pageDictionaryObject.GetPtr(), "Parent"): 
					NULL);
			if(!parentDict) {
				formResourcesNames = WriteNewResourcesDictionary(objectContext);
			}
			else {
				PDFObjectCastPtr<PDFDictionary> inheritedResources = findInheritedResources(copyingContext->GetSourceDocumentParser(),parentDict.GetPtr());
				if(!inheritedResources) {
					formResourcesNames = WriteNewResourcesDictionary(objectContext);
				} else {
					formResourcesNames = WriteModifiedResourcesDict(copyingContext->GetSourceDocumentParser(), inheritedResources.GetPtr(), objectContext, copyingContext);
				}
			}
		}
		else
		{
			// resources may be direct, or indirect. if direct, write as is, adding the new form xobject, otherwise wait till page object ends and write then
			PDFObjectCastPtr<PDFIndirectObjectReference> resourceDictRef(pageDictionaryObject->QueryDirectObject("Resources"));
			if (!resourceDictRef)
			{
				PDFObjectCastPtr<PDFDictionary> resourceDict(pageDictionaryObject->QueryDirectObject("Resources"));
				formResourcesNames = WriteModifiedResourcesDict(copyingContext->GetSourceDocumentParser(), resourceDict.GetPtr(), objectContext, copyingContext);
			}
			else
			{
				resourcesIndirect = resourceDictRef->mObjectID;
				// later will write a modified version of the resources dictionary, with the new form.
				// only modify the resources dict object if wasn't already modified (can happen when sharing resources dict between multiple pages).
				// in the case where it was alrady modified, create a new resources dictionary that's a copy, and use it instead, to avoid overwriting
				// the previous modification
				GetObjectWriteInformationResult res = objectContext.GetInDirectObjectsRegistry().GetObjectWriteInformation(resourcesIndirect);
				if (res.first && res.second.mIsDirty)
				{
					newResourcesIndirect = objectContext.GetInDirectObjectsRegistry().AllocateNewObjectID();
					modifiedPageObject->WriteObjectReferenceValue(newResourcesIndirect);
				}
				else
					modifiedPageObject->WriteObjectReferenceValue(resourcesIndirect);
			}
		}

		objectContext.EndDictionary(modifiedPageObject);
		objectContext.EndIndirectObject();

		if (resourcesIndirect != 0)
		{
			if (newResourcesIndirect != 0)
				objectContext.StartNewIndirectObject(newResourcesIndirect);
			else
				objectContext.StartModifiedIndirectObject(resourcesIndirect);
			PDFObjectCastPtr<PDFDictionary> resourceDict(copyingContext->GetSourceDocumentParser()->ParseNewObject(resourcesIndirect));
			formResourcesNames = WriteModifiedResourcesDict(copyingContext->GetSourceDocumentParser(), resourceDict.GetPtr(), objectContext, copyingContext);
			objectContext.EndIndirectObject();
		}

		// if required write encapsulation code, so that new stream is independent of graphic context of original
		PDFStream* newStream;
		PrimitiveObjectsWriter primitivesWriter;
		if (newEncapsulatingObjectID != 0)
		{
			objectContext.StartNewIndirectObject(newEncapsulatingObjectID);
			newStream = objectContext.StartPDFStream();
			primitivesWriter.SetStreamForWriting(newStream->GetWriteStream());
			primitivesWriter.WriteKeyword("q");
			objectContext.EndPDFStream(newStream);
			delete newStream;
		}

		// last but not least, create the actual content stream object, placing the form
		objectContext.StartNewIndirectObject(newContentObjectID);
		newStream = objectContext.StartPDFStream();
		primitivesWriter.SetStreamForWriting(newStream->GetWriteStream());

		if (newEncapsulatingObjectID != 0) {
			primitivesWriter.WriteKeyword("Q");
		}

		vector<string>::iterator it = formResourcesNames.begin();
		for (; it != formResourcesNames.end(); ++it)
		{
			primitivesWriter.WriteKeyword("q");
			primitivesWriter.WriteInteger(1);
			primitivesWriter.WriteInteger(0);
			primitivesWriter.WriteInteger(0);
			primitivesWriter.WriteInteger(1);
			primitivesWriter.WriteInteger(0);
			primitivesWriter.WriteInteger(0);
			primitivesWriter.WriteKeyword("cm");
			primitivesWriter.WriteName(*it);
			primitivesWriter.WriteKeyword("Do");
			primitivesWriter.WriteKeyword("Q");
		}

		objectContext.EndPDFStream(newStream);

		delete newStream;
		delete copyingContext;
	} while (false);

	return status;
}

vector<string> PDFModifiedPage::WriteModifiedResourcesDict(PDFParser* inParser,PDFDictionary* inResourcesDictionary,ObjectsContext& inObjectContext,PDFDocumentCopyingContext* inCopyingContext)
{
	vector<string> formResourcesNames;

    MapIterator<PDFNameToPDFObjectMap>  resourcesDictionaryIt = inResourcesDictionary->GetIterator();
        
    // create modified page object
    DictionaryContext* dict = mWriter->GetObjectsContext().StartDictionary();
        
    // copy all elements of the page to the new page object, but the "Contents" and "Resources" elements
    while(resourcesDictionaryIt.MoveNext())
    {
        if(resourcesDictionaryIt.GetKey()->GetValue() != "XObject")
        {
            dict->WriteKey(resourcesDictionaryIt.GetKey()->GetValue());
            inCopyingContext->CopyDirectObjectAsIs(resourcesDictionaryIt.GetValue());
        }
    }   

    // now write a new xobject entry.
	dict->WriteKey("XObject");
	DictionaryContext* xobjectDict = inObjectContext.StartDictionary();

	PDFObjectCastPtr<PDFDictionary> existingXObjectDict(inParser->QueryDictionaryObject(inResourcesDictionary,"XObject"));
    string imageObjectName;
	if(existingXObjectDict.GetPtr())
	{
        // i'm having a very sophisticated algo here to create a new unique name. 
        // i'm making sure it's different in one letter from any name, using a well known discrete math proof method

		MapIterator<PDFNameToPDFObjectMap>  itExisting = existingXObjectDict->GetIterator();
		unsigned long i=0;
		while(itExisting.MoveNext())
		{
			string name = itExisting.GetKey()->GetValue();
			xobjectDict->WriteKey(name);
			inCopyingContext->CopyDirectObjectAsIs(itExisting.GetValue());
			imageObjectName.push_back((char)(GetDifferentChar((name.length() >= i+1) ? name[i]:0x39)));
			++i;
		}
		inObjectContext.EndLine();
	}

	PDFFormXObjectVector::iterator itForms = mContenxts.begin();
	imageObjectName.push_back('_');
	for(int i=0;itForms != mContenxts.end();++i,++itForms)
	{
		string formObjectName = imageObjectName + Int(i).ToString();
		xobjectDict->WriteKey(formObjectName);
		xobjectDict->WriteObjectReferenceValue((*itForms)->GetObjectID());
		formResourcesNames.push_back(formObjectName);
	}

	inObjectContext.EndDictionary(xobjectDict);
	inObjectContext.EndDictionary(dict);

	return formResourcesNames;
}


unsigned char PDFModifiedPage::GetDifferentChar(unsigned char inCharCode)
{
    // numerals
    if(inCharCode >= 0x30 && inCharCode <= 0x38)
        return inCharCode+1;
    if(inCharCode == 0x39)
        return 0x30;

    // lowercase
    if(inCharCode >= 0x61 && inCharCode <= 0x79)
        return inCharCode+1;
    if(inCharCode == 0x7a)
        return 0x61;

    // uppercase
    if(inCharCode >= 0x41 && inCharCode <= 0x59)
        return inCharCode+1;
    if(inCharCode == 0x5a)
        return 0x41;

    return 0x41;
}

PDFFormXObject* PDFModifiedPage::GetCurrentFormContext()
{
	return mCurrentContext;
}

ResourcesDictionary* PDFModifiedPage::GetCurrentResourcesDictionary()
{
	return mCurrentContext ? &(mCurrentContext->GetResourcesDictionary()):NULL;
}
