/*
   Source File : PNGImageHandler.h


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

#ifndef PDFHUMMUS_NO_PNG
#include "ObjectsBasicTypes.h"

#include <utility>

class ObjectsContext;
class PDFFormXObject;
class IByteReaderWithPosition;

namespace PDFHummus
{
	class DocumentContext;
}

using namespace PDFHummus;

typedef std::pair<double, double> DoubleAndDoublePair;

class PNGImageHandler
{
public:
	struct PNGImageInfo
	{
		DoubleAndDoublePair dimensions;
		int colorComponents;
		bool hasAlpha;
	};



	PNGImageHandler();
	~PNGImageHandler(void);

	PDFFormXObject* CreateFormXObjectFromPNGStream(IByteReaderWithPosition* inPNGStream, ObjectIDType inFormXObjectID);

	void SetOperationsContexts(PDFHummus::DocumentContext* inDocumentContext,ObjectsContext* inObjectsContext);


	DoubleAndDoublePair ReadImageDimensions(IByteReaderWithPosition* inPNGStream);
	PNGImageInfo ReadImageInfo(IByteReaderWithPosition* inPNGStream);


private:
	ObjectsContext* mObjectsContext;
	PDFHummus::DocumentContext* mDocumentContext;
};
#endif