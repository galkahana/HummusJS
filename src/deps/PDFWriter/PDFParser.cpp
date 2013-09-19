/*
   Source File : PDFParser.cpp


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
#include "PDFParser.h"
#include "IByteReaderWithPosition.h"
#include "PDFParserTokenizer.h"
#include "Trace.h"
#include "PDFInteger.h"
#include "PDFObject.h"
#include "PDFSymbol.h"
#include "BoxingBase.h"
#include "PDFDictionary.h"
#include "BoxingBase.h"
#include "PDFIndirectObjectReference.h"
#include "PDFName.h"
#include "PDFArray.h"
#include "RefCountPtr.h"
#include "PDFObjectCast.h"
#include "PDFStreamInput.h"
#include "InputLimitedStream.h"
#include "InputFlateDecodeStream.h"
#include "InputStreamSkipperStream.h"
#include "InputPredictorPNGUpStream.h"
#include "InputPredictorPNGNoneStream.h"
#include "InputPredictorPNGSubStream.h"
#include "InputPredictorPNGAverageStream.h"
#include "InputPredictorPNGPaethStream.h"
#include "InputPredictorPNGOptimumStream.h"
#include "InputPredictorTIFFSubStream.h"
#include "InputAscii85DecodeStream.h"
#include "IPDFParserExtender.h"
#include "InputDCTDecodeStream.h"

#include  <algorithm>
using namespace PDFHummus;

PDFParser::PDFParser(void)
{
	mStream = NULL;
	mTrailer = NULL;
	mXrefTable = NULL;
	mPagesObjectIDs = NULL;
	mParserExtender = NULL;
    mAllowExtendingSegments = true; // Gal 19.9.2013: here's some policy changer. basically i'm supposed to ignore all segments that declare objects past the trailer
                                    // declared size. but i would like to allow files that do extend. as this is incompatible with the specs, i'll make
                                    // this boolean dendent. i will sometimes make it public so ppl can actually modify this policy. for now, it's internal
}

PDFParser::~PDFParser(void)
{
	ResetParser();
}

void PDFParser::ResetParser()
{
	mTrailer = NULL;
	delete[] mXrefTable;
	mXrefTable = NULL;
	delete[] mPagesObjectIDs;
	mPagesObjectIDs = NULL;
	mStream = NULL;
	mCurrentPositionProvider.Assign(NULL);

	ObjectIDTypeToObjectStreamHeaderEntryMap::iterator it = mObjectStreamsCache.begin();
	for(; it != mObjectStreamsCache.end();++it)
		delete[] it->second;
	mObjectStreamsCache.clear();

}

EStatusCode PDFParser::StartPDFParsing(IByteReaderWithPosition* inSourceStream)
{
	EStatusCode status;

	ResetParser();

	mStream = inSourceStream;
	mCurrentPositionProvider.Assign(mStream);
	mObjectParser.SetReadStream(inSourceStream,&mCurrentPositionProvider);

	do
	{
		status = ParseHeaderLine();
		if(status != PDFHummus::eSuccess)
			break;

		// initialize reading from end
		mLastReadPositionFromEnd = 0;
		mEncounteredFileStart = false;
		mLastAvailableIndex = mCurrentBufferIndex = mLinesBuffer;

		status = ParseEOFLine();
		if(status != PDFHummus::eSuccess)
			break;

		status = ParseLastXrefPosition();
		if(status != PDFHummus::eSuccess)
			break;

		status = ParseFileDirectory(); // that would be the xref and trailer
		if(status != PDFHummus::eSuccess)
			break;

		if(IsEncrypted())
		{
			// not parsing pages for encrypted docs. 
			// not commiting..and there's a practical reason. 
			// lower level objects will be in object streams (for those PDFs that have them)
			// and the may not be accessed
			mPagesCount = 0;
			mPagesObjectIDs = NULL;
		}
		else
		{
			status = ParsePagesObjectIDs();
			if(status != PDFHummus::eSuccess)
				break;
		}
	}while(false);

	return status;
}

PDFObjectParser& PDFParser::GetObjectParser()
{
	return mObjectParser;
}

static const std::string scPDFMagic = "%PDF-";
EStatusCode PDFParser::ParseHeaderLine()
{
	PDFParserTokenizer tokenizer;

	tokenizer.SetReadStream(mStream);
	BoolAndString tokenizerResult = tokenizer.GetNextToken();

	if(!tokenizerResult.first)
	{
		TRACE_LOG("PDFParser::ParseHeaderLine, no tokens in PDF input. in other words - it's empty.");
		return PDFHummus::eFailure;
	}

	if(tokenizerResult.second.compare(0,scPDFMagic.size(),scPDFMagic) != 0)
	{
		TRACE_LOG1("PDFParser::ParseHeaderLine, file does not begin as a PDF file. a PDF file should start with \"%PDF-\". file header = %s",tokenizerResult.second.c_str());
		return PDFHummus::eFailure;
	}

	mPDFLevel = Double(tokenizerResult.second.substr(scPDFMagic.size()));
	return PDFHummus::eSuccess;
}

static const std::string scEOF = "%%EOF";
EStatusCode PDFParser::ParseEOFLine()
{
	/* go back till you hit token. this should be the EOF. go back till line start and get the token...if it's not EOF, fail.
	   since EOF is a comment, then if there's anything else in that line it will either be before %%EOF, which means %%EOF won't be taken, or after - 
	   in which case it'd be part of the comment. in any case - if it's not exactly EOF, there will be a failure. but i am allowing 
	   extra empty lines after %%EOF
	*/
	if(GoBackTillToken())
	{
		GoBackTillLineStart();
		mStream->SetPositionFromEnd(GetCurrentPositionFromEnd());

		PDFParserTokenizer aTokenizer;
		aTokenizer.SetReadStream(mStream);
		BoolAndString token = aTokenizer.GetNextToken();

		if(token.first && (token.second.substr(0,scEOF.length()) == scEOF))
		{
			return PDFHummus::eSuccess;
		}
		else
		{
			TRACE_LOG("PDFParser::ParseEOFLine, failure, last line not %%EOF");
			return PDFHummus::eFailure;
		}
	}
	else
	{
		TRACE_LOG("PDFParser::ParseEOFLine, Couldn't find tokens in file");
		return PDFHummus::eFailure;
	}
}

LongBufferSizeType PDFParser::GetCurrentPositionFromEnd()
{
	return mLastReadPositionFromEnd-(mCurrentBufferIndex-mLinesBuffer);
}

bool PDFParser::GoBackTillToken()
{
	Byte buffer;
	bool foundToken = false;

	while(ReadBack(buffer))
	{
		if(!IsPDFWhiteSpace(buffer))
		{
			foundToken = true;
			break;
		}
	}
	return foundToken;
}

bool PDFParser::GoBackTillNonToken()
{
	Byte buffer;
	bool foundNonToken = false;

	while(ReadBack(buffer))
	{
		if(IsPDFWhiteSpace(buffer))
		{
			foundNonToken = true;
			break;
		}
	}
	return foundNonToken;
}

static const Byte scWhiteSpaces[] = {0,0x9,0xA,0xC,0xD,0x20};
bool PDFParser::IsPDFWhiteSpace(Byte inCharacter)
{
	bool isWhiteSpace = false;
	for(int i=0; i < 6 && !isWhiteSpace; ++i)
		isWhiteSpace =  (scWhiteSpaces[i] == inCharacter);
	return isWhiteSpace;
}


static const char scCR = '\r';
static const char scLN = '\n';
void PDFParser::GoBackTillLineStart()
{
	Byte buffer;

	while(ReadBack(buffer))
	{
		if(scLN == buffer || scCR == buffer)
			break;
	}
}

bool PDFParser::ReadBack(Byte& outValue)
{
	if(IsBeginOfFile())
		return false;

	if(mCurrentBufferIndex > mLinesBuffer)
	{
		--mCurrentBufferIndex;
		outValue = *mCurrentBufferIndex;
		return true;
	}
	else
	{
		ReadNextBufferFromEnd(); // must be able to read...but could be 0 bytes
		if(mCurrentBufferIndex > mLinesBuffer)
		{
			--mCurrentBufferIndex;
			outValue = *mCurrentBufferIndex;
			return true;
		}
		else
			return false;
	}
}

bool PDFParser::ReadNextBufferFromEnd()
{
	if(mEncounteredFileStart)
	{
		return false;
	}
	else
	{
		mStream->SetPositionFromEnd(mLastReadPositionFromEnd + LINE_BUFFER_SIZE);
		LongBufferSizeType readAmount = mStream->Read(mLinesBuffer,LINE_BUFFER_SIZE);
		if(0 == readAmount)
			return false;
		mLastAvailableIndex = mLinesBuffer + readAmount;
		mCurrentBufferIndex = mLastAvailableIndex;
		mLastReadPositionFromEnd+= readAmount;
		mEncounteredFileStart = readAmount < LINE_BUFFER_SIZE;
		return true;
	}
}

bool PDFParser::IsBeginOfFile()
{
	return mEncounteredFileStart && (mCurrentBufferIndex == mLinesBuffer);
}

static const std::string scStartxref = "startxref";
EStatusCode PDFParser::ParseLastXrefPosition()
{
	EStatusCode status = PDFHummus::eSuccess;
	
	// next two lines should be the xref position and then "startxref"
	
	do
	{

		// find and read xref position
		if(!GoBackTillToken())
		{
			status = PDFHummus::eFailure;
			TRACE_LOG("PDFParser::ParseXrefPosition, couldn't find xref position token");
			break;
		}

		GoBackTillLineStart();

		// now go forward, and here i'm guessing a bit, till you get to either and integer, or the startxref keyword
		mStream->SetPositionFromEnd(GetCurrentPositionFromEnd());
		
		mObjectParser.ResetReadState();
		RefCountPtr<PDFObject> anObject(mObjectParser.ParseNewObject(mParserExtender));

		if(anObject->GetType() == PDFObject::ePDFObjectInteger)
		{
			mLastXrefPosition = (LongFilePositionType)((PDFInteger*)anObject.GetPtr())->GetValue();

			// find and read startxref keyword
			if(!GoBackTillToken())
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFParser::ParseXrefPosition, couldn't find startxref keyword");
				break;
			}

			GoBackTillLineStart();
			mStream->SetPositionFromEnd(GetCurrentPositionFromEnd());
			
			mObjectParser.ResetReadState();
			PDFObjectCastPtr<PDFSymbol> startxRef(mObjectParser.ParseNewObject(mParserExtender));

			if(!startxRef || startxRef->GetValue() != scStartxref)
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFParser::ParseXrefPosition, syntax error in reading xref position");
				break;
			}
		}
		else // this means that the line is not only integer, a bit more complicated path, look for startxref and then the next would be the number
		{
			bool foundStartXref = (anObject->GetType() == PDFObject::ePDFObjectSymbol) && (((PDFSymbol*)anObject.GetPtr())->GetValue() == scStartxref);

			while(!foundStartXref && mStream->NotEnded())
			{
				PDFObjectCastPtr<PDFSymbol> startxRef(mObjectParser.ParseNewObject(mParserExtender));
				foundStartXref = startxRef.GetPtr() && (startxRef->GetValue() == scStartxref);
			}
			
			if(!foundStartXref)
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFParser::ParseXrefPosition, could not find startxref keyword");
				break;
			}

			PDFObjectCastPtr<PDFInteger> xrefPosition(mObjectParser.ParseNewObject(mParserExtender));
			if(!xrefPosition)
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFParser::ParseXrefPosition, syntax error in reading xref position");
				break;
			}		

			mLastXrefPosition = xrefPosition->GetValue();
		}

	}while(false);

	return status;

}

static const std::string scTrailer = "trailer";
EStatusCode PDFParser::ParseTrailerDictionary()
{

	EStatusCode status = PDFHummus::eSuccess;
	bool foundTrailer = false;

	do
	{
		PDFParserTokenizer aTokenizer;
		aTokenizer.SetReadStream(mStream);
		
		do
		{
			BoolAndString token = aTokenizer.GetNextToken();
			if(!token.first)
				break;
			foundTrailer = (scTrailer == token.second);
		}while(!foundTrailer);


		if(!foundTrailer)
		{
			status = PDFHummus::eFailure;
			TRACE_LOG("PDFParser::ParseTrailerDictionary, trailer not found...");
			break;
		}

		// k. now that all is well, just parse the damn dictionary, which is actually...the easiest part.
		mObjectParser.ResetReadState();
		PDFObjectCastPtr<PDFDictionary> dictionaryObject(mObjectParser.ParseNewObject(mParserExtender));
		if(!dictionaryObject)
		{
			status = PDFHummus::eFailure;
			TRACE_LOG("PDFParser::ParseTrailerDictionary, failure to parse trailer dictionary");
			break;
		}

		mTrailer = dictionaryObject;
	}while(false);

	return status;
}

EStatusCode PDFParser::BuildXrefTableFromTable()
{
	EStatusCode status;

	do
	{
		status = DetermineXrefSize();
		if(status != PDFHummus::eSuccess)
			break;

		status = InitializeXref();
		if(status != PDFHummus::eSuccess)
			break;

		if(mTrailer->Exists("Prev"))
		{
			status = ParsePreviousXrefs(mTrailer.GetPtr());
			if(status != PDFHummus::eSuccess)
				break;
		}

        XrefEntryInput* extendedTable = NULL;
        ObjectIDType extendedTableSize;
		status = ParseXrefFromXrefTable(mXrefTable,mXrefSize,mLastXrefPosition,&extendedTable,&extendedTableSize);
		if(status != PDFHummus::eSuccess)
			break;
        
        // Table may have been extended, in which case replace the pointer and current size
        if(extendedTable)
        {
            mXrefSize = extendedTableSize;
            delete[] mXrefTable;
            mXrefTable = extendedTable;
        }

		// For hybrids, check also XRefStm entry
		PDFObjectCastPtr<PDFInteger> xrefStmReference(mTrailer->QueryDirectObject("XRefStm"));
		if(!xrefStmReference)
			break;
		// if exists, merge update xref
		status = ParseXrefFromXrefStream(mXrefTable,mXrefSize,xrefStmReference->GetValue(),&extendedTable,&extendedTableSize);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("PDFParser::ParseDirectory, failure to parse xref in hybrid mode");
			break;
		}
        if(extendedTable)
        {
            mXrefSize = extendedTableSize;
            delete[] mXrefTable;
            mXrefTable = extendedTable;
        }
	}while(false);

	return status;
}

EStatusCode PDFParser::DetermineXrefSize()
{
	PDFObjectCastPtr<PDFInteger> aSize(mTrailer->QueryDirectObject("Size"));

	if(!aSize)
	{
		return PDFHummus::eFailure;
	}
	else
	{
		mXrefSize = (ObjectIDType)aSize->GetValue();
		return PDFHummus::eSuccess;
	}
}

EStatusCode PDFParser::InitializeXref()
{
	mXrefTable = new XrefEntryInput[mXrefSize];
	return PDFHummus::eSuccess;
}

typedef BoxingBaseWithRW<ObjectIDType> ObjectIDTypeBox;
typedef BoxingBaseWithRW<unsigned long> ULong;
typedef BoxingBaseWithRW<LongFilePositionType> LongFilePositionTypeBox;

static const std::string scXref = "xref";
EStatusCode PDFParser::ParseXrefFromXrefTable(XrefEntryInput* inXrefTable,
                                              ObjectIDType inXrefSize,
                                              LongFilePositionType inXrefPosition,
                                              XrefEntryInput** outExtendedTable,
                                              ObjectIDType* outExtendedTableSize)
{
	// K. cross ref starts at  xref position
	// and ends with trailer (or when exahausted the number of objects...whichever first)
	// i'm gonna tokanize them, for easier reading
	PDFParserTokenizer tokenizer;
	BoolAndString token;
	EStatusCode status = PDFHummus::eSuccess;
	ObjectIDType firstNonSectionObject;
	Byte entry[20];

    *outExtendedTable = NULL;
    
	tokenizer.SetReadStream(mStream);
	MovePositionInStream(inXrefPosition);

	// Note that at times, the xref is being read "on empty". meaning - entries will be read but they will not affect the actual xref.
	// This is done because final xref might be smaller than the prev xrefs, and i'm only interested in objects that are in the final xref.
	// That said - i still want to be in the position of the trailer after this function is being executed.

	do
	{
		// first token must be "xref", so just verify
		token = tokenizer.GetNextToken();
		if(!token.first || token.second != scXref)
		{
			TRACE_LOG1("PDFParser::ParseXref, error in parsing xref, expected to find \"xref\" keyword, found = %s",token.second.c_str());
			status = PDFHummus::eFailure;
			break;
		}
		
		ObjectIDType currentObject = 0;

		while(PDFHummus::eSuccess == status)
		{
			token = tokenizer.GetNextToken();
			if(!token.first)
			{
				TRACE_LOG("PDFParser::ParseXref, failed to read tokens, while reading xref");
				status = PDFHummus::eFailure;
				break;
			}
			
			// token may be either start of section or "trailer"
			if(scTrailer == token.second)
				break;

			currentObject = ObjectIDTypeBox(token.second);
			token = tokenizer.GetNextToken();
			if(!token.first)
			{
				TRACE_LOG("PDFParser::ParseXref, unable to read section size, while reading xref");
				status = PDFHummus::eFailure;
				break;
			}
			if(ObjectIDTypeBox(token.second) == 0)
				continue; // probably will never happen
			firstNonSectionObject = currentObject + ObjectIDTypeBox(token.second);

            // if the segment declared objects above the xref size, consult policy on what to do
            if(firstNonSectionObject > inXrefSize && mAllowExtendingSegments)
            {
                inXrefTable = ExtendXrefTableToSize(inXrefTable,inXrefSize,firstNonSectionObject);
                inXrefSize = firstNonSectionObject;
                if(*outExtendedTable)
                    delete[] *outExtendedTable;
                *outExtendedTable = inXrefTable;
                *outExtendedTableSize = firstNonSectionObject;
            }
            

			// first row...not sure where starts...so skip till passing all endlines
			do
			{
				if(mStream->Read(entry,1) != 1)
				{
					TRACE_LOG("PDFParser::ParseXref, failed to read xref entry");
					status = PDFHummus::eFailure;
					break;
				}
			}while(IsPDFWhiteSpace(entry[0]));

			// now read extra 19
			if(mStream->Read(entry+1,19) != 19)
			{
				TRACE_LOG("PDFParser::ParseXref, failed to read xref entry");
				status = PDFHummus::eFailure;
				break;
			}
			if(currentObject < inXrefSize)
			{
				inXrefTable[currentObject].mObjectPosition = LongFilePositionTypeBox((const char*)entry);
				inXrefTable[currentObject].mRivision = ULong((const char*)(entry+11));
				inXrefTable[currentObject].mType = entry[17] == 'n' ? eXrefEntryExisting:eXrefEntryDelete;
			}
			++currentObject;



			// now parse the section. 
			while(currentObject < firstNonSectionObject)
			{
				if(mStream->Read(entry,20) != 20)
				{
					TRACE_LOG("PDFParser::ParseXref, failed to read xref entry");
					status = PDFHummus::eFailure;
					break;
				}
				if(currentObject < inXrefSize)
				{
					inXrefTable[currentObject].mObjectPosition = LongFilePositionTypeBox((const char*)entry);
					inXrefTable[currentObject].mRivision = ULong((const char*)(entry+11));
					inXrefTable[currentObject].mType = entry[17] == 'n' ? eXrefEntryExisting:eXrefEntryDelete;
				}
				++currentObject;
			}
		}
		if(status != PDFHummus::eSuccess)
			break;

	}while(false);	
	mObjectParser.ResetReadState(); // cause read without consulting with the tokenizer...so now there shouldn't be available tokens

	return status;
}

XrefEntryInput* PDFParser::ExtendXrefTableToSize(XrefEntryInput* inXrefTable,ObjectIDType inOldSize,ObjectIDType inNewSize)
{
    XrefEntryInput* newTable = new XrefEntryInput[inNewSize];

	for(ObjectIDType i = 0; i < inOldSize; ++i)
        newTable[i] =	inXrefTable[i];
    return newTable;
}

PDFDictionary* PDFParser::GetTrailer()
{
	return mTrailer.GetPtr();
}

double PDFParser::GetPDFLevel()
{
	return mPDFLevel;
}

PDFObject* PDFParser::ParseNewObject(ObjectIDType inObjectId)
{
	if(inObjectId >= mXrefSize)
	{
		return NULL;
	}
	else if(eXrefEntryExisting == mXrefTable[inObjectId].mType)
	{
		return ParseExistingInDirectObject(inObjectId);
	}
	else if(eXrefEntryStreamObject == mXrefTable[inObjectId].mType)
	{
		return ParseExistingInDirectStreamObject(inObjectId);
	}
	else
		return NULL;
}

ObjectIDType PDFParser::GetObjectsCount()
{
	return mXrefSize;
}

static const std::string scObj = "obj";
PDFObject* PDFParser::ParseExistingInDirectObject(ObjectIDType inObjectID)
{
	PDFObject* readObject = NULL;

	MovePositionInStream(mXrefTable[inObjectID].mObjectPosition);

	do
	{
		// should get us to the ObjectNumber ObjectVersion obj section
		// verify that it's good and if so continue to parse the object itself

		// verify object ID
		PDFObjectCastPtr<PDFInteger> idObject(mObjectParser.ParseNewObject(mParserExtender));

		if(!idObject)
		{
			TRACE_LOG("PDFParser::ParseExistingInDirectObject, failed to read object declaration, ID");
			break;
		}

		if((ObjectIDType)idObject->GetValue() != inObjectID)
		{
			TRACE_LOG2("PDFParser::ParseExistingInDirectObject, failed to read object declaration, exepected ID = %ld, found %ld",
				inObjectID,idObject->GetValue());
			break;
		}

		// verify object Version
		PDFObjectCastPtr<PDFInteger> versionObject(mObjectParser.ParseNewObject(mParserExtender));

		if(!versionObject)
		{
			TRACE_LOG("PDFParser::ParseExistingInDirectObject, failed to read object declaration, Version");
			break;
		}

		if(mParserExtender)
			mParserExtender->OnObjectStart(inObjectID,versionObject->GetValue());

		if((unsigned long)versionObject->GetValue() != mXrefTable[inObjectID].mRivision)
		{
			TRACE_LOG2("PDFParser::ParseExistingInDirectObject, failed to read object declaration, exepected version = %ld, found %ld",
				mXrefTable[inObjectID].mRivision,versionObject->GetValue());
			break;
		}

		// now the obj keyword
		PDFObjectCastPtr<PDFSymbol> objKeyword(mObjectParser.ParseNewObject(mParserExtender));

		if(!objKeyword)
		{
			TRACE_LOG("PDFParser::ParseExistingInDirectObject, failed to read object declaration, obj keyword");
			break;
		}

		if(objKeyword->GetValue() != scObj)
		{
			TRACE_LOG1("PDFParser::ParseExistingInDirectObject, failed to read object declaration, expected obj keyword found %s",
				objKeyword->GetValue().c_str());
			break;
		}

		readObject = mObjectParser.ParseNewObject(mParserExtender);

		if(mParserExtender)
			mParserExtender->OnObjectEnd(readObject);
	}while(false);

	return readObject;
}

EStatusCode PDFParser::ParsePagesObjectIDs()
{
	EStatusCode status = PDFHummus::eSuccess;

	// m.k plan is to look for the catalog, then find the pages, then initialize the array to the count at the root, and then just recursively loop
	// the pages by order of pages and fill up the IDs. easy.
	
	do
	{
		// get catalogue, verify indirect reference
		PDFObjectCastPtr<PDFIndirectObjectReference> catalogReference(mTrailer->QueryDirectObject("Root"));
		if(!catalogReference)
		{
			TRACE_LOG("PDFParser::ParsePagesObjectIDs, failed to read catalog reference in trailer");
			status = PDFHummus::eFailure;
			break;
		}

		PDFObjectCastPtr<PDFDictionary> catalog(ParseNewObject(catalogReference->mObjectID));
		if(!catalog)
		{
			TRACE_LOG("PDFParser::ParsePagesObjectIDs, failed to read catalog");
			status = PDFHummus::eFailure;
			break;
		}

		// get pages, verify indirect reference
		PDFObjectCastPtr<PDFIndirectObjectReference> pagesReference(catalog->QueryDirectObject("Pages"));
		if(!pagesReference)
		{
			TRACE_LOG("PDFParser::ParsePagesObjectIDs, failed to read pages reference in catalog");
			status = PDFHummus::eFailure;
			break;
		}

		PDFObjectCastPtr<PDFDictionary> pages(ParseNewObject(pagesReference->mObjectID));
		if(!pages)
		{
			TRACE_LOG("PDFParser::ParsePagesObjectIDs, failed to read pages");
			status = PDFHummus::eFailure;
			break;
		}

		PDFObjectCastPtr<PDFInteger> totalPagesCount(QueryDictionaryObject(pages.GetPtr(),"Count"));
		if(!totalPagesCount)
		{
			TRACE_LOG("PDFParser::ParsePagesObjectIDs, failed to read pages count");
			status = PDFHummus::eFailure;
			break;
		}
	
		mPagesCount = (unsigned long)totalPagesCount->GetValue();
		mPagesObjectIDs = new ObjectIDType[mPagesCount];

		// now iterate through pages objects, and fill up the IDs [don't really need the object ID for the root pages tree...but whatever
		status = ParsePagesIDs(pages.GetPtr(),pagesReference->mObjectID);

	}while(false);

	return status;
}

EStatusCode PDFParser::ParsePagesIDs(PDFDictionary* inPageNode,ObjectIDType inNodeObjectID)
{
	unsigned long currentPageIndex = 0;

	return ParsePagesIDs(inPageNode,inNodeObjectID,currentPageIndex);
}

static const std::string scPage = "Page";
static const std::string scPages = "Pages";
EStatusCode PDFParser::ParsePagesIDs(PDFDictionary* inPageNode,ObjectIDType inNodeObjectID,unsigned long& ioCurrentPageIndex)
{
	// recursion.
	// if this is a page, write it's node object ID in the current page index and +1
	// if this is a pagetree, loop it's kids, for each parsing the kid, running the recursion on it, and deleting

	EStatusCode status = PDFHummus::eSuccess;

	do
	{
		PDFObjectCastPtr<PDFName> objectType(inPageNode->QueryDirectObject("Type"));
		if(!objectType)
		{
			TRACE_LOG("PDFParser::ParsePagesIDs, can't read object type");
			status = PDFHummus::eFailure;
			break;
		}
		
		if(scPage == objectType->GetValue())
		{
			// a Page
			if(ioCurrentPageIndex >= mPagesCount)
			{
				TRACE_LOG("PDFParser::ParsePagesIDs, there are more pages than the page count specifies. fail.");
				status = PDFHummus::eFailure;
				break;
			}

			mPagesObjectIDs[ioCurrentPageIndex] = inNodeObjectID;
			++ioCurrentPageIndex;
		}
		else if(scPages == objectType->GetValue())
		{
			// a Page tree node
			PDFObjectCastPtr<PDFArray> kidsObject(inPageNode->QueryDirectObject("Kids"));
			if(!kidsObject)
			{
				TRACE_LOG("PDFParser::ParsePagesIDs, unable to find page kids array");
				status = PDFHummus::eFailure;
				break;
			}

			SingleValueContainerIterator<PDFObjectVector> it = kidsObject->GetIterator();
			
			while(it.MoveNext() && PDFHummus::eSuccess == status)
			{
				if(it.GetItem()->GetType() != PDFObject::ePDFObjectIndirectObjectReference)
				{
					TRACE_LOG1("PDFParser::ParsePagesIDs, unexpected type for a Kids array object, type = %s",PDFObject::scPDFObjectTypeLabel[it.GetItem()->GetType()]);
					status = PDFHummus::eFailure;
					break;
				}

				PDFObjectCastPtr<PDFDictionary> pageNodeObject(ParseNewObject(((PDFIndirectObjectReference*)it.GetItem())->mObjectID));
				if(!pageNodeObject)
				{
					TRACE_LOG("PDFParser::ParsePagesIDs, unable to parse page node object from kids reference");
					status = PDFHummus::eFailure;
					break;
				}

				status = ParsePagesIDs(pageNodeObject.GetPtr(),((PDFIndirectObjectReference*)it.GetItem())->mObjectID,ioCurrentPageIndex);
			}
		}
		else 
		{
			TRACE_LOG1("PDFParser::ParsePagesIDs, unexpected object type. should be either Page or Pages, found %s",objectType->GetValue().c_str());
			status = PDFHummus::eFailure;
			break;
		}
	}while(false);

	return status;
}

unsigned long  PDFParser::GetPagesCount()
{
	return mPagesCount;
}

ObjectIDType PDFParser::GetPageObjectID(unsigned long inPageIndex)
{
	if(mPagesCount <= inPageIndex)
		return 0;

	return mPagesObjectIDs[inPageIndex];
}


PDFDictionary* PDFParser::ParsePage(unsigned long inPageIndex)
{
	if(mPagesCount <= inPageIndex)
		return NULL;

	PDFObjectCastPtr<PDFDictionary> pageObject(ParseNewObject(mPagesObjectIDs[inPageIndex]));

	if(!pageObject)
	{
		TRACE_LOG1("PDFParser::ParsePage, couldn't find page object for index %ld",inPageIndex);
		return NULL;
	}

	PDFObjectCastPtr<PDFName> objectType(pageObject->QueryDirectObject("Type"));

	if(scPage == objectType->GetValue())
	{
		pageObject->AddRef();
		return pageObject.GetPtr();
	}
	else
	{
		TRACE_LOG1("PDFParser::ParsePage, page object listed in page array for %ld is actually not a page",inPageIndex);
		return NULL;
	}
}

PDFObject* PDFParser::QueryDictionaryObject(PDFDictionary* inDictionary,const std::string& inName)
{
	RefCountPtr<PDFObject> anObject(inDictionary->QueryDirectObject(inName));

	if(anObject.GetPtr() == NULL)
		return NULL;

	if(anObject->GetType() == PDFObject::ePDFObjectIndirectObjectReference)
	{
		PDFObject* theActualObject = ParseNewObject(((PDFIndirectObjectReference*)anObject.GetPtr())->mObjectID);
		return theActualObject;
	}
	else
	{
		anObject->AddRef(); // adding ref to increase owners
		return anObject.GetPtr();
	}
}

PDFObject* PDFParser::QueryArrayObject(PDFArray* inArray,unsigned long inIndex)
{
	RefCountPtr<PDFObject> anObject(inArray->QueryObject(inIndex));

	if(anObject.GetPtr() == NULL)
		return NULL;
	
	if(anObject->GetType() == PDFObject::ePDFObjectIndirectObjectReference)
	{
		PDFObject* theActualObject = ParseNewObject(((PDFIndirectObjectReference*)anObject.GetPtr())->mObjectID);
		return theActualObject;
	}
	else
	{
		anObject->AddRef(); // adding ref to increase owners
		return anObject.GetPtr();
	}

}

EStatusCode PDFParser::ParsePreviousXrefs(PDFDictionary* inTrailer)
{
	PDFObjectCastPtr<PDFInteger> previousPosition(inTrailer->QueryDirectObject("Prev"));
	if(!previousPosition)
	{
		TRACE_LOG("PDFParser::ParsePreviousXrefs, unexpected, prev is not integer");
		return PDFHummus::eFailure;
	}

	EStatusCode status;

	XrefEntryInput* aTable = new XrefEntryInput[mXrefSize];
	do
	{
		PDFDictionary* trailerP = NULL;

        XrefEntryInput* extendedTable = NULL;
        ObjectIDType extendedTableSize;
		status = ParseDirectory(previousPosition->GetValue(),aTable,mXrefSize,&trailerP,&extendedTable,&extendedTableSize);
		if(status != PDFHummus::eSuccess)
			break;
		RefCountPtr<PDFDictionary> trailer(trailerP);

		if(trailer->Exists("Prev"))
		{
			status = ParsePreviousXrefs(trailer.GetPtr());
			if(status != PDFHummus::eSuccess)
				break;
		}
        
        
        // Table may have been extended, in which case replace the pointer and current size
        ObjectIDType newTableSize;
        if(extendedTable)
        {
            newTableSize = extendedTableSize;
            delete[] aTable;
            aTable = extendedTable;
        }
        else
            newTableSize = mXrefSize;
        MergeXrefWithMainXref(aTable,newTableSize);
	}
	while(false);

	delete[] aTable;
	return status;
}

EStatusCode PDFParser::ParseDirectory(LongFilePositionType inXrefPosition,
									  XrefEntryInput* inXrefTable,
									  ObjectIDType inXrefSize,
									  PDFDictionary** outTrailer,
                                      XrefEntryInput** outExtendedTable,
                                      ObjectIDType* outExtendedTableSize)
{
	EStatusCode status = PDFHummus::eSuccess;
	
	MovePositionInStream(inXrefPosition);

	do
	{
		// take the object, so that we can check whether this is an Xref or an Xref stream
		RefCountPtr<PDFObject> anObject(mObjectParser.ParseNewObject(mParserExtender));
		if(!anObject)
		{
			status = PDFHummus::eFailure;
			break;
		}

		if(anObject->GetType() == PDFObject::ePDFObjectSymbol && ((PDFSymbol*)anObject.GetPtr())->GetValue() == scXref)
		{
			// This is the case of a regular xref table. note that as oppose to the main trailer case
			// i already have a limit of Xrefsize (which is determined by the main trailer Size entry)
			// so i don't have to parse the trailer in advance, but rather just read the file in the natural order:
			// first - the xref then the trailer.
			status = ParseXrefFromXrefTable(inXrefTable,inXrefSize,inXrefPosition,outExtendedTable,outExtendedTableSize);
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG1("PDFParser::ParseDirectory, failed to parse xref table in %ld",inXrefPosition);
				break;
			}
            
            if(outExtendedTable)
            {
                inXrefTable = *outExtendedTable;
                inXrefSize = *outExtendedTableSize;
            }

			// at this point we should be after the token of the "trailer"
			PDFObjectCastPtr<PDFDictionary> trailerDictionary(mObjectParser.ParseNewObject(mParserExtender));
			if(!trailerDictionary)
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFParser::ParseDirectory, failure to parse trailer dictionary");
				break;
			}	

			// For hybrids, check also XRefStm entry
			PDFObjectCastPtr<PDFInteger> xrefStmReference(trailerDictionary->QueryDirectObject("XRefStm"));
			if(xrefStmReference.GetPtr())
			{
				// if exists, merge update xref
				status = ParseXrefFromXrefStream(inXrefTable,inXrefSize,xrefStmReference->GetValue(),outExtendedTable,outExtendedTableSize);
				if(status != PDFHummus::eSuccess)
				{
					TRACE_LOG("PDFParser::ParseDirectory, failure to parse xref in hybrid mode");
					break;
				}
			}

			trailerDictionary->AddRef();
			*outTrailer = trailerDictionary.GetPtr();
		}
		else if(anObject->GetType() == PDFObject::ePDFObjectInteger && ((PDFInteger*)anObject.GetPtr())->GetValue() > 0)
		{
			// Xref stream case. make some validations, grab the xref stream object details, and parse it

			PDFObjectCastPtr<PDFInteger> versionObject(mObjectParser.ParseNewObject(mParserExtender));

			if(!versionObject)
			{
				TRACE_LOG("PDFParser::ParseDirectory, failed to read xref object declaration, Version");
				status = PDFHummus::eFailure;
				break;
			}

			if(mParserExtender)
				mParserExtender->OnObjectStart(((PDFInteger*)anObject.GetPtr())->GetValue(),versionObject->GetValue());


			PDFObjectCastPtr<PDFSymbol> objKeyword(mObjectParser.ParseNewObject(mParserExtender));

			if(!objKeyword)
			{
				TRACE_LOG("PDFParser::ParseDirectory, failed to read xref object declaration, obj keyword");
				status = PDFHummus::eFailure;
				break;
			}

			if(objKeyword->GetValue() != scObj)
			{
				TRACE_LOG1("PDFParser::ParseDirectory, failed to read xref object declaration, expected obj keyword found %s",
					objKeyword->GetValue().c_str());
				status = PDFHummus::eFailure;
				break;
			}

			PDFObjectCastPtr<PDFStreamInput> xrefStream(mObjectParser.ParseNewObject(mParserExtender));
			if(!xrefStream)
			{
				TRACE_LOG("PDFParser::BuildXrefTableAndTrailerFromXrefStream, failure to parse xref stream");
				status = PDFHummus::eFailure;
				break;
			}

			if(mParserExtender)
				mParserExtender->OnObjectEnd(xrefStream.GetPtr());

			*outTrailer = xrefStream->QueryStreamDictionary();

			status = ParseXrefFromXrefStream(inXrefTable,inXrefSize,xrefStream.GetPtr(),outExtendedTable,outExtendedTableSize);
			if(status != PDFHummus::eSuccess)
				break;
		}
		else
		{
			TRACE_LOG("PDFParser::ParseDirectory,Unexpected object at xref start");
			status = PDFHummus::eFailure;
		}
	}while(false);
	return status;
}	

void PDFParser::MergeXrefWithMainXref(XrefEntryInput* inTableToMerge,ObjectIDType inMergedTableSize)
{
    if(inMergedTableSize > mXrefSize)
    {
        XrefEntryInput* newTable = ExtendXrefTableToSize(mXrefTable, mXrefSize, inMergedTableSize);
        mXrefSize = inMergedTableSize;
        delete[] mXrefTable;
        mXrefTable = newTable;
    }
    
	for(ObjectIDType i = 0; i < mXrefSize; ++i)
	{
		if(inTableToMerge[i].mType != eXrefEntryUndefined)
			mXrefTable[i] =	inTableToMerge[i];
	}
}


EStatusCode PDFParser::ParseFileDirectory()
{
	EStatusCode status = PDFHummus::eSuccess;


	MovePositionInStream(mLastXrefPosition);

	do
	{
		// take the object, so that we can check whether this is an Xref or an Xref stream
		RefCountPtr<PDFObject> anObject(mObjectParser.ParseNewObject(mParserExtender));
		if(!anObject)
		{
			status = PDFHummus::eFailure;
			break;
		}

		if(anObject->GetType() == PDFObject::ePDFObjectSymbol && ((PDFSymbol*)anObject.GetPtr())->GetValue() == scXref)
		{
			// this would be a normal xref case
			// jump lines till you get to a line where the token is "trailer". then parse.
			status = ParseTrailerDictionary();
			if(status != PDFHummus::eSuccess)
				break;

			status = BuildXrefTableFromTable();
			if(status != PDFHummus::eSuccess)
				break;
		}
		else if(anObject->GetType() == PDFObject::ePDFObjectInteger && ((PDFInteger*)anObject.GetPtr())->GetValue() > 0)
		{
			// Xref stream case
			status = BuildXrefTableAndTrailerFromXrefStream(((PDFInteger*)anObject.GetPtr())->GetValue());
			if(status != PDFHummus::eSuccess)
				break;

		}
		else
		{
			TRACE_LOG("PDFParser::ParseFileDirectory,Unexpected object at xref start");
			status = eFailure;
		}


	}while(false);



	return status;
}

EStatusCode PDFParser::BuildXrefTableAndTrailerFromXrefStream(long long inXrefStreamObjectID)
{
	// xref stream is trailer and stream togather. need to parse them both.
	// the object parser is now after the object ID. so verify that next we goot a version and the obj keyword
	// then parse the xref stream
	EStatusCode status = PDFHummus::eSuccess;
	
	PDFObjectCastPtr<PDFInteger> versionObject(mObjectParser.ParseNewObject(mParserExtender));

	do
	{
		if(!versionObject)
		{
			TRACE_LOG("PDFParser::BuildXrefTableAndTrailerFromXrefStream, failed to read xref object declaration, Version");
			status = PDFHummus::eFailure;
			break;
		}


		if(mParserExtender)
			mParserExtender->OnObjectStart(inXrefStreamObjectID,versionObject->GetValue());

		PDFObjectCastPtr<PDFSymbol> objKeyword(mObjectParser.ParseNewObject(mParserExtender));

		if(!objKeyword)
		{
			TRACE_LOG("PDFParser::BuildXrefTableAndTrailerFromXrefStream, failed to read xref object declaration, obj keyword");
			status = PDFHummus::eFailure;
			break;
		}

		if(objKeyword->GetValue() != scObj)
		{
			TRACE_LOG1("PDFParser::BuildXrefTableAndTrailerFromXrefStream, failed to read xref object declaration, expected obj keyword found %s",
				objKeyword->GetValue().c_str());
			status = PDFHummus::eFailure;
			break;
		}

		// k. now just parse the object which should be a stream
		PDFObjectCastPtr<PDFStreamInput> xrefStream(mObjectParser.ParseNewObject(mParserExtender));
		if(!xrefStream)
		{
			TRACE_LOG("PDFParser::BuildXrefTableAndTrailerFromXrefStream, failure to parse xref stream");
			status = PDFHummus::eFailure;
			break;
		}

		if(mParserExtender)
			mParserExtender->OnObjectEnd(xrefStream.GetPtr());

		RefCountPtr<PDFDictionary> xrefDictionary(xrefStream->QueryStreamDictionary());
		mTrailer = xrefDictionary;

		status = DetermineXrefSize();
		if(status != PDFHummus::eSuccess)
			break;

		status = InitializeXref();
		if(status != PDFHummus::eSuccess)
			break;

		if(mTrailer->Exists("Prev"))
		{
			status = ParsePreviousXrefs(mTrailer.GetPtr());
			if(status != PDFHummus::eSuccess)
				break;
		}

        XrefEntryInput* extendedTable = NULL;
        ObjectIDType extendedTableSize;
		status = ParseXrefFromXrefStream(mXrefTable,mXrefSize,xrefStream.GetPtr(),&extendedTable,&extendedTableSize);
		if(status != PDFHummus::eSuccess)
			break;
        
        // Table may have been extended, in which case replace the pointer and current size
        if(extendedTable)
        {
            mXrefSize = extendedTableSize;
            delete[] mXrefTable;
            mXrefTable = extendedTable;
        }
        
	}while(false);

	return status;

}

EStatusCode PDFParser::ParseXrefFromXrefStream(XrefEntryInput* inXrefTable,
                                               ObjectIDType inXrefSize,
                                               LongFilePositionType inXrefPosition,
                                               XrefEntryInput** outExtendedTable,
                                               ObjectIDType* outExtendedTableSize)
{
	EStatusCode status = PDFHummus::eSuccess;
	
	MovePositionInStream(inXrefPosition);

	do
	{
		// take the object, so that we can check whether this is an Xref or an Xref stream
		PDFObjectCastPtr<PDFInteger> anObject(mObjectParser.ParseNewObject(mParserExtender));
		if(!anObject || anObject->GetValue() <= 0)
		{
			TRACE_LOG1("PDFParser::ParseXrefFromXrefStream, expecting object number for xref stream at %ld",inXrefPosition);
			status = PDFHummus::eFailure;
			break;
		}

		PDFObjectCastPtr<PDFInteger> versionObject(mObjectParser.ParseNewObject(mParserExtender));

		if(!versionObject)
		{
			TRACE_LOG("PDFParser::ParseXrefFromXrefStream, failed to read xref object declaration, Version");
			status = PDFHummus::eFailure;
			break;
		}

		if(mParserExtender)
			mParserExtender->OnObjectStart(anObject->GetValue(),versionObject->GetValue());

		PDFObjectCastPtr<PDFSymbol> objKeyword(mObjectParser.ParseNewObject(mParserExtender));

		if(!objKeyword)
		{
			TRACE_LOG("PDFParser::ParseXrefFromXrefStream, failed to read xref object declaration, obj keyword");
			status = PDFHummus::eFailure;
			break;
		}

		if(objKeyword->GetValue() != scObj)
		{
			TRACE_LOG1("PDFParser::ParseXrefFromXrefStream, failed to read xref object declaration, expected obj keyword found %s",
				objKeyword->GetValue().c_str());
			status = PDFHummus::eFailure;
			break;
		}

		PDFObjectCastPtr<PDFStreamInput> xrefStream(mObjectParser.ParseNewObject(mParserExtender));
		if(!xrefStream)
		{
			TRACE_LOG("PDFParser::ParseXrefFromXrefStream, failure to parse xref stream");
			status = PDFHummus::eFailure;
			break;
		}

		if(mParserExtender)
			mParserExtender->OnObjectEnd(xrefStream.GetPtr());

		status = ParseXrefFromXrefStream(inXrefTable,inXrefSize,xrefStream.GetPtr(),outExtendedTable,outExtendedTableSize);
	}while(false);
	return status;
}

EStatusCode PDFParser::ParseXrefFromXrefStream(XrefEntryInput* inXrefTable,
                                               ObjectIDType inXrefSize,
                                               PDFStreamInput* inXrefStream,
                                               XrefEntryInput** outExtendedTable,
                                               ObjectIDType* outExtendedTableSize)
{
	// 1. Setup the stream to read from the stream start location
	// 2. Set it up with an input stream to decode if required
	// 3. if there are subsections, loop them, otherwise assume a single section of 0..size
	// 4. for each subsection use the base number as starting, and count as well, to read the stream entries to the right position in the table
	//    The entries are read using the "W" value. make sure to read even values that you don't need.

	EStatusCode status = PDFHummus::eSuccess;
    
    outExtendedTable = NULL;

	IByteReader* xrefStreamSource = CreateInputStreamReader(inXrefStream);
	int* widthsArray = NULL;
	
	do
	{
		if(!xrefStreamSource)
		{
			status = PDFHummus::eFailure;
			break;
		}

		RefCountPtr<PDFDictionary> streamDictionary(inXrefStream->QueryStreamDictionary());
	
		// setup w array
		PDFObjectCastPtr<PDFArray> wArray(QueryDictionaryObject(streamDictionary.GetPtr(),"W"));
		if(!wArray)
		{
			TRACE_LOG("PDFParser::ParseXrefFromXrefStream, W array not available. failing");
			status = PDFHummus::eFailure;
			break;
		}

		widthsArray = new int[wArray->GetLength()];
		for(unsigned long i=0;i <wArray->GetLength();++i)
		{
			PDFObjectCastPtr<PDFInteger> widthObject(wArray->QueryObject(i));
			if(!widthObject)
			{
				TRACE_LOG("PDFParser::ParseXrefFromXrefStream, wrong items in width array (supposed to have only integers)");
				status = PDFHummus::eFailure;
				break;
			}
			widthsArray[i] = (int)widthObject->GetValue();
		}
		if(status != PDFHummus::eSuccess)
			break;

		// read the segments from the stream
		PDFObjectCastPtr<PDFArray> subsectionsIndex(QueryDictionaryObject(streamDictionary.GetPtr(),"Index"));
		MovePositionInStream(inXrefStream->GetStreamContentStart());

		if(!subsectionsIndex)
		{
			PDFObjectCastPtr<PDFInteger> xrefSize(QueryDictionaryObject(streamDictionary.GetPtr(),"Size"));	
			if(!xrefSize)
			{
				TRACE_LOG("PDFParser::ParseXrefFromXrefStream, xref size does not exist for this stream");
				status = PDFHummus::eFailure;
				break;
			}

            // if reading objects past expected range interesting consult policy
            ObjectIDType readXrefSize = (ObjectIDType)xrefSize->GetValue();
            if(readXrefSize > inXrefSize)
            {
                if(mAllowExtendingSegments)
                {
                    inXrefTable = ExtendXrefTableToSize(inXrefTable,inXrefSize,readXrefSize);
                    inXrefSize = readXrefSize;
                    if(*outExtendedTable)
                        delete[] *outExtendedTable;
                    *outExtendedTable = inXrefTable;
                    *outExtendedTableSize = readXrefSize;
                }
                else
                    break;
            }
			status = ReadXrefStreamSegment(inXrefTable,0,readXrefSize,xrefStreamSource,widthsArray,wArray->GetLength());
		}
		else
		{
			SingleValueContainerIterator<PDFObjectVector> segmentsIterator  = subsectionsIndex->GetIterator();
			PDFObjectCastPtr<PDFInteger> segmentValue;
			while(segmentsIterator.MoveNext() && PDFHummus::eSuccess == status)
			{
				segmentValue = segmentsIterator.GetItem();
				if(!segmentValue)
				{
					TRACE_LOG("PDFParser::ParseXrefFromXrefStream, found non integer value in Index array of xref stream");
					status = PDFHummus::eFailure;
					break;
				}
				ObjectIDType startObject = (ObjectIDType)segmentValue->GetValue();
				if(!segmentsIterator.MoveNext())
				{
					TRACE_LOG("PDFParser::ParseXrefFromXrefStream,Index array of xref stream should have an even number of values");
					status = PDFHummus::eFailure;
					break;
				}
				
				segmentValue = segmentsIterator.GetItem();
				if(!segmentValue)
				{
					TRACE_LOG("PDFParser::ParseXrefFromXrefStream, found non integer value in Index array of xref stream");
					status = PDFHummus::eFailure;
					break;
				}
				ObjectIDType objectsCount = (ObjectIDType)segmentValue->GetValue();
				// if reading objects past expected range interesting consult policy
				if(startObject +  objectsCount > inXrefSize)
                {
                    if(mAllowExtendingSegments)
                    {
                        inXrefTable = ExtendXrefTableToSize(inXrefTable,inXrefSize,startObject +  objectsCount);
                        inXrefSize = startObject +  objectsCount;
                        if(*outExtendedTable)
                            delete[] *outExtendedTable;
                        *outExtendedTable = inXrefTable;
                        *outExtendedTableSize = startObject +  objectsCount;
                    }
                    else
                        break;
                }
				status = ReadXrefStreamSegment(inXrefTable,startObject,std::min<ObjectIDType>(objectsCount,inXrefSize - startObject),xrefStreamSource,widthsArray,wArray->GetLength());
			}
		}
	}while(false);

	delete xrefStreamSource;
	delete[] widthsArray;
	return status;
}

void PDFParser::MovePositionInStream(LongFilePositionType inPosition)
{
	mStream->SetPosition(inPosition);
	mObjectParser.ResetReadState();
}

EStatusCode PDFParser::ReadXrefStreamSegment(XrefEntryInput* inXrefTable,
											 ObjectIDType inSegmentStartObject,
											 ObjectIDType inSegmentCount,
											 IByteReader* inReadFrom,
											 int* inEntryWidths,
											 unsigned long inEntryWidthsSize)
{
	ObjectIDType objectToRead = inSegmentStartObject;
	EStatusCode status = PDFHummus::eSuccess;
	if(inEntryWidthsSize != 3)
	{
		TRACE_LOG("PDFParser::ReadXrefStreamSegment, can handle only 3 length entries");
		return PDFHummus::eFailure;
	}

	// Note - i'm also checking that the stream is not ended. in non-finite segments, it could be that the particular
	// stream does no define all objects...just the "updated" ones
	for(; (objectToRead < inSegmentStartObject + inSegmentCount) && PDFHummus::eSuccess == status && inReadFrom->NotEnded();++objectToRead)
	{
		long long entryType;
		status = ReadXrefSegmentValue(inReadFrom,inEntryWidths[0],entryType);
		if(status != PDFHummus::eSuccess)
			break;
		status = ReadXrefSegmentValue(inReadFrom,inEntryWidths[1],inXrefTable[objectToRead].mObjectPosition);
		if(status != PDFHummus::eSuccess)
			break;
		status = ReadXrefSegmentValue(inReadFrom,inEntryWidths[2],inXrefTable[objectToRead].mRivision);
		if(status != PDFHummus::eSuccess)
			break;

		if(0 == entryType)
		{
			inXrefTable[objectToRead].mType = eXrefEntryDelete;
		}
		else if (1 == entryType)
		{
			inXrefTable[objectToRead].mType = eXrefEntryExisting;
		}
		else if(2 == entryType)
		{
			inXrefTable[objectToRead].mType = eXrefEntryStreamObject;
		}
		else
		{
			TRACE_LOG("PDFParser::ReadXrefStreamSegment, unfamiliar entry type. must be either 0,1 or 2");
			status = PDFHummus::eFailure;
		}
	}
	return status;
}

EStatusCode PDFParser::ReadXrefSegmentValue(IByteReader* inSource,int inEntrySize,long long& outValue)
{
	outValue = 0;
	Byte buffer;
	EStatusCode status = PDFHummus::eSuccess;

	for(int i=0;i<inEntrySize && PDFHummus::eSuccess == status;++i)
	{
		status = (inSource->Read(&buffer,1) == 1 ? PDFHummus::eSuccess : PDFHummus::eFailure);
		if(status != PDFHummus::eFailure)
			outValue = (outValue<<8) + buffer;
	}
	return status;
}

EStatusCode PDFParser::ReadXrefSegmentValue(IByteReader* inSource,int inEntrySize,ObjectIDType& outValue)
{
	outValue = 0;
	Byte buffer;
	EStatusCode status = PDFHummus::eSuccess;

	for(int i=0;i<inEntrySize && PDFHummus::eSuccess == status;++i)
	{
		status = (inSource->Read(&buffer,1) == 1 ? PDFHummus::eSuccess : PDFHummus::eFailure);
		if(status != PDFHummus::eFailure)
			outValue = (outValue<<8) + buffer;
	}
	return status;
}

PDFObject* PDFParser::ParseExistingInDirectStreamObject(ObjectIDType inObjectId)
{
	// parsing an object in an object stream requires the following:
	// 1. Setting the position to this object stream
	// 2. Reading the stream First and N. store.
	// 3. Creating a stream reader for the initial stream position and length, possibly decoding with flate
	// 4. Read the stream header. store.
	// 5. Jump to the right object position (or decode till its position)
	// 6. Read the object

	EStatusCode status = PDFHummus::eSuccess;
	ObjectStreamHeaderEntry* objectStreamHeader;
	IByteReader* objectSource = NULL;

	InputStreamSkipperStream skipperStream;
	ObjectIDType objectStreamID;
	PDFObject* anObject = NULL;

	do
	{
		objectStreamID = (ObjectIDType)mXrefTable[inObjectId].mObjectPosition;
		PDFObjectCastPtr<PDFStreamInput> objectStream(ParseNewObject(objectStreamID));
		if(!objectStream)
		{
			TRACE_LOG2("PDFParser::ParseExistingInDirectStreamObject, failed to parse object %ld. failed to find object stream for it, which should be %ld",
						inObjectId,mXrefTable[inObjectId].mObjectPosition);
			status = PDFHummus::eFailure;
			break;
		}

		RefCountPtr<PDFDictionary> streamDictionary(objectStream->QueryStreamDictionary());

		PDFObjectCastPtr<PDFInteger> streamObjectsCount(QueryDictionaryObject(streamDictionary.GetPtr(),"N"));
		if(!streamObjectsCount)
		{
			TRACE_LOG1("PDFParser::ParseExistingInDirectStreamObject, no N key in stream dictionary %ld",objectStreamID);
			status = PDFHummus::eFailure;
			break;
		}
		ObjectIDType objectsCount = (ObjectIDType)streamObjectsCount->GetValue();

		PDFObjectCastPtr<PDFInteger> firstStreamObjectPosition(QueryDictionaryObject(streamDictionary.GetPtr(),"First"));
		if(!streamObjectsCount)
		{
			TRACE_LOG1("PDFParser::ParseExistingInDirectStreamObject, no First key in stream dictionary %ld",objectStreamID);
			status = PDFHummus::eFailure;
			break;
		}		

		objectSource = CreateInputStreamReader(objectStream.GetPtr());
		skipperStream.Assign(objectSource);
		MovePositionInStream(objectStream->GetStreamContentStart());

		mObjectParser.SetReadStream(&skipperStream,&skipperStream);

		ObjectIDTypeToObjectStreamHeaderEntryMap::iterator it = mObjectStreamsCache.find(objectStreamID);
		
		if(it == mObjectStreamsCache.end())
		{
			objectStreamHeader = new ObjectStreamHeaderEntry[objectsCount];
			status = ParseObjectStreamHeader(objectStreamHeader,objectsCount);
			if(status != PDFHummus::eSuccess)
			{
				delete[] objectStreamHeader;
				break;
			}
			it = mObjectStreamsCache.insert(ObjectIDTypeToObjectStreamHeaderEntryMap::value_type(objectStreamID,objectStreamHeader)).first;
		}
		objectStreamHeader = it->second;

		// verify that i got the right object ID
		if(objectsCount <= mXrefTable[inObjectId].mRivision || objectStreamHeader[mXrefTable[inObjectId].mRivision].mObjectNumber != inObjectId)
		{
			TRACE_LOG2("PDFParser::ParseXrefFromXrefStream, wrong object. expecting to find object ID %ld, and found %ld",
						inObjectId,
						objectsCount <= mXrefTable[inObjectId].mRivision ? 
							-1 :
							objectStreamHeader[mXrefTable[inObjectId].mRivision].mObjectNumber);
			status = PDFHummus::eFailure;
			break;
		}

		// when parsing the header, should be at position already..so don't skip if already there [using GetCurrentPosition to see if parsed some]
		if(mXrefTable[inObjectId].mRivision != 0 || skipperStream.GetCurrentPosition() == 0)
		{
			LongFilePositionType objectPositionInStream = objectStreamHeader[mXrefTable[inObjectId].mRivision].mObjectOffset + 
														  firstStreamObjectPosition->GetValue();
			skipperStream.SkipTo(objectPositionInStream);
			mObjectParser.ResetReadState();
		}
		
		anObject = mObjectParser.ParseNewObject(mParserExtender);

	}while(false);

	mObjectParser.SetReadStream(mStream,&mCurrentPositionProvider);

	if(PDFHummus::eSuccess == status)
	{
		return anObject;
	}
	else
	{
		if(anObject)
			anObject->Release();
		return NULL;
	}
}

EStatusCode PDFParser::ParseObjectStreamHeader(ObjectStreamHeaderEntry* inHeaderInfo,ObjectIDType inObjectsCount)
{
	ObjectIDType currentObject = 0;
	EStatusCode status = PDFHummus::eSuccess;

	while(currentObject < inObjectsCount && (PDFHummus::eSuccess == status))
	{
		PDFObjectCastPtr<PDFInteger> objectNumber(mObjectParser.ParseNewObject(mParserExtender));
		if(!objectNumber)
		{
			TRACE_LOG("PDFParser::ParseObjectStreamHeader, parsing failed when reading object number. either not enough objects, or of the wrong type");
			status = PDFHummus::eFailure;
			break;
		}

		PDFObjectCastPtr<PDFInteger> objectPosition(mObjectParser.ParseNewObject(mParserExtender));
		if(!objectPosition)
		{
			TRACE_LOG("PDFParser::ParseObjectStreamHeader, parsing failed when reading object position. either not enough objects, or of the wrong type");
			status = PDFHummus::eFailure;
			break;
		}
		inHeaderInfo[currentObject].mObjectNumber = (ObjectIDType)(objectNumber->GetValue());
		inHeaderInfo[currentObject].mObjectOffset = objectPosition->GetValue();
		++currentObject;
	}
	return status;
}

IByteReader* PDFParser::CreateInputStreamReader(PDFStreamInput* inStream)
{
	RefCountPtr<PDFDictionary> streamDictionary(inStream->QueryStreamDictionary());
	IByteReader* result = NULL;
	EStatusCode status = PDFHummus::eSuccess;

	do
	{

		// setup stream according to length and possible filter
		PDFObjectCastPtr<PDFInteger> lengthObject(QueryDictionaryObject(streamDictionary.GetPtr(),"Length"));	
		if(!lengthObject)
		{
			TRACE_LOG("PDFParser::CreateInputStreamReader, stream does not have length, failing");
			status = PDFHummus::eFailure;
			break;
		}	

		result = new InputLimitedStream(mStream,lengthObject->GetValue(),false);

		// call for parser extender for encryption implementation
		if(mParserExtender)
			result = mParserExtender->CreateDecryptionFilterForStream(result);

		RefCountPtr<PDFObject> filterObject(QueryDictionaryObject(streamDictionary.GetPtr(),"Filter"));
		if(!filterObject)
		{
			// no filter, so stop here
			break;
		} 

		if(filterObject->GetType() == PDFObject::ePDFObjectArray)
		{
			PDFArray* filterObjectArray = (PDFArray*)filterObject.GetPtr();
			PDFObjectCastPtr<PDFArray> decodeParams(QueryDictionaryObject(streamDictionary.GetPtr(),"DecodeParms"));
			for(unsigned long i=0; i < filterObjectArray->GetLength() && eSuccess == status;++i)
			{
				PDFObjectCastPtr<PDFName> filterObjectItem(filterObjectArray->QueryObject(i));
				if(!filterObjectItem)
				{
					TRACE_LOG("PDFParser::CreateInputStreamReader, filter item in an array is not a name. should be a name");
					status = PDFHummus::eFailure;
					break;
				}

				EStatusCodeAndIByteReader createStatus;
				if(!decodeParams)
				{
					 createStatus = CreateFilterForStream(result,filterObjectItem.GetPtr(), NULL);
				}
				else
				{
					PDFObjectCastPtr<PDFDictionary> decodeParamsItem(QueryArrayObject(decodeParams.GetPtr(),i));

					createStatus = CreateFilterForStream(result,(PDFName*)filterObject.GetPtr(), !decodeParamsItem ? NULL: decodeParamsItem.GetPtr());
				}

				if(createStatus.first != eSuccess)
				{
					status = PDFHummus::eFailure;
					break;
				}
				else
					result = createStatus.second;
			}
		}
		else if(filterObject->GetType() == PDFObject::ePDFObjectName)
		{
			PDFObjectCastPtr<PDFDictionary> decodeParams(QueryDictionaryObject(streamDictionary.GetPtr(),"DecodeParms"));

			EStatusCodeAndIByteReader createStatus = CreateFilterForStream(result,(PDFName*)filterObject.GetPtr(), !decodeParams ? NULL: decodeParams.GetPtr());
			if(createStatus.first != eSuccess)
			{
				status = PDFHummus::eFailure;
				break;
			}
			else
				result = createStatus.second;

		}
		else
		{
			TRACE_LOG("PDFParser::CreateInputStreamReader, filter parameter is of unkown type. only array and name are supported.");
			status = PDFHummus::eFailure;
			break;			
		}
		
	}while(false);


	if(status != PDFHummus::eSuccess)
	{
		delete result;
		result = NULL;
	}
	return result;
}

EStatusCodeAndIByteReader PDFParser::CreateFilterForStream(IByteReader* inStream,PDFName* inFilterName,PDFDictionary* inDecodeParams)
{
	EStatusCode status = eSuccess;
	IByteReader* result = NULL;

	do
	{

		if(inFilterName->GetValue() == "FlateDecode")
		{
			InputFlateDecodeStream* flateStream;
			flateStream = new InputFlateDecodeStream(NULL); // assigning null, so later delete, if failure occurs won't delete the input stream
			result = flateStream;

			// check for predictor n' such
			if(!inDecodeParams)
			{
				// no predictor, stop here
				flateStream->Assign(inStream);
				break;
			}

			// read predictor, and apply the relevant predictor function
			PDFObjectCastPtr<PDFInteger> predictor(QueryDictionaryObject(inDecodeParams,"Predictor"));
			
			if(!predictor || predictor->GetValue() == 1)
			{
				// no predictor or default, stop here
				flateStream->Assign(inStream);
				break;
			}

			PDFObjectCastPtr<PDFInteger> columns(QueryDictionaryObject(inDecodeParams,"Columns"));
			LongBufferSizeType columnsValue = columns.GetPtr() ? 
																(IOBasicTypes::LongBufferSizeType)columns->GetValue() :
																1;

			switch(predictor->GetValue())
			{
				case 2:
				{
					PDFObjectCastPtr<PDFInteger> colors(QueryDictionaryObject(inDecodeParams,"Colors"));
					PDFObjectCastPtr<PDFInteger> bitsPerComponent(QueryDictionaryObject(inDecodeParams,"BitsPerComponent"));
					result = new InputPredictorTIFFSubStream(result,
															 colors.GetPtr() ? 
																(IOBasicTypes::LongBufferSizeType)colors->GetValue() :
																1,
															 bitsPerComponent.GetPtr() ?
																(IOBasicTypes::Byte)colors->GetValue() :
																8,
																columnsValue);
					break;
				}
				case 10:
				{
					result = new InputPredictorPNGNoneStream(result,columnsValue);
					break;
				}
				case 11:
				{
					result = new InputPredictorPNGSubStream(result,columnsValue);
					break;
				}
				case 12:
				{

					result =  new InputPredictorPNGUpStream(result,columnsValue);
					break;
				}
				case 13:
				{

					result =  new InputPredictorPNGAverageStream(result,columnsValue);
					break;
				}
				case 14:
				{
					result =  new InputPredictorPNGPaethStream(result,columnsValue);
					break;
				}
				case 15:
				{
					result =  new InputPredictorPNGOptimumStream(result,columnsValue);
					break;
				}
				default:
				{
					TRACE_LOG("PDFParser::CreateFilterForStream, supporting only predictor of types 1,2,10,11,12,13,14,15, failing");
					status = PDFHummus::eFailure;
					break;
				}
			}
			flateStream->Assign(inStream);
		}
		else if(inFilterName->GetValue() == "ASCII85Decode")
		{
			result = new InputAscii85DecodeStream(inStream);
		}
#ifndef PDFHUMMUS_NO_DCT
        else if(inFilterName->GetValue() == "DCTDecode")
        {
            result = new InputDCTDecodeStream(inStream);
        }
#endif
		else if(mParserExtender)
		{
			result = mParserExtender->CreateFilterForStream(inStream,inFilterName,inDecodeParams);
			if(result == inStream)
			{
				TRACE_LOG1("PDFParser::CreateFilterForStream, filter is not supported by extender - %s",inFilterName->GetValue().c_str());
				status = PDFHummus::eFailure;
				break;
			}
		}
		else
		{
			TRACE_LOG("PDFParser::CreateFilterForStream, supporting only flate decode and ascii 85 decode, failing");
			status = PDFHummus::eFailure;
			break;
		}
	}while(false);

	if(status != PDFHummus::eSuccess)
	{
		delete result;
		result = NULL;
	}
	return EStatusCodeAndIByteReader(status,result);

}

IByteReader* PDFParser::StartReadingFromStream(PDFStreamInput* inStream)
{
    IByteReader* result = CreateInputStreamReader(inStream);
    if(result)
        MovePositionInStream(inStream->GetStreamContentStart());
    return result;
}

EStatusCode PDFParser::StartStateFileParsing(IByteReaderWithPosition* inSourceStream)
{
	EStatusCode status;

	ResetParser();

	mStream = inSourceStream;
	mCurrentPositionProvider.Assign(mStream);
	mObjectParser.SetReadStream(inSourceStream,&mCurrentPositionProvider);

	do
	{
		// initialize reading from end
		mLastReadPositionFromEnd = 0;
		mEncounteredFileStart = false;
		mLastAvailableIndex = mCurrentBufferIndex = mLinesBuffer;

		status = ParseEOFLine();
		if(status != PDFHummus::eSuccess)
			break;

		status = ParseLastXrefPosition();
		if(status != PDFHummus::eSuccess)
			break;

		status = ParseFileDirectory(); // that would be the xref and trailer
		if(status != PDFHummus::eSuccess)
			break;

	}while(false);

	return status;	
}

bool PDFParser::IsEncrypted()
{
	PDFObjectCastPtr<PDFDictionary> encryptionDictionary(QueryDictionaryObject(mTrailer.GetPtr(),"Encrypt"));
	return encryptionDictionary.GetPtr() != NULL ;
}

void PDFParser::SetParserExtender(IPDFParserExtender* inParserExtender)
{
	mParserExtender = inParserExtender;
}

bool PDFParser::IsEncryptionSupported()
{
	return mParserExtender && mParserExtender->DoesSupportEncryption();
}

ObjectIDType PDFParser::GetXrefSize()
{
    return mXrefSize;
}

XrefEntryInput* PDFParser::GetXrefEntry(ObjectIDType inObjectID)
{
    return (inObjectID < mXrefSize) ? mXrefTable+inObjectID : NULL;
}

LongFilePositionType PDFParser::GetXrefPosition()
{
    return mLastXrefPosition;
}

IByteReaderWithPosition* PDFParser::GetParserStream()
{
    return mStream;
}
