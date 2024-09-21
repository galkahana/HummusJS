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
#include "InputAESDecodeStream.h"
#include "Trace.h"
#include "Deletable.h"
#include <memory>
#include <algorithm>

using namespace std;
using namespace PDFHummus;
using namespace IOBasicTypes;

DecryptionHelper::DecryptionHelper(void)
{
	Reset();
}

void DecryptionHelper::Release() {
	StringToXCryptionCommonMap::iterator it = mXcrypts.begin();
	for (; it != mXcrypts.end(); ++it)
		delete it->second;
	mXcrypts.clear();
}

DecryptionHelper::~DecryptionHelper(void)
{
	Release();
}

void DecryptionHelper::Reset() {
	mSupportsDecryption = false;
	mFailedPasswordVerification = false;
	mDidSucceedOwnerPasswordVerification = false;
	mIsEncrypted = false;
	mXcryptStreams = NULL;
	mXcryptStrings = NULL;
	mXcryptAuthentication = NULL;
	mParser = NULL;
	mDecryptionPauseLevel = 0;
	Release();
}

unsigned int ComputeByteLength(PDFObject* inLengthObject) {
	// The bit length of the file encryption key shall be a multiple of 8 in the range of 40 to 256 bits. This function returns the length in bytes.
	ParsedPrimitiveHelper lengthHelper(inLengthObject);
	unsigned int value = lengthHelper.IsNumber() ? (unsigned int)lengthHelper.GetAsInteger() : 40;
	return value < 40 ? std::max(value, (unsigned int)5) : value / 8; // this small check here is based on some errors i saw, where the length was given in bytes instead of bits
}

XCryptionCommon* GetFilterForName(const StringToXCryptionCommonMap& inXcryptions, const string& inName) {
	StringToXCryptionCommonMap::const_iterator it = inXcryptions.find(inName);

	if (it == inXcryptions.end())
		return NULL;
	else
		return it->second;
}

static const string scStdCF = "StdCF";
EStatusCode DecryptionHelper::Setup(PDFParser* inParser, const string& inPassword) {
	mSupportsDecryption = false;
	mFailedPasswordVerification = false;
	mDidSucceedOwnerPasswordVerification = false;
	mParser = inParser;

	// setup encrypted flag through the existance of encryption dict
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
				TRACE_LOG1("DecryptionHelper::Setup, Only Standard encryption filter is supported. Unsupported filter encountered - %s",filter->GetValue().substr(0, MAX_TRACE_SIZE - 200).c_str());
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

		// supporting versions 1,2 and 4
		if (mV != 1 && mV != 2 && mV != 4) {
			TRACE_LOG1("DecryptionHelper::Setup, Only 1 and 2 are supported values for V. Unsupported filter encountered - %d", mV);
			break;
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
			mO = XCryptionCommon::stringToByteList(oHelper.ToString());
		}

		RefCountPtr<PDFObject> u(inParser->QueryDictionaryObject(encryptionDictionary.GetPtr(), "U"));
		if (!u) {
			break;
		}
		else {
			ParsedPrimitiveHelper uHelper(u.GetPtr());
			mU = XCryptionCommon::stringToByteList(uHelper.ToString());
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
				mFileIDPart1 = XCryptionCommon::stringToByteList(idPart1ObjectHelper.ToString());
			}
		}


		RefCountPtr<PDFObject> length(inParser->QueryDictionaryObject(encryptionDictionary.GetPtr(), "Length"));
		if (!length) {
			mLength = 40 / 8;
		}
		else {
			mLength = ComputeByteLength(length.GetPtr());
		}

		// Setup crypt filters, or a default filter
		if (mV == 4) {
			// multiple xcryptions. read crypt filters, determine which does what
			PDFObjectCastPtr<PDFDictionary> cryptFilters(inParser->QueryDictionaryObject(encryptionDictionary.GetPtr(), "CF"));
			if (!!cryptFilters) {
				MapIterator<PDFNameToPDFObjectMap>  cryptFiltersIt = cryptFilters->GetIterator();

				// read crypt filters
				while (cryptFiltersIt.MoveNext())
				{
					PDFObjectCastPtr<PDFDictionary> cryptFilter;
					// A little caveat of those smart ptrs need to be handled here
					// make sure to pass the pointer after init...otherwise cast wont do addref
					// and object will be released
					cryptFilter = cryptFiltersIt.GetValue();
					if (!!cryptFilter) {
						PDFObjectCastPtr<PDFName> cfmName(inParser->QueryDictionaryObject(cryptFilter.GetPtr(), "CFM"));
						RefCountPtr<PDFObject> lengthObject(inParser->QueryDictionaryObject(cryptFilter.GetPtr(), "Length"));
						unsigned int length = !lengthObject ? mLength : ComputeByteLength(lengthObject.GetPtr());

						XCryptionCommon* encryption = new XCryptionCommon();
						encryption->Setup(cfmName->GetValue() == "AESV2"); // singe xcryptions are always RC4
						encryption->SetupInitialEncryptionKey(
							inPassword,
							mRevision,
							length,
							mO,
							mP,
							mFileIDPart1,
							mEncryptMetaData);
						mXcrypts.insert(StringToXCryptionCommonMap::value_type(cryptFiltersIt.GetKey()->GetValue(), encryption));
					}
				}
				

				PDFObjectCastPtr<PDFName> streamsFilterName(inParser->QueryDictionaryObject(encryptionDictionary.GetPtr(), "StmF"));
				PDFObjectCastPtr<PDFName> stringsFilterName(inParser->QueryDictionaryObject(encryptionDictionary.GetPtr(), "StrF"));
				mXcryptStreams = GetFilterForName(mXcrypts, !streamsFilterName ? "Identity": streamsFilterName->GetValue());
				mXcryptStrings = GetFilterForName(mXcrypts, !stringsFilterName ? "Identity" : stringsFilterName->GetValue());
				mXcryptAuthentication = GetFilterForName(mXcrypts, scStdCF);
			}

		}
		else {
			// single xcryption, use as the single encryption source
			XCryptionCommon* defaultEncryption = new XCryptionCommon();
			defaultEncryption->Setup(false); // single xcryptions are always RC4
			defaultEncryption->SetupInitialEncryptionKey(
				inPassword,
				mRevision,
				mLength,
				mO,
				mP,
				mFileIDPart1,
				mEncryptMetaData);

			mXcrypts.insert(StringToXCryptionCommonMap::value_type(scStdCF, defaultEncryption));
			mXcryptStreams = defaultEncryption;
			mXcryptStrings = defaultEncryption;
			mXcryptAuthentication = defaultEncryption;
		}

		// authenticate password, try to determine if user or owner
		ByteList password = XCryptionCommon::stringToByteList(inPassword);
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

bool HasCryptFilterDefinition(PDFParser* inParser, PDFStreamInput* inStream) {
	RefCountPtr<PDFDictionary> streamDictionary(inStream->QueryStreamDictionary());

	// check if stream has a crypt filter
	RefCountPtr<PDFObject> filterObject(inParser->QueryDictionaryObject(streamDictionary.GetPtr(), "Filter"));
	if (!filterObject)
	{
		// no filter, so stop here
		return false;
	}

	if (filterObject->GetType() == PDFObject::ePDFObjectArray)
	{
		PDFArray* filterObjectArray = (PDFArray*)filterObject.GetPtr();
		bool foundCrypt = false;
		for (unsigned long i = 0; i < filterObjectArray->GetLength() && !foundCrypt; ++i)
		{
			PDFObjectCastPtr<PDFName> filterObjectItem(filterObjectArray->QueryObject(i));
			if (!filterObjectItem)
			{
				// error
				break;
			}
			foundCrypt = filterObjectItem->GetValue() == "Crypt";
		}
		return foundCrypt;
	}
	else if (filterObject->GetType() == PDFObject::ePDFObjectName)
	{
		return ((PDFName*)(filterObject.GetPtr()))->GetValue() == "Crypt";
	}
	else
		return false; //???
}


IByteReader* DecryptionHelper::CreateDefaultDecryptionFilterForStream(PDFStreamInput* inStream, IByteReader* inToWrapStream) {
	// This will create a decryption filter for streams that dont have their own defined crypt filters. null for no decryption filter
	if (!IsEncrypted() || !CanDecryptDocument() || HasCryptFilterDefinition(mParser, inStream) || !mXcryptStreams)
		return NULL;
	
	IDeletable* savedEcnryptionKey = inStream->GetMetadata(scEcnryptionKeyMetadataKey);
	if (savedEcnryptionKey) {
		return CreateDecryptionReader(inToWrapStream, *(((Deletable<ByteList>*)savedEcnryptionKey)->GetPtr()), mXcryptStreams->IsUsingAES());
	}
	else 
		return NULL;
}

IByteReader*  DecryptionHelper::CreateDecryptionFilterForStream(PDFStreamInput* inStream, IByteReader* inToWrapStream, const std::string& inCryptName) {
	// note that here the original stream is returned instead of null
	if (!IsEncrypted() || !CanDecryptDocument())
		return inToWrapStream;

	IDeletable* savedEcnryptionKey = inStream->GetMetadata(scEcnryptionKeyMetadataKey);
	if (!savedEcnryptionKey) {
		// sign for no encryption here
		return inToWrapStream;
	}
	XCryptionCommon* xcryption = GetFilterForName(mXcrypts, inCryptName);

	if (xcryption && savedEcnryptionKey) {
		return CreateDecryptionReader(inToWrapStream, *(((Deletable<ByteList>*)savedEcnryptionKey)->GetPtr()), xcryption->IsUsingAES());
	}
	else
		return inToWrapStream;

}

bool DecryptionHelper::IsDecrypting() {
	return IsEncrypted() && CanDecryptDocument() && mDecryptionPauseLevel == 0;
}

std::string DecryptionHelper::DecryptString(const std::string& inStringToDecrypt) {
	if (!IsDecrypting() || !mXcryptStrings)
		return inStringToDecrypt;

	IByteReader* decryptStream = CreateDecryptionReader(new InputStringStream(inStringToDecrypt), mXcryptStrings->GetCurrentObjectKey(), mXcryptStrings->IsUsingAES());
	if (decryptStream) {
		OutputStringBufferStream outputStream;
		OutputStreamTraits traits(&outputStream);
		traits.CopyToOutputStream(decryptStream);

		delete decryptStream;
		return outputStream.ToString();
	}
	else
		return inStringToDecrypt;
}

void DecryptionHelper::OnObjectStart(long long inObjectID, long long inGenerationNumber) {
	StringToXCryptionCommonMap::iterator it = mXcrypts.begin();
	for (; it != mXcrypts.end(); ++it) {
		it->second->OnObjectStart(inObjectID, inGenerationNumber);
	}
}


XCryptionCommon* DecryptionHelper::GetCryptForStream(PDFStreamInput* inStream) {
	// Get crypt for stream will return the right crypt filter thats supposed to be used for stream
	// whether its the default stream encryption or a specific filter defined in the stream
	// not the assumption (well, one that's all over) that if the name is not found in the CF dict, it
	// will be "identity" which is the same as providing NULL as the xcryptioncommon return value

	if (HasCryptFilterDefinition(mParser, inStream)) {
		// find position of crypt filter, and get the name of the crypt filter from the decodeParams
		RefCountPtr<PDFDictionary> streamDictionary(inStream->QueryStreamDictionary());

		RefCountPtr<PDFObject> filterObject(mParser->QueryDictionaryObject(streamDictionary.GetPtr(), "Filter"));
		if (filterObject->GetType() == PDFObject::ePDFObjectArray)
		{
			PDFArray* filterObjectArray = (PDFArray*)filterObject.GetPtr();
			unsigned long i = 0;
			for (; i < filterObjectArray->GetLength(); ++i)
			{
				PDFObjectCastPtr<PDFName> filterObjectItem(filterObjectArray->QueryObject(i));
				if (!filterObjectItem || filterObjectItem->GetValue() == "Crypt")
					break;
			}
			if (i < filterObjectArray->GetLength()) {
				PDFObjectCastPtr<PDFArray> decodeParams(mParser->QueryDictionaryObject(streamDictionary.GetPtr(), "DecodeParms"));
				if (!decodeParams)
					return mXcryptStreams;
				// got index, look for the name in the decode params array
				PDFObjectCastPtr<PDFDictionary> decodeParamsItem((mParser->QueryArrayObject(decodeParams.GetPtr(), i)));
				if (!decodeParamsItem)
					return mXcryptStreams;

				PDFObjectCastPtr<PDFName> cryptFilterName(mParser->QueryDictionaryObject(decodeParamsItem.GetPtr(), "Name"));
				return GetFilterForName(mXcrypts, cryptFilterName->GetValue());

			}
			else
				return mXcryptStreams; // this shouldn't realy happen
		}
		else if (filterObject->GetType() == PDFObject::ePDFObjectName)
		{
			// has to be crypt filter, look for the name in decode params
			PDFObjectCastPtr<PDFDictionary> decodeParamsItem((mParser->QueryDictionaryObject(streamDictionary.GetPtr(), "DecodeParms")));
			if (!decodeParamsItem)
				return mXcryptStreams;

			PDFObjectCastPtr<PDFName> cryptFilterName(mParser->QueryDictionaryObject(decodeParamsItem.GetPtr(), "Name"));
			return GetFilterForName(mXcrypts, cryptFilterName->GetValue());
		}
		else
			return mXcryptStreams; // ???
	}
	else {
		return mXcryptStreams;
	}
}

void DecryptionHelper::OnObjectEnd(PDFObject* inObject) {
	if (inObject == NULL)
		return;
	
	// for streams, retain the encryption key with them, so i can later decrypt them when needed
	if ((inObject->GetType() == PDFObject::ePDFObjectStream) && IsDecrypting()) {
		XCryptionCommon* streamCryptFilter = GetCryptForStream((PDFStreamInput*)inObject);
		if (streamCryptFilter) {
			ByteList* savedKey = new ByteList(streamCryptFilter->GetCurrentObjectKey());
			inObject->SetMetadata(scEcnryptionKeyMetadataKey,new Deletable<ByteList>(savedKey));
		}
	}

	StringToXCryptionCommonMap::iterator it = mXcrypts.begin();
	for (; it != mXcrypts.end(); ++it) {
		it->second->OnObjectEnd();
	}
}

IByteReader* DecryptionHelper::CreateDecryptionReader(IByteReader* inSourceStream, const ByteList& inEncryptionKey, bool inIsUsingAES) {
	if (inIsUsingAES)
		return new InputAESDecodeStream(inSourceStream, inEncryptionKey);
	else
		return new InputRC4XcodeStream(inSourceStream, inEncryptionKey);
}


bool DecryptionHelper::AuthenticateUserPassword(const ByteList& inPassword) {
	if (!mXcryptAuthentication)
		return true;
	return mXcryptAuthentication->algorithm3_6(mRevision,
						mLength,
						inPassword,
						mO,
						mP,
						mFileIDPart1,
						mEncryptMetaData,
						mU);
}

bool DecryptionHelper::AuthenticateOwnerPassword(const ByteList& inPassword) {
	if (!mXcryptAuthentication)
		return true;

	return mXcryptAuthentication->algorithm3_7(mRevision,
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
	return mXcryptAuthentication->GetInitialEncryptionKey();
}

void DecryptionHelper::PauseDecryption() {
	++mDecryptionPauseLevel;
}

void DecryptionHelper::ReleaseDecryption() {
	--mDecryptionPauseLevel;
}

const StringToXCryptionCommonMap& DecryptionHelper::GetXcrypts() const {
	return mXcrypts;
}

XCryptionCommon* DecryptionHelper::GetStreamXcrypt() const {
	return mXcryptStreams;
}

XCryptionCommon* DecryptionHelper::GetStringXcrypt() const {
	return mXcryptStrings;
}

XCryptionCommon* DecryptionHelper::GetAuthenticationXcrypt() const {
	return mXcryptAuthentication;
}
