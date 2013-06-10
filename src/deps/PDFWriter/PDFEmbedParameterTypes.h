/*
   Source File : PDFEmbedParameterTypes.h


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
#pragma once
#include "EStatusCode.h"
#include "ObjectsBasicTypes.h"

#include <list>
#include <utility>

class PDFFormXObject;
class PDFPage;



typedef std::pair<unsigned long,unsigned long> ULongAndULong;
typedef std::list<ULongAndULong> ULongAndULongList;
typedef std::list<PDFFormXObject*> PDFFormXObjectList;
typedef std::pair<PDFHummus::EStatusCode,PDFFormXObjectList> EStatusCodeAndPDFFormXObjectList;

typedef std::list<ObjectIDType> ObjectIDTypeList;
typedef std::pair<PDFHummus::EStatusCode,ObjectIDTypeList> EStatusCodeAndObjectIDTypeList;
typedef std::pair<PDFHummus::EStatusCode,ObjectIDType> EStatusCodeAndObjectIDType;

/*
	PDFPageRange is to be used as a method to select pages ranges. quite simply it's either "All" or "Specific".
	The choice is represented by mType member.
	With specific you get to describe the particular ranges. the ranges are a strictly ascending list of pairs, where each
	pair marks an inclusive range of pages. e.g good paris are : [1,3], [4,6], [7,7].  indexes should be zero based.
*/
struct PDFPageRange
{
	enum ERangeType
	{
		eRangeTypeAll,
		eRangeTypeSpecific
	};

	PDFPageRange() {mType = eRangeTypeAll;}

	ERangeType mType;
	ULongAndULongList mSpecificRanges;
};

/*
	EPDFPageBox enumerates PDF boxes, to select from, when in need.
*/
enum EPDFPageBox
{
	ePDFPageBoxMediaBox,
	ePDFPageBoxCropBox,
	ePDFPageBoxBleedBox,
	ePDFPageBoxTrimBox,
	ePDFPageBoxArtBox
};