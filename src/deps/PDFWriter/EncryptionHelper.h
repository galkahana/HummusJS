/*
Source File : EncryptionHelper.h


Copyright 2016 Gal Kahana PDFWriter

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
#include "XCryptionCommon.h"

#include <string>

class IByteWriterWithPosition;
class ObjectsContext;
class DecryptionHelper;
class PDFParser;

class EncryptionHelper {

public:
	EncryptionHelper(void);
	virtual ~EncryptionHelper(void);

	/*
		Initial step. starts (or not) encryption. note that you need file ID here, so you need to compute it before calling
		this method.
		will return status ok always. if you want to know whether ecnryption is actually supported for this input, call SupportsEncryption later.
		No need to call this setup prior to creating the first object. so you can delay till you are ready to create the file id.
	*/
	PDFHummus::EStatusCode Setup(
		bool inShouldEncrypt,
		double inPDFLevel,
		const std::string& inUserPassword,
		const std::string& inOwnerPassword,
		long long inUserProtectionOptionsFlag,
		bool inEncryptMetadata,
		const std::string inFileIDPart1
	);
	// short one for setting up no ecnryption
	void SetupNoEncryption();
	// Setup with existing decryption helper. This can be used to setup encryption with another PDF existing setup, or in modified PDF scenarios
	PDFHummus::EStatusCode Setup(const DecryptionHelper& inDecryptionSource);

	/*
		SupportsEncryption will respond true, if the encryption requested is supported. this includes true on the case that encryption was not requested
	*/
	bool SupportsEncryption();
	/*
		IsDocumentEncrypted will respond true, if encryption was requested and is supported. this will return false if encryption was not requested
	*/
	bool IsDocumentEncrypted();
	/*
		IsEncrypting will return true if actually encrypting now. it may be false if encryption was not requested, not supported or that the encrytion
		mechanism is not currently in a state that encrypts (say when writing an encryption dictionary).
	*/
	bool IsEncrypting();

	// use this to flag that encryption should not happen now. like when you are writing the encryption dictionary (well in this case it is called internally)
	// unencrypted metadata, or the trailer file ID
	void PauseEncryption();
	// use this to flag that a previous pause of encryption may now be released. encryption can continue
	void ReleaseEncryption();

	/*
		be nice and let us know when objects start and end. be used internally to recomputing encryption key
	*/
	void OnObjectStart(long long inObjectID, long long inGenerationNumber);
	void OnObjectEnd();

	// encrypt a string (if hex, do the hexing later. if literal do escaping later)
	std::string EncryptString(const std::string& inStringToEncrypt);

	// create encryption wrapper for a stream (all stream filter shoud be placed to wrap the encryption wrapper)
	IByteWriterWithPosition* CreateEncryptionStream(IByteWriterWithPosition* inToWrapStream);

	// when finalizing the document call this to write the encryption dictionary. If you want
	// it as an indirect object make sure to start and end one before and after calling this method
	// [remember to pause encryption internally]
	PDFHummus::EStatusCode WriteEncryptionDictionary(ObjectsContext* inObjectsContext);


	// state read/write support
	PDFHummus::EStatusCode WriteState(ObjectsContext* inStateWriter, ObjectIDType inObjectID);
	PDFHummus::EStatusCode ReadState(PDFParser* inStateReader, ObjectIDType inObjectID);

private:
	// named xcrypts, for V4
	StringToXCryptionCommonMap mXcrypts;
	// xcrypt to use for streams
	XCryptionCommon* mXcryptStreams;
	// xcrypt to use for strings
	XCryptionCommon* mXcryptStrings;
	// xcrypt to use for password authentication
	XCryptionCommon* mXcryptAuthentication;


	bool mIsDocumentEncrypted;
	int mEncryptionPauseLevel;
	bool mSupportsEncryption;

	IByteWriterWithPosition* CreateEncryptionWriter(IByteWriterWithPosition* inToWrapStream, const ByteList& inEncryptionKey, bool inUsingAES);
	void Release();

	// Generic encryption
	unsigned int mV;
	unsigned int mLength; // mLength is in bytes!

	// Standard filter specific

	unsigned int mRevision;
	ByteList mO;
	ByteList mU;
	long long mP;
	bool mEncryptMetaData;
	ByteList mFileIDPart1;
};