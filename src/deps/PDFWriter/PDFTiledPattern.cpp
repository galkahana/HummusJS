/*
   Source File : PDFTiledPattern.cpp


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
#include "PDFTiledPattern.h"
#include "PDFStream.h"
#include "TiledPatternContentContext.h"
#include "ObjectsContext.h"

PDFTiledPattern::PDFTiledPattern(PDFHummus::DocumentContext* inDocumentContext,ObjectIDType inObjectID, PDFStream* inStream, ObjectIDType inResourcesDictionaryID)
{
	mObjectID = inObjectID;
	mResourcesDictionaryID = inResourcesDictionaryID;
	mContentStream = inStream;
	mContentContext = new TiledPatternContentContext(inDocumentContext, this);
}

PDFTiledPattern::~PDFTiledPattern(void)
{
	delete mContentStream;
	delete mContentContext;
}

ObjectIDType PDFTiledPattern::GetObjectID()
{
	return mObjectID;
}

ObjectIDType PDFTiledPattern::GetResourcesDictionaryObjectID()
{
	return mResourcesDictionaryID;
}

ResourcesDictionary& PDFTiledPattern::GetResourcesDictionary()
{
	return mResources;
}

PDFStream* PDFTiledPattern::GetContentStream()
{
	return mContentStream;
}

TiledPatternContentContext* PDFTiledPattern::GetContentContext()
{
	return mContentContext;
}