/*
   Source File : FreeTypeWrapper.cpp


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
#include "FreeTypeWrapper.h"
#include "Trace.h"
#include "InputFile.h"
#include "IByteReaderWithPosition.h"


using namespace PDFHummus;


#undef __FTERRORS_H__                                           
#define FT_ERRORDEF( e, v, s )  { e, s },                       
#define FT_ERROR_START_LIST     {                               
#define FT_ERROR_END_LIST       { 0, 0 } };                     
                                                             
static const struct                                                    
{                                                               
int          err_code;                                        
const char*  err_msg;                                         
} ft_errors[] =                                                 
                                                             
#include FT_ERRORS_H                                            

FreeTypeWrapper::FreeTypeWrapper(void)
{
	if(FT_Init_FreeType(&mFreeType))
	{
		TRACE_LOG("FreeTypeWrapper::FreeTypeWrapper, unexpected failure. failed to initialize Free Type");
		mFreeType = NULL;
	}
}

FreeTypeWrapper::~FreeTypeWrapper(void)
{
	FTFaceToFTStreamListMap::iterator it = mOpenStreams.begin();
	for (; it != mOpenStreams.end(); ++it)
	{
		FTStreamList::iterator itStreams = it->second.begin();
		for (; itStreams != it->second.end(); ++itStreams)
		{
			delete *itStreams;
		}
	}
	mOpenStreams.clear();
	if (mFreeType)
		FT_Done_FreeType(mFreeType);

}

// using my own streams, to implement UTF8 paths
FT_Face FreeTypeWrapper::NewFace(const std::string& inFilePath,FT_Long inFontIndex)
{
	FT_Face face;
	FT_Open_Args openFaceArguments;

	do
	{
		if(FillOpenFaceArgumentsForUTF8String(inFilePath,openFaceArguments) != PDFHummus::eSuccess)
		{
			face = NULL;
			break;
		}

		FT_Error ftStatus =  FT_Open_Face(mFreeType,&openFaceArguments,inFontIndex,&face);

		if(ftStatus)
		{
			TRACE_LOG2("FreeTypeWrapper::NewFace, unable to load font named %s with index %ld",inFilePath.c_str(),inFontIndex);
			TRACE_LOG2("FreeTypeWrapper::NewFace, Free Type Error, Code = %d, Message = %s",ft_errors[ftStatus].err_code,ft_errors[ftStatus].err_msg);
			face = NULL;
		}

	}while(false);

	if(!face)
		CloseOpenFaceArgumentsStream(openFaceArguments);
	else
		RegisterStreamForFace(face,openFaceArguments.stream);
	return face;
}

EStatusCode FreeTypeWrapper::FillOpenFaceArgumentsForUTF8String(const std::string& inFilePath, FT_Open_Args& ioArgs)
{
	ioArgs.flags = FT_OPEN_STREAM;
	ioArgs.memory_base = NULL;
	ioArgs.memory_size = 0;
	ioArgs.pathname = NULL;
	ioArgs.driver = NULL;
	ioArgs.num_params = 0;
	ioArgs.params = NULL;
	ioArgs.stream = CreateFTStreamForPath(inFilePath);
	
	if(ioArgs.stream)
	{
		return PDFHummus::eSuccess;
	}
	else
	{
		TRACE_LOG1("FreeTypeWrapper::FillOpenFaceArgumentsForWideString, Cannot Open file for reading %s",inFilePath.c_str());
		return PDFHummus::eFailure;
	}
}

void FreeTypeWrapper::CloseOpenFaceArgumentsStream(FT_Open_Args& ioArgs)
{
	if(ioArgs.stream)
	{
		delete (InputFile*)(ioArgs.stream->descriptor.pointer);
		delete ioArgs.stream;
	}
}

void FreeTypeWrapper::RegisterStreamForFace(FT_Face inFace,FT_Stream inStream)
{
	FTFaceToFTStreamListMap::iterator it = mOpenStreams.find(inFace);
	if(it == mOpenStreams.end())
		it = mOpenStreams.insert(FTFaceToFTStreamListMap::value_type(inFace,FTStreamList())).first;
	it->second.push_back(inStream);
}


FT_Face FreeTypeWrapper::NewFace(const std::string& inFilePath,const std::string& inSecondaryFilePath,FT_Long inFontIndex)
{
	FT_Open_Args attachStreamArguments;

	FT_Face face = NewFace(inFilePath,inFontIndex);
	if(face)
	{
		do
		{
			if(FillOpenFaceArgumentsForUTF8String(inSecondaryFilePath,attachStreamArguments) != PDFHummus::eSuccess)
			{
				DoneFace(face);
				face = NULL;
				break;
			}

			FT_Error ftStatus = FT_Attach_Stream(face,&attachStreamArguments);
			if(ftStatus != 0)
			{
				TRACE_LOG1("FreeTypeWrapper::NewFace, unable to load secondary file %s",inSecondaryFilePath.c_str());
				TRACE_LOG2("FreeTypeWrapper::NewFace, Free Type Error, Code = %d, Message = %s",ft_errors[ftStatus].err_code,ft_errors[ftStatus].err_msg);
				DoneFace(face);
				face = NULL;
			}
		}while(false);

		if(!face)
			CloseOpenFaceArgumentsStream(attachStreamArguments);
		else
			RegisterStreamForFace(face,attachStreamArguments.stream);

	}

	return face;	
}


FT_Error FreeTypeWrapper::DoneFace(FT_Face ioFace)
{
	FT_Error status = FT_Done_Face(ioFace);
	CleanStreamsForFace(ioFace);
	return status;
}

void FreeTypeWrapper::CleanStreamsForFace(FT_Face inFace)
{
	FTFaceToFTStreamListMap::iterator it = mOpenStreams.find(inFace);
	if(it != mOpenStreams.end())
	{
		FTStreamList::iterator itStreams = it->second.begin();
		for(; itStreams != it->second.end();++itStreams)
		{
			delete *itStreams;
		}
	}
	mOpenStreams.erase(it);
}


FT_Library FreeTypeWrapper::operator->()
{
	return mFreeType;
}

static unsigned long InputFileReadSeek(	   FT_Stream	   stream,
										   unsigned long   offset,
										   unsigned char*  buffer,
										   unsigned long   count)
{
	IByteReaderWithPosition* inputFileStream = ((InputFile*)(stream->descriptor.pointer))->GetInputStream();	
	unsigned long readBytes = 0;
	
	inputFileStream->SetPosition(offset);
	if(count > 0)
		readBytes = (unsigned long)inputFileStream->Read(buffer,count);
	return readBytes;
}

static void InputFileClose(FT_Stream  stream)
{
	delete (InputFile*)(stream->descriptor.pointer);
	stream->descriptor.pointer = NULL;
}

FT_Stream FreeTypeWrapper::CreateFTStreamForPath(const std::string& inFilePath)
{
	InputFile* inputFile = new InputFile;

	if(inputFile->OpenFile(inFilePath) != PDFHummus::eSuccess)
		return NULL;

	FT_Stream aStream = new FT_StreamRec();

	aStream->base = NULL;
	aStream->size = (unsigned long)inputFile->GetFileSize();
	aStream->pos = 0;
	aStream->descriptor.pointer = inputFile;
	aStream->pathname.pointer = NULL;
	aStream->read = InputFileReadSeek;
	aStream->close = InputFileClose;
	aStream->memory = NULL;
	aStream->cursor = NULL;
	aStream->limit = NULL;

	return aStream;
}

FreeTypeWrapper::operator FT_Library() const
{
	return mFreeType;
}
