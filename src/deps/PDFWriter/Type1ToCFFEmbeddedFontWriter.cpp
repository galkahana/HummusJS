/*
   Source File : Type1ToCFFEmbeddedFontWriter.cpp


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
#include "Type1ToCFFEmbeddedFontWriter.h"
#include "FreeTypeFaceWrapper.h"
#include "ObjectsContext.h"
#include "DictionaryContext.h"
#include "InputStringBufferStream.h"
#include "OutputStreamTraits.h"
#include "PDFStream.h"
#include "Trace.h"
#include "Type1ToType2Converter.h"
#include "FSType.h"
#include "StandardEncoding.h"

#include <algorithm>

using namespace PDFHummus;

Type1ToCFFEmbeddedFontWriter::Type1ToCFFEmbeddedFontWriter(void)
{
	mCharset = NULL;
}

Type1ToCFFEmbeddedFontWriter::~Type1ToCFFEmbeddedFontWriter(void)
{
}

#define N_STD_STRINGS 391

// NOTE - this is not the original array order. it's a sorted one. i'm using it to determine whether and at what position
// a string might be in the standard strings array.
static const char* scSortedStandardStrings[N_STD_STRINGS] = {
	".notdef","001.000","001.001","001.002","001.003","A","AE","AEsmall","Aacute","Aacutesmall",
	"Acircumflex","Acircumflexsmall","Acutesmall","Adieresis","Adieresissmall","Agrave","Agravesmall","Aring","Aringsmall","Asmall",
	"Atilde","Atildesmall","B","Black","Bold","Book","Brevesmall","Bsmall","C","Caronsmall",
	"Ccedilla","Ccedillasmall","Cedillasmall","Circumflexsmall","Csmall","D","Dieresissmall","Dotaccentsmall","Dsmall","E",
	"Eacute","Eacutesmall","Ecircumflex","Ecircumflexsmall","Edieresis","Edieresissmall","Egrave","Egravesmall","Esmall","Eth",
	"Ethsmall","F","Fsmall","G","Gravesmall","Gsmall","H","Hsmall","Hungarumlautsmall","I",
	"Iacute","Iacutesmall","Icircumflex","Icircumflexsmall","Idieresis","Idieresissmall","Igrave","Igravesmall","Ismall","J",
	"Jsmall","K","Ksmall","L","Light","Lslash","Lslashsmall","Lsmall","M","Macronsmall",
	"Medium","Msmall","N","Nsmall","Ntilde","Ntildesmall","O","OE","OEsmall","Oacute",
	"Oacutesmall","Ocircumflex","Ocircumflexsmall","Odieresis","Odieresissmall","Ogoneksmall","Ograve","Ogravesmall","Oslash","Oslashsmall",
	"Osmall","Otilde","Otildesmall","P","Psmall","Q","Qsmall","R","Regular","Ringsmall",
	"Roman","Rsmall","S","Scaron","Scaronsmall","Semibold","Ssmall","T","Thorn","Thornsmall",
	"Tildesmall","Tsmall","U","Uacute","Uacutesmall","Ucircumflex","Ucircumflexsmall","Udieresis","Udieresissmall","Ugrave",
	"Ugravesmall","Usmall","V","Vsmall","W","Wsmall","X","Xsmall","Y","Yacute",
	"Yacutesmall","Ydieresis","Ydieresissmall","Ysmall","Z","Zcaron","Zcaronsmall","Zsmall","a","aacute",
	"acircumflex","acute","adieresis","ae","agrave","ampersand","ampersandsmall","aring","asciicircum","asciitilde",
	"asterisk","asuperior","at","atilde","b","backslash","bar","braceleft","braceright","bracketleft",
	"bracketright","breve","brokenbar","bsuperior","bullet","c","caron","ccedilla","cedilla","cent",
	"centinferior","centoldstyle","centsuperior","circumflex","colon","colonmonetary","comma","commainferior","commasuperior","copyright",
	"currency","d","dagger","daggerdbl","degree","dieresis","divide","dollar","dollarinferior","dollaroldstyle",
	"dollarsuperior","dotaccent","dotlessi","dsuperior","e","eacute","ecircumflex","edieresis","egrave","eight",
	"eightinferior","eightoldstyle","eightsuperior","ellipsis","emdash","endash","equal","esuperior","eth","exclam",
	"exclamdown","exclamdownsmall","exclamsmall","f","ff","ffi","ffl","fi","figuredash","five",
	"fiveeighths","fiveinferior","fiveoldstyle","fivesuperior","fl","florin","four","fourinferior","fouroldstyle","foursuperior",
	"fraction","g","germandbls","grave","greater","guillemotleft","guillemotright","guilsinglleft","guilsinglright","h",
	"hungarumlaut","hyphen","hypheninferior","hyphensuperior","i","iacute","icircumflex","idieresis","igrave","isuperior",
	"j","k","l","less","logicalnot","lslash","lsuperior","m","macron","minus",
	"msuperior","mu","multiply","n","nine","nineinferior","nineoldstyle","ninesuperior","nsuperior","ntilde",
	"numbersign","o","oacute","ocircumflex","odieresis","oe","ogonek","ograve","one","onedotenleader",
	"oneeighth","onefitted","onehalf","oneinferior","oneoldstyle","onequarter","onesuperior","onethird","ordfeminine","ordmasculine",
	"oslash","osuperior","otilde","p","paragraph","parenleft","parenleftinferior","parenleftsuperior","parenright","parenrightinferior",
	"parenrightsuperior","percent","period","periodcentered","periodinferior","periodsuperior","perthousand","plus","plusminus","q",
	"question","questiondown","questiondownsmall","questionsmall","quotedbl","quotedblbase","quotedblleft","quotedblright","quoteleft","quoteright",
	"quotesinglbase","quotesingle","r","registered","ring","rsuperior","rupiah","s","scaron","section",
	"semicolon","seven","seveneighths","seveninferior","sevenoldstyle","sevensuperior","six","sixinferior","sixoldstyle","sixsuperior",
	"slash","space","ssuperior","sterling","t","thorn","three","threeeighths","threeinferior","threeoldstyle",
	"threequarters","threequartersemdash","threesuperior","tilde","trademark","tsuperior","two","twodotenleader","twoinferior","twooldstyle",
	"twosuperior","twothirds","u","uacute","ucircumflex","udieresis","ugrave","underscore","v","w",
	"x","y","yacute","ydieresis","yen","z","zcaron","zero","zeroinferior","zerooldstyle",
	"zerosuperior"
};

static unsigned short scSortedStandardStringsPositions[N_STD_STRINGS] = {
	0,379,380,381,382,34,138,353,171,348,
	172,349,234,173,351,174,347,175,352,274,
	176,350,35,383,384,385,310,275,36,311,
	177,354,318,271,276,37,309,312,277,38,
	178,356,179,357,180,358,181,355,278,154,
	363,39,279,40,273,280,41,281,230,42,
	182,360,183,361,184,362,185,359,282,43,
	283,44,284,45,386,140,306,285,46,313,
	387,286,47,287,186,364,48,142,370,187,
	366,188,367,189,369,316,190,365,141,371,
	288,191,368,49,289,50,290,51,388,317,
	389,291,52,192,307,390,292,53,157,377,
	303,293,54,193,373,194,374,195,375,196,
	372,294,55,295,56,296,57,297,58,197,
	376,198,378,298,59,199,308,299,66,200,
	201,125,202,144,203,7,233,204,63,95,
	11,253,33,205,67,61,93,92,94,60,
	62,129,160,254,116,68,136,206,133,97,
	343,305,255,126,27,300,13,346,249,170,
	103,69,112,113,161,131,159,5,344,231,
	232,130,145,256,70,207,208,209,210,25,
	341,247,331,121,137,111,30,257,167,2,
	96,304,229,71,266,267,268,109,314,22,
	322,338,244,328,110,101,21,337,243,327,
	99,72,149,124,31,106,120,107,108,73,
	134,14,315,272,74,211,212,213,214,258,
	75,76,77,29,151,146,259,78,128,166,
	260,152,168,79,26,342,248,332,261,215,
	4,80,216,217,218,148,135,219,18,238,
	320,301,155,334,240,158,150,324,139,143,
	147,262,220,81,115,9,269,235,10,270,
	236,6,15,114,345,251,122,12,156,82,
	32,123,319,252,3,118,105,119,65,8,
	117,104,83,165,132,263,302,84,221,102,
	28,24,323,340,246,330,23,339,245,329,
	16,1,264,98,85,162,20,321,336,242,
	163,250,169,127,153,265,19,237,335,241,
	164,325,86,222,223,224,225,64,87,88,
	89,90,226,227,100,91,228,17,333,239,
	326
};

static const std::string scSubtype = "Subtype";

EStatusCode Type1ToCFFEmbeddedFontWriter::WriteEmbeddedFont(	
															FreeTypeFaceWrapper& inFontInfo,
															const UIntVector& inSubsetGlyphIDs,
															const std::string& inFontFile3SubType,
															const std::string& inSubsetFontName,
															ObjectsContext* inObjectsContext,
															ObjectIDType& outEmbeddedFontObjectID)
{
	MyStringBuf rawFontProgram; 
	bool notEmbedded;
		// as oppose to true type, the reason for using a memory stream here is mainly peformance - i don't want to start
		// setting file pointers and move in a file stream
	EStatusCode status;

	do
	{
		status = CreateCFFSubset(inFontInfo,inSubsetGlyphIDs,inSubsetFontName,notEmbedded,rawFontProgram);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::WriteEmbeddedFont, failed to write embedded font program");
			break;
		}	

		if(notEmbedded)
		{
			// can't embed. mark succesful, and go back empty
			outEmbeddedFontObjectID = 0;
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::WriteEmbeddedFont, font may not be embedded. so not embedding");
			return PDFHummus::eSuccess;
		}

		outEmbeddedFontObjectID = inObjectsContext->StartNewIndirectObject();
		
		DictionaryContext* fontProgramDictionaryContext = inObjectsContext->StartDictionary();

		rawFontProgram.pubseekoff(0,std::ios_base::beg);

		fontProgramDictionaryContext->WriteKey(scSubtype);
		fontProgramDictionaryContext->WriteNameValue(inFontFile3SubType);
		PDFStream* pdfStream = inObjectsContext->StartPDFStream(fontProgramDictionaryContext);


		// now copy the created font program to the output stream
		InputStringBufferStream fontProgramStream(&rawFontProgram);
		OutputStreamTraits streamCopier(pdfStream->GetWriteStream());
		status = streamCopier.CopyToOutputStream(&fontProgramStream);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::WriteEmbeddedFont, failed to copy font program into pdf stream");
			break;
		}


		inObjectsContext->EndPDFStream(pdfStream);
		delete pdfStream;
	}while(false);

	return status;		

}

EStatusCode Type1ToCFFEmbeddedFontWriter::CreateCFFSubset(	
															FreeTypeFaceWrapper& inFontInfo,
															const UIntVector& inSubsetGlyphIDs,
															const std::string& inSubsetFontName,
															bool& outNotEmbedded,
															MyStringBuf& outFontProgram)
{
	EStatusCode status;

	do
	{
		UIntVector subsetGlyphIDs = inSubsetGlyphIDs;
		StringVector subsetGlyphNames;

		if(subsetGlyphIDs.front() != 0) // make sure 0 glyph is in
			subsetGlyphIDs.insert(subsetGlyphIDs.begin(),0);

		status = mType1File.OpenFile(inFontInfo.GetFontFilePath());
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG1("Type1ToCFFEmbeddedFontWriter::CreateCFFSubset, cannot open Type 1 font file at %s",inFontInfo.GetFontFilePath().c_str());
			break;
		}

		status = mType1Input.ReadType1File(mType1File.GetInputStream());
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::CreateCFFSubset, failed to read Type 1 file");
			break;
		}

		// see if font may be embedded
		if(mType1Input.mFontDictionary.FSTypeValid || mType1Input.mFontInfoDictionary.FSTypeValid)
		{
			if(!FSType(
					mType1Input.mFontInfoDictionary.FSTypeValid ? 
						mType1Input.mFontInfoDictionary.fsType :
						mType1Input.mFontDictionary.fsType).CanEmbed())
			{
				outNotEmbedded = true;
				return PDFHummus::eSuccess;
			}
			else
				outNotEmbedded = false;
		}
		else
			outNotEmbedded = false;


		// Found big gap between FreeType indexing and the way it's in the Type 1. obvioulsy due to encoding differences.
		// So i'm replacing the indexes of free type, with names...should be safer (also cleans up invalid glyph ids, in case
        // direct glyphs placement put them here)
        TranslateFromFreeTypeToType1(inFontInfo,subsetGlyphIDs,subsetGlyphNames);

		status = AddDependentGlyphs(subsetGlyphNames);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::CreateCFFSubset, failed to add dependent glyphs");
			break;
		}

		mFontFileStream.Assign(&outFontProgram);
		mPrimitivesWriter.SetStream(&mFontFileStream);

		status = WriteCFFHeader();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::CreateCFFSubset, failed to write CFF header");
			break;
		}

		status = WriteName(inSubsetFontName);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::CreateCFFSubset, failed to write CFF Name");
			break;
		}

		status = WriteTopIndex();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::CreateCFFSubset, failed to write Top Index");
			break;
		}

		// prepraring charset happens here, so that any added strings to the string index will happen...before 
		// the index is written
		PrepareCharSetArray(subsetGlyphNames);

		status = WriteStringIndex();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::CreateCFFSubset, failed to write String Index");
			break;
		}

		status = WriteGlobalSubrsIndex();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::CreateCFFSubset, failed to write global subrs index");
			break;
		}

		status = WriteEncodings(subsetGlyphNames);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::CreateCFFSubset, failed to write encodings");
			break;
		}

		status = WriteCharsets(subsetGlyphNames);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::CreateCFFSubset, failed to write charstring");
			break;
		}

		status = WriteCharStrings(subsetGlyphNames);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::CreateCFFSubset, failed to write charstring");
			break;
		}

		status = WritePrivateDictionary();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::CreateCFFSubset, failed to write private");
			break;
		}

		status = UpdateIndexesAtTopDict();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("Type1ToCFFEmbeddedFontWriter::CreateCFFSubset, failed to update indexes");
			break;			
		}
	}while(false);

	mType1File.CloseFile();
	FreeTemporaryStructs();
	return status;	
}

void Type1ToCFFEmbeddedFontWriter::FreeTemporaryStructs()
{
	mStrings.clear();
	mNonStandardStringToIndex.clear();
	delete[] mCharset;
}

EStatusCode Type1ToCFFEmbeddedFontWriter::AddDependentGlyphs(StringVector& ioSubsetGlyphIDs)
{
	EStatusCode status = PDFHummus::eSuccess;
	StringSet glyphsSet;
	StringVector::iterator it = ioSubsetGlyphIDs.begin();
	bool hasCompositeGlyphs = false;

	for(;it != ioSubsetGlyphIDs.end() && PDFHummus::eSuccess == status; ++it)
	{
		bool localHasCompositeGlyphs;
		status = AddComponentGlyphs(*it,glyphsSet,localHasCompositeGlyphs);
		hasCompositeGlyphs |= localHasCompositeGlyphs;
	}

	if(hasCompositeGlyphs)
	{
		StringSet::iterator itNewGlyphs;

		for(it = ioSubsetGlyphIDs.begin();it != ioSubsetGlyphIDs.end(); ++it)
			glyphsSet.insert(*it);

		ioSubsetGlyphIDs.clear();
		for(itNewGlyphs = glyphsSet.begin(); itNewGlyphs != glyphsSet.end(); ++itNewGlyphs)
			ioSubsetGlyphIDs.push_back(*itNewGlyphs);
		
		sort(ioSubsetGlyphIDs.begin(),ioSubsetGlyphIDs.end());
	}	
	return status;	
}

EStatusCode Type1ToCFFEmbeddedFontWriter::AddComponentGlyphs(const std::string& inGlyphID,StringSet& ioComponents,bool &outFoundComponents)
{
	CharString1Dependencies dependencies;
	StandardEncoding standardEncoding;
	EStatusCode status = mType1Input.CalculateDependenciesForCharIndex(inGlyphID,dependencies);

	if(PDFHummus::eSuccess == status && dependencies.mCharCodes.size() !=0)
	{
		ByteSet::iterator it = dependencies.mCharCodes.begin();
		for(; it != dependencies.mCharCodes.end() && PDFHummus::eSuccess == status; ++it)
		{
			bool dummyFound;
			/*
				Using standard encoding instead of the font encoding, because SEAC (the only operator to create glyph dependency in type 1)
				relies on standard encoding indexes by definition.
			*/
			std::string glyphName = standardEncoding.GetEncodedGlyphName(*it);
			ioComponents.insert(glyphName);
			status = AddComponentGlyphs(glyphName,ioComponents,dummyFound);
		}
		outFoundComponents = true;
	}
	else
		outFoundComponents = false;
	return status;
}

EStatusCode Type1ToCFFEmbeddedFontWriter::WriteCFFHeader()
{
	// write a default CFF header. i'm using dummies for the offset sizes, cause i don't think they really
	// matter anyways.
	mPrimitivesWriter.WriteCard8(1);
	mPrimitivesWriter.WriteCard8(0);
	mPrimitivesWriter.WriteCard8(4);
	mPrimitivesWriter.WriteOffSize(2);

	return mPrimitivesWriter.GetInternalState();
}

EStatusCode Type1ToCFFEmbeddedFontWriter::WriteName(const std::string& inSubsetFontName)
{	
	// get the first name from the name table, and write it here

	std::string fontName = inSubsetFontName.size() == 0 ? mType1Input.mFontDictionary.FontName : inSubsetFontName;

	Byte sizeOfOffset = GetMostCompressedOffsetSize((unsigned long)fontName.size() + 1);

	mPrimitivesWriter.WriteCard16(1);
	mPrimitivesWriter.WriteOffSize(sizeOfOffset);
	mPrimitivesWriter.SetOffSize(sizeOfOffset);
	mPrimitivesWriter.WriteOffset(1);
	mPrimitivesWriter.WriteOffset((unsigned long)fontName.size() + 1);
	mPrimitivesWriter.Write((const Byte*)fontName.c_str(),fontName.size());	

	return mPrimitivesWriter.GetInternalState();

}

Byte Type1ToCFFEmbeddedFontWriter::GetMostCompressedOffsetSize(unsigned long inOffset)
{
	if(inOffset < 256)
		return 1;
	
	if(inOffset < 65536)
		return 2;

	if(inOffset < 1<<24)
		return 3;

	return 4;
}

EStatusCode Type1ToCFFEmbeddedFontWriter::WriteTopIndex()
{

	EStatusCode status;
	MyStringBuf topDictSegment; 

	do
	{
		status = WriteTopDictSegment(topDictSegment);
		if(status != PDFHummus::eSuccess)
			break;


		// write index section
		Byte sizeOfOffset = GetMostCompressedOffsetSize((unsigned long)topDictSegment.GetCurrentWritePosition() + 1);

		mPrimitivesWriter.WriteCard16(1);
		mPrimitivesWriter.WriteOffSize(sizeOfOffset);
		mPrimitivesWriter.SetOffSize(sizeOfOffset);
		mPrimitivesWriter.WriteOffset(1);
		mPrimitivesWriter.WriteOffset((unsigned long)topDictSegment.GetCurrentWritePosition() + 1);

		topDictSegment.pubseekoff(0,std::ios_base::beg);

		LongFilePositionType topDictDataOffset = mFontFileStream.GetCurrentPosition();

		// Write data
		InputStringBufferStream topDictStream(&topDictSegment);
		OutputStreamTraits streamCopier(&mFontFileStream);
		status = streamCopier.CopyToOutputStream(&topDictStream);
		if(status != PDFHummus::eSuccess)
			break;

		// Adjust position locators for important placeholders
		mCharsetPlaceHolderPosition+=topDictDataOffset;
		mEncodingPlaceHolderPosition+=topDictDataOffset;
		mCharstringsPlaceHolderPosition+=topDictDataOffset;
		mPrivatePlaceHolderPosition+=topDictDataOffset;

	}while(false);

	if(status != PDFHummus::eSuccess)
		return status;
	else
		return mPrimitivesWriter.GetInternalState();
	return status;
}

static const unsigned short scCharset = 15;
static const unsigned short scEncoding = 16;
static const unsigned short scCharstrings = 17;
static const unsigned short scPrivate = 18;

EStatusCode Type1ToCFFEmbeddedFontWriter::WriteTopDictSegment(MyStringBuf& ioTopDictSegment)
{
	OutputStringBufferStream topDictStream(&ioTopDictSegment);
	CFFPrimitiveWriter dictPrimitiveWriter;
	dictPrimitiveWriter.SetStream(&topDictStream);

	// write dictionary keys

	AddStringOperandIfNotEmpty(dictPrimitiveWriter,mType1Input.mFontInfoDictionary.version,0);
	AddStringOperandIfNotEmpty(dictPrimitiveWriter,mType1Input.mFontInfoDictionary.Notice,1);
	AddStringOperandIfNotEmpty(dictPrimitiveWriter,mType1Input.mFontInfoDictionary.Copyright,0xC00);
	AddStringOperandIfNotEmpty(dictPrimitiveWriter,mType1Input.mFontInfoDictionary.FullName,2);
	AddStringOperandIfNotEmpty(dictPrimitiveWriter,mType1Input.mFontInfoDictionary.FamilyName,3);
	AddStringOperandIfNotEmpty(dictPrimitiveWriter,mType1Input.mFontInfoDictionary.Weight,4);
	AddNumberOperandIfNotDefault(dictPrimitiveWriter,(int)(mType1Input.mFontInfoDictionary.isFixedPitch ? 1:0),0xC01,0);
	AddNumberOperandIfNotDefault(dictPrimitiveWriter,mType1Input.mFontInfoDictionary.ItalicAngle,0xC02,0.0);
	AddNumberOperandIfNotDefault(dictPrimitiveWriter,mType1Input.mFontInfoDictionary.UnderlinePosition,0xC03,-100.0);
	AddNumberOperandIfNotDefault(dictPrimitiveWriter,mType1Input.mFontInfoDictionary.UnderlineThickness,0xC04,50.0);
	if (mType1Input.mFontDictionary.UniqueID >= 0)
	  AddNumberOperandIfNotDefault(dictPrimitiveWriter,mType1Input.mFontDictionary.UniqueID,13,0);
	AddNumberOperandIfNotDefault(dictPrimitiveWriter,mType1Input.mFontDictionary.StrokeWidth,0xC08,0.0);
	
	// FontMatrix
	if(	mType1Input.mFontDictionary.FontMatrix[0] != 0.001 ||
		mType1Input.mFontDictionary.FontMatrix[1] != 0 ||
		mType1Input.mFontDictionary.FontMatrix[2] != 0 ||
		mType1Input.mFontDictionary.FontMatrix[3] != 0.001 ||
		mType1Input.mFontDictionary.FontMatrix[4] != 0 ||
		mType1Input.mFontDictionary.FontMatrix[5] != 0)
	{
		dictPrimitiveWriter.WriteRealOperand(mType1Input.mFontDictionary.FontMatrix[0]);
		dictPrimitiveWriter.WriteRealOperand(mType1Input.mFontDictionary.FontMatrix[1]);
		dictPrimitiveWriter.WriteRealOperand(mType1Input.mFontDictionary.FontMatrix[2]);
		dictPrimitiveWriter.WriteRealOperand(mType1Input.mFontDictionary.FontMatrix[3]);
		dictPrimitiveWriter.WriteRealOperand(mType1Input.mFontDictionary.FontMatrix[4]);
		dictPrimitiveWriter.WriteRealOperand(mType1Input.mFontDictionary.FontMatrix[5]);
		dictPrimitiveWriter.WriteDictOperator(0xC07);
	}

	// FontBBox
	if(	mType1Input.mFontDictionary.FontBBox[0] != 0 ||
		mType1Input.mFontDictionary.FontBBox[1] != 0 ||
		mType1Input.mFontDictionary.FontBBox[2] != 0 ||
		mType1Input.mFontDictionary.FontBBox[3] != 0)
	{
		dictPrimitiveWriter.WriteRealOperand(mType1Input.mFontDictionary.FontBBox[0]);
		dictPrimitiveWriter.WriteRealOperand(mType1Input.mFontDictionary.FontBBox[1]);
		dictPrimitiveWriter.WriteRealOperand(mType1Input.mFontDictionary.FontBBox[2]);
		dictPrimitiveWriter.WriteRealOperand(mType1Input.mFontDictionary.FontBBox[3]);
		dictPrimitiveWriter.WriteDictOperator(5);
	}

	// FSType if required. format as an embedded postscript string. /FSType fstype def
	if(mType1Input.mFontDictionary.FSTypeValid || mType1Input.mFontInfoDictionary.FSTypeValid)
	{
		std::stringstream formatter;
		formatter<<"/FSType "<<
						(mType1Input.mFontInfoDictionary.FSTypeValid ? 
							mType1Input.mFontInfoDictionary.fsType :
							mType1Input.mFontDictionary.fsType)<<
					" def";
		dictPrimitiveWriter.WriteIntegerOperand(
			AddStringToStringsArray(formatter.str()));
		dictPrimitiveWriter.WriteDictOperator(0xC15);
	}

	// now leave placeholders, record their positions

	// charset
	mCharsetPlaceHolderPosition = topDictStream.GetCurrentPosition();
	dictPrimitiveWriter.Pad5Bytes();
	dictPrimitiveWriter.WriteDictOperator(scCharset);
	
	// charstrings
	mCharstringsPlaceHolderPosition = topDictStream.GetCurrentPosition();
	dictPrimitiveWriter.Pad5Bytes();
	dictPrimitiveWriter.WriteDictOperator(scCharstrings);

	// private
	mPrivatePlaceHolderPosition = topDictStream.GetCurrentPosition();
	dictPrimitiveWriter.Pad5Bytes(); // for private it's two places - size and position
	dictPrimitiveWriter.Pad5Bytes();
	dictPrimitiveWriter.WriteDictOperator(scPrivate);

	// encoding
	mEncodingPlaceHolderPosition = topDictStream.GetCurrentPosition();
	dictPrimitiveWriter.Pad5Bytes();
	dictPrimitiveWriter.WriteDictOperator(scEncoding);


	return dictPrimitiveWriter.GetInternalState();
}

void Type1ToCFFEmbeddedFontWriter::AddStringOperandIfNotEmpty(CFFPrimitiveWriter& inWriter,const std::string& inString,unsigned short inOperator)
{
	if(inString.size() != 0)
	{
		inWriter.WriteIntegerOperand(
			AddStringToStringsArray(inString));
		inWriter.WriteDictOperator(inOperator);
	}
}

void Type1ToCFFEmbeddedFontWriter::AddNumberOperandIfNotDefault(CFFPrimitiveWriter& inWriter,int inOperand,unsigned short inOperator,int inDefault)
{
	if(inOperand != inDefault)
	{
		inWriter.WriteIntegerOperand(inOperand);
		inWriter.WriteDictOperator(inOperator);
	}
}

void Type1ToCFFEmbeddedFontWriter::AddNumberOperandIfNotDefault(CFFPrimitiveWriter& inWriter,double inOperand,unsigned short inOperator,double inDefault)
{
	if(inOperand != inDefault)
	{
		inWriter.WriteRealOperand(inOperand);
		inWriter.WriteDictOperator(inOperator);
	}
}



unsigned short Type1ToCFFEmbeddedFontWriter::AddStringToStringsArray(const std::string& inString)
{
	// first - see if this string exists in the standard strings array
	BoolAndUShort findResult = FindStandardString(inString);	

	if(findResult.first)
	{
		// it is - return the index in the standard strings array
		return findResult.second;
	}

	// it's not - add to strings vector and return the index
	StringToUShortMap::iterator it = mNonStandardStringToIndex.find(inString);
	if(it == mNonStandardStringToIndex.end())
	{
		it = mNonStandardStringToIndex.insert(StringToUShortMap::value_type(inString,(unsigned short)mStrings.size())).first;
		mStrings.push_back(inString);
	}
	return it->second + N_STD_STRINGS;
}

BoolAndUShort Type1ToCFFEmbeddedFontWriter::FindStandardString(const std::string& inStringToFind)
{
	unsigned short upperBound = N_STD_STRINGS-1;
	unsigned short lowerBound = 0;
	unsigned short pivot = upperBound/2;

	while(upperBound > lowerBound+1)
	{
		if(strcmp(inStringToFind.c_str(),scSortedStandardStrings[pivot]) < 0)
			upperBound = (pivot-1);
		else
			lowerBound = pivot;
		pivot = (upperBound + lowerBound)/2;
	}

	if(strcmp(inStringToFind.c_str(),scSortedStandardStrings[lowerBound]) == 0)
		return BoolAndUShort(true,scSortedStandardStringsPositions[lowerBound]);
	else if(strcmp(inStringToFind.c_str(),scSortedStandardStrings[upperBound]) == 0)
		return BoolAndUShort(true,scSortedStandardStringsPositions[upperBound]);
	else
		return BoolAndUShort(false,scSortedStandardStringsPositions[0]);
		
}

EStatusCode Type1ToCFFEmbeddedFontWriter::WriteStringIndex()
{
	
	mPrimitivesWriter.WriteCard16((unsigned short)mStrings.size());
	if(mStrings.size() > 0)
	{
		// calculate the total data size to determine the required offset size
		unsigned long totalSize=0;
		StringVector::iterator it = mStrings.begin();
		for(; it != mStrings.end(); ++it)
			totalSize += (unsigned long)it->size();
		
		Byte sizeOfOffset = GetMostCompressedOffsetSize(totalSize + 1);
		mPrimitivesWriter.WriteOffSize(sizeOfOffset);
		mPrimitivesWriter.SetOffSize(sizeOfOffset);
	
		unsigned long currentOffset = 1;

		// write the offsets
		for(it = mStrings.begin(); it != mStrings.end(); ++it)
		{
			mPrimitivesWriter.WriteOffset(currentOffset);
			currentOffset += (unsigned long)it->size();
		}
		mPrimitivesWriter.WriteOffset(currentOffset);

		// write the data
		for(it = mStrings.begin(); it != mStrings.end(); ++it)
			mFontFileStream.Write((const Byte*)(it->c_str()),it->size());
	}

	return mPrimitivesWriter.GetInternalState();
}

EStatusCode Type1ToCFFEmbeddedFontWriter::WriteGlobalSubrsIndex()
{
	// global subrs index is empty!. no subrs in my CFF outputs. all charstrings are flattened

	return mPrimitivesWriter.WriteCard16(0);	
}

EStatusCode Type1ToCFFEmbeddedFontWriter::WriteEncodings(const StringVector& inSubsetGlyphIDs)
{
	// k. get the encoding for the input glyphs

	mEncodingPosition = mFontFileStream.GetCurrentPosition();

	mPrimitivesWriter.WriteCard8(0);

	// assuming that 0 is in the subset glyphs IDs, which does not require encoding
	// get the encodings count
	Byte encodingGlyphsCount = (Byte)(std::min<size_t>(inSubsetGlyphIDs.size()-1,255)); 

	mPrimitivesWriter.WriteCard8(encodingGlyphsCount);
	for(Byte i=0; i < encodingGlyphsCount;++i)
		mPrimitivesWriter.WriteCard8(mType1Input.GetEncoding(inSubsetGlyphIDs[i+1]));

	return mPrimitivesWriter.GetInternalState();
}

void Type1ToCFFEmbeddedFontWriter::PrepareCharSetArray(const StringVector& inSubsetGlyphIDs)
{
	mCharset = new unsigned short[inSubsetGlyphIDs.size()-1]; // no need to have the 0 glyphs

	for(size_t i=1; i < inSubsetGlyphIDs.size();++i)
	{
		mCharset[i-1] = AddStringToStringsArray(inSubsetGlyphIDs[i]);
	}
}

EStatusCode Type1ToCFFEmbeddedFontWriter::WriteCharsets(const StringVector& inSubsetGlyphIDs)
{
	mCharsetPosition = mFontFileStream.GetCurrentPosition();

	mPrimitivesWriter.WriteCard8(0);

	for(size_t i=0;i < inSubsetGlyphIDs.size()-1;++i)
		mPrimitivesWriter.WriteSID(mCharset[i]);
	return mPrimitivesWriter.GetInternalState();	
}

EStatusCode Type1ToCFFEmbeddedFontWriter::WriteCharStrings(const StringVector& inSubsetGlyphIDs)
{
	/*
		1. build the charstrings data, looping the glyphs charstrings and writing a flattened
		   version of each charstring
		2. write the charstring index based on offsets inside the data (size should be according to the max)
		3. copy the data into the stream
	*/


	unsigned long* offsets = new unsigned long[inSubsetGlyphIDs.size() + 1];
	MyStringBuf charStringsData;
	OutputStringBufferStream charStringsDataWriteStream(&charStringsData);
	Type1ToType2Converter charStringConverter;
	StringVector::const_iterator itGlyphs = inSubsetGlyphIDs.begin();
	EStatusCode status = PDFHummus::eSuccess;

	do
	{
		unsigned short i=0;
		for(; itGlyphs != inSubsetGlyphIDs.end() && PDFHummus::eSuccess == status; ++itGlyphs,++i)
		{
			offsets[i] = (unsigned long)charStringsDataWriteStream.GetCurrentPosition();
			status = charStringConverter.WriteConvertedFontProgram(*itGlyphs,
																   &(mType1Input),
																   &charStringsDataWriteStream);
		}
		if(status != PDFHummus::eSuccess)
			break;

		offsets[i] = (unsigned long)charStringsDataWriteStream.GetCurrentPosition();

		charStringsData.pubseekoff(0,std::ios_base::beg);

		// write index section
		mCharStringPosition = mFontFileStream.GetCurrentPosition();
		Byte sizeOfOffset = GetMostCompressedOffsetSize(offsets[i] + 1);
		mPrimitivesWriter.WriteCard16((unsigned short)inSubsetGlyphIDs.size());
		mPrimitivesWriter.WriteOffSize(sizeOfOffset);
		mPrimitivesWriter.SetOffSize(sizeOfOffset);
		for(i=0;i<=inSubsetGlyphIDs.size();++i)
			mPrimitivesWriter.WriteOffset(offsets[i] + 1);

		// Write data
		InputStringBufferStream charStringsDataReadStream(&charStringsData);
		OutputStreamTraits streamCopier(&mFontFileStream);
		status = streamCopier.CopyToOutputStream(&charStringsDataReadStream);
	}while(false);

	delete[] offsets;
	return status;	
}

EStatusCode Type1ToCFFEmbeddedFontWriter::WritePrivateDictionary()
{
	mPrivatePosition = mFontFileStream.GetCurrentPosition();

	AddDeltaVectorIfNotEmpty(mPrimitivesWriter,mType1Input.mPrivateDictionary.BlueValues,6);
	AddDeltaVectorIfNotEmpty(mPrimitivesWriter,mType1Input.mPrivateDictionary.OtherBlues,7);
	AddDeltaVectorIfNotEmpty(mPrimitivesWriter,mType1Input.mPrivateDictionary.FamilyBlues,8);
	AddDeltaVectorIfNotEmpty(mPrimitivesWriter,mType1Input.mPrivateDictionary.FamilyOtherBlues,9);
	AddNumberOperandIfNotDefault(mPrimitivesWriter,mType1Input.mPrivateDictionary.BlueScale,0xC09,0.039625);
	AddNumberOperandIfNotDefault(mPrimitivesWriter,mType1Input.mPrivateDictionary.BlueShift,0xC0A,7);
	AddNumberOperandIfNotDefault(mPrimitivesWriter,mType1Input.mPrivateDictionary.BlueFuzz,0xC0B,1);
	
	// StdHW
	mPrimitivesWriter.WriteRealOperand(mType1Input.mPrivateDictionary.StdHW);
	mPrimitivesWriter.WriteDictOperator(0xA);

	// StdVW
	mPrimitivesWriter.WriteRealOperand(mType1Input.mPrivateDictionary.StdVW);
	mPrimitivesWriter.WriteDictOperator(0xB);

	AddDeltaVectorIfNotEmpty(mPrimitivesWriter,mType1Input.mPrivateDictionary.StemSnapH,0xC0C);
	AddDeltaVectorIfNotEmpty(mPrimitivesWriter,mType1Input.mPrivateDictionary.StemSnapV,0xC0D);
	AddNumberOperandIfNotDefault(mPrimitivesWriter,(int)(mType1Input.mPrivateDictionary.ForceBold ? 1:0),0xC0E,0);
	AddNumberOperandIfNotDefault(mPrimitivesWriter,mType1Input.mPrivateDictionary.LanguageGroup,0xC11,0);

	mPrivateSize = mFontFileStream.GetCurrentPosition() - mPrivatePosition;
	return mPrimitivesWriter.GetInternalState();
}

void Type1ToCFFEmbeddedFontWriter::AddDeltaVectorIfNotEmpty(CFFPrimitiveWriter& inWriter,const std::vector<int>& inArray,unsigned short inOperator)
{
	if(inArray.size() == 0)
		return;

	int currentValue = 0;

	std::vector<int>::const_iterator it = inArray.begin();
	for(; it != inArray.end();++it)
	{
		inWriter.WriteIntegerOperand(*it - currentValue);
		currentValue = *it;
	}
	inWriter.WriteDictOperator(inOperator);
}

void Type1ToCFFEmbeddedFontWriter::AddDeltaVectorIfNotEmpty(CFFPrimitiveWriter& inWriter,const std::vector<double>& inArray,unsigned short inOperator)
{
	if(inArray.size() == 0)
		return;

	double currentValue = 0;

	std::vector<double>::const_iterator it = inArray.begin();
	for(; it != inArray.end();++it)
	{
		inWriter.WriteRealOperand(*it - currentValue);
		currentValue = *it;
	}
	inWriter.WriteDictOperator(inOperator);
}

EStatusCode Type1ToCFFEmbeddedFontWriter::UpdateIndexesAtTopDict()
{
	mFontFileStream.SetPosition(mCharsetPlaceHolderPosition);
	mPrimitivesWriter.Write5ByteDictInteger((long)mCharsetPosition);

	mFontFileStream.SetPosition(mCharstringsPlaceHolderPosition);
	mPrimitivesWriter.Write5ByteDictInteger((long)mCharStringPosition);

	mFontFileStream.SetPosition(mPrivatePlaceHolderPosition);
	mPrimitivesWriter.Write5ByteDictInteger((long)mPrivateSize);
	mPrimitivesWriter.Write5ByteDictInteger((long)mPrivatePosition);
	
	mFontFileStream.SetPosition(mEncodingPlaceHolderPosition);
	mPrimitivesWriter.Write5ByteDictInteger((long)mEncodingPosition);

	return mPrimitivesWriter.GetInternalState();
}

void Type1ToCFFEmbeddedFontWriter::TranslateFromFreeTypeToType1(FreeTypeFaceWrapper& inFontInfo,
																const UIntVector& inSubsetGlyphIDs,
																StringVector& outGlyphNames)
{
	UIntVector::const_iterator it = inSubsetGlyphIDs.begin();
	
    for(; it != inSubsetGlyphIDs.end(); ++it)
        outGlyphNames.push_back(inFontInfo.GetGlyphName(*it));
    
}


