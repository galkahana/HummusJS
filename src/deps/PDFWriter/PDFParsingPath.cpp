
/*
   Source File : PDFParsingPath.cpp


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

#include "PDFParsingPath.h"
#include "Trace.h"

#include <sstream>
#include <algorithm>

using namespace std;
using namespace PDFHummus;

PDFParsingPath::PDFParsingPath() {

}


EStatusCode PDFParsingPath::EnterObject(ObjectIDType inObjectId) {
    ObjectIDTypeList::iterator it = find(mObjectsPath.begin(), mObjectsPath.end(), inObjectId);
    if(it != mObjectsPath.end()) {
        TRACE_LOG2("PDFParsingPath::EnterObject, attempting to enter object %ld, where the object already exists in the current path: %s",inObjectId,PrintPath().c_str());
        return eFailure;
    }

    mObjectsPath.push_back(inObjectId);
    return eSuccess;
}

EStatusCode PDFParsingPath::ExitObject(ObjectIDType inObjectId) {
    if(mObjectsPath.size() == 0 || mObjectsPath.back() != inObjectId) {
        TRACE_LOG2("PDFParsingPath::ExitObject, attempting to exit object %ld, where the object is NOT the last entered: %s",inObjectId,PrintPath().c_str());
        return eFailure;
    }

    mObjectsPath.pop_back();
    return eSuccess;

}

void PDFParsingPath::Reset() {
    mObjectsPath.clear();
}

std::string PDFParsingPath::PrintPath() {
	std::stringstream pathWriter;
    ObjectIDTypeList::iterator it = mObjectsPath.begin();
	
    if(it != mObjectsPath.end()) {
        pathWriter<<*it;
        ++it;
        for(; it != mObjectsPath.end(); ++it) {
            pathWriter<<", "<<*it;
        }
    }

    return pathWriter.str();
}
