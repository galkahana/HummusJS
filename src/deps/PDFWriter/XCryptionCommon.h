/*
Source File : XCryptionCommon.h


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

#include "IOBasicTypes.h"
#include "ObjectsBasicTypes.h"

#include <list>
#include <string>
#include <map>

class XCryptionCommon;

typedef std::list<IOBasicTypes::Byte> ByteList;
typedef std::list<ByteList> ByteListList;
typedef std::map<std::string, XCryptionCommon*> StringToXCryptionCommonMap;


class XCryptionCommon {
public:

	XCryptionCommon(void);
	virtual ~XCryptionCommon(void);

	/*
		To start using XCryptionCommon do this:

		1. Create an instance of the object
		2. Call Initial Setup with the PDF level. return bool will tell you if XCryption is supported for this input. if not...don't bother with the rest
		3. Call SetupInitialEncryptionKey. This may require some initial computations with other algorithms of this object, per what input you may possess. It is fine
			to call any method prior to SetupInitialEncryptionKey, but those that relate to state - OnObjectStart,OnObjectEnd, GetCurrentObjectKey. these require the computation.
			Call this whenever you acquired the required input for it, no matter when. 
		4. now you can call all methods freely
	*/

	// call this whenever you first can. 
	void Setup(bool inUsingAES);

	void SetupInitialEncryptionKey(const std::string& inUserPassword,
		unsigned int inRevision,
		unsigned int inLength,
		const ByteList& inO,
		long long inP,
		const ByteList& inFileIDPart1,
		bool inEncryptMetaData
		);
	// Setup key directly
	void SetupInitialEncryptionKey(const ByteList& inEncryptionKey);


	// Call on object start, will recompute a key for the new object (returns, so you can use if makes sense)
	const ByteList& OnObjectStart(long long inObjectID, long long inGenerationNumber);
	// Call on object end, will pop the computed key for this object
	void OnObjectEnd();
	const ByteList& GetCurrentObjectKey(); // will return empty key if stack is empty

	// get original encryption key (without particular object additions)
	const ByteList& GetInitialEncryptionKey() const;

	// bytelist operations (class methods)
	static ByteList stringToByteList(const std::string& inString);
	static ByteList substr(const ByteList& inList, IOBasicTypes::LongBufferSizeType inStart, IOBasicTypes::LongBufferSizeType inEnd);
	static void append(ByteList& ioTargetList, const ByteList& inSource);
	static ByteList add(const ByteList& inA, const ByteList& inB);
	static std::string ByteListToString(const ByteList& inByteList);

	// PDF xcryption algorithms (Important! length must be passed in bytes and not in bits. which normally means - the PDF value / 8)
	ByteList algorithm3_1(ObjectIDType inObjectNumber,
		unsigned long inGenerationNumber,
		const ByteList& inEncryptionKey,
		bool inIsUsingAES);
	ByteList algorithm3_2(unsigned int inRevision,
		unsigned int inLength,
		const ByteList& inPassword,
		const ByteList& inO,
		long long inP,
		const ByteList& inFileIDPart1,
		bool inEncryptMetaData);
	ByteList algorithm3_3(unsigned int inRevision,
		unsigned int inLength,
		const ByteList& inOwnerPassword,
		const ByteList& inUserPassword);
	ByteList algorithm3_4(unsigned int inLength,
		const ByteList& inUserPassword,
		const ByteList& inO,
		long long inP,
		const ByteList& inFileIDPart1,
		bool inEncryptMetaData);
	ByteList algorithm3_5(unsigned int inRevision,
		unsigned int inLength,
		const ByteList& inUserPassword,
		const ByteList& inO,
		long long inP,
		const ByteList& inFileIDPart1,
		bool inEncryptMetaData);
	bool algorithm3_6(unsigned int inRevision,
		unsigned int inLength,
		const ByteList& inPassword,
		const ByteList& inO,
		long long inP,
		const ByteList& inFileIDPart1,
		bool inEncryptMetaData,
		const ByteList inU);
	bool algorithm3_7(unsigned int inRevision,
		unsigned int inLength,
		const ByteList& inPassword,
		const ByteList& inO,
		long long inP,
		const ByteList& inFileIDPart1,
		bool inEncryptMetaData,
		const ByteList inU);

	bool IsUsingAES();

private:
	ByteList mPaddingFiller;
	ByteListList mEncryptionKeysStack;
	bool mUsingAES;
	ByteList mEncryptionKey;

	ByteList RC4Encode(const ByteList& inKey, const ByteList& inToEncode);
	ByteList ComputeEncryptionKeyForObject(ObjectIDType inObjectNumber, unsigned long inGenerationNumber); // with algorithm3_1

};