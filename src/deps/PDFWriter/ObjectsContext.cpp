/*
   Source File : ObjectsContext.cpp


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
#include "ObjectsContext.h"
#include "IOBasicTypes.h"
#include "IByteWriterWithPosition.h"
#include "SafeBufferMacrosDefs.h"
#include "DictionaryContext.h"
#include "Trace.h"
#include "OutputStreamTraits.h"
#include "PDFStream.h"
#include "PDFParser.h"
#include "PDFObjectCast.h"
#include "PDFDictionary.h"
#include "PDFIndirectObjectReference.h"
#include "PDFBoolean.h"
#include "PDFLiteralString.h"

using namespace PDFHummus;

ObjectsContext::ObjectsContext(void)
{
	mOutputStream = NULL;
	mCompressStreams = true;
	mExtender = NULL;
}

ObjectsContext::~ObjectsContext(void)
{
}




void ObjectsContext::SetOutputStream(IByteWriterWithPosition* inOutputStream)
{
	mOutputStream = inOutputStream;
	mPrimitiveWriter.SetStreamForWriting(inOutputStream);
}

static const IOBasicTypes::Byte scComment[1] = {'%'};
void ObjectsContext::WriteComment(const std::string& inCommentText)
{
	mOutputStream->Write(scComment,1);
	mOutputStream->Write((const IOBasicTypes::Byte *)inCommentText.c_str(),inCommentText.size());
	EndLine();
}

void ObjectsContext::WriteName(const std::string& inName,ETokenSeparator inSeparate)
{
	mPrimitiveWriter.WriteName(inName,inSeparate);
}

void ObjectsContext::WriteLiteralString(const std::string& inString,ETokenSeparator inSeparate)
{
	mPrimitiveWriter.WriteLiteralString(inString,inSeparate);
}

void ObjectsContext::WriteHexString(const std::string& inString,ETokenSeparator inSeparate)
{
	mPrimitiveWriter.WriteHexString(inString,inSeparate);
}

void ObjectsContext::WriteIndirectObjectReference(const ObjectReference& inObjectReference,ETokenSeparator inSeparate)
{
    WriteIndirectObjectReference(inObjectReference.ObjectID,inObjectReference.GenerationNumber,inSeparate);
}

void ObjectsContext::WriteNewIndirectObjectReference(ObjectIDType indirectObjectID,ETokenSeparator inSeparate)
{
    WriteIndirectObjectReference(indirectObjectID,0,inSeparate);
}

static const IOBasicTypes::Byte scR[1] = {'R'};
void ObjectsContext::WriteIndirectObjectReference(ObjectIDType inIndirectObjectID,unsigned long inGenerationNumber,ETokenSeparator inSeparate)
{
	mPrimitiveWriter.WriteInteger(inIndirectObjectID);
	mPrimitiveWriter.WriteInteger(inGenerationNumber);
	mOutputStream->Write(scR,1);
	mPrimitiveWriter.WriteTokenSeparator(inSeparate);
}

IByteWriterWithPosition* ObjectsContext::StartFreeContext()
{
	return mOutputStream;
}

void ObjectsContext::EndFreeContext()
{
	// currently just a marker, do nothing. allegedly used to return to a "controlled" context
}

static const IOBasicTypes::Byte scXref[] = {'x','r','e','f'};

EStatusCode ObjectsContext::WriteXrefTable(LongFilePositionType& outWritePosition)
{
	EStatusCode status = PDFHummus::eSuccess;
	outWritePosition = mOutputStream->GetCurrentPosition();
	
	// write xref keyword
	mOutputStream->Write(scXref,4);
	mPrimitiveWriter.EndLine();

    
    ObjectIDType startID = 0;
    ObjectIDType firstIDNotInRange;
    ObjectIDType nextFreeObject = 0;
    
    // write subsections
    while((startID < mReferencesRegistry.GetObjectsCount()) && (PDFHummus::eSuccess == status))
    {
        firstIDNotInRange = startID;
        
        // look for first ID that does not require update [for first version of PDF...it will be the end]
        while(firstIDNotInRange < mReferencesRegistry.GetObjectsCount() &&
                mReferencesRegistry.GetNthObjectReference(firstIDNotInRange).mIsDirty)
            ++firstIDNotInRange;
        
    
        // write section header
        mPrimitiveWriter.WriteInteger(startID);
        mPrimitiveWriter.WriteInteger(firstIDNotInRange - startID,eTokenSeparatorEndLine);
        
        // write used/free objects
        char entryBuffer[21];
        
        for(ObjectIDType i = startID; i < firstIDNotInRange && (PDFHummus::eSuccess == status);++i)
        {
            const ObjectWriteInformation& objectReference = mReferencesRegistry.GetNthObjectReference(i);
            if(objectReference.mObjectReferenceType == ObjectWriteInformation::Used)
            {
                // used object
                
                if(objectReference.mObjectWritten)
                {
                    SAFE_SPRINTF_2(entryBuffer,21,"%010lld %05ld n\r\n",objectReference.mWritePosition,objectReference.mGenerationNumber);
                    mOutputStream->Write((const IOBasicTypes::Byte *)entryBuffer,20);
                }
                else
                {
                    // object not written. at this point this should not happen, and indicates a failure
                    status = PDFHummus::eFailure;
                    TRACE_LOG1("ObjectsContext::WriteXrefTable, Unexpected Failure. Object of ID = %ld was not registered as written. probably means it was not written",i);
                }
            }
            else 
            {
                // free object
                
                ++nextFreeObject;
                // look for next dirty & free object, to be the next item of linked list
                while(nextFreeObject < mReferencesRegistry.GetObjectsCount() &&
                      (!mReferencesRegistry.GetNthObjectReference(nextFreeObject).mIsDirty ||
                      mReferencesRegistry.GetNthObjectReference(nextFreeObject).mObjectReferenceType != ObjectWriteInformation::Free))
                    ++nextFreeObject;
                
                // if reached end of list, then link back to head - 0
                if(nextFreeObject == mReferencesRegistry.GetObjectsCount())
                    nextFreeObject = 0;

                SAFE_SPRINTF_2(entryBuffer,21,"%010ld %05ld f\r\n",nextFreeObject,objectReference.mGenerationNumber);
                mOutputStream->Write((const IOBasicTypes::Byte *)entryBuffer,20);
                
            }
        }
        
        if(status != PDFHummus::eSuccess)
            break;
        
        startID = firstIDNotInRange;
        
        // now promote startID to the next object to update
        while(startID < mReferencesRegistry.GetObjectsCount() &&
              !mReferencesRegistry.GetNthObjectReference(startID).mIsDirty)
            ++startID;        
    }
    

	return status;
}

DictionaryContext* ObjectsContext::StartDictionary()
{
	DictionaryContext* newDictionary = new DictionaryContext(this,mDictionaryStack.size());

	mDictionaryStack.push_back(newDictionary);
	return newDictionary;
}

EStatusCode ObjectsContext::EndDictionary(DictionaryContext* ObjectsContext)
{
	if(mDictionaryStack.size() > 0)
	{
		if(mDictionaryStack.back() == ObjectsContext)
		{
			delete mDictionaryStack.back();
			mDictionaryStack.pop_back();
			return PDFHummus::eSuccess;
		}
		else
		{
			TRACE_LOG("ObjectsContext::EndDictionary, nesting violation. Trying to close a dictionary while one of it's children is still open. First End the children");
			return PDFHummus::eFailure;
		}
	}
	else
	{
		TRACE_LOG("ObjectsContext::EndDictionary, stack underflow. Trying to end a dictionary when there's no open dictionaries");
		return PDFHummus::eFailure;
	}
}

IndirectObjectsReferenceRegistry& ObjectsContext::GetInDirectObjectsRegistry()
{
	return mReferencesRegistry;
}


void ObjectsContext::EndLine()
{
	mPrimitiveWriter.EndLine();
}

void ObjectsContext::WriteTokenSeparator(ETokenSeparator inSeparate)
{
	mPrimitiveWriter.WriteTokenSeparator(inSeparate);
}

void ObjectsContext::WriteKeyword(const std::string& inKeyword)
{
	mPrimitiveWriter.WriteKeyword(inKeyword);
}

void ObjectsContext::WriteInteger(long long inIntegerToken,ETokenSeparator inSeparate)
{
	mPrimitiveWriter.WriteInteger(inIntegerToken,inSeparate);
}

void ObjectsContext::WriteDouble(double inDoubleToken,ETokenSeparator inSeparate)
{
	mPrimitiveWriter.WriteDouble(inDoubleToken,inSeparate);
}

void ObjectsContext::WriteBoolean(bool inBooleanToken,ETokenSeparator inSeparate)
{
	mPrimitiveWriter.WriteBoolean(inBooleanToken,inSeparate);
}

void ObjectsContext::WriteNull(ETokenSeparator inSeparate)
{
	mPrimitiveWriter.WriteNull(inSeparate);
}

static const std::string scObj = "obj";
ObjectIDType ObjectsContext::StartNewIndirectObject()
{
	ObjectIDType newObjectID = mReferencesRegistry.AllocateNewObjectID();
	mReferencesRegistry.MarkObjectAsWritten(newObjectID,mOutputStream->GetCurrentPosition());
	mPrimitiveWriter.WriteInteger(newObjectID);
	mPrimitiveWriter.WriteInteger(0);
	mPrimitiveWriter.WriteKeyword(scObj);
	return newObjectID;
}

void ObjectsContext::StartNewIndirectObject(ObjectIDType inObjectID)
{
	mReferencesRegistry.MarkObjectAsWritten(inObjectID,mOutputStream->GetCurrentPosition());
	mPrimitiveWriter.WriteInteger(inObjectID);
	mPrimitiveWriter.WriteInteger(0);
	mPrimitiveWriter.WriteKeyword(scObj);
}

void ObjectsContext::StartModifiedIndirectObject(ObjectIDType inObjectID)
{
	mReferencesRegistry.MarkObjectAsUpdated(inObjectID,mOutputStream->GetCurrentPosition());
	mPrimitiveWriter.WriteInteger(inObjectID);
	mPrimitiveWriter.WriteInteger(0);
	mPrimitiveWriter.WriteKeyword(scObj);    
}

static const std::string scEndObj = "endobj";
void ObjectsContext::EndIndirectObject()
{
	mPrimitiveWriter.WriteKeyword(scEndObj);
}

void ObjectsContext::StartArray()
{
	mPrimitiveWriter.StartArray();
}

void ObjectsContext::EndArray(ETokenSeparator inSeparate)
{
	mPrimitiveWriter.EndArray(inSeparate);
}

void ObjectsContext::SetCompressStreams(bool inCompressStreams)
{
	mCompressStreams = inCompressStreams;
}

static const std::string scLength = "Length";
static const std::string scStream = "stream";
static const std::string scEndStream = "endstream";
static const std::string scFilter = "Filter";
static const std::string scFlateDecode = "FlateDecode";

PDFStream* ObjectsContext::StartPDFStream(DictionaryContext* inStreamDictionary,bool inForceDirectExtentObject)
{
	// write stream header and allocate PDF stream.
	// PDF stream will take care of maintaining state for the stream till writing is finished

	// Write the stream header
	// Write Stream Dictionary (note that inStreamDictionary is optionally used)
	DictionaryContext* streamDictionaryContext = (NULL == inStreamDictionary ? StartDictionary() : inStreamDictionary);

	// Compression (if necessary)
	if(mCompressStreams)
	{
		streamDictionaryContext->WriteKey(scFilter);
		streamDictionaryContext->WriteNameValue(scFlateDecode);
	}

    if(!inForceDirectExtentObject)
    {
    
        // Length (write as an indirect object)
        streamDictionaryContext->WriteKey(scLength);
        ObjectIDType lengthObjectID = mReferencesRegistry.AllocateNewObjectID();
        streamDictionaryContext->WriteNewObjectReferenceValue(lengthObjectID);
            

        EndDictionary(streamDictionaryContext);

        // Write Stream Content
        WriteKeyword(scStream);
        
        return new PDFStream(mCompressStreams,mOutputStream,lengthObjectID,mExtender);
    }
    else
        return new PDFStream(mCompressStreams,mOutputStream,streamDictionaryContext,mExtender);
	
}

PDFStream* ObjectsContext::StartUnfilteredPDFStream(DictionaryContext* inStreamDictionary)
{
	// write stream header and allocate PDF stream.
	// PDF stream will take care of maintaining state for the stream till writing is finished

	// Write the stream header
	// Write Stream Dictionary (note that inStreamDictionary is optionally used)
	DictionaryContext* streamDictionaryContext = (NULL == inStreamDictionary ? StartDictionary() : inStreamDictionary);

	// Length (write as an indirect object)
	streamDictionaryContext->WriteKey(scLength);
	ObjectIDType lengthObjectID = mReferencesRegistry.AllocateNewObjectID();
	streamDictionaryContext->WriteNewObjectReferenceValue(lengthObjectID);
		
	EndDictionary(streamDictionaryContext);

	// Write Stream Content
	WriteKeyword(scStream);

	// now begin the stream itself
	return new PDFStream(false,mOutputStream,lengthObjectID,NULL);
}

void ObjectsContext::EndPDFStream(PDFStream* inStream)
{
	// finalize the stream write to end stream context and calculate length
	inStream->FinalizeStreamWrite();

	if(inStream->GetExtentObjectID() == 0)
    {
        DictionaryContext* streamDictionaryContext = inStream->GetStreamDictionaryForDirectExtentStream();
        
        // Length (write as a direct object)
        streamDictionaryContext->WriteKey(scLength);
        streamDictionaryContext->WriteIntegerValue(inStream->GetLength());
        
        
        EndDictionary(streamDictionaryContext);
        
        // Write Stream Content
        WriteKeyword(scStream);
        
        inStream->FlushStreamContentForDirectExtentStream();
        
        EndLine();
		WriteKeyword(scEndStream);
        EndIndirectObject();
        
    }
    else
    {
        WritePDFStreamEndWithoutExtent();
        EndIndirectObject();
        WritePDFStreamExtent(inStream);
    }
}
 
	
void ObjectsContext::WritePDFStreamEndWithoutExtent()
{
		EndLine(); // this one just to make sure
		WriteKeyword(scEndStream);
}

void ObjectsContext::WritePDFStreamExtent(PDFStream* inStream)
{
	StartNewIndirectObject(inStream->GetExtentObjectID());
	WriteInteger(inStream->GetLength(),eTokenSeparatorEndLine);
	EndIndirectObject();
}

void ObjectsContext::SetObjectsContextExtender(IObjectsContextExtender* inExtender)
{
	mExtender = inExtender;
}

std::string ObjectsContext::GenerateSubsetFontPrefix()
{
	return mSubsetFontsNamesSequance.GetNextValue();
}

EStatusCode ObjectsContext::WriteState(ObjectsContext* inStateWriter,ObjectIDType inObjectID)
{
	EStatusCode status;
		
	do
	{
		inStateWriter->StartNewIndirectObject(inObjectID);

		ObjectIDType referencesRegistryObjectID = inStateWriter->GetInDirectObjectsRegistry().AllocateNewObjectID();
		ObjectIDType subsetFontsNameSequanceID = inStateWriter->GetInDirectObjectsRegistry().AllocateNewObjectID();

		DictionaryContext* objectsContextDict = inStateWriter->StartDictionary();

		objectsContextDict->WriteKey("Type");
		objectsContextDict->WriteNameValue("ObjectsContext");

		objectsContextDict->WriteKey("mReferencesRegistry");
		objectsContextDict->WriteNewObjectReferenceValue(referencesRegistryObjectID);

		objectsContextDict->WriteKey("mCompressStreams");
		objectsContextDict->WriteBooleanValue(mCompressStreams);

		objectsContextDict->WriteKey("mSubsetFontsNamesSequance");
		objectsContextDict->WriteNewObjectReferenceValue(subsetFontsNameSequanceID);

		inStateWriter->EndDictionary(objectsContextDict);

		inStateWriter->EndIndirectObject();

		status = mReferencesRegistry.WriteState(inStateWriter,referencesRegistryObjectID);
		if(status != PDFHummus::eSuccess)
			break;

		// write subset fonts names sequance
		inStateWriter->StartNewIndirectObject(subsetFontsNameSequanceID);

		DictionaryContext* sequanceDict = inStateWriter->StartDictionary();

		sequanceDict->WriteKey("Type");
		sequanceDict->WriteNameValue("UppercaseSequance");


		sequanceDict->WriteKey("mSequanceString");
		sequanceDict->WriteLiteralStringValue(mSubsetFontsNamesSequance.ToString());

		inStateWriter->EndDictionary(sequanceDict);

		inStateWriter->EndIndirectObject();		
	}while(false);

	return status;
}

EStatusCode ObjectsContext::ReadState(PDFParser* inStateReader,ObjectIDType inObjectID)
{
	PDFObjectCastPtr<PDFDictionary> objectsContext(inStateReader->ParseNewObject(inObjectID));

	PDFObjectCastPtr<PDFBoolean> compressStreams(objectsContext->QueryDirectObject("mCompressStreams"));
	mCompressStreams = compressStreams->GetValue();

	PDFObjectCastPtr<PDFDictionary> subsetFontsNamesSequance(inStateReader->QueryDictionaryObject(objectsContext.GetPtr(),"mSubsetFontsNamesSequance"));
	PDFObjectCastPtr<PDFLiteralString> sequanceString(subsetFontsNamesSequance->QueryDirectObject("mSequanceString"));
	mSubsetFontsNamesSequance.SetSequanceString(sequanceString->GetValue());

	PDFObjectCastPtr<PDFIndirectObjectReference> referencesObject(objectsContext->QueryDirectObject("mReferencesRegistry"));

	return mReferencesRegistry.ReadState(inStateReader,referencesObject->mObjectID);

}

void ObjectsContext::Cleanup()
{
	mOutputStream = NULL;
	mCompressStreams = true;
	mExtender = NULL;

	mSubsetFontsNamesSequance.Reset();
	mReferencesRegistry.Reset();
}

void ObjectsContext::SetupModifiedFile(PDFParser* inModifiedFileParser)
{
    mReferencesRegistry.SetupXrefFromModifiedFile(inModifiedFileParser);
}

EStatusCode ObjectsContext::WriteXrefStream(DictionaryContext* inDictionaryContext)
{
    // k. complement input dictionary with the relevant entries - W and Index
    // then continue with a regular stream, forced to have "length" as direct object
    
    // write Index entry
    inDictionaryContext->WriteKey("Index");
    StartArray();
    
    ObjectIDType startID = 0;
    ObjectIDType firstIDNotInRange;
 
    while(startID < mReferencesRegistry.GetObjectsCount())
    {
        firstIDNotInRange = startID;
        
        // look for first ID that does not require update [for first version of PDF...it will be the end]
        while(firstIDNotInRange < mReferencesRegistry.GetObjectsCount() &&
              mReferencesRegistry.GetNthObjectReference(firstIDNotInRange).mIsDirty)
            ++firstIDNotInRange;
        
        // write section header
        mPrimitiveWriter.WriteInteger(startID);
        mPrimitiveWriter.WriteInteger(firstIDNotInRange - startID);
        
        startID = firstIDNotInRange;
        
        // now promote startID to the next object to update
        while(startID < mReferencesRegistry.GetObjectsCount() &&
              !mReferencesRegistry.GetNthObjectReference(startID).mIsDirty)
            ++startID;        
    }
    
    EndArray();
    EndLine();
    
    // write W entry, which is going to be 1 sizeof(long long) and sizeof(unsigned long), per the types i'm using
    
    size_t typeSize = 1;
    size_t locationSize = sizeof(LongFilePositionType);
    size_t generationSize = sizeof(unsigned long);
    
    inDictionaryContext->WriteKey("W");
    StartArray();
    WriteInteger(typeSize);
    WriteInteger(locationSize);
    WriteInteger(generationSize);
    EndArray();
    EndLine();
    
    // start the xref stream itself
    PDFStream* aStream = StartPDFStream(inDictionaryContext,true);
    
    // now write the table data itself
    EStatusCode status = eSuccess;
    ObjectIDType nextFreeObject = 0;
    
    do {
    
        for(ObjectIDType i = 0; i < mReferencesRegistry.GetObjectsCount() && eSuccess == status;++i)
        {
            if(!mReferencesRegistry.GetNthObjectReference(i).mIsDirty)
                continue;
     
            const ObjectWriteInformation& objectReference = mReferencesRegistry.GetNthObjectReference(i);

            if(objectReference.mObjectReferenceType == ObjectWriteInformation::Used)
            {
                // used object
                
                if(objectReference.mObjectWritten)
                {
                    WriteXrefNumber(aStream->GetWriteStream(),1,typeSize);
                    WriteXrefNumber(aStream->GetWriteStream(),objectReference.mWritePosition,locationSize);
                    WriteXrefNumber(aStream->GetWriteStream(),objectReference.mGenerationNumber,generationSize);
                }
                else
                {
                    // object not written. at this point this should not happen, and indicates a failure
                    status = PDFHummus::eFailure;
                    TRACE_LOG1("ObjectsContext::WriteXrefStream, Unexpected Failure. Object of ID = %ld was not registered as written. probably means it was not written",i);
                }
            }
            else 
            {
                // free object
                
                ++nextFreeObject;
                // look for next dirty & free object, to be the next item of linked list
                while(nextFreeObject < mReferencesRegistry.GetObjectsCount() &&
                      (!mReferencesRegistry.GetNthObjectReference(nextFreeObject).mIsDirty ||
                       mReferencesRegistry.GetNthObjectReference(nextFreeObject).mObjectReferenceType != ObjectWriteInformation::Free))
                    ++nextFreeObject;
                
                // if reached end of list, then link back to head - 0
                if(nextFreeObject == mReferencesRegistry.GetObjectsCount())
                    nextFreeObject = 0;
     
                WriteXrefNumber(aStream->GetWriteStream(),0,typeSize);
                WriteXrefNumber(aStream->GetWriteStream(),nextFreeObject,locationSize);
                WriteXrefNumber(aStream->GetWriteStream(),objectReference.mGenerationNumber,generationSize);
                
            }

        }
        
        if(status != eSuccess)
            break;
            
        // end the stream and g'bye
        EndPDFStream(aStream);

    } 
    while (false);

    return status;
}

void ObjectsContext::WriteXrefNumber(IByteWriter* inStream,LongFilePositionType inElement, size_t inElementSize)
{
    // xref numbers are written high order byte first (big endian)
    Byte* buffer = new Byte[inElementSize];
    
    for(size_t i = inElementSize; i>0; --i)
    {
        buffer[i-1] = (Byte)(inElement & 0xff);
        inElement = inElement >> 8;
    }
    inStream->Write(buffer,inElementSize);
	delete[] buffer;
}
