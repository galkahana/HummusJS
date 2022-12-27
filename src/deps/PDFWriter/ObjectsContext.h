/*
   Source File : ObjectsContext.h


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
#include "IndirectObjectsReferenceRegistry.h"
#include "ETokenSeparator.h"
#include "PrimitiveObjectsWriter.h"
#include "UppercaseSequance.h"
#include <string>
#include <list>



class IByteWriterWithPosition;
class DictionaryContext;
class PDFStream;
class IObjectsContextExtender;
class ObjectsContext;
class PDFParser;
class EncryptionHelper;

typedef std::list<DictionaryContext*> DictionaryContextList;

class ObjectsContext
{
public:
	ObjectsContext(void);
	~ObjectsContext(void);

	// Cleanup method. called in order to reset the object context for another production
	void Cleanup();

	void SetOutputStream(IByteWriterWithPosition* inOutputStream);
	void SetEncryptionHelper(EncryptionHelper* inEncryptionHelper);
	

	// pre 1.5 xref writing
	PDFHummus::EStatusCode WriteXrefTable(LongFilePositionType& outWritePosition);
    // post 1.5 xref writing (only used now for modified files)
    PDFHummus::EStatusCode WriteXrefStream(DictionaryContext* inDictionaryContext);
    
	// Free Context, for direct writing to output stream
	IByteWriterWithPosition* StartFreeContext();
	void EndFreeContext();

	// Get current output stream position
	LongFilePositionType GetCurrentPosition();

	// Get objects management object
	IndirectObjectsReferenceRegistry& GetInDirectObjectsRegistry();
	
	// Token Writing

	void WriteTokenSeparator(ETokenSeparator inSeparate);
	void EndLine();

	// Objects Writing

	// write comment line. ends line
	void WriteComment(const std::string& inCommentText);
	// write keyword. ends line
	void WriteKeyword(const std::string& inKeyword);

	void WriteName(const std::string& inName,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteInteger(long long inIntegerToken,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteLiteralString(const std::string& inString,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteHexString(const std::string& inString,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteEncodedHexString(const std::string& inString, ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteIndirectObjectReference(ObjectIDType inIndirectObjectID,unsigned long inGenerationNumber,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteIndirectObjectReference(const ObjectReference& inObjectReference,ETokenSeparator inSeparate = eTokenSeparatorSpace);
    void WriteNewIndirectObjectReference(ObjectIDType indirectObjectID,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteDouble(double inDoubleToken,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteBoolean(bool inBooleanToken,ETokenSeparator inSeparate = eTokenSeparatorSpace);
	void WriteNull(ETokenSeparator inSeparate = eTokenSeparatorSpace);

	// Dictionary writing. StartDictionary starts a context which allows writing.
	// this context enforces some simple values
	DictionaryContext* StartDictionary();
	// ends dictionary context, releases the dictionary started in "StartDictionary", and now returned
	PDFHummus::EStatusCode EndDictionary(DictionaryContext* inDictionaryContext); 

	// Array writing, currently just writing begin and end brackets
	void StartArray();
	void EndArray(ETokenSeparator inSeparate = eTokenSeparatorNone);

	// Indirect objects writing
	// override that allocate a new object ID and returns it
	ObjectIDType StartNewIndirectObject();
	// override for objects that already have been allocated in advance, and have an object ID
	void StartNewIndirectObject(ObjectIDType inObjectID);
	void EndIndirectObject();

    // for modified files scenarios, modify an existing object
    void StartModifiedIndirectObject(ObjectIDType inObjectID);

	// Sets whether streams created by the objects context will be compressed (with flate) or not
	void SetCompressStreams(bool inCompressStreams);
	bool IsCompressingStreams();

	// Create PDF stream and write it's header. note that stream are written with indirect object for Length, to allow one pass writing.
	// inStreamDictionary can be passed in order to include stream generic information in an already written stream dictionary
	// that is type specific. [the method will take care of closing the dictionary.
	PDFStream* StartPDFStream(DictionaryContext* inStreamDictionary=NULL,bool inForceDirectExtentObject = false);
	// same as StartPDFStream but forces the stream to create an unfiltered stream
	PDFStream* StartUnfilteredPDFStream(DictionaryContext* inStreamDictionary=NULL);
	void EndPDFStream(PDFStream* inStream);

	// Extensibility
	void SetObjectsContextExtender(IObjectsContextExtender* inExtender);
	

	// as the obly common context around...i'm using the objects context to create
	// subset fonts prefixes. might want to consider a more relevant object...
	std::string GenerateSubsetFontPrefix();

    // setup for modified file workflow
    void SetupModifiedFile(PDFParser* inModifiedFileParser);

	PDFHummus::EStatusCode WriteState(ObjectsContext* inStateWriter,ObjectIDType inObjectID);
	PDFHummus::EStatusCode ReadState(PDFParser* inStateReader,ObjectIDType inObjectID);

private:
	IObjectsContextExtender* mExtender;
	IByteWriterWithPosition* mOutputStream;
	IndirectObjectsReferenceRegistry mReferencesRegistry;
	PrimitiveObjectsWriter mPrimitiveWriter;
	bool mCompressStreams;
	UppercaseSequance mSubsetFontsNamesSequance;
	EncryptionHelper* mEncryptionHelper;

	DictionaryContextList mDictionaryStack;

	void WritePDFStreamEndWithoutExtent();
	void WritePDFStreamExtent(PDFStream* inStream);
    void WriteXrefNumber(IByteWriter* inStream,LongFilePositionType inElement, size_t inElementSize);
	bool IsEncrypting();
	std::string MaybeEncryptString(const std::string& inString);
	std::string DecodeHexString(const std::string& inString);

};
