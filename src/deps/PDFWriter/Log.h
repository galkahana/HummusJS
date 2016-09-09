/*
   Source File : Log.h


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

#include "IOBasicTypes.h"
#include "OutputFile.h"

#include <string>


using namespace IOBasicTypes;

class Log;

typedef void (*LogFileMethod)(Log* inThis,const Byte* inMessage, LongBufferSizeType inMessageSize);

class IByteWriter;

class Log
{
public:

	// log writes are in UTF8. so i'm asking here if you want a bom
	Log(const std::string& inLogFilePath,bool inPlaceUTF8Bom);
	Log(IByteWriter* inLogStream);
	~Log(void);

	void LogEntry(const std::string& inMessage);
	void LogEntry(const Byte* inMessage, LongBufferSizeType inMessageSize);


	// don't use
	void LogEntryToFile(const Byte* inMessage, LongBufferSizeType inMessageSize);
	void LogEntryToStream(const Byte* inMessage, LongBufferSizeType inMessageSize);

private:

	std::string mFilePath;
	OutputFile mLogFile;
	IByteWriter* mLogStream;
	LogFileMethod mLogMethod;

	std::string GetFormattedTimeString();
	void WriteLogEntryToStream(const Byte* inMessage, LongBufferSizeType inMessageSize,IByteWriter* inStream);
};
