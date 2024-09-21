/*
Source File : EncryptionHelper.cpp


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

#include "EncryptionHelper.h"
#include "OutputStringBufferStream.h"
#include "InputStringStream.h"
#include "OutputStreamTraits.h"
#include "OutputRC4XcodeStream.h"
#include "OutputAESEncodeStream.h"
#include "ObjectsContext.h"
#include "DictionaryContext.h"
#include "DecryptionHelper.h"
#include "PDFParser.h"
#include "PDFDictionary.h"
#include "PDFObjectCast.h"
#include "PDFLiteralString.h"
#include "PDFInteger.h"
#include "PDFBoolean.h"

#include <stdint.h>

using namespace PDFHummus;
using namespace std;

EncryptionHelper::EncryptionHelper(void)
{
	mIsDocumentEncrypted = false;
	mEncryptionPauseLevel = 0;
	mSupportsEncryption = true;
	mXcryptAuthentication = NULL;
	mXcryptStreams = NULL;
	mXcryptStrings = NULL;

	mV = 0;
	mLength = 0;
	mRevision = 0;
	mP = 0;
	mEncryptMetaData = false;
}

EncryptionHelper::~EncryptionHelper(void)
{
	Release();
}

void EncryptionHelper::Release() {
	StringToXCryptionCommonMap::iterator it = mXcrypts.begin();
	for (; it != mXcrypts.end(); ++it)
		delete it->second;
	mXcrypts.clear();
}


bool EncryptionHelper::SupportsEncryption() {
	return mSupportsEncryption;
}

bool EncryptionHelper::IsDocumentEncrypted() {
	return mIsDocumentEncrypted;
}

bool EncryptionHelper::IsEncrypting() {
	return IsDocumentEncrypted() && mEncryptionPauseLevel == 0;
}

void EncryptionHelper::PauseEncryption() {
	++mEncryptionPauseLevel;
}

void EncryptionHelper::ReleaseEncryption() {
	--mEncryptionPauseLevel;
}

void EncryptionHelper::OnObjectStart(long long inObjectID, long long inGenerationNumber) {
	if (!IsEncrypting())
		return;

	StringToXCryptionCommonMap::iterator it = mXcrypts.begin();
	for (; it != mXcrypts.end(); ++it) {
		it->second->OnObjectStart(inObjectID, inGenerationNumber);
	}
}
void EncryptionHelper::OnObjectEnd() {
	if (!IsEncrypting())
		return;

	StringToXCryptionCommonMap::iterator it = mXcrypts.begin();
	for (; it != mXcrypts.end(); ++it) {
		it->second->OnObjectEnd();
	}
}

std::string EncryptionHelper::EncryptString(const std::string& inStringToEncrypt) {
	if (!IsEncrypting() || !mXcryptStrings)
		return inStringToEncrypt;

	OutputStringBufferStream buffer;

	IByteWriterWithPosition* encryptStream = CreateEncryptionWriter(&buffer, mXcryptStrings->GetCurrentObjectKey(), mXcryptStrings->IsUsingAES());
	if (encryptStream) {
		InputStringStream inputStream(inStringToEncrypt);
		OutputStreamTraits traits(encryptStream);
		traits.CopyToOutputStream(&inputStream);
		delete encryptStream; // free encryption stream (sometimes it will also mean flushing the output stream)

		return buffer.ToString();
	}
	else
		return inStringToEncrypt;
}

IByteWriterWithPosition* EncryptionHelper::CreateEncryptionStream(IByteWriterWithPosition* inToWrapStream) {
	return  CreateEncryptionWriter(inToWrapStream, mXcryptStreams->GetCurrentObjectKey(),mXcryptStreams->IsUsingAES());
}

IByteWriterWithPosition* EncryptionHelper::CreateEncryptionWriter(IByteWriterWithPosition* inToWrapStream, const ByteList& inEncryptionKey, bool inIsUsingAES) {
	if (inIsUsingAES) {
		return new OutputAESEncodeStream(inToWrapStream, inEncryptionKey, false);
	}
	else {
		return new OutputRC4XcodeStream(inToWrapStream, inEncryptionKey, false);
	}
}


static const string scFilter = "Filter";
static const string scStandard = "Standard";
static const string scV = "V";
static const string scLength = "Length";
static const string scR = "R";
static const string scO = "O";
static const string scU = "U";
static const string scP = "P";
static const string scEncryptMetadata = "EncryptMetadata";

EStatusCode EncryptionHelper::WriteEncryptionDictionary(ObjectsContext* inObjectsContext) {
	if (!IsDocumentEncrypted()) // document not encrypted, nothing to write. unexpected
		return eFailure;

	PauseEncryption();
	DictionaryContext* encryptContext = inObjectsContext->StartDictionary();

	// Filter
	encryptContext->WriteKey(scFilter);
	encryptContext->WriteNameValue(scStandard);

	// V
	encryptContext->WriteKey(scV);
	encryptContext->WriteIntegerValue(mV);

	// Length (if not 40. this would allow simple non usage control)
	if (mLength != 5) {
		encryptContext->WriteKey(scLength);
		encryptContext->WriteIntegerValue(mLength * 8);
	}

	// R
	encryptContext->WriteKey(scR);
	encryptContext->WriteIntegerValue(mRevision);

	// O
	encryptContext->WriteKey(scO);
	encryptContext->WriteHexStringValue(XCryptionCommon::ByteListToString(mO));

	// U
	encryptContext->WriteKey(scU);
	encryptContext->WriteHexStringValue(XCryptionCommon::ByteListToString(mU));

	// P
	encryptContext->WriteKey(scP);
	encryptContext->WriteIntegerValue(mP);

	// EncryptMetadata
	encryptContext->WriteKey(scEncryptMetadata);
	encryptContext->WriteBooleanValue(mEncryptMetaData);

	// Now. if using V4, define crypt filters
	if (mV == 4) {
		encryptContext->WriteKey("CF");
		DictionaryContext* cf = inObjectsContext->StartDictionary();
		cf->WriteKey("StdCF");

		DictionaryContext* stdCf = inObjectsContext->StartDictionary();
		stdCf->WriteKey("Type");
		stdCf->WriteNameValue("CryptFilter");


		stdCf->WriteKey("CFM");
		stdCf->WriteNameValue("AESV2");

		stdCf->WriteKey("AuthEvent");
		stdCf->WriteNameValue("DocOpen");

		stdCf->WriteKey("Length");
		stdCf->WriteIntegerValue(128);

		inObjectsContext->EndDictionary(stdCf);
		inObjectsContext->EndDictionary(cf);

		encryptContext->WriteKey("StmF");
		encryptContext->WriteNameValue("StdCF");

		encryptContext->WriteKey("StrF");
		encryptContext->WriteNameValue("StdCF");

	}

	ReleaseEncryption();

	return inObjectsContext->EndDictionary(encryptContext);
}

static const string scStdCF = "StdCF";

EStatusCode EncryptionHelper::Setup(
	bool inShouldEncrypt,
	double inPDFLevel,
	const string& inUserPassword,
	const string& inOwnerPassword,
	long long inUserProtectionOptionsFlag,
	bool inEncryptMetadata,
	const string inFileIDPart1
	) {

	if (!inShouldEncrypt) {
		SetupNoEncryption();
		return eSuccess;
	}

	mIsDocumentEncrypted = false;
	mSupportsEncryption = false;

	bool usingAES = inPDFLevel >= 1.6;
	XCryptionCommon* defaultEncryption = new XCryptionCommon();


	if (inPDFLevel >= 1.4) {
		mLength = 16;

		if (usingAES) {
			mV = 4;
			mRevision = 4;
		}
		else 
		{
			mV = 2;
			mRevision = 3;
		}
	}
	else {
		mLength = 5;
		mV = 1;
		mRevision = (inUserProtectionOptionsFlag & 0xF00) ? 3 : 2;
		usingAES = false;
	}

	defaultEncryption->Setup(usingAES);
	mXcrypts.insert(StringToXCryptionCommonMap::value_type(scStdCF, defaultEncryption));
	mXcryptStreams = defaultEncryption;
	mXcryptStrings = defaultEncryption;
	mXcryptAuthentication = defaultEncryption;


	// compute P out of inUserProtectionOptionsFlag. inUserProtectionOptionsFlag can be a more relaxed number setting as 1s only the enabled access. mP will restrict to PDF Expected bits
	int32_t truncP = int32_t(((inUserProtectionOptionsFlag | 0xFFFFF0C0) & 0xFFFFFFFC));
	mP = truncP;

	ByteList ownerPassword = XCryptionCommon::stringToByteList(inOwnerPassword.size() > 0 ? inOwnerPassword : inUserPassword);
	ByteList userPassword = XCryptionCommon::stringToByteList(inUserPassword);
	mEncryptMetaData = inEncryptMetadata;
	mFileIDPart1 = XCryptionCommon::stringToByteList(inFileIDPart1);

	mO = mXcryptAuthentication->algorithm3_3(mRevision,mLength,ownerPassword,userPassword);
	if (mRevision == 2)
		mU = mXcryptAuthentication->algorithm3_4(mLength,userPassword,mO,mP,mFileIDPart1,mEncryptMetaData);
	else
		mU = mXcryptAuthentication->algorithm3_5(mRevision,mLength,userPassword, mO, mP, mFileIDPart1, mEncryptMetaData);

	defaultEncryption->SetupInitialEncryptionKey(
		inUserPassword,
		mRevision,
		mLength,
		mO,
		mP,
		mFileIDPart1,
		mEncryptMetaData);

	mIsDocumentEncrypted = true;
	mSupportsEncryption = true;

	return eSuccess;
}

void EncryptionHelper::SetupNoEncryption() 
{
	mIsDocumentEncrypted = false;
	mSupportsEncryption = true;
}

EStatusCode EncryptionHelper::Setup(const DecryptionHelper& inDecryptionSource) 
{
	if (!inDecryptionSource.IsEncrypted() || !inDecryptionSource.CanDecryptDocument()) {
		SetupNoEncryption();
		return eSuccess;
	}

	mIsDocumentEncrypted = false;
	mSupportsEncryption = false;

	do {

		mLength = inDecryptionSource.GetLength();
		mV = inDecryptionSource.GetV();
		mRevision = inDecryptionSource.GetRevision();
		mP = inDecryptionSource.GetP();
		mEncryptMetaData = inDecryptionSource.GetEncryptMetaData();
		mFileIDPart1 = inDecryptionSource.GetFileIDPart1();
		mO = inDecryptionSource.GetO();
		mU = inDecryptionSource.GetU();

		// initialize xcryptors
		mXcryptStreams = NULL;
		// xcrypt to use for strings
		mXcryptStrings = NULL;
		// xcrypt to use for password authentication
		mXcryptAuthentication = NULL;
		StringToXCryptionCommonMap::const_iterator it = inDecryptionSource.GetXcrypts().begin();
		StringToXCryptionCommonMap::const_iterator itEnd = inDecryptionSource.GetXcrypts().end();
		for (; it != itEnd; ++it) {
			XCryptionCommon* xCryption = new XCryptionCommon();
			xCryption->Setup(it->second->IsUsingAES());
			xCryption->SetupInitialEncryptionKey(it->second->GetInitialEncryptionKey());
			mXcrypts.insert(StringToXCryptionCommonMap::value_type(it->first, xCryption));

			// see if it fits any of the global xcryptors
			if (it->second == inDecryptionSource.GetStreamXcrypt())
				mXcryptStreams = xCryption;
			if (it->second == inDecryptionSource.GetStringXcrypt())
				mXcryptStrings = xCryption;
			if (it->second == inDecryptionSource.GetAuthenticationXcrypt())
				mXcryptAuthentication = xCryption;
		}


		mIsDocumentEncrypted = true;
		mSupportsEncryption = true;
	} while (false);

	return eSuccess;
}


PDFHummus::EStatusCode EncryptionHelper::WriteState(ObjectsContext* inStateWriter, ObjectIDType inObjectID)
{
	inStateWriter->StartNewIndirectObject(inObjectID);
	DictionaryContext* encryptionObject = inStateWriter->StartDictionary();

	encryptionObject->WriteKey("Type");
	encryptionObject->WriteNameValue("EncryptionHelper");

	encryptionObject->WriteKey("mIsDocumentEncrypted");
	encryptionObject->WriteBooleanValue(mIsDocumentEncrypted);

	encryptionObject->WriteKey("mSupportsEncryption");
	encryptionObject->WriteBooleanValue(mSupportsEncryption);

	encryptionObject->WriteKey("mUsingAES");
	encryptionObject->WriteBooleanValue(mXcryptAuthentication ? mXcryptAuthentication->IsUsingAES():false);

	encryptionObject->WriteKey("mLength");
	encryptionObject->WriteIntegerValue(mLength);

	encryptionObject->WriteKey("mV");
	encryptionObject->WriteIntegerValue(mV);

	encryptionObject->WriteKey("mRevision");
	encryptionObject->WriteIntegerValue(mRevision);

	encryptionObject->WriteKey("mP");
	encryptionObject->WriteIntegerValue(mP);

	encryptionObject->WriteKey("mEncryptMetaData");
	encryptionObject->WriteBooleanValue(mEncryptMetaData);

	encryptionObject->WriteKey("mFileIDPart1");
	encryptionObject->WriteLiteralStringValue(XCryptionCommon::ByteListToString(mFileIDPart1));

	encryptionObject->WriteKey("mO");
	encryptionObject->WriteLiteralStringValue(XCryptionCommon::ByteListToString(mO));

	encryptionObject->WriteKey("mU");
	encryptionObject->WriteLiteralStringValue(XCryptionCommon::ByteListToString(mU));

	encryptionObject->WriteKey("InitialEncryptionKey");
	encryptionObject->WriteLiteralStringValue(mXcryptAuthentication ? XCryptionCommon::ByteListToString(mXcryptAuthentication->GetInitialEncryptionKey()) : "");

	inStateWriter->EndDictionary(encryptionObject);
	inStateWriter->EndIndirectObject();

	return eSuccess;
}

PDFHummus::EStatusCode EncryptionHelper::ReadState(PDFParser* inStateReader, ObjectIDType inObjectID)
{
	PDFObjectCastPtr<PDFDictionary> encryptionObjectState(inStateReader->ParseNewObject(inObjectID));

	PDFObjectCastPtr<PDFBoolean> isDocumentEncrypted = encryptionObjectState->QueryDirectObject("mIsDocumentEncrypted");
	mIsDocumentEncrypted = isDocumentEncrypted->GetValue();

	PDFObjectCastPtr<PDFBoolean> supportsEncryption = encryptionObjectState->QueryDirectObject("mSupportsEncryption");
	mSupportsEncryption = supportsEncryption->GetValue();

	PDFObjectCastPtr<PDFBoolean> usingAESObject = encryptionObjectState->QueryDirectObject("mUsingAES");
	bool usingAES = usingAESObject->GetValue();

	PDFObjectCastPtr<PDFInteger> length = encryptionObjectState->QueryDirectObject("mLength");
	mLength = (unsigned int)length->GetValue();

	PDFObjectCastPtr<PDFInteger> v = encryptionObjectState->QueryDirectObject("mV");
	mV = (unsigned int)v->GetValue();

	PDFObjectCastPtr<PDFInteger> revision = encryptionObjectState->QueryDirectObject("mRevision");
	mRevision = (unsigned int)revision->GetValue();

	PDFObjectCastPtr<PDFInteger> p = encryptionObjectState->QueryDirectObject("mP");
	mP = p->GetValue();

	PDFObjectCastPtr<PDFBoolean> encryptMetaData = encryptionObjectState->QueryDirectObject("mEncryptMetaData");
	mEncryptMetaData = encryptMetaData->GetValue();

	PDFObjectCastPtr<PDFLiteralString> fileIDPart1 = encryptionObjectState->QueryDirectObject("mFileIDPart1");
	mFileIDPart1 = XCryptionCommon::stringToByteList(fileIDPart1->GetValue());

	PDFObjectCastPtr<PDFLiteralString> o = encryptionObjectState->QueryDirectObject("mO");
	mO = XCryptionCommon::stringToByteList(o->GetValue());

	PDFObjectCastPtr<PDFLiteralString> u = encryptionObjectState->QueryDirectObject("mU");
	mU = XCryptionCommon::stringToByteList(u->GetValue());

	PDFObjectCastPtr<PDFLiteralString> InitialEncryptionKey = encryptionObjectState->QueryDirectObject("InitialEncryptionKey");
	XCryptionCommon* defaultEncryption = new XCryptionCommon();

	// setup encryption
	defaultEncryption->Setup(usingAES);
	mXcrypts.insert(StringToXCryptionCommonMap::value_type(scStdCF, defaultEncryption));
	mXcryptStreams = defaultEncryption;
	mXcryptStrings = defaultEncryption;
	mXcryptAuthentication = defaultEncryption;
	mXcryptAuthentication->SetupInitialEncryptionKey(XCryptionCommon::stringToByteList(InitialEncryptionKey->GetValue()));

	return eSuccess;
}
