/*
   Source File : ShadingWriter.cpp


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

// The following implementation derives a lot of its ideas and some actual code from Google's Skia 2D library (https://skia.org/).
// Specifically the PDF Shader implementation via SkGradientShader class. I'm therefore placing here below Skia's license and copyright notice.

/*
    Copyright (c) 2011 Google Inc. All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in
        the documentation and/or other materials provided with the
        distribution.

    * Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "ShadingWriter.h"
#include "DocumentContext.h"
#include "ObjectsContext.h"
#include "PDFTiledPattern.h"
#include "TiledPatternContentContext.h"
#include "DictionaryContext.h"
#include "PDFFormXObject.h"
#include "XObjectContentContext.h"
#include "PDFStream.h"
#include "IByteWriter.h"
#include "PrimitiveObjectsWriter.h"

#include <math.h>

using namespace PDFHummus;
using namespace std;

ShadingWriter::ShadingWriter(ObjectsContext* inObjectsContext, PDFHummus::DocumentContext* inDocumentContext) {
    mObjectsContext = inObjectsContext;
    mDocumentContext = inDocumentContext;
}

ShadingWriter::~ShadingWriter() {

}

PDFHummus::EStatusCode ShadingWriter::WriteRadialShadingPatternObject(
    double inX0,
    double inY0,
    double inR0,
    double inX1,
    double inY1,
    double inR1,
    const InterpretedGradientStopList& inColorLine,
    FT_PaintExtend inGradientExtend,
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId
) {
    RadialShading radialParams = {
        inX0,
        inY0,
        inR0,
        inX1,
        inY1,
        inR1,
        inColorLine,
        inGradientExtend
    };

    if(ColorLineHasTransparency(inColorLine))
        return WriteRGBATiledPatternObject(radialParams, inBounds, inMatrix, inPatternObjectId);
    else
        return WriteRGBShadingPatternObject(radialParams, inBounds, inMatrix, inPatternObjectId);
}

bool ShadingWriter::ColorLineHasTransparency(const InterpretedGradientStopList& inColorLine) {
    bool no = true;

    InterpretedGradientStopList::const_iterator it = inColorLine.begin();

    for(; it != inColorLine.end() & no;++it) {
        no = (it->alpha == 1.0) && (it->color.alpha == 255);
    }

    return !no;
}

EStatusCode ShadingWriter::WriteSoftmaskPatternObjectForRGBAShades(
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId,
    ObjectIDType& outAlphaShadeObjectId,
    ObjectIDType& outRGBShadeObjectId
) {
    EStatusCode status = eSuccess;

    do {
        ObjectIDType transparencySmaskExtGStateObjectId = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
        ObjectIDType alphaShadingPatternObjectId = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
        ObjectIDType rgbShadingPatternObjectId = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();

        PDFRectangle tiledPatternBounds = inMatrix.Transform(inBounds);

        PDFTiledPattern* tiledPattern = mDocumentContext->StartTiledPattern(
                                        1,
                                        1,
                                        tiledPatternBounds,
                                        tiledPatternBounds.GetWidth(),
                                        tiledPatternBounds.GetHeight(),
                                        inPatternObjectId,
                                        NULL);

        TiledPatternContentContext* patternContentContext = tiledPattern->GetContentContext();
        patternContentContext->gs(tiledPattern->GetResourcesDictionary().AddExtGStateMapping(transparencySmaskExtGStateObjectId));
        patternContentContext->cs("Pattern");
        patternContentContext->scn(tiledPattern->GetResourcesDictionary().AddPatternMapping(rgbShadingPatternObjectId));
        patternContentContext->re(tiledPatternBounds.LowerLeftX,tiledPatternBounds.LowerLeftY,tiledPatternBounds.GetWidth(),tiledPatternBounds.GetHeight());
        patternContentContext->fStar();
        status = mDocumentContext->EndTiledPatternAndRelease(tiledPattern);
        if(status != eSuccess)
            break;

        status = WriteAlphaSoftMaskExtGStateObject(inBounds, inMatrix, alphaShadingPatternObjectId, transparencySmaskExtGStateObjectId);    
        if(status != eSuccess)
            break;

        outAlphaShadeObjectId = alphaShadingPatternObjectId;
        outRGBShadeObjectId = rgbShadingPatternObjectId;
    } while(false);

    return status;  
}

PDFHummus::EStatusCode ShadingWriter::WriteRGBATiledPatternObject(
    const RadialShading& inRadialShading, 
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId
) {
    EStatusCode status = eSuccess;
    ObjectIDType alphaShadingPatternObjectId;
    ObjectIDType rgbShadingPatternObjectId;

    do {
        status = WriteSoftmaskPatternObjectForRGBAShades(inBounds, inMatrix, inPatternObjectId, alphaShadingPatternObjectId, rgbShadingPatternObjectId);
        if(status != eSuccess)
            break;

        // now, draw the alpha shading, which has the same pysical properties of the rgb one, but colors are all the alpha
        // component transitioning as a gradient
        RadialShading alphaRadialShading = inRadialShading;
        alphaRadialShading.colorLine = CreateAlphaShadingColorLine(inRadialShading.colorLine);

        status = WriteRGBShadingPatternObject(alphaRadialShading, inBounds, inMatrix, alphaShadingPatternObjectId);
        if(status != eSuccess)
            break;

        status = WriteRGBShadingPatternObject(inRadialShading, inBounds, inMatrix, rgbShadingPatternObjectId);

    } while(false);


    return status;    

}

InterpretedGradientStopList ShadingWriter::CreateAlphaShadingColorLine(const InterpretedGradientStopList& inColorLine) {
    InterpretedGradientStopList alphaAsRGBColorLine;
    InterpretedGradientStopList::const_iterator it = inColorLine.begin();
    for(; it != inColorLine.end();++it) {
        FT_Byte alphaColor = FT_Byte(floor(it->alpha * it->color.alpha));
        FT_Color color = {
            alphaColor,
            alphaColor,
            alphaColor
        };

        InterpretedGradientStop stop = {
            it->stopOffset,
            1,
            color  
        };

        alphaAsRGBColorLine.push_back(stop);
    }

    return alphaAsRGBColorLine;
}

PDFHummus::EStatusCode ShadingWriter::WriteAlphaSoftMaskExtGStateObject(
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inAlphaShadingPatternObjectId,
    ObjectIDType inExtGStateObjectId
) {
    EStatusCode status = eSuccess;

    do {
        ObjectIDType smaskFormObjectId = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();

        // write ExtGState with soft mask reference
        mObjectsContext->StartNewIndirectObject(inExtGStateObjectId);
        DictionaryContext* extGStateDict = mObjectsContext->StartDictionary();

        extGStateDict->WriteKey("Type");
        extGStateDict->WriteNameValue("ExtGState");
        extGStateDict->WriteKey("SMask");
        DictionaryContext* smaskDict = mObjectsContext->StartDictionary();
        
        smaskDict->WriteKey("Type");
        smaskDict->WriteNameValue("Mask");
        smaskDict->WriteKey("S");
        smaskDict->WriteNameValue("Luminosity");
        smaskDict->WriteKey("G");
        smaskDict->WriteNewObjectReferenceValue(smaskFormObjectId);
        status = mObjectsContext->EndDictionary(smaskDict);
        if(status != eSuccess)
            break;
        status = mObjectsContext->EndDictionary(extGStateDict);
        if(status != eSuccess)
            break;
        mObjectsContext->EndIndirectObject();

        status = WriteAlphaSoftMaskXObjectFormObject(
            inBounds,
            inMatrix,
            inAlphaShadingPatternObjectId,
            smaskFormObjectId  
        );

    } while(false);

    return status;    
}

class FromTransparencyGroupWriter: public DocumentContextExtenderAdapter {
    public:
        FromTransparencyGroupWriter(){};
        virtual ~FromTransparencyGroupWriter(){};

        virtual PDFHummus::EStatusCode OnFormXObjectWrite(
                        ObjectIDType inFormXObjectID,
                        ObjectIDType inFormXObjectResourcesDictionaryID,
                        DictionaryContext* inFormDictionaryContext,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext){
            inFormDictionaryContext->WriteKey("Group");
            DictionaryContext* transparencyGroup = inPDFWriterObjectContext->StartDictionary();
            transparencyGroup->WriteKey("Type");
            transparencyGroup->WriteNameValue("Group");
            transparencyGroup->WriteKey("CS");
            transparencyGroup->WriteNameValue("DeviceRGB");
            transparencyGroup->WriteKey("S");
            transparencyGroup->WriteNameValue("Transparency");
            transparencyGroup->WriteKey("I");
            transparencyGroup->WriteBooleanValue(true);
            return inPDFWriterObjectContext->EndDictionary(transparencyGroup);
        }
};

PDFHummus::EStatusCode ShadingWriter::WriteAlphaSoftMaskXObjectFormObject(
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inAlphaShadingPatternObjectId,
    ObjectIDType inXObjectFormObjectId            
) {
    EStatusCode status = eSuccess;

    PDFRectangle formBounds = inMatrix.Transform(inBounds);

    do {
        // adding a small extender to allow writing full transparency group data
        FromTransparencyGroupWriter groupWriter;
        mDocumentContext->AddDocumentContextExtender(&groupWriter);
        PDFFormXObject* form = mDocumentContext->StartFormXObject(formBounds,inXObjectFormObjectId,NULL,false);
        mDocumentContext->RemoveDocumentContextExtender(&groupWriter);

        XObjectContentContext* xobjectContentContext = form->GetContentContext();

        xobjectContentContext->cs("Pattern");
        xobjectContentContext->scn(form->GetResourcesDictionary().AddPatternMapping(inAlphaShadingPatternObjectId));
        xobjectContentContext->re(formBounds.LowerLeftX,formBounds.LowerLeftY,formBounds.GetWidth(),formBounds.GetHeight());
        xobjectContentContext->fStar();

        status = mDocumentContext->EndFormXObjectAndRelease(form);
        if(status != eSuccess)
            break;

    } while(false);
    return status;
}

PDFHummus::EStatusCode ShadingWriter::WriteRGBShadingPatternObject(
    const RadialShading& inRadialShading, 
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId
) {
    bool usePDFNativeFunctions = inRadialShading.gradientExtend == FT_COLR_PAINT_EXTEND_PAD;

    if(usePDFNativeFunctions) {
        return WriteShadingPatternObjectWithPDFNativeFunctions(
            inRadialShading, 
            inBounds,
            inMatrix,
            inPatternObjectId
        );
    } else {
        return WriteShadingPatternObjectWithPDFCustomFunctions(
            inRadialShading, 
            inBounds,
            inMatrix,
            inPatternObjectId
        );
    }
}

PDFHummus::EStatusCode ShadingWriter::WriteShadingPatternObjectWithPDFNativeFunctions(
    const RadialShading& inRadialShading, 
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId
) {
    double coords[] = {inRadialShading.x0, inRadialShading.y0, inRadialShading.r0,
                       inRadialShading.x1, inRadialShading.y1, inRadialShading.r1};

    return WriteShadingPatternObjectWithPDFNativeFunctions(
        3,
        coords,
        6,
        inRadialShading.colorLine,
        inBounds,
        inMatrix,
        inPatternObjectId
    );
}

EStatusCode ShadingWriter::WriteShadingPatternObjectWithPDFNativeFunctions(
    unsigned short inShadingType,
    double* inCoords,
    unsigned short inCoordsSize,
    const InterpretedGradientStopList& inColorLine,
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId
) {
    EStatusCode status = eSuccess;

    ObjectIDType shadingObjectId;

    do {
        status = WriteShadingPatternObject(inMatrix, inPatternObjectId, shadingObjectId);
        if(status != eSuccess)
            break;

        mObjectsContext->StartNewIndirectObject(shadingObjectId);
        DictionaryContext* shadingDict = mObjectsContext->StartDictionary();
        shadingDict->WriteKey("ShadingType");
        shadingDict->WriteIntegerValue(inShadingType);
        shadingDict->WriteKey("Coords");
        mObjectsContext->StartArray();
        for(unsigned short i = 0; i< inCoordsSize; ++i) {
            mObjectsContext->WriteDouble(inCoords[i]);
        }
        mObjectsContext->EndArray(eTokenSeparatorEndLine);    
        shadingDict->WriteKey("ColorSpace");
        shadingDict->WriteNameValue("DeviceRGB");
        shadingDict->WriteKey("Extend");
        mObjectsContext->StartArray();
        mObjectsContext->WriteBoolean(true);
        mObjectsContext->WriteBoolean(true);
        mObjectsContext->EndArray(eTokenSeparatorEndLine);
        shadingDict->WriteKey("Function");
        DictionaryContext* functionDict = mObjectsContext->StartDictionary();
        functionDict->WriteKey("FunctionType");
        functionDict->WriteIntegerValue(3);
        functionDict->WriteKey("Bounds");
        mObjectsContext->StartArray();
        // bound contains whats within the edges, so should skip
        // first and last (which are expected to be 0 and 1)
        InterpretedGradientStopList::const_iterator it1 = inColorLine.begin();
        InterpretedGradientStopList::const_iterator it2 = inColorLine.begin();
        ++it1;
        ++it2;
        ++it1;
        for(; it1 != inColorLine.end();++it1,++it2) {
            mObjectsContext->WriteDouble(it2->stopOffset);
        }
        mObjectsContext->EndArray(eTokenSeparatorEndLine);
        // last stop, which should probably be 1, is saved cause will be used in domains
        double lastStop = it2->stopOffset;
        functionDict->WriteKey("Domain");
        mObjectsContext->StartArray();
        mObjectsContext->WriteDouble(0);
        mObjectsContext->WriteDouble(1);
        mObjectsContext->EndArray(eTokenSeparatorEndLine);
        functionDict->WriteKey("Encode");
        mObjectsContext->StartArray();
        // write encodes for size-1 functions
        for(int i=0; i<inColorLine.size()-1;++i) {
            mObjectsContext->WriteDouble(0);
            mObjectsContext->WriteDouble(1);
        }
        mObjectsContext->EndArray(eTokenSeparatorEndLine);
        functionDict->WriteKey("Functions");
        // draw all stops, each in its own function. note that there should be size-1 functions!
        mObjectsContext->StartArray();
        it1 = inColorLine.begin();
        it2 = inColorLine.begin();
        ++it1;
        for(; it1 != inColorLine.end() && eSuccess == status;++it1,++it2) {
            DictionaryContext* colorStopDict = mObjectsContext->StartDictionary();
            colorStopDict->WriteKey("FunctionType");
            colorStopDict->WriteIntegerValue(2);
            colorStopDict->WriteKey("N");
            colorStopDict->WriteIntegerValue(1);
            colorStopDict->WriteKey("Domain");
            mObjectsContext->StartArray();
            mObjectsContext->WriteDouble(0);
            mObjectsContext->WriteDouble(lastStop);
            mObjectsContext->EndArray(eTokenSeparatorEndLine);
            colorStopDict->WriteKey("C0");
            mObjectsContext->StartArray();
            mObjectsContext->WriteDouble(it2->color.red/255.0);
            mObjectsContext->WriteDouble(it2->color.green/255.0);
            mObjectsContext->WriteDouble(it2->color.blue/255.0);
            mObjectsContext->EndArray(eTokenSeparatorEndLine);
            colorStopDict->WriteKey("C1");
            mObjectsContext->StartArray();
            mObjectsContext->WriteDouble(it1->color.red/255.0);
            mObjectsContext->WriteDouble(it1->color.green/255.0);
            mObjectsContext->WriteDouble(it1->color.blue/255.0);
            mObjectsContext->EndArray(eTokenSeparatorEndLine);
            status = mObjectsContext->EndDictionary(colorStopDict);
            if(status != eSuccess)
                break;
        } 
        if(status != eSuccess)
            break;

        mObjectsContext->EndArray(eTokenSeparatorEndLine);
        status = mObjectsContext->EndDictionary(functionDict);
        if(status != eSuccess)
            break;
        status = mObjectsContext->EndDictionary(shadingDict);
        if(status != eSuccess)
            break;
        mObjectsContext->EndIndirectObject();
    }while(false);
    return status;
}


EStatusCode ShadingWriter::WriteShadingPatternObject(
    const PDFMatrix& inShadingMatrix,
    ObjectIDType inPatternObjectId,
    ObjectIDType& outShadingObjectId
) {
    ObjectIDType shadingObjectId = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();

    mObjectsContext->StartNewIndirectObject(inPatternObjectId);
    DictionaryContext* patternDict = mObjectsContext->StartDictionary();
    patternDict->WriteKey("Type");
    patternDict->WriteNameValue("Pattern");
    patternDict->WriteKey("PatternType");
    patternDict->WriteIntegerValue(2);
    patternDict->WriteKey("Matrix");
    mObjectsContext->StartArray();
    mObjectsContext->WriteDouble(inShadingMatrix.a);
    mObjectsContext->WriteDouble(inShadingMatrix.b);
    mObjectsContext->WriteDouble(inShadingMatrix.c);
    mObjectsContext->WriteDouble(inShadingMatrix.d);
    mObjectsContext->WriteDouble(inShadingMatrix.e);
    mObjectsContext->WriteDouble(inShadingMatrix.f);
    mObjectsContext->EndArray(eTokenSeparatorEndLine);
    patternDict->WriteKey("Shading");
    patternDict->WriteNewObjectReferenceValue(shadingObjectId);
    EStatusCode status = mObjectsContext->EndDictionary(patternDict);
    if(eSuccess == status)
        mObjectsContext->EndIndirectObject();

    outShadingObjectId = shadingObjectId;

    return status;
}

PDFHummus::EStatusCode ShadingWriter::WriteShadingPatternObjectWithPDFCustomFunctions(
    const RadialShading& inRadialShading, 
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId
) {
    EStatusCode status = eSuccess;
    PDFStream* pdfStream = NULL;

    // apply transform matrix, to map pattern based on function range
    PDFMatrix patternProgramMatrix(1,0,0,1,inRadialShading.x0,inRadialShading.y0);

    do {
        pdfStream = StartCustomFunctionShadingPatternStream(patternProgramMatrix, inBounds, inMatrix, inPatternObjectId);
        if(!pdfStream) {
            status = eFailure;
            break;
        }

        IByteWriter* writeStream = pdfStream->GetWriteStream();
        PrimitiveObjectsWriter primitiveWriter(writeStream);
        WriteGradientCustomFunctionProgram(inRadialShading, writeStream, &primitiveWriter);
        mObjectsContext->EndPDFStream(pdfStream);
    } while(false);

    delete pdfStream;

    return status;
}   

PDFStream* ShadingWriter::StartCustomFunctionShadingPatternStream(
    const PDFMatrix& inPatternProgramMatrix,
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId
) {
    EStatusCode status = eSuccess;
    PDFStream* pdfStream = NULL;

    ObjectIDType shadingObjectId;
    ObjectIDType functionObjectId; 


    PDFMatrix patternMatrix = inPatternProgramMatrix.Multiply(inMatrix);
    PDFRectangle domainBounds = inPatternProgramMatrix.Inverse().Transform(inBounds);

    do {
        status = WriteShadingPatternObject(patternMatrix, inPatternObjectId, shadingObjectId);
        if(status != eSuccess)
            break;

        status = WriteShadingType1Object(domainBounds, shadingObjectId, functionObjectId);
        if(status != eSuccess)
            break;            

        // function object
        mObjectsContext->StartNewIndirectObject(functionObjectId);
        DictionaryContext* functionDict = mObjectsContext->StartDictionary();
        functionDict->WriteKey("FunctionType");
        functionDict->WriteIntegerValue(4);
        functionDict->WriteKey("Domain");
        mObjectsContext->StartArray();
        mObjectsContext->WriteDouble(domainBounds.LowerLeftX);
        mObjectsContext->WriteDouble(domainBounds.UpperRightX);
        mObjectsContext->WriteDouble(domainBounds.LowerLeftY);
        mObjectsContext->WriteDouble(domainBounds.UpperRightY);
        mObjectsContext->EndArray(eTokenSeparatorEndLine);    
        functionDict->WriteKey("Range");
        mObjectsContext->StartArray();
        mObjectsContext->WriteInteger(0);
        mObjectsContext->WriteInteger(1);
        mObjectsContext->WriteInteger(0);
        mObjectsContext->WriteInteger(1);
        mObjectsContext->WriteInteger(0);
        mObjectsContext->WriteInteger(1);
        mObjectsContext->EndArray(eTokenSeparatorEndLine);            
        
        // now lets make this a stream so we can write the Postscript code
        pdfStream = mObjectsContext->StartPDFStream(functionDict);

    } while(false);

    return pdfStream;
}

PDFHummus::EStatusCode ShadingWriter::WriteShadingType1Object(const PDFRectangle& inBounds, ObjectIDType inShadingObjectId, ObjectIDType& outFunctionObjectId) {
    ObjectIDType functionObjectId = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();

    mObjectsContext->StartNewIndirectObject(inShadingObjectId);
    DictionaryContext* shadingDict = mObjectsContext->StartDictionary();
    shadingDict->WriteKey("ShadingType");
    shadingDict->WriteIntegerValue(1);
    shadingDict->WriteKey("Domain");
    mObjectsContext->StartArray();
    mObjectsContext->WriteDouble(inBounds.LowerLeftX);
    mObjectsContext->WriteDouble(inBounds.UpperRightX);
    mObjectsContext->WriteDouble(inBounds.LowerLeftY);
    mObjectsContext->WriteDouble(inBounds.UpperRightY);
    mObjectsContext->EndArray(eTokenSeparatorEndLine);    
    shadingDict->WriteKey("ColorSpace");
    shadingDict->WriteNameValue("DeviceRGB");
    shadingDict->WriteKey("Function");
    shadingDict->WriteNewObjectReferenceValue(functionObjectId);
    EStatusCode status = mObjectsContext->EndDictionary(shadingDict);
    if(eSuccess == status)
        mObjectsContext->EndIndirectObject();

    outFunctionObjectId = functionObjectId;

    return status;
}

void ShadingWriter::WriteGradientCustomFunctionProgram(
    const RadialShading& inRadialShading, 
    IByteWriter* inWriter,
    PrimitiveObjectsWriter* inPrimitiveWriter) {
    double dx = inRadialShading.x1-inRadialShading.x0;
    double dy = inRadialShading.y1-inRadialShading.y0;
    double dr = inRadialShading.r1-inRadialShading.r0;
    double a = dx * dx + dy * dy - dr * dr;

    WriteStreamText(inWriter,"{2 copy ");
    inPrimitiveWriter->WriteDouble(dy);
    WriteStreamText(inWriter,"mul exch ");
    inPrimitiveWriter->WriteDouble(dx);
    WriteStreamText(inWriter,"mul add ");
    inPrimitiveWriter->WriteDouble(inRadialShading.r0*dr);        
    WriteStreamText(inWriter," add -2 mul dup dup mul\n4 2 roll dup mul exch dup mul add ");
    inPrimitiveWriter->WriteDouble(inRadialShading.r0*inRadialShading.r0);        
    WriteStreamText(inWriter,"sub dup 4 1 roll\n");

    if(a == 0) {
        WriteStreamText(inWriter,"pop pop div neg dup ");
        inPrimitiveWriter->WriteDouble(dr);
        WriteStreamText(inWriter,"mul ");
        inPrimitiveWriter->WriteDouble(inRadialShading.r0);
        WriteStreamText(inWriter,"add\n0 lt {pop false} {true} ifelse\n");
    } else {
        inPrimitiveWriter->WriteDouble(a*4);  
        WriteStreamText(inWriter,"mul sub dup\n0 ge {\nsqrt exch dup 0 lt {exch -1 mul} if add -0.5 mul dup\n");
        inPrimitiveWriter->WriteDouble(a);  
        WriteStreamText(inWriter,"div\n3 1 roll div\n2 copy gt {exch} if\ndup ");
        inPrimitiveWriter->WriteDouble(dr);  
        WriteStreamText(inWriter,"mul ");
        inPrimitiveWriter->WriteDouble(inRadialShading.r0);
        WriteStreamText(inWriter,"add\n 0 gt {exch pop true}\n{pop dup\n");
        inPrimitiveWriter->WriteDouble(dr);  
        WriteStreamText(inWriter,"mul ");
        inPrimitiveWriter->WriteDouble(inRadialShading.r0);
        WriteStreamText(inWriter,"add\n");
        WriteStreamText(inWriter,"0 le {pop false} {true} ifelse\n} ifelse\n} {pop pop pop false} ifelse\n");
    }

    WriteStreamText(inWriter,"{");
        
    // write the rest of the steps using shared code
    WriteColorLineStepsProgram(inWriter,inPrimitiveWriter,inRadialShading.gradientExtend, inRadialShading.colorLine);

    WriteStreamText(inWriter,"} {0 0 0} ifelse}");
}

void ShadingWriter::WriteStreamText(IByteWriter* inWriter, const std::string& inText) {
    inWriter->Write((const Byte*)(inText.c_str()),inText.size());
}

void ShadingWriter::WriteColorLineStepsProgram(
    IByteWriter* inWriter,
    PrimitiveObjectsWriter* inPrimitiveWriter,
    FT_PaintExtend inGradientExtend,
    const InterpretedGradientStopList& inColorLine) {

    // function start, setup header per extend policy
    if(inGradientExtend == FT_COLR_PAINT_EXTEND_REPEAT)
        WriteStreamText(inWriter,"dup truncate sub\ndup 0 le {1 add} if\n");
    else if(inGradientExtend == FT_COLR_PAINT_EXTEND_REFLECT)
        WriteStreamText(inWriter,"abs dup truncate dup cvi 2 mod 0 gt 3 1 roll sub exch {1 exch sub} if\n");
    
    double firstStop = inColorLine.front().stopOffset;
    double lastStop = inColorLine.back().stopOffset;

    // padding unto first stop
    WriteStreamText(inWriter,"dup ");
    inPrimitiveWriter->WriteDouble(firstStop);
    WriteStreamText(inWriter,"le {pop ");
    inPrimitiveWriter->WriteDouble(inColorLine.front().color.red / 255.0);
    inPrimitiveWriter->WriteDouble(inColorLine.front().color.green / 255.0);
    inPrimitiveWriter->WriteDouble(inColorLine.front().color.blue / 255.0);
    WriteStreamText(inWriter,"0} if\n");        

    // wrapper for in between stops code
    WriteStreamText(inWriter,"dup dup ");
    inPrimitiveWriter->WriteDouble(firstStop);
    WriteStreamText(inWriter,"gt exch ");
    inPrimitiveWriter->WriteDouble(lastStop);
    WriteStreamText(inWriter,"le and ");

    // k. interim stops, lets prep those cursers
    InterpretedGradientStopList::const_iterator itStartRange = inColorLine.begin();
    InterpretedGradientStopList::const_iterator itEndRange = inColorLine.begin();
    ++itEndRange;

    // k. write stops, and interpolate.
    for(; itEndRange != inColorLine.end(); ++itStartRange,++itEndRange) {
        if(itStartRange->stopOffset > firstStop) {
            // all but first, close previous if of ifelse statement
            WriteStreamText(inWriter,"}");
        }
        WriteStreamText(inWriter,"{\n");
        if(itEndRange->stopOffset < lastStop) {
            // all but last check value (last will enjoy the last else of ifelse)
            WriteStreamText(inWriter,"dup ");
            inPrimitiveWriter->WriteDouble(itEndRange->stopOffset);
            WriteStreamText(inWriter,"le ");
            WriteStreamText(inWriter,"{\n");
        }
        inPrimitiveWriter->WriteDouble(itStartRange->stopOffset);
        WriteStreamText(inWriter,"sub ");

        // if all colors are the same, just write as is

        // otherwise, interpolate
        if(itStartRange->color.red == itEndRange->color.red &&
            itStartRange->color.green == itEndRange->color.green &&
            itStartRange->color.blue == itEndRange->color.blue) {

            WriteStreamText(inWriter,"pop ");
            inPrimitiveWriter->WriteDouble(itEndRange->color.red / 255.0);
            inPrimitiveWriter->WriteDouble(itEndRange->color.green / 255.0);
            inPrimitiveWriter->WriteDouble(itEndRange->color.blue / 255.0);
        } else {
            double rangeDiff = itEndRange->stopOffset - itStartRange->stopOffset;
            WriteColorInterpolation(inWriter, inPrimitiveWriter, itStartRange->color.red, itEndRange->color.red, rangeDiff);
            WriteColorInterpolation(inWriter, inPrimitiveWriter, itStartRange->color.green, itEndRange->color.green, rangeDiff);
            WriteColorInterpolation(inWriter, inPrimitiveWriter, itStartRange->color.blue, itEndRange->color.blue, rangeDiff);
            WriteStreamText(inWriter,"pop ");
        }
    }

    // "ifselse" (basically size -2, cause only all but first, and then always dual)
    for(int i=0;i<inColorLine.size()-2;++i) {
        WriteStreamText(inWriter,"} ifelse\n");
    }

    WriteStreamText(inWriter,"0} if\n");

    // padding after last stop
    WriteStreamText(inWriter,"0 gt {");
    inPrimitiveWriter->WriteDouble(inColorLine.back().color.red / 255.0);
    inPrimitiveWriter->WriteDouble(inColorLine.back().color.green / 255.0);
    inPrimitiveWriter->WriteDouble(inColorLine.back().color.blue / 255.0);
    WriteStreamText(inWriter,"} if\n");

}

void ShadingWriter::WriteColorInterpolation(
    IByteWriter* inWriter,
    PrimitiveObjectsWriter* inPrimitiveWriter,
    FT_Byte inColorStart, 
    FT_Byte inColorEnd, 
    double inStopDiff) {
    if(inColorEnd == inColorStart) {
        // same color...so just put it there
        inPrimitiveWriter->WriteDouble(inColorEnd/255.0);
    } else {
        // interpolate
        WriteStreamText(inWriter, "dup ");
        inPrimitiveWriter->WriteDouble((inColorEnd - inColorStart)/(inStopDiff*255.0));
        WriteStreamText(inWriter, "mul ");
        inPrimitiveWriter->WriteDouble(inColorStart/255.0);
        WriteStreamText(inWriter, "add ");
    }
    WriteStreamText(inWriter, "exch ");
}

EStatusCode ShadingWriter::WriteLinearShadingPatternObject(
    double inX0,
    double inY0,
    double inX1,
    double inY1,
    const InterpretedGradientStopList& inColorLine,
    FT_PaintExtend inGradientExtend,
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId
) {
    LinearShading linearParams = {
        inX0,
        inY0,
        inX1,
        inY1,
        inColorLine,
        inGradientExtend
    };

    if(ColorLineHasTransparency(inColorLine))
        return WriteRGBATiledPatternObject(linearParams, inBounds, inMatrix, inPatternObjectId);
    else
        return WriteRGBShadingPatternObject(linearParams, inBounds, inMatrix, inPatternObjectId);

}

EStatusCode ShadingWriter::WriteRGBATiledPatternObject(
    const LinearShading& inLinearShading, 
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId
) {
    EStatusCode status = eSuccess;

    do {
        ObjectIDType alphaShadingPatternObjectId;
        ObjectIDType rgbShadingPatternObjectId;

        status = WriteSoftmaskPatternObjectForRGBAShades(inBounds, inMatrix, inPatternObjectId, alphaShadingPatternObjectId, rgbShadingPatternObjectId);
        if(status != eSuccess) {
            break;
        }

        // now, draw the alpha shading, which has the same pysical properties of the rgb one, but colors are all the alpha
        // component transitioning as a gradient
        LinearShading alphaLinearShading = inLinearShading;
        alphaLinearShading.colorLine = CreateAlphaShadingColorLine(inLinearShading.colorLine);

        status = WriteRGBShadingPatternObject(alphaLinearShading, inBounds, inMatrix, alphaShadingPatternObjectId);
        if(status != eSuccess)
            break;

        status = WriteRGBShadingPatternObject(inLinearShading, inBounds, inMatrix, rgbShadingPatternObjectId);

    } while(false);


    return status;    
}

EStatusCode ShadingWriter::WriteRGBShadingPatternObject(
    const LinearShading& inLinearShading, 
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId
) {
    bool usePDFNativeFunctions = inLinearShading.gradientExtend == FT_COLR_PAINT_EXTEND_PAD;

    if(usePDFNativeFunctions) {
        return WriteShadingPatternObjectWithPDFNativeFunctions(
            inLinearShading, 
            inBounds,
            inMatrix,
            inPatternObjectId
        );
    } else {
        return WriteShadingPatternObjectWithPDFCustomFunctions(
            inLinearShading, 
            inBounds,
            inMatrix,
            inPatternObjectId
        );
    }
    
}

EStatusCode ShadingWriter::WriteShadingPatternObjectWithPDFNativeFunctions(
    const LinearShading& inLinearShading, 
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId
) {
    double coords[] = {inLinearShading.x0, inLinearShading.y0,
                       inLinearShading.x1, inLinearShading.y1};

    return WriteShadingPatternObjectWithPDFNativeFunctions(
        2,
        coords,
        4,
        inLinearShading.colorLine,
        inBounds,
        inMatrix,
        inPatternObjectId
    );
}

EStatusCode ShadingWriter::WriteShadingPatternObjectWithPDFCustomFunctions(
    const LinearShading& inLinearShading, 
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId
) {
    EStatusCode status = eSuccess;
    PDFStream* pdfStream = NULL;

    // apply transform matrix, to map pattern based on function range
    double p0p1[2] = {inLinearShading.x1-inLinearShading.x0,inLinearShading.y1-inLinearShading.y0};
    double magP0P1 = sqrt(pow(p0p1[0],2) + pow(p0p1[1],2));
    double p0p1Unit[2] = {magP0P1 == 0 ? 0:p0p1[0]/magP0P1, magP0P1 == 0 ? 0:p0p1[1]/magP0P1};
    PDFMatrix patternProgramMatrix(p0p1Unit[0],p0p1Unit[1],-p0p1Unit[1],p0p1Unit[0],0,0);
    patternProgramMatrix = patternProgramMatrix.Multiply(PDFMatrix(magP0P1,0,0,magP0P1,0,0));
    patternProgramMatrix.e+=inLinearShading.x0;
    patternProgramMatrix.f+=inLinearShading.y0;

    do {
        pdfStream = StartCustomFunctionShadingPatternStream(patternProgramMatrix, inBounds, inMatrix, inPatternObjectId);
        if(!pdfStream) {
            status = eFailure;
            break;
        }

        IByteWriter* writeStream = pdfStream->GetWriteStream();
        PrimitiveObjectsWriter primitiveWriter(writeStream);
        WriteGradientCustomFunctionProgram(inLinearShading, writeStream, &primitiveWriter);
        mObjectsContext->EndPDFStream(pdfStream);
    } while(false);

    delete pdfStream;

    return status;
}

void ShadingWriter::WriteGradientCustomFunctionProgram(
    const LinearShading& inLinearShading, 
    IByteWriter* inWriter,
    PrimitiveObjectsWriter* inPrimitiveWriter) {

    WriteStreamText(inWriter, "{pop\n");

    // write the rest of the steps using shared code
    WriteColorLineStepsProgram(inWriter,inPrimitiveWriter,inLinearShading.gradientExtend, inLinearShading.colorLine);

    // function end
    WriteStreamText(inWriter,  "}");   
}

EStatusCode ShadingWriter::WriteSweepShadingPatternObject(
    double inCX,
    double inCY,
    double inStartAngleRad,
    double inEndAngleRad,
    const InterpretedGradientStopList& inColorLine,
    FT_PaintExtend inGradientExtend,
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId) {

    // Note: at this point start and end angles are ignored. 0..360 is the range used at all times.
    // As a side effect this also means there's no support for extend, as the full range is always implemented.
    SweepShading sweepParams = {
        inCX,
        inCY,
        inStartAngleRad,
        inEndAngleRad,
        inColorLine,
        inGradientExtend
    };

    if(ColorLineHasTransparency(inColorLine))
        return WriteRGBATiledPatternObject(sweepParams, inBounds, inMatrix, inPatternObjectId);
    else
        return WriteRGBShadingPatternObject(sweepParams, inBounds, inMatrix, inPatternObjectId);

}
EStatusCode ShadingWriter::WriteRGBATiledPatternObject(
    const SweepShading& inSweepShading, 
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId) {
    EStatusCode status = eSuccess;

    do {
        ObjectIDType alphaShadingPatternObjectId;
        ObjectIDType rgbShadingPatternObjectId;

        status = WriteSoftmaskPatternObjectForRGBAShades(inBounds, inMatrix, inPatternObjectId, alphaShadingPatternObjectId, rgbShadingPatternObjectId);
        if(status != eSuccess) {
            break;
        }

        // now, draw the alpha shading, which has the same pysical properties of the rgb one, but colors are all the alpha
        // component transitioning as a gradient
        SweepShading alphaSweepShading = inSweepShading;
        alphaSweepShading.colorLine = CreateAlphaShadingColorLine(inSweepShading.colorLine);

        status = WriteRGBShadingPatternObject(alphaSweepShading, inBounds, inMatrix, alphaShadingPatternObjectId);
        if(status != eSuccess)
            break;

        status = WriteRGBShadingPatternObject(inSweepShading, inBounds, inMatrix, rgbShadingPatternObjectId);

    } while(false);


    return status;    

}

EStatusCode ShadingWriter::WriteRGBShadingPatternObject(
    const SweepShading& inSweepShading, 
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId) {
        // for sweep there's no good native PDF parallel, so always use a custom function

    return WriteShadingPatternObjectWithPDFCustomFunctions(
        inSweepShading, 
        inBounds,
        inMatrix,
        inPatternObjectId
    );
}

EStatusCode ShadingWriter::WriteShadingPatternObjectWithPDFCustomFunctions(
    const SweepShading& inSweepShading, 
    const PDFRectangle& inBounds,
    const PDFMatrix& inMatrix,
    ObjectIDType inPatternObjectId) {
    EStatusCode status = eSuccess;
    PDFStream* pdfStream = NULL;

    // apply transform matrix, to map pattern based on function range
    PDFMatrix patternProgramMatrix = PDFMatrix(1,0,0,1,inSweepShading.cX,inSweepShading.cY);

    do {
        pdfStream = StartCustomFunctionShadingPatternStream(patternProgramMatrix, inBounds, inMatrix, inPatternObjectId);
        if(!pdfStream) {
            status = eFailure;
            break;
        }

        IByteWriter* writeStream = pdfStream->GetWriteStream();
        PrimitiveObjectsWriter primitiveWriter(writeStream);
        WriteGradientCustomFunctionProgram(inSweepShading, writeStream, &primitiveWriter);
        mObjectsContext->EndPDFStream(pdfStream);
    } while(false);

    delete pdfStream;

    return status;
}   

void ShadingWriter::WriteGradientCustomFunctionProgram(
    const SweepShading& inSweepShading, 
    IByteWriter* inWriter,
    PrimitiveObjectsWriter* inPrimitiveWriter) {

    WriteStreamText(inWriter, "{exch atan 360 div\n");

    // write the rest of the steps using shared code
    WriteColorLineStepsProgram(inWriter,inPrimitiveWriter,inSweepShading.gradientExtend, inSweepShading.colorLine);

    // function end
    WriteStreamText(inWriter,  "}");          

}