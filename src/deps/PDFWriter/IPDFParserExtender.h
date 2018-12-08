#pragma once

#include <string>

class IByteReader;
class PDFName;
class PDFDictionary;
class PDFStreamInput;


class IPDFParserExtender
{
public:

	virtual ~IPDFParserExtender(){}

	// for extending fliter support in stream read
	virtual IByteReader* CreateFilterForStream(IByteReader* inStream,PDFName* inFilterName,PDFDictionary* inDecodeParams, PDFStreamInput* inPDFStream) = 0;

	// for decryption extension

	// DoesSupportEncryption should return true if encryption is supported
	virtual bool DoesSupportEncryption() = 0;
	// return input stream if did nothing, or a filter stream owning the input stream if encryption is required
	virtual IByteReader* CreateDefaultDecryptionFilterForStream(PDFStreamInput* inStream, IByteReader* inToWrapStream) = 0;
	// decrypt input string. this serves both literal and hexadecimal strings
	virtual std::string DecryptString(std::string inStringToDecrypt) = 0;
	// event for indirect object parsing start
	virtual void OnObjectStart(long long inObjectID, long long inGenerationNumber) = 0;
	// event for indirect object parsing end (you will be getting null if parsing failed, or not more objects. so check)
	virtual void OnObjectEnd(PDFObject* inObject) = 0;
};