/*
   Source File : PDFParser.h


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
#include "PDFObjectParser.h"
#include "IOBasicTypes.h"
#include "ObjectsBasicTypes.h"
#include "RefCountPtr.h"
#include "PDFDictionary.h"
#include "IByteReaderWithPosition.h"
#include "AdapterIByteReaderWithPositionToIReadPositionProvider.h"
#include "DecryptionHelper.h"
#include "PDFParsingOptions.h"
#include "InputOffsetStream.h"
#include "PDFParsingPath.h"

#include <map>
#include <set>
#include <vector>
#include <utility>

class PDFArray;
class PDFStreamInput;
class PDFDictionary;
class PDFName;
class IPDFParserExtender;

typedef std::pair<PDFHummus::EStatusCode,IByteReader*> EStatusCodeAndIByteReader;

#define LINE_BUFFER_SIZE 1024

enum EXrefEntryType
{
	eXrefEntryExisting,
	eXrefEntryDelete,
	eXrefEntryStreamObject,
	eXrefEntryUndefined
};

struct XrefEntryInput
{
	XrefEntryInput(){mObjectPosition = 0;mRivision=0;mType = eXrefEntryUndefined;}

	// well...it's more like...the first number in a pair on an xref, and the second one. the names
	// are true only for "n" type of entries
	LongFilePositionType mObjectPosition;
	unsigned long mRivision;
	EXrefEntryType mType;	
};

typedef std::vector<XrefEntryInput> XrefEntryInputVector;

typedef std::set<LongFilePositionType> LongFilePositionTypeSet;

struct ObjectStreamHeaderEntry
{
	ObjectIDType mObjectNumber;
	LongFilePositionType mObjectOffset;
};

typedef std::map<ObjectIDType,ObjectStreamHeaderEntry*> ObjectIDTypeToObjectStreamHeaderEntryMap;

class PDFParser
{
public:
	PDFParser(void);
	virtual ~PDFParser(void);

	// sets the stream to parse, then parses for enough information to be able
	// to parse objects later
	PDFHummus::EStatusCode StartPDFParsing(IByteReaderWithPosition* inSourceStream, 
											const PDFParsingOptions& inOptions = PDFParsingOptions::DefaultPDFParsingOptions());

	// get a parser that can parse objects
	PDFObjectParser& GetObjectParser();

	// get decryption helper - useful to decrypt streams if not using standard operation
	DecryptionHelper& GetDecryptionHelper();

	// below become available after initial parsing [this level is from the header]
	double GetPDFLevel();

	// GetTrailer, not calling AddRef
	PDFDictionary* GetTrailer();

	// IMPORTANT! All non "Get" prefix methods below return an object after calling AddRef (or at least make sure reference is added)
	// to handle refcount use the RefCountPtr object, or just make sure to call Release when you are done.
	
	// Creates a new object, use smart pointers to control ownership
	PDFObject* ParseNewObject(ObjectIDType inObjectId);
	ObjectIDType GetObjectsCount();

	// Query a dictinary object, if indirect, go and fetch the indirect object and return it instead
	// [if you want the direct dictionary value, use PDFDictionary::QueryDirectObject [will AddRef automatically]
	PDFObject* QueryDictionaryObject(PDFDictionary* inDictionary,const std::string& inName);
	
	// Query an array object, if indirect, go and fetch the indirect object and return it instead
	// [if you want the direct array value, use the PDFArray direct access to the vector [and use AddRef, cause it won't]
	PDFObject* QueryArrayObject(PDFArray* inArray,unsigned long inIndex);

	unsigned long GetPagesCount();
	// don't be confused - pass number of pages here. returns the dictionary, and verifies that it's actually a page (via type)
	PDFDictionary* ParsePage(unsigned long inPageIndex);
	// get page object ID for an input index
	ObjectIDType GetPageObjectID(unsigned long inPageIndex);

	// Create a reader that will be able to read the stream. when filters are included
    // in the stream definition it will add them. delete the returned object when done.
    // Note that it DOES NOT setup the reading position of the file for reading the stream,
    // so if you want to read it, you have to also move the strem position, or use StartReadingFromStream instead
	IByteReader* CreateInputStreamReader(PDFStreamInput* inStream);

	/* 
		Create a reader that will be able to read the stream, but without defiltering it.
		It will only decrypt it, if decryption is supported. This is ideal for copying
	*/
	IByteReader* CreateInputStreamReaderForPlainCopying(PDFStreamInput* inStream);
    
    // prepare parser so that you can read from the input stream object.
    // create filters and move the stream to the beginning of the stream position.
    // delete the result when done
    IByteReader* StartReadingFromStream(PDFStreamInput* inStream);

	// creates a PDFObjectParser object that you can use for reading objects
	// from the input stream. very userful for reading content streams for
	// interpreting them
	PDFObjectParser* StartReadingObjectsFromStream(PDFStreamInput* inStream);
	// same, but for an array of streams, in case of page contents that are arrays. need to count as one
	PDFObjectParser* StartReadingObjectsFromStreams(PDFArray* inArrayOfStreams);

	/*
		Same as above, but reading only decrypts, but does not defiler. ideal for copying
	*/
	IByteReader* StartReadingFromStreamForPlainCopying(PDFStreamInput* inStream);

	// use this to explictly free used objects. quite obviously this means that you'll have to parse the file again
	void ResetParser();

	// using PDFParser also for state information reading. this is a specialized version of the StartParsing for reading state
	PDFHummus::EStatusCode StartStateFileParsing(IByteReaderWithPosition* inSourceStream);

	// check if this file is encrypted. considering that the library can't really handle these files, this shoud be handy.
	bool IsEncrypted();
	// encryption is supported if there's an extender that supports it in the parser
	bool IsEncryptionSupported();

	// set extender for parser, to enhance parsing capabilities
	void SetParserExtender(IPDFParserExtender* inParserExtender);

    // advanced, direct xref access
    ObjectIDType GetXrefSize();
    XrefEntryInput* GetXrefEntry(ObjectIDType inObjectID);   
    LongFilePositionType GetXrefPosition();
    
    IByteReaderWithPosition* GetParserStream();
    
private:
	PDFObjectParser mObjectParser;
	DecryptionHelper mDecryptionHelper;
	InputOffsetStream mStream;
	AdapterIByteReaderWithPositionToIReadPositionProvider mCurrentPositionProvider;
	PDFParsingPath mNewObjectParsingPath;
	
	// we'll use this items for bacwkards reading. might turns this into a proper stream object
	IOBasicTypes::Byte mLinesBuffer[LINE_BUFFER_SIZE];
	IOBasicTypes::Byte* mCurrentBufferIndex;
	IOBasicTypes::Byte* mLastAvailableIndex;
	LongBufferSizeType mLastReadPositionFromEnd;
	bool mEncounteredFileStart;
	ObjectIDTypeToObjectStreamHeaderEntryMap mObjectStreamsCache;
	LongFilePositionTypeSet mParsedXrefs;

	double mPDFLevel;
	LongFilePositionType mLastXrefPosition;
	RefCountPtr<PDFDictionary> mTrailer;
	ObjectIDType mXrefSize; // logical size of table (declared). note that mXrefTable keeps its own size() as well.
	XrefEntryInputVector mXrefTable;
	unsigned long mPagesCount;
	ObjectIDType* mPagesObjectIDs;
	IPDFParserExtender* mParserExtender;
    bool mAllowExtendingSegments;

	PDFHummus::EStatusCode ParseHeaderLine();
	PDFHummus::EStatusCode ParseEOFLine();
	PDFHummus::EStatusCode ParseLastXrefPosition();
	PDFHummus::EStatusCode ParseTrailerDictionary(PDFDictionary** outTrailer);
	PDFHummus::EStatusCode BuildXrefTableFromTable();
	PDFHummus::EStatusCode DetermineXrefSize();
	PDFHummus::EStatusCode InitializeXref();
	PDFHummus::EStatusCode ParseXrefFromXrefTable(XrefEntryInputVector& inXrefTable,
                                                  ObjectIDType inXrefSize,
                                                  LongFilePositionType inXrefPosition,
												  bool inIsFirstXref,
                                                  ObjectIDType* outReadTableSize);
	PDFHummus::EStatusCode ReadNextXrefEntry(Byte inBuffer[20]);
	PDFObject*  ParseExistingInDirectObject(ObjectIDType inObjectID);
	PDFHummus::EStatusCode SetupDecryptionHelper(const std::string& inPassword);
	PDFHummus::EStatusCode ParsePagesObjectIDs();
	PDFHummus::EStatusCode ParsePagesIDs(PDFDictionary* inPageNode,ObjectIDType inNodeObjectID);
	PDFHummus::EStatusCode ParsePagesIDs(PDFDictionary* inPageNode,ObjectIDType inNodeObjectID,unsigned long& ioCurrentPageIndex, PDFParsingPath& ioParsingPath);
	PDFHummus::EStatusCode ParsePreviousXrefs(PDFDictionary* inTrailer);
	PDFHummus::EStatusCode MergeXrefWithMainXref(XrefEntryInputVector& inTableToMerge,ObjectIDType inMergedTableSize);
	PDFHummus::EStatusCode ParseFileDirectory();
	PDFHummus::EStatusCode BuildXrefTableAndTrailerFromXrefStream(long long inXrefStreamObjectID);
	// an overload for cases where the xref stream object is already parsed
	PDFHummus::EStatusCode ParseXrefFromXrefStream(XrefEntryInputVector& inXrefTable,
                                                   ObjectIDType inXrefSize,
                                                   PDFStreamInput* inXrefStream,
                                                   ObjectIDType* outReadTableSize);
	// an overload for cases where the position should hold a stream object, and it should be parsed
	PDFHummus::EStatusCode ParseXrefFromXrefStream(XrefEntryInputVector& inXrefTable,
                                                   ObjectIDType inXrefSize,
                                                   LongFilePositionType inXrefPosition,
                                                   ObjectIDType* outReadTableSize);
	PDFHummus::EStatusCode ReadXrefStreamSegment(XrefEntryInputVector& inXrefTable,
									 ObjectIDType inSegmentStartObject,
									 ObjectIDType inSegmentCount,
									 IByteReader* inReadFrom,
									 int* inEntryWidths,
									 unsigned long inEntryWidthsSize);
	PDFHummus::EStatusCode ReadXrefSegmentValue(IByteReader* inSource,int inEntrySize,long long& outValue);
	PDFHummus::EStatusCode ReadXrefSegmentValue(IByteReader* inSource,int inEntrySize,ObjectIDType& outValue);
	PDFHummus::EStatusCode ParsePreviousFileDirectory(LongFilePositionType inXrefPosition,
                                          XrefEntryInputVector& inXrefTable,
                                          ObjectIDType inXrefSize,
                                          PDFDictionary** outTrailer,
                                          ObjectIDType* outReadTableSize);
	PDFObject* ParseExistingInDirectStreamObject(ObjectIDType inObjectId);
	PDFHummus::EStatusCode ParseObjectStreamHeader(ObjectStreamHeaderEntry* inHeaderInfo,ObjectIDType inObjectsCount);
	void MovePositionInStream(LongFilePositionType inPosition);
	EStatusCodeAndIByteReader WrapWithPredictorStream(IByteReader* inputStream, PDFDictionary* inDecodeParams);
	EStatusCodeAndIByteReader CreateFilterForStream(IByteReader* inStream,PDFName* inFilterName,PDFDictionary* inDecodeParams, PDFStreamInput* inPDFStream);

	void NotifyIndirectObjectStart(long long inObjectID, long long inGenerationNumber);
	void NotifyIndirectObjectEnd(PDFObject* inObject);

	IByteReader* WrapWithDecryptionFilter(PDFStreamInput* inStream, IByteReader* inToWrapStream);

	// Backward reading
	bool ReadNextBufferFromEnd();
	LongBufferSizeType GetCurrentPositionFromEnd();
	bool ReadBack(IOBasicTypes::Byte& outValue);
	bool IsBeginOfFile();

	bool GoBackTillToken();
	bool GoBackTillNonToken();
	void GoBackTillLineStart();
	bool IsPDFWhiteSpace(IOBasicTypes::Byte inCharacter);

	PDFHummus::EStatusCode ExtendXrefToSize(XrefEntryInputVector& inXrefTable, ObjectIDType inXrefSize);
};
