/*
Source File : DecryptionHelper.h


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

class PDFParser;
class IByteReader;
class PDFStreamInput;
class PDFObject;

class DecryptionHelper {

public:
	DecryptionHelper(void);
	virtual ~DecryptionHelper(void);


	/*
		Setup will fail for case of bad password, for decryption that can be handled.
	*/
	PDFHummus::EStatusCode Setup(PDFParser* inParser,const std::string& inPassword);

	/*
		Queries.

		1. IsEncrypted means PDF is encrypted
		2. SupportsDecryption means that DecryptionHelper supports the kind of encryption defined in the PDF
		3. CanDecryptDocument means that DecryptionHelper can decrypt document. means decryption is supported, and password is good
		4. DidFailPasswordVerification meams that password verification didnt went through. either decryption not supported, or password no good
		5. DidSucceedOwnerPasswordVerification means that password verification succeeded, and the user has owner priviliges
	*/
	bool IsEncrypted() const;
	bool SupportsDecryption() const;
	bool CanDecryptDocument() const;
	bool DidFailPasswordVerification() const;
	bool DidSucceedOwnerPasswordVerification() const;

	std::string DecryptString(const std::string& inStringToDecrypt);

	void OnObjectStart(long long inObjectID, long long inGenerationNumber);
	void OnObjectEnd(PDFObject* inObject);
	IByteReader* CreateDecryptionFilterForStream(PDFStreamInput* inStream, IByteReader* inToWrapStream);

	unsigned int GetLength() const;
	unsigned int GetV() const;
	unsigned int GetRevision() const;
	long long GetP() const;
	bool GetEncryptMetaData() const;
	const ByteList& GetFileIDPart1() const;
	const ByteList& GetO() const;
	const ByteList& GetU() const;
	const ByteList& GetInitialEncryptionKey() const;

	// Reset after or before usage
	void Reset();
private:
	XCryptionCommon mXcryption;

	bool mIsEncrypted;
	bool mSupportsDecryption;

	// Generic encryption
	unsigned int mV;
	unsigned int mLength; // mLength is in bytes!
	
	IByteReader* CreateDecryptionReader(IByteReader* inSourceStream,const ByteList& inEncryptionKey);

	// Standard filter specific
	bool mFailedPasswordVerification;
	bool mDidSucceedOwnerPasswordVerification;

	unsigned int mRevision;
	ByteList mO;
	ByteList mU;
	long long mP;
	bool mEncryptMetaData;
	ByteList mFileIDPart1;

	bool AuthenticateUserPassword(const ByteList& inPassword);
	bool AuthenticateOwnerPassword(const ByteList& inPassword);
};