/*
   Source File : IObjectsContextExtender.h


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

class IByteWriter;
class IByteWriterWithPosition;

class IObjectsContextExtender
{

public:
	virtual ~IObjectsContextExtender(){}

	// PDFStream object compression extensibility

	// flag to determine if the extender takes over compression or not
	// if true than other compression methods will be called in case compression is required
	virtual bool OverridesStreamCompression() = 0;
	
	// GetCompressionWriteStream is called when PDFStream Object is created 
	// if compression was required and OverridesStreamCompression returned true.
	// input is the stream to write to, output is the new stream to write to - the compressed stream
	virtual IByteWriter* GetCompressionWriteStream(IByteWriterWithPosition* inOutputStream) = 0;

	// FinalizeCompressedStreamWrite is called when the PDFStream is finalized.
	// it recieves as input the compressed stream that was returned from GetCompressionWriteStream.
	// the implementation should finalize the compression stream write [any remaning footer buffers should be written now].
	// this would allow the PDFStream to calculate the extent on the actual write stream (given as input for GetCompressionWriteStream).
	virtual void FinalizeCompressedStreamWrite(IByteWriter* inCompressedStream) = 0;

};