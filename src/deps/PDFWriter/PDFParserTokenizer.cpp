/*
   Source File : PDFParserTokenizer.cpp


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
#include "PDFParserTokenizer.h"
#include "IByteReader.h"
#include "OutputStringBufferStream.h"
#include "Trace.h"

using namespace PDFHummus;
using namespace IOBasicTypes;

PDFParserTokenizer::PDFParserTokenizer(void)
{
	mStream = NULL;
	ResetReadState();
}

PDFParserTokenizer::~PDFParserTokenizer(void)
{
}

void PDFParserTokenizer::SetReadStream(IByteReader* inSourceStream)
{
	mStream = inSourceStream;
	ResetReadState();
}

void PDFParserTokenizer::ResetReadState()
{
	mHasTokenBuffer = false;
	mStreamPositionTracker = 0;
	mRecentTokenPosition = 0;
}


void PDFParserTokenizer::ResetReadState(const PDFParserTokenizer& inExternalTokenizer)
{
	mTokenBuffer = inExternalTokenizer.mTokenBuffer;
	mHasTokenBuffer = inExternalTokenizer.mHasTokenBuffer;
	mStreamPositionTracker = inExternalTokenizer.mStreamPositionTracker;
	mRecentTokenPosition = inExternalTokenizer.mRecentTokenPosition;
}

static const Byte scBackSlash[] = {'\\'};
static const std::string scStream = "stream";
static const char scCR = '\r';
static const char scLF = '\n';
BoolAndString PDFParserTokenizer::GetNextToken()
{
	BoolAndString result;
	Byte buffer;
	OutputStringBufferStream tokenBuffer;
	
	if(!CanGetNextByte())
	{
		result.first = false;
		return result;
	}

	do
	{
		SkipTillToken();
		if(!CanGetNextByte())
		{
			result.first = false;
			break;
		}


		// before reading the first byte save the token position, for external queries
		mRecentTokenPosition = mStreamPositionTracker;

		// get the first byte of the token
		if(GetNextByteForToken(buffer) != PDFHummus::eSuccess)
		{
			result.first = false;
			break;
		}
		tokenBuffer.Write(&buffer,1);

		result.first = true; // will only be changed to false in case of read error

		// now determine how to continue based on the first byte of the token (there are some special cases)
		switch(buffer)
		{
		
			case '%':
			{
				// for a comment, the token goes on till the end of line marker [not including]
				while(CanGetNextByte())
				{
					if(GetNextByteForToken(buffer) != PDFHummus::eSuccess)
					{	
						result.first = !CanGetNextByte();
						break;
					}
					if(0xD == buffer|| 0xA == buffer)
						break;
					tokenBuffer.Write(&buffer,1);
				}
				result.second = tokenBuffer.ToString();
				break;
			}

			case '(':
			{
				// for a literal string, the token goes on until the balanced-closing right paranthesis
				int balanceLevel = 1;
				bool backSlashEncountered = false;
				while(balanceLevel > 0 && CanGetNextByte())
				{
					if(GetNextByteForToken(buffer) != PDFHummus::eSuccess)
					{	
						result.first = !CanGetNextByte();
						break;
					}
			
					if(backSlashEncountered)
					{
						backSlashEncountered = false;
						if(0xA == buffer || 0xD == buffer)
						{
							// ignore backslash and newline. might also need to read extra
							// for cr-ln
							if(0xD == buffer && CanGetNextByte())
							{
								if(GetNextByteForToken(buffer) != PDFHummus::eSuccess)
								{
									result.first = !CanGetNextByte();
									break;
								}
								if(buffer != 0xA)
									SaveTokenBuffer(buffer);
							}
						}
						else
						{
							tokenBuffer.Write(scBackSlash,1);					
							tokenBuffer.Write(&buffer,1);
						}
					}
					else
					{
						if('\\' == buffer)
						{
							backSlashEncountered = true; 
							continue;
						}
						else if('(' == buffer)
							++balanceLevel;
						else if(')' == buffer)
							--balanceLevel;
						tokenBuffer.Write(&buffer,1);
					}
				}
				if(result.first)
					result.second = tokenBuffer.ToString();
				break;
			}

			case '<':
			{
				// k. this might be a dictionary start marker or a hax string start. depending on whether it has a < following it or not

				// Hex string, read till end of hex string marker
				if(!CanGetNextByte())
				{
					result.second = tokenBuffer.ToString();
					break;
				}

				if(GetNextByteForToken(buffer) != PDFHummus::eSuccess)
				{	
						result.first = !CanGetNextByte();
						break;
				}

				if('<' == buffer)
				{
					// Dictionary start marker
					tokenBuffer.Write(&buffer,1);
					result.second = tokenBuffer.ToString();
					break;
				}
				else
				{
					// Hex string 

					tokenBuffer.Write(&buffer,1);

					while(CanGetNextByte() && buffer != '>')
					{
						if(GetNextByteForToken(buffer) != PDFHummus::eSuccess)
						{	
							result.first = !CanGetNextByte();
							break;
						}

						if(IsPDFWhiteSpace(buffer))
							continue;

						// verify that getting a hex char or ending char, so that we are sure there's no orcish mischief
						if(!(buffer>='0' && buffer<='9') &&
						   !(buffer>='A' && buffer<='F') &&
						   !(buffer>='a' && buffer<='f') &&
						   buffer != '>') {
							TRACE_LOG1("PDFParserTokenizer::GetNextToken, encountered non ascii char in hex string. probably a corruption. byte value = %d", buffer);
							result.first = false;
							break;
						}

						tokenBuffer.Write(&buffer,1);
					}
				}
				result.second = tokenBuffer.ToString();
				break;
			}
			case '[': // for all array or executable tokanizers, the tokanizer is just the mark
			case ']':
			case '{':
			case '}':
				result.second = tokenBuffer.ToString();
				break;
			case '>': // parse end dictionary marker as a single entity or a hex string end marker
			{
				if(!CanGetNextByte()) // this means a loose end string marker...wierd
				{
					result.second = tokenBuffer.ToString();
					break;
				}

				if(GetNextByteForToken(buffer) != PDFHummus::eSuccess)
				{	
					result.first = !CanGetNextByte();
					break;
				}

				if('>' == buffer)
				{
					tokenBuffer.Write(&buffer,1);
					result.second = tokenBuffer.ToString();
					break;
				}
				else
				{
					// hex string loose end
					SaveTokenBuffer(buffer);
					result.second = tokenBuffer.ToString();
					break;
				}

				break;
			}

			default: // regular token. read till next breaker or whitespace
			{
				while(CanGetNextByte())
				{
					if(GetNextByteForToken(buffer) != PDFHummus::eSuccess)
					{	
						result.first = !CanGetNextByte();
						break;
					}
					if(IsPDFWhiteSpace(buffer))
					{
						break;
					}
					else if(IsPDFEntityBreaker(buffer))
					{
						SaveTokenBuffer(buffer); // for a non-space breaker, save the token for next token read
						break;
					}
					else
						tokenBuffer.Write(&buffer,1);
				}
				result.second = tokenBuffer.ToString();
				
				if(result.first && CanGetNextByte() && scStream == result.second)
				{
					// k. a bit of a special case here for streams. the reading changes after the keyword "stream", 
					// essentially forcing the next content to start after either CR, CR-LF or LF. so there might be a little
					// skip to do here.
					// if indeed there's a "stream", so the last buffer read should have been either CR or LF, which means (respectively)
					// that we should either skip one more "LF" or do nothing (based on what was parsed)
					
					// verify that when whitespaces are finished buffer is either CR or LF, and behave accordingly
					while(CanGetNextByte()) {
						if (!IsPDFWhiteSpace(buffer)) {
							result.first = !CanGetNextByte(); // something wrong! not whitespace
							break;
						}

						if (scCR == buffer) // CR. should be CR-LF or CR alone
						{
							if (GetNextByteForToken(buffer) == PDFHummus::eSuccess)
							{
								// if CR-LF treat as a single line, otherwise put back token nicely cause CR is alone
								if (buffer != scLF)
									SaveTokenBuffer(buffer);
							}
							result.first = true;
							break;
						}
						else if (scLF == buffer) {
							result.first = true; 
							break;
						} // else - some other white space

						if (GetNextByteForToken(buffer) != PDFHummus::eSuccess) {
							result.first = !CanGetNextByte(); //can't read but not eof. fail
							break;
						}
					}
				}
				break;
			}

		}

	}while(false);



	return result;
}

void PDFParserTokenizer::SkipTillToken()
{
	Byte buffer = 0;

	// skip till hitting first non space, or segment end
	while(CanGetNextByte())
	{
		if(GetNextByteForToken(buffer) != PDFHummus::eSuccess)
			break;

		if(!IsPDFWhiteSpace(buffer))
		{
			SaveTokenBuffer(buffer);
			break;
		}
	}
}

bool PDFParserTokenizer::CanGetNextByte() {
	return !!mStream && (mHasTokenBuffer || mStream->NotEnded());
}

EStatusCode PDFParserTokenizer::GetNextByteForToken(Byte& outByte)
{
	++mStreamPositionTracker; // advance position tracker, because we are reading the next byte.
	if(mHasTokenBuffer)
	{
		outByte = mTokenBuffer;
		mHasTokenBuffer = false;
		return PDFHummus::eSuccess;
	}
	else
		return (mStream->Read(&outByte,1) != 1) ? PDFHummus::eFailure:PDFHummus::eSuccess;
}

static const Byte scWhiteSpaces[] = {0,0x9,0xA,0xC,0xD,0x20};
bool PDFParserTokenizer::IsPDFWhiteSpace(Byte inCharacter)
{
	bool isWhiteSpace = false;
	for(int i=0; i < 6 && !isWhiteSpace; ++i)
		isWhiteSpace =  (scWhiteSpaces[i] == inCharacter);
	return isWhiteSpace;
}

void PDFParserTokenizer::SaveTokenBuffer(Byte inToSave)
{
	mHasTokenBuffer = true;
	mTokenBuffer = inToSave;
	--mStreamPositionTracker; // decreasing position trakcer, because it is as if the byte is put back in the stream
}

IOBasicTypes::LongFilePositionType PDFParserTokenizer::GetReadBufferSize()
{
	return mHasTokenBuffer ? 1 : 0;
}

static const Byte scEntityBreakers[] = {'(',')','<','>',']','[','{','}','/','%'};
bool PDFParserTokenizer::IsPDFEntityBreaker(Byte inCharacter)
{
	bool isEntityBreak = false;
	for(int i=0; i < 10 && !isEntityBreak; ++i)
		isEntityBreak =  (scEntityBreakers[i] == inCharacter);
	return isEntityBreak;
}

LongFilePositionType PDFParserTokenizer::GetRecentTokenPosition()
{
	return mRecentTokenPosition;
}