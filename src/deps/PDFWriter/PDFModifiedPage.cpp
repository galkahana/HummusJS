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
#include "DictionaryContext.h"
#include "PDFIndirectObjectReference.h"
#include "PDFArray.h"
#include "BoxingBase.h"

#include <string>

using namespace std;

PDFModifiedPage::PDFModifiedPage(PDFWriter* inWriter,unsigned long inPageIndex)
{
	mWriter = inWriter;
	mPageIndex = inPageIndex;
	mCurrentContext = NULL;
}

PDFModifiedPage::~PDFModifiedPage(void)
{
}

AbstractContentContext* PDFModifiedPage::StartContentContext()
{
	if(!mCurrentContext)
	{
		PDFRectangle mediaBox = PDFPageInput(&mWriter->GetModifiedFileParser(),mWriter->GetModifiedFileParser().ParsePage(mPageIndex)).GetMediaBox();
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
	EStatusCode status = mWriter->EndFormXObject(mCurrentContext);
	mContenxts.push_back(mCurrentContext);
	mCurrentContext = NULL;
	return status;
}

AbstractContentContext* PDFModifiedPage::GetContentContext()
{
	return mCurrentContext ? mCurrentContext->GetContentContext():NULL;
}


PDFHummus::EStatusCode PDFModifiedPage::WritePage()
{
    // allocate an object ID for the new contents stream (for placing the form)
    // we first create the modified page object, so that we can define a name for the new form xobject
    // that is unique
	ObjectsContext& objectContext  = mWriter->GetObjectsContext();
	ObjectIDType newContentObjectID = objectContext.GetInDirectObjectsRegistry().AllocateNewObjectID();


    // create a copying context, so we can copy the page dictionary, and modify its contents + resources dict
	PDFDocumentCopyingContext* copyingContext = mWriter->CreatePDFCopyingContextForModifiedFile();

	// get the page object
	ObjectIDType pageObjectID = copyingContext->GetSourceDocumentParser()->GetPageObjectID(mPageIndex);
    PDFObjectCastPtr<PDFDictionary> pageDictionaryObject = copyingContext->GetSourceDocumentParser()->ParsePage(mPageIndex);
    MapIterator<PDFNameToPDFObjectMap>  pageDictionaryObjectIt = pageDictionaryObject->GetIterator();
        
    // create modified page object
    objectContext.StartModifiedIndirectObject(pageObjectID);
    DictionaryContext* modifiedPageObject = mWriter->GetObjectsContext().StartDictionary();
        
    // copy all elements of the page to the new page object, but the "Contents" and "Resources" elements
    while(pageDictionaryObjectIt.MoveNext())
    {
        if(pageDictionaryObjectIt.GetKey()->GetValue() != "Resources" &&
			pageDictionaryObjectIt.GetKey()->GetValue() != "Contents")
        {
            modifiedPageObject->WriteKey(pageDictionaryObjectIt.GetKey()->GetValue());
            copyingContext->CopyDirectObjectAsIs(pageDictionaryObjectIt.GetValue());
        }
    }   

    // Write new contents entry, joining the existing contents with the new one. take care of various scenarios of the existing Contents
	modifiedPageObject->WriteKey("Contents");
	if(!pageDictionaryObject->Exists("Contents"))
	{	// no contents
		objectContext.WriteIndirectObjectReference(newContentObjectID);
	}
	else
	{
		objectContext.StartArray();
		PDFObjectCastPtr<PDFArray> anArray(pageDictionaryObject->QueryDirectObject("Contents"));
		if(!anArray)
		{
			// single content stream
			PDFObjectCastPtr<PDFIndirectObjectReference> ref(pageDictionaryObject->QueryDirectObject("Contents"));
			objectContext.WriteIndirectObjectReference(ref->mObjectID,ref->mVersion);
		}
		else
		{
			// multiple content streams
			SingleValueContainerIterator<PDFObjectVector> refs = anArray->GetIterator();
			PDFObjectCastPtr<PDFIndirectObjectReference> ref;
			while(refs.MoveNext())
			{
				ref = refs.GetItem();
				objectContext.WriteIndirectObjectReference(ref->mObjectID,ref->mVersion);
			}
		}
		objectContext.WriteIndirectObjectReference(newContentObjectID);
		objectContext.EndArray();
		objectContext.EndLine();
	}

    // Write a new resource entry. copy all but the "XObject" entry, which needs to be modified. Just for kicks i'm keeping the original 
    // form (either direct dictionary, or indirect object)
	ObjectIDType resourcesIndirect = 0;
	ObjectIDType newResourcesIndirect = 0;
	vector<string> formResourcesNames;

	modifiedPageObject->WriteKey("Resources");
	if(!pageDictionaryObject->Exists("Resources"))
	{
        // no existing resource dictionary, so write a new one
		DictionaryContext*  dict = objectContext.StartDictionary();
		dict->WriteKey("XObject");
		DictionaryContext* xobjectDict = objectContext.StartDictionary();
		for(unsigned long i=0;i<mContenxts.size();++i)
		{
			string formObjectName = string("myForm_") + Int(i).ToString();
            dict->WriteKey(formObjectName);
            dict->WriteObjectReferenceValue(mContenxts[i]->GetObjectID());
            formResourcesNames.push_back(formObjectName);
		}
		objectContext.EndDictionary(xobjectDict);
		objectContext.EndDictionary(dict);
	}
	else
	{
        // resources may be direct, or indirect. if direct, write as is, adding the new form xobject, otherwise wait till page object ends and write then
		PDFObjectCastPtr<PDFIndirectObjectReference> resourceDictRef(pageDictionaryObject->QueryDirectObject("Resources"));
		if(!resourceDictRef)
		{
			PDFObjectCastPtr<PDFDictionary> resourceDict(pageDictionaryObject->QueryDirectObject("Resources"));
			formResourcesNames = WriteModifiedResourcesDict(copyingContext->GetSourceDocumentParser(),resourceDict.GetPtr(),objectContext,copyingContext);
		}
		else
		{
            resourcesIndirect = resourceDictRef->mObjectID;
			// later will write a modified version of the resources dictionary, with the new form.
			// only modify the resources dict object if wasn't already modified (can happen when sharing resources dict between multiple pages).
			// in the case where it was alrady modified, create a new resources dictionary that's a copy, and use it instead, to avoid overwriting
			// the previous modification
			GetObjectWriteInformationResult res =  objectContext.GetInDirectObjectsRegistry().GetObjectWriteInformation(resourcesIndirect);
			if(res.first && res.second.mIsDirty)
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

	if(resourcesIndirect!=0)
	{
		if(newResourcesIndirect != 0)
			objectContext.StartNewIndirectObject(newResourcesIndirect);
		else
			objectContext.StartModifiedIndirectObject(resourcesIndirect);
		PDFObjectCastPtr<PDFDictionary> resourceDict(copyingContext->GetSourceDocumentParser()->ParseNewObject(resourcesIndirect));
		formResourcesNames =  WriteModifiedResourcesDict(copyingContext->GetSourceDocumentParser(),resourceDict.GetPtr(),objectContext,copyingContext);
		objectContext.EndIndirectObject();
	}

    // last but not least, create the actual content stream object, placing the form
	objectContext.StartNewIndirectObject(newContentObjectID);
	PDFStream* newStream = objectContext.StartUnfilteredPDFStream();

	vector<string>::iterator it = formResourcesNames.begin();
	for(;it!=formResourcesNames.end();++it)
	{
		objectContext.WriteKeyword("q");
		objectContext.WriteInteger(1);
		objectContext.WriteInteger(0);
		objectContext.WriteInteger(0);
		objectContext.WriteInteger(1);
		objectContext.WriteInteger(0);
		objectContext.WriteInteger(0);
		objectContext.WriteKeyword("cm");
		objectContext.WriteName(*it);
		objectContext.WriteKeyword("Do");
		objectContext.WriteKeyword("Q");
	}

	objectContext.EndPDFStream(newStream);
	objectContext.EndIndirectObject();

	return eSuccess;
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
