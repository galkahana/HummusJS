/*
   Source File : ExtGStateRegistry.cpp


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

#include "ExtGStateRegistry.h"
#include "ObjectsContext.h"
#include "IObjectEndWritingTask.h"
#include "IndirectObjectsReferenceRegistry.h"
#include "DictionaryContext.h"

using namespace PDFHummus;

class WriteExtGStateForOpacityWritingTask: public IObjectEndWritingTask {
public:
    WriteExtGStateForOpacityWritingTask(ObjectIDType inObjectID, double inAlphaValue) {
        mObjectID = inObjectID;
        mAlphaValue = inAlphaValue;
    }

    virtual ~WriteExtGStateForOpacityWritingTask(){}

    virtual PDFHummus::EStatusCode Write(ObjectsContext* inObjectsContext,
                                         PDFHummus::DocumentContext* inDocumentContext) {
        inObjectsContext->StartNewIndirectObject(mObjectID);
        DictionaryContext* dict = inObjectsContext->StartDictionary();
        dict->WriteKey("Type");
        dict->WriteNameValue("ExtGState");
        dict->WriteKey("ca");
        dict->WriteDoubleValue(mAlphaValue);
        dict->WriteKey("CA");
        dict->WriteDoubleValue(mAlphaValue);
        EStatusCode status = inObjectsContext->EndDictionary(dict);
        inObjectsContext->EndIndirectObject();

        return status;
    }    

private:
    ObjectIDType mObjectID;
    double mAlphaValue;
};


ExtGStateRegistry::ExtGStateRegistry() {
    mObjectsContext = NULL;
}

void ExtGStateRegistry::SetObjectsContext(ObjectsContext* inObjectsContext) {
    mObjectsContext = inObjectsContext;
}

ObjectIDTypeAndBool ExtGStateRegistry::RegisterExtGStateForOpacity(double inAlphaValue) {
    if(!mObjectsContext)
        return ObjectIDTypeAndBool(0,false);

    DoubleToObjectIDTypeMap::iterator it = mAlphaExtGstates.find(inAlphaValue);

    if(it == mAlphaExtGstates.end()) {
        ObjectIDType objectId = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
        it = mAlphaExtGstates.insert(DoubleToObjectIDTypeMap::value_type(inAlphaValue,objectId)).first;
        return ObjectIDTypeAndBool(it->second,true);
    }

    return ObjectIDTypeAndBool(it->second, false);
}

IObjectEndWritingTask* ExtGStateRegistry::CreateExtGStateForOpacityWritingTask(ObjectIDType inObjectID,double inAlphaValue) {
    return new WriteExtGStateForOpacityWritingTask(inObjectID, inAlphaValue);
}

void ExtGStateRegistry::Reset() {
    mAlphaExtGstates.clear();
}
