/*
Source File : DecryptionHelper.cpp


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
#include "DecryptionHelper.h"
#include "PDFParser.h"
#include "PDFObjectCast.h"
#include "PDFDictionary.h"
#include "PDFArray.h"
#include "PDFObject.h"
#include "PDFName.h"
#include "PDFBoolean.h"
#include "PDFStreamInput.h"
#include "InputStringStream.h"
#include "OutputStringBufferStream.h"
#include "OutputStreamTraits.h"
#include "ParsedPrimitiveHelper.h"
#include "RefCountPtr.h"
#include "OutputStringBufferStream.h"
#include "InputRC4XcodeStream.h"
#include "Trace.h"

using namespace std;
using namespace PDFHummus;
using namespace IOBasicTypes;

DecryptionHelper::DecryptionHelper(void)
{
	Reset();
}

DecryptionHelper::~DecryptionHelper(void)
{
}

void DecryptionHelper::Reset() {
	mSupportsDecryption = false;
	mFailedPasswordVerification = false;
	mDidSucceedOwnerPasswordVerification = false;
	mIsEncrypted = false;
}

EStatusCode DecryptionHelper::Setup(PDFParser* inParser, const std::string& inPassword) {
	mSupportsDecryption = false;
	mFailedPasswordVerification = false;
	mDidSucceedOwnerPasswordVerification = false;

	// setup encrypted flag
	PDFObjectCastPtr<PDFDictionary> encryptionDictionary(inParser->QueryDictionaryObject(inParser->GetTrailer(), "Encrypt"));
	mIsEncrypted = encryptionDictionary.GetPtr() != NULL;

	do {
		if (!mIsEncrypted)
			break;

		PDFObjectCastPtr<PDFName> filter(inParser->QueryDictionaryObject(encryptionDictionary.GetPtr(), "Filter"));
		if (!filter || filter->GetValue() != "Standard") {
			// Supporting only standard filter
			if(!filter)
				TRACE_LOG("DecryptionHelper::Setup, no filter defined");
			else
				TRACE_LOG1("DecryptionHelper::Setup, Only Standard encryption filter is supported. Unsupported filter encountered - %s",filter->GetValue().c_str());
			break;
		}

		RefCountPtr<PDFObject> v(inParser->QueryDictionaryObject(encryptionDictionary.GetPtr(), "V"));
		if (!v) {
			mV = 0;
		} else {
			ParsedPrimitiveHelper vHelper(v.GetPtr());
			if (!vHelper.IsNumber())
				break;
			mV = (unsigned int)vHelper.GetAsInteger();
		}

		// supporting only  versions 1 and 2 (no crypt filters and nothing unpublished)
		if (mV != 1 && mV != 2) {
			TRACE_LOG1("DecryptionHelper::Setup, Only 1 and 2 are supported values for V. Unsupported filter encountered - %d", mV);
			break;
		}

		RefCountPtr<PDFObject> length(inParser->QueryDictionaryObject(encryptionDictionary.GetPtr(), "Length"));
		if (!length) {
			mLength = 40/8;
		}
		else {
			ParsedPrimitiveHelper lengthHelper(length.GetPtr());
			if (!lengthHelper.IsNumber())
				break;
			mLength = (unsigned int)lengthHelper.GetAsInteger()/8;
		}

		RefCountPtr<PDFObject> revision(inParser->QueryDictionaryObject(encryptionDictionary.GetPtr(), "R"));
		if (!revision) {
			break;
		}
		else {
			ParsedPrimitiveHelper revisionHelper(revision.GetPtr());
			if (!revisionHelper.IsNumber())
				break;
			mRevision = (unsigned int)revisionHelper.GetAsInteger();
		}

		RefCountPtr<PDFObject> o(inParser->QueryDictionaryObject(encryptionDictionary.GetPtr(), "O"));
		if (!o) {
			break;
		}
		else {
			ParsedPrimitiveHelper oHelper(o.GetPtr());
			mO = mXcryption.stringToByteList(oHelper.ToString());
		}

		RefCountPtr<PDFObject> u(inParser->QueryDictionaryObject(encryptionDictionary.GetPtr(), "U"));
		if (!u) {
			break;
		}
		else {
			ParsedPrimitiveHelper uHelper(u.GetPtr());
			mU = mXcryption.stringToByteList(uHelper.ToString());
		}

		RefCountPtr<PDFObject> p(inParser->QueryDictionaryObject(encryptionDictionary.GetPtr(), "P"));
		if (!p) {
			break;
		}
		else {
			ParsedPrimitiveHelper pHelper(p.GetPtr());
			if (!pHelper.IsNumber())
				break;
			mP = pHelper.GetAsInteger();
		}

		PDFObjectCastPtr<PDFBoolean> encryptMetadata(inParser->QueryDictionaryObject(encryptionDictionary.GetPtr(), "EncryptMetadata"));
		if (!encryptMetadata) {
			mEncryptMetaData = true;
		}
		else {
			mEncryptMetaData = encryptMetadata->GetValue();
		}

		// grab file ID from trailer
		mFileIDPart1 = ByteList();
		PDFObjectCastPtr<PDFArray> idArray(inParser->QueryDictionaryObject(inParser->GetTrailer(), "ID"));
		if (!!idArray && idArray->GetLength() > 0) {
			RefCountPtr<PDFObject> idPart1Object(inParser->QueryArrayObject(idArray.GetPtr(), 0));
			if (!!idPart1Object) {
				ParsedPrimitiveHelper idPart1ObjectHelper(idPart1Object.GetPtr());
				mFileIDPart1 = mXcryption.stringToByteList(idPart1ObjectHelper.ToString());
			}
		}

		mXcryption.Setup(false);
		if (!mXcryption.CanXCrypt()) 
			break;

		mXcryption.SetupInitialEncryptionKey(
			inPassword,
			mRevision,
			mLength,
			mO,
			mP,
			mFileIDPart1,
			mEncryptMetaData);

		// authenticate password, try to determine if user or owner
		ByteList password = mXcryption.stringToByteList(inPassword);

		mDidSucceedOwnerPasswordVerification = AuthenticateOwnerPassword(password);
		mFailedPasswordVerification = !mDidSucceedOwnerPasswordVerification && !AuthenticateUserPassword(password);


		mSupportsDecryption = true;
	}while(false);

	return eSuccess;
}

bool DecryptionHelper::IsEncrypted() const{
	return mIsEncrypted;
}

bool DecryptionHelper::SupportsDecryption() const {
	return mSupportsDecryption;
}

bool DecryptionHelper::CanDecryptDocument() const {
	return mSupportsDecryption && !mFailedPasswordVerification;
}

bool DecryptionHelper::DidFailPasswordVerification() const {
	return mFailedPasswordVerification;
}

bool DecryptionHelper::DidSucceedOwnerPasswordVerification() const {
	return mDidSucceedOwnerPasswordVerification;
}

static const string scEcnryptionKeyMetadataKey = "DecryptionHelper.EncryptionKey";

IByteReader* DecryptionHelper::CreateDecryptionFilterForStream(PDFStreamInput* inStream, IByteReader* inToWrapStream) {
	if (!IsEncrypted() || !CanDecryptDocument())
		return NULL;
	
	void* savedEcnryptionKey = inStream->GetMetadata(scEcnryptionKeyMetadataKey);
	if (savedEcnryptionKey) {
		return CreateDecryptionReader(inToWrapStream, *((ByteList*)savedEcnryptionKey));
	}
	else 
		return NULL;
}

std::string DecryptionHelper::DecryptString(const std::string& inStringToDecrypt) {
	if (!IsEncrypted() || !CanDecryptDocument())
		return inStringToDecrypt;

	IByteReader* decryptStream = CreateDecryptionReader(new InputStringStream(inStringToDecrypt), mXcryption.GetCurrentObjectKey());
	if (decryptStream) {
		OutputStringBufferStream outputStream;
		OutputStreamTraits traits(&outputStream);
		traits.CopyToOutputStream(decryptStream);

		return outputStream.ToString();
	}
	else
		return inStringToDecrypt;
}

void DecryptionHelper::OnObjectStart(long long inObjectID, long long inGenerationNumber) {
	if (!IsEncrypted() || !CanDecryptDocument())
		return;

	mXcryption.OnObjectStart(inObjectID,inGenerationNumber);
}
void DecryptionHelper::OnObjectEnd(PDFObject* inObject) {
	if (!IsEncrypted() || !CanDecryptDocument())
		return;

	// for streams, retain the encryption key with them, so i can later decrypt them when needed
	if (inObject->GetType() == PDFObject::ePDFObjectStream) {
		ByteList* savedKey = new ByteList(mXcryption.GetCurrentObjectKey());
		inObject->SetMetadata(scEcnryptionKeyMetadataKey, savedKey);
	}
	mXcryption.OnObjectEnd();
}

IByteReader* DecryptionHelper::CreateDecryptionReader(IByteReader* inSourceStream, const ByteList& inEncryptionKey) {
	return new InputRC4XcodeStream(inSourceStream, inEncryptionKey);
}


bool DecryptionHelper::AuthenticateUserPassword(const ByteList& inPassword) {
	return mXcryption.algorithm3_6(mRevision,
						mLength,
						inPassword,
						mO,
						mP,
						mFileIDPart1,
						mEncryptMetaData,
						mU);
}

bool DecryptionHelper::AuthenticateOwnerPassword(const ByteList& inPassword) {
	return mXcryption.algorithm3_7(mRevision,
						mLength,
						inPassword,
						mO,
						mP,
						mFileIDPart1,
						mEncryptMetaData,
						mU);
}

unsigned int DecryptionHelper::GetLength() const
{
	return mLength;
}

unsigned int DecryptionHelper::GetV() const
{
	return mV;
}

unsigned int DecryptionHelper::GetRevision() const
{
	return mRevision;
}

long long DecryptionHelper::GetP() const
{
	return mP;
}

bool DecryptionHelper::GetEncryptMetaData() const
{
	return mEncryptMetaData;
}

const ByteList& DecryptionHelper::GetFileIDPart1() const
{
	return mFileIDPart1;
}

const ByteList& DecryptionHelper::GetO() const
{
	return mO;
}

const ByteList& DecryptionHelper::GetU() const
{
	return mU;
}

const ByteList& DecryptionHelper::GetInitialEncryptionKey() const
{
	return mXcryption.GetInitialEncryptionKey();
}