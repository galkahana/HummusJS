/*
   Source File : Trace.cpp


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
#include "Trace.h"
#include "Log.h"
#include "SafeBufferMacrosDefs.h"

#include <stdio.h>
#include <stdarg.h>

Trace& Trace::DefaultTrace(){
	static Trace default_trace;
	return default_trace;
}

Trace::Trace(void)
{
	mLog = NULL;
	mLogFilePath = "Log.txt";
	mShouldLog = false;
}

Trace::~Trace(void)
{
	delete mLog;
}

void Trace::SetLogSettings(const std::string& inLogFilePath,bool inShouldLog,bool inPlaceUTF8Bom)
{
	mShouldLog = inShouldLog;
	mPlaceUTF8Bom = inPlaceUTF8Bom;
	mLogFilePath = inLogFilePath;
	mLogStream = NULL;
	if(mLog != NULL)
	{
		delete mLog;
		mLog = NULL;
		//if(mShouldLog)
		//	mLog = new Log(mLogFilePath,inPlaceUTF8Bom);
	}
}

void Trace::SetLogSettings(IByteWriter* inLogStream,bool inShouldLog)
{
	mShouldLog = inShouldLog;
	mLogStream = inLogStream;
	mPlaceUTF8Bom = false;
	if(mLog != NULL)
	{
		delete mLog;
		mLog = NULL;
		if(mShouldLog)
			mLog = new Log(mLogStream);
	}
}


void Trace::TraceToLog(const char* inFormat,...)
{
	if(mShouldLog)
	{
		if(NULL == mLog)
		{
			if(mLogStream)
				mLog = new Log(mLogStream);
			else
				mLog = new Log(mLogFilePath,mPlaceUTF8Bom);
		}

		va_list argptr;
		va_start(argptr, inFormat);

		SAFE_VSPRINTF(mBuffer, MAX_TRACE_SIZE,inFormat,argptr);
		va_end(argptr);

		mLog->LogEntry(std::string(mBuffer));
	}
}

void Trace::TraceToLog(const char* inFormat,va_list inList)
{
	if(mShouldLog)
	{
		if(NULL == mLog)
		{
			if(mLogStream)
				mLog = new Log(mLogStream);
			else
				mLog = new Log(mLogFilePath,mPlaceUTF8Bom);
		}

		SAFE_VSPRINTF(mBuffer, MAX_TRACE_SIZE,inFormat,inList);

		mLog->LogEntry(std::string(mBuffer));
	}

}

