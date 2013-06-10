/*
 Source File : PDFPageMergingHelper.cpp
 
 
 Copyright 2012 Gal Kahana PDFWriter
 
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

#include "PDFPageMergingHelper.h"
#include "PDFPage.h"
#include "PDFDocumentCopyingContext.h"
#include "PDFWriter.h"

using namespace PDFHummus;


PDFPageMergingHelper::PDFPageMergingHelper(PDFPage* inPage)
{
    mPage = inPage;
}

PDFPageMergingHelper::~PDFPageMergingHelper(void)
{
    
}

EStatusCode PDFPageMergingHelper::MergePageContent(PDFDocumentCopyingContext* inCopyingContext,
                                                   unsigned long inPageIndex)
{
    return inCopyingContext->MergePDFPageToPage(mPage, inPageIndex);
}


EStatusCode PDFPageMergingHelper::MergePageContent(PDFWriter* inWriter,const std::string& inPDFFilePath,unsigned long inPageIndex)
{
    EStatusCode status = eSuccess;
    
    do 
    {
        PDFDocumentCopyingContext* copyingContext = inWriter->CreatePDFCopyingContext(inPDFFilePath);
        
        if(!copyingContext)
        {
            status = eFailure;
            break;
        }
        
        status = MergePageContent(copyingContext,inPageIndex);
        
    } while (false);
    
    return status;
}

EStatusCode PDFPageMergingHelper::MergePageContent(PDFWriter* inWriter,IByteReaderWithPosition* inPDFStream,unsigned long inPageIndex)
{
    EStatusCode status = eSuccess;
    
    do 
    {
        PDFDocumentCopyingContext* copyingContext = inWriter->CreatePDFCopyingContext(inPDFStream);
        
        if(!copyingContext)
        {
            status = eFailure;
            break;
        }
        
        status = MergePageContent(copyingContext,inPageIndex);
        
    } while (false);
    
    return status;    
}
