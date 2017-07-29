/*
   Source File : PDFPage.cpp


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
#include "PDFPage.h"
#include "Trace.h"

PDFPage::PDFPage(void)
{
	mContentContext = NULL;
	mRotate.first = false;
    mTrimBox.first = false;
    mArtBox.first = false;
    mCropBox.first = false;
    mBleedBox.first = false;
}

PDFPage::~PDFPage(void)
{
}


void PDFPage::SetMediaBox(const PDFRectangle& inMediaBox)
{
	mMediaBox = inMediaBox;
}

const PDFRectangle& PDFPage::GetMediaBox() const
{
	return mMediaBox;
}

void PDFPage::SetRotate( int inRotate )
{
	if ( inRotate % 90 )
	{ 
        TRACE_LOG("PDFPage::SetRotate, Exception, the value must be a multiple of 90. defaulting to 0");
        inRotate = 0;
	}
	mRotate.first = true;
	mRotate.second = inRotate;
}

const BoolAndInt& PDFPage::GetRotate() const
{
	return mRotate;
}

void PDFPage::AddContentStreamReference(ObjectIDType inStreamReference)
{
	mContentStreamReferences.push_back(inStreamReference);
}

ObjectIDType PDFPage::GetContentStreamsCount()
{
	return (ObjectIDType)mContentStreamReferences.size();
}

SingleValueContainerIterator<ObjectIDTypeList> PDFPage::GetContentStreamReferencesIterator()
{
	return SingleValueContainerIterator<ObjectIDTypeList>(mContentStreamReferences);
}

ResourcesDictionary& PDFPage::GetResourcesDictionary()
{
	return mResources;
}

PageContentContext* PDFPage::GetAssociatedContentContext()
{
	return mContentContext;
}

void PDFPage::DisassociateContentContext()
{
	mContentContext = NULL;
}

void PDFPage::AssociateContentContext(PageContentContext* inContentContext)
{
	mContentContext = inContentContext;
}

void PDFPage::SetCropBox(const PDFRectangle& inCropBox)
{
    mCropBox.first = true;
    mCropBox.second = inCropBox;
}

const BoolAndPDFRectangle& PDFPage::GetCropBox() const
{
    return mCropBox;
}

void PDFPage::SetBleedBox(const PDFRectangle& inBleedBox)
{
    mBleedBox.first = true;
    mBleedBox.second = inBleedBox;
}

const BoolAndPDFRectangle& PDFPage::GetBleedBox() const
{
    return mBleedBox;
}

void PDFPage::SetTrimBox(const PDFRectangle& inTrimBox)
{
    mTrimBox.first = true;
    mTrimBox.second = inTrimBox;
}

const BoolAndPDFRectangle& PDFPage::GetTrimBox() const
{
    return mTrimBox;
}

void PDFPage::SetArtBox(const PDFRectangle& inArtBox)
{
    mArtBox.first = true;
    mArtBox.second = inArtBox;
}

const BoolAndPDFRectangle& PDFPage::GetArtBox() const
{
    return mArtBox;
}
