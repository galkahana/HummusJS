/*
   Source File : Log.cpp


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
#include "Log.h"
#include "SafeBufferMacrosDefs.h"
#include "IByteWriterWithPosition.h"
#include <ctime>
#include <stdio.h>
#ifdef __MINGW32__
#include <share.h>
#endif

void STATIC_LogEntryToFile(Log* inThis,const Byte* inMessage, LongBufferSizeType inMessageSize)
{
	inThis->LogEntryToFile(inMessage,inMessageSize);
}

void STATIC_LogEntryToStream(Log* inThis,const Byte* inMessage, LongBufferSizeType inMessageSize)
{
	inThis->LogEntryToStream(inMessage,inMessageSize);
}

static const Byte scUTF8Bom[3] = {0xEF,0xBB,0xBF};

Log::Log(const std::string& inLogFilePath,bool inPlaceUTF8Bom)
{
	// check if file exists or not...if not, create new one and place a bom in its beginning
	FILE* logFile;
	bool exists;
	SAFE_FOPEN(logFile,inLogFilePath.c_str(),"r")
	exists = (logFile!= NULL);
	if(logFile)
		fclose(logFile);

	if(!exists)
	{
        // first, test if i can open the file. don't use mLogFile, becuase if file cannot be opened
        // will get into infinite recursion due to trying to log the fact that it cant open the file...
        SAFE_FOPEN(logFile,inLogFilePath.c_str(),"wb")
        if(logFile)
        {
            fclose(logFile); // continue with using the regular file implementation
            mLogFile.OpenFile(inLogFilePath);
            
            // put utf 8 header
            if(inPlaceUTF8Bom)
                mLogFile.GetOutputStream()->Write(scUTF8Bom,3);
            mLogFile.CloseFile();
            mFilePath = inLogFilePath;
        }
        else
        {
            mFilePath = "";
        }
	}
    else
        mFilePath = inLogFilePath;
    mLogStream = NULL;
    mLogMethod = STATIC_LogEntryToFile;
}

Log::Log(IByteWriter* inLogStream)
{
	// when writing stream, no bom is written. assuming that the input will take care, because most chances are that this is a non-file stream, and may be part of something else.
	mLogStream = inLogStream;
	mLogMethod = STATIC_LogEntryToStream;

}


Log::~Log(void)
{
}

void Log::LogEntry(const std::string& inMessage)
{
	LogEntry((const Byte*)inMessage.c_str(), inMessage.length());
}

void Log::LogEntry(const Byte* inMessage, LongBufferSizeType inMessageSize)
{
	mLogMethod(this,inMessage,inMessageSize);
}

static const Byte scEndLine[2] = {'\r','\n'};

void Log::LogEntryToFile(const Byte* inMessage, LongBufferSizeType inMessageSize)
{
    if(mFilePath.size() > 0)
    {
        mLogFile.OpenFile(mFilePath,true);
        WriteLogEntryToStream(inMessage,inMessageSize,mLogFile.GetOutputStream());
        mLogFile.CloseFile();
    }
}

void Log::LogEntryToStream(const Byte* inMessage, LongBufferSizeType inMessageSize)
{
    if(mLogStream)
        WriteLogEntryToStream(inMessage,inMessageSize,mLogStream);
}


void Log::WriteLogEntryToStream(const Byte* inMessage, LongBufferSizeType inMessageSize,IByteWriter* inStream)
{
	std::string formattedTimeString = GetFormattedTimeString();
	inStream->Write((const Byte*)formattedTimeString.c_str(),formattedTimeString.length());
	inStream->Write(inMessage,inMessageSize);
	inStream->Write(scEndLine,2);
}

std::string Log::GetFormattedTimeString()
{
	// create a local time string (date + time) that looks like this: "[ dd/mm/yyyy hh:mm:ss ] "
	char buffer[26];
	
	time_t currentTime;
	tm structuredLocalTime;

	time(&currentTime);
	SAFE_LOCAL_TIME(structuredLocalTime,currentTime);

	SAFE_SPRINTF_6(buffer,26,"[ %02d/%02d/%04d %02d:%02d:%02d ] ",structuredLocalTime.tm_mday,
																	structuredLocalTime.tm_mon + 1,
																	structuredLocalTime.tm_year + 1900,
																	structuredLocalTime.tm_hour,
																	structuredLocalTime.tm_min,
																	structuredLocalTime.tm_sec);
	return buffer;
}
