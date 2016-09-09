/*
   Source File : PDFFormXObject.cpp


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
#include "PDFFormXObject.h"
#include "PDFStream.h"
#include "XObjectContentContext.h"
#include "ObjectsContext.h"

PDFFormXObject::PDFFormXObject(PDFHummus::DocumentContext* inDocumentContext,ObjectIDType inFormXObjectID,PDFStream* inXObjectStream,ObjectIDType inFormXObjectResourcesDictionaryID)
{
	mXObjectID = inFormXObjectID;
	mResourcesDictionaryID = inFormXObjectResourcesDictionaryID;
	mContentStream = inXObjectStream;
	mContentContext = new XObjectContentContext(inDocumentContext,this);	
}

PDFFormXObject::~PDFFormXObject(void)
{
	delete mContentStream;
	delete mContentContext;
}

ObjectIDType PDFFormXObject::GetObjectID()
{
	return mXObjectID;
}

ObjectIDType PDFFormXObject::GetResourcesDictionaryObjectID()
{
	return mResourcesDictionaryID;
}

ResourcesDictionary& PDFFormXObject::GetResourcesDictionary()
{
	return mResources;
}

PDFStream* PDFFormXObject::GetContentStream()
{
	return mContentStream;
}

XObjectContentContext* PDFFormXObject::GetContentContext()
{
	return mContentContext;
}