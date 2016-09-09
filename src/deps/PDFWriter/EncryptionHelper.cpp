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
}

EncryptionHelper::~EncryptionHelper(void)
{
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

	mXcryption.OnObjectStart(inObjectID, inGenerationNumber);
}
void EncryptionHelper::OnObjectEnd() {
	if (!IsEncrypting())
		return;

	mXcryption.OnObjectEnd();
}

std::string EncryptionHelper::EncryptString(const std::string& inStringToEncrypt) {
	if (!IsEncrypting())
		return inStringToEncrypt;

	OutputStringBufferStream buffer;

	IByteWriterWithPosition* encryptStream = CreateEncryptionWriter(&buffer, mXcryption.GetCurrentObjectKey()); 
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
	return  CreateEncryptionWriter(inToWrapStream, mXcryption.GetCurrentObjectKey());
}

IByteWriterWithPosition* EncryptionHelper::CreateEncryptionWriter(IByteWriterWithPosition* inToWrapStream, const ByteList& inEncryptionKey) {
	if (mUsingAES) {
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
	encryptContext->WriteHexStringValue(mXcryption.ByteListToString(mO));

	// U
	encryptContext->WriteKey(scU);
	encryptContext->WriteHexStringValue(mXcryption.ByteListToString(mU));

	// P
	encryptContext->WriteKey(scP);
	encryptContext->WriteIntegerValue(mP);

	// EncryptMetadata
	encryptContext->WriteKey(scEncryptMetadata);
	encryptContext->WriteBooleanValue(mEncryptMetaData);

	// Now. if using AES to encrypt it requires to add default crypt filter definitions. so do that now
	if (mUsingAES) {
		encryptContext->WriteKey("CF");
		DictionaryContext* cf = inObjectsContext->StartDictionary();
		cf->WriteKey("StdCF");

		DictionaryContext* aes = inObjectsContext->StartDictionary();
		aes->WriteKey("Type");
		aes->WriteNameValue("CryptFilter");


		aes->WriteKey("CFM");
		aes->WriteNameValue("AESV2");

		aes->WriteKey("AuthEvent");
		aes->WriteNameValue("DocOpen");

		aes->WriteKey("Length");
		aes->WriteIntegerValue(16);

		inObjectsContext->EndDictionary(aes);
		inObjectsContext->EndDictionary(cf);

		encryptContext->WriteKey("StmF");
		encryptContext->WriteNameValue("StdCF");

		encryptContext->WriteKey("StrF");
		encryptContext->WriteNameValue("StdCF");

	}

	ReleaseEncryption();

	return inObjectsContext->EndDictionary(encryptContext);
}

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

	do {
		mUsingAES = inPDFLevel >= 1.6;

		mUsingAES = false; // GAL, forcing this to false. something doesn't work well with the encryption now and i really got other things i want to attend to.

		mXcryption.Setup(mUsingAES);
		if (!mXcryption.CanXCrypt())
			break;

		if (inPDFLevel >= 1.4) {
			mLength = 16;

			if (mUsingAES) {
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
			mUsingAES = false;
		}



		// compute P out of inUserProtectionOptionsFlag. inUserProtectionOptionsFlag can be a more relaxed number setting as 1s only the enabled access. mP will restrict to PDF Expected bits
		int32_t truncP = int32_t(((inUserProtectionOptionsFlag | 0xFFFFF0C0) & 0xFFFFFFFC));
		mP = truncP;

		ByteList ownerPassword = mXcryption.stringToByteList(inOwnerPassword.size() > 0 ? inOwnerPassword : inUserPassword);
		ByteList userPassword = mXcryption.stringToByteList(inUserPassword);
		mEncryptMetaData = inEncryptMetadata;
		mFileIDPart1 = mXcryption.stringToByteList(inFileIDPart1);

		mO = mXcryption.algorithm3_3(mRevision,mLength,ownerPassword,userPassword);
		if (mRevision == 2)
			mU = mXcryption.algorithm3_4(mLength,userPassword,mO,mP,mFileIDPart1,mEncryptMetaData);
		else
			mU = mXcryption.algorithm3_5(mRevision,mLength,userPassword, mO, mP, mFileIDPart1, mEncryptMetaData);


		mXcryption.SetupInitialEncryptionKey(inUserPassword, mRevision, mLength, mO, mP, mFileIDPart1, mEncryptMetaData);

		mIsDocumentEncrypted = true;
		mSupportsEncryption = true;
	} while (false);

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
		mUsingAES = false;
		mXcryption.Setup(mUsingAES);
		if (!mXcryption.CanXCrypt())
			break;

		mLength = inDecryptionSource.GetLength();
		mV = inDecryptionSource.GetV();
		mRevision = inDecryptionSource.GetRevision();
		mP = inDecryptionSource.GetP();
		mEncryptMetaData = inDecryptionSource.GetEncryptMetaData();
		mFileIDPart1 = inDecryptionSource.GetFileIDPart1();
		mO = inDecryptionSource.GetO();
		mU = inDecryptionSource.GetU();
		mXcryption.SetupInitialEncryptionKey(inDecryptionSource.GetInitialEncryptionKey());

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
	encryptionObject->WriteBooleanValue(mUsingAES);

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
	encryptionObject->WriteLiteralStringValue(mXcryption.ByteListToString(mFileIDPart1));

	encryptionObject->WriteKey("mO");
	encryptionObject->WriteLiteralStringValue(mXcryption.ByteListToString(mO));

	encryptionObject->WriteKey("mU");
	encryptionObject->WriteLiteralStringValue(mXcryption.ByteListToString(mU));

	encryptionObject->WriteKey("InitialEncryptionKey");
	encryptionObject->WriteLiteralStringValue(mXcryption.ByteListToString(mXcryption.GetInitialEncryptionKey()));

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

	PDFObjectCastPtr<PDFBoolean> usingAES = encryptionObjectState->QueryDirectObject("mUsingAES");
	mUsingAES = usingAES->GetValue();

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
	mFileIDPart1 = mXcryption.stringToByteList(fileIDPart1->GetValue());

	PDFObjectCastPtr<PDFLiteralString> o = encryptionObjectState->QueryDirectObject("mO");
	mO = mXcryption.stringToByteList(o->GetValue());

	PDFObjectCastPtr<PDFLiteralString> u = encryptionObjectState->QueryDirectObject("mU");
	mU = mXcryption.stringToByteList(u->GetValue());

	PDFObjectCastPtr<PDFLiteralString> InitialEncryptionKey = encryptionObjectState->QueryDirectObject("InitialEncryptionKey");
	mXcryption.SetupInitialEncryptionKey(mXcryption.stringToByteList(InitialEncryptionKey->GetValue()));

	return eSuccess;
}
