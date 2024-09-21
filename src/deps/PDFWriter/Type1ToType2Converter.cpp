/*
   Source File : Type1ToType2Converter.cpp


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
#include "Type1ToType2Converter.h"
#include "IByteWriter.h"
#include "Type1Input.h"
#include "Trace.h"
#include "CharStringType1Interpreter.h"
#include "Type2CharStringWriter.h"

#include <algorithm>

using namespace PDFHummus;

Type1ToType2Converter::Type1ToType2Converter(void)
{
}

Type1ToType2Converter::~Type1ToType2Converter(void)
{
}

EStatusCode Type1ToType2Converter::WriteConvertedFontProgram(const std::string& inGlyphName,
															 Type1Input* inType1Input,
															 IByteWriter* inByteWriter)
{
	EStatusCode status;

	do
	{
		CharStringType1Interpreter interpreter;

		mHelper = inType1Input;
		mHintReplacementEncountered = false;
		mHintAdditionEncountered = false;
		mFirstPathConstructionEncountered = false;
		mInFlexCollectionMode = false;
        mIsFirst2Coordinates = false;
		mCurrentHints.clear();
		mFlexParameters.clear();

		Type1CharString* charString = inType1Input->GetGlyphCharString(inGlyphName);
		if(!charString)
		{
			TRACE_LOG1("Type1ToType2Converter::WriteConvertedFontProgram, Exception, cannot find glyph name %s",inGlyphName.substr(0, MAX_TRACE_SIZE - 200).c_str());
			status = eFailure;
			break;
		}

		status = interpreter.Intepret(*charString,this);
		if(status != eSuccess)
		{
			TRACE_LOG("Type1ToType2Converter::WriteConvertedFontProgram, Exception, failed to interpret glyph");
			break;
		}

		// convert stem commands - put them at the beginning, and use hint masks if required
		ConvertStems();

		// convert operators to the type 2 short writing, dropping 0's, and set the first moveto
		// to offset by sidebearing
		ConvertPathConsturction();

		// need to place width of character at the first stack clearing operator
		AddInitialWidthParameter();

		status = WriteProgramToStream(inByteWriter);
		if(status != eSuccess)
		{
			TRACE_LOG("Type1ToType2Converter::WriteConvertedFontProgram, Exception, can't write program to target stream");
			break;
		}
	}while(false);

	mVStems.clear();
	mHStems.clear();
	mConversionProgram.clear();
	return status;
}

Type1CharString* Type1ToType2Converter::GetSubr(long inSubrIndex)
{
	return mHelper->GetSubr(inSubrIndex);	
}

bool Type1ToType2Converter::IsOtherSubrSupported(long inOtherSubrsIndex)
{
	// use callothersubr op code as a marker for the start of hint replacement segment
	if(3 == inOtherSubrsIndex)
	{
		mHintReplacementEncountered = true;
		RecordOperatorMarker(0x0c10);
	} 
	else if(1 == inOtherSubrsIndex)
	{
		mInFlexCollectionMode = true;
        mIsFirst2Coordinates = true;
	}
	else if(0 == inOtherSubrsIndex)
	{
		// for 0 othersubr, i need the 3rd parameter as the FD for implementing flex in type 2. so "support" it
		return true;
	}
	return false;
}

unsigned long Type1ToType2Converter::GetLenIV()
{
	return mHelper->GetLenIV();
}

EStatusCode Type1ToType2Converter::CallOtherSubr(const LongList& inOperandList,LongList& outPostScriptOperandStack)
{
	if(inOperandList.size() < 3) {
		TRACE_LOG1("Type1ToType2Converter::CallOtherSubr, expecting at least 3 arguments for 0 callothersubr (0, arguments count, flex depth...and some other args), but operand stack has only %d arguments. aborting", inOperandList.size());
		return eFailure;
	}

	// should get here onther for 0 othersubr, to mark flex segment end. implement it...and also insert a node
	// for flex
	LongList::const_reverse_iterator it = inOperandList.rbegin();
	++it;
	long argumentsCount = *it;
	if(inOperandList.size() - 2 < argumentsCount) {
		TRACE_LOG2("Type1ToType2Converter::CallOtherSubr, arguments count for other subr is %ld, but there's only %d arguments available on the stack. aborting", argumentsCount,inOperandList.size()-2);
		return eFailure;

	}
	++it;

	// the next argument should be the FD...place it in the parameters collection
	long flexDepth = *it;

	mFlexParameters.push_back(flexDepth);

	// take care of the postscript operand stack
	for(long i=0;i<argumentsCount && it != inOperandList.rend();++i)
	{
		outPostScriptOperandStack.push_back(*it);
		++it;
	}

	// now finalize flex, by placing a type 2 flex command
	EStatusCode status = RecordOperatorWithParameters(0x0c23,mFlexParameters);

	// cleanup flex mode
	mFlexParameters.clear();
	mInFlexCollectionMode = false;
    mIsFirst2Coordinates = false;
	return status;
}


EStatusCode Type1ToType2Converter::Type1Pop(const LongList& inOperandList,const LongList& inPostScriptOperandStack){(void) inOperandList; (void) inPostScriptOperandStack; return eSuccess;}
EStatusCode Type1ToType2Converter::Type1SetCurrentPoint(const LongList& inOperandList){(void) inOperandList; return eSuccess;}
EStatusCode Type1ToType2Converter::Type1InterpretNumber(long inOperand){(void) inOperand;return eSuccess;}
EStatusCode Type1ToType2Converter::Type1Div(const LongList& inOperandList){(void) inOperandList; return eSuccess;}
EStatusCode Type1ToType2Converter::Type1DotSection(const LongList& inOperandList){(void) inOperandList; return eSuccess;}
EStatusCode Type1ToType2Converter::Type1Return(const LongList& inOperandList){(void) inOperandList; return eSuccess;}

void Type1ToType2Converter::RecordOperatorMarker(unsigned short inMarkerType)
{
	ConversionNode node;
	mConversionProgram.push_back(node);
	mConversionProgram.back().mMarkerType = inMarkerType;
}

EStatusCode Type1ToType2Converter::Type1Hsbw(const LongList& inOperandList)
{
	if(inOperandList.size() < 2) {
		TRACE_LOG1("Type1ToType2Converter::Type1Hsbw, hsbw expecting 2 arguments, got %d. aborting.", inOperandList.size());
		return eFailure;
	}

	LongList::const_reverse_iterator it = inOperandList.rbegin();

	mWidth[1] = 0;
	mWidth[0] = *it;
	++it;
	mSideBearing[1] = 0;
	mSideBearing[0] = *it;
	return eSuccess;
}

EStatusCode Type1ToType2Converter::Type1Sbw(const LongList& inOperandList)
{
	if(inOperandList.size() < 4) {
		TRACE_LOG1("Type1ToType2Converter::Type1Sbw, sbw expecting 4 arguments, got %d. aborting.", inOperandList.size());
		return eFailure;
	}

	LongList::const_reverse_iterator it = inOperandList.rbegin();

	mWidth[1] = *it;
	++it;
	mWidth[0] = *it;
	++it;
	mSideBearing[1] = *it;
	++it;
	mSideBearing[0] = *it;
	return eSuccess;
}

EStatusCode Type1ToType2Converter::Type1Hstem(const LongList& inOperandList)
{
	if(inOperandList.size() < 2) {
		TRACE_LOG1("Type1ToType2Converter::Type1Sbw, sbw expecting 2 arguments, got %d. aborting.", inOperandList.size());
		return eFailure;
	}

	LongList::const_reverse_iterator it = inOperandList.rbegin();

	long extent = *it;
	++it;
	long origin = *it;
	AddHStem(origin,extent);
	if(mFirstPathConstructionEncountered)
		mHintAdditionEncountered = true;
	return RecordOperatorWithParameters(1,inOperandList);
}

EStatusCode Type1ToType2Converter::AddHStem(long inOrigin,long inExtent)
{
	Stem aStem(inOrigin,inExtent);
	StemToSizeTMap::iterator it = mHStems.find(aStem);

	if(it == mHStems.end())
		mHStems.insert(StemToSizeTMap::value_type(aStem,0));
	return eSuccess;
}

EStatusCode Type1ToType2Converter::AddVStem(long inOrigin,long inExtent)
{
	Stem aStem(inOrigin,inExtent);
	StemToSizeTMap::iterator it = mVStems.find(aStem);

	if(it == mVStems.end())
		mVStems.insert(StemToSizeTMap::value_type(aStem,0));
	return eSuccess;
}

EStatusCode Type1ToType2Converter::RecordOperatorWithParameters(unsigned short inMarkerType,const LongList& inOperandList)
{
	ConversionNode node;
	mConversionProgram.push_back(node);
	mConversionProgram.back().mMarkerType = inMarkerType;
	mConversionProgram.back().mOperands = inOperandList;
	return eSuccess;
}


EStatusCode Type1ToType2Converter::Type1Vstem(const LongList& inOperandList)
{
	if(inOperandList.size() < 2) {
		TRACE_LOG1("Type1ToType2Converter::Type1Vstem, vestem expecting 2 arguments, got %d. aborting.", inOperandList.size());
		return eFailure;
	}
		
	LongList::const_reverse_iterator it = inOperandList.rbegin();

	long extent = *it;
	++it;
	long origin = *it;
	AddVStem(origin,extent);
	if(mFirstPathConstructionEncountered)
		mHintAdditionEncountered = true;
	return RecordOperatorWithParameters(3,inOperandList);
}

EStatusCode Type1ToType2Converter::Type1VStem3(const LongList& inOperandList)
{
	if(inOperandList.size() < 6) {
		TRACE_LOG1("Type1ToType2Converter::Type1VStem3, vstem3 expecting 6 arguments, got %d. aborting.", inOperandList.size());
		return eFailure;
	}

	LongList::const_reverse_iterator it = inOperandList.rbegin();

	long extent = *it;
	++it;
	long origin = *it;
	AddVStem(origin,extent);
	++it;
	extent = *it;
	++it;
	origin = *it;
	AddVStem(origin,extent);
	++it;
	extent = *it;
	++it;
	origin = *it;
	AddVStem(origin,extent);
	if(mFirstPathConstructionEncountered)
		mHintAdditionEncountered = true;
	return RecordOperatorWithParameters(0x0c01,inOperandList);
}

EStatusCode Type1ToType2Converter::Type1HStem3(const LongList& inOperandList)
{
	if(inOperandList.size() < 6) {
		TRACE_LOG1("Type1ToType2Converter::Type1HStem3, hstem3 expecting 6 arguments, got %d. aborting.", inOperandList.size());
		return eFailure;
	}
	
	LongList::const_reverse_iterator it = inOperandList.rbegin();

	long extent = *it;
	++it;
	long origin = *it;
	AddHStem(origin,extent);
	++it;
	extent = *it;
	++it;
	origin = *it;
	AddHStem(origin,extent);
	++it;
	extent = *it;
	++it;
	origin = *it;
	AddHStem(origin,extent);
	if(mFirstPathConstructionEncountered)
		mHintAdditionEncountered = true;
	return RecordOperatorWithParameters(0x0c02,inOperandList);
}

EStatusCode Type1ToType2Converter::Type1VMoveto(const LongList& inOperandList)
{
	if(inOperandList.size() < 1) {
		TRACE_LOG1("Type1ToType2Converter::Type1VMoveto, vmoveto expecting 1 argument, got %d. aborting.", inOperandList.size());
		return eFailure;
	}

	mFirstPathConstructionEncountered = true;
	return RecordOperatorWithParameters(4,inOperandList);
}

EStatusCode Type1ToType2Converter::Type1RMoveto(const LongList& inOperandList)
{
	if(inOperandList.size() < 2) {
		TRACE_LOG1("Type1ToType2Converter::Type1RMoveto, rmoveto expecting 2 arguments, got %d. aborting.", inOperandList.size());
		return eFailure;
	}


	mFirstPathConstructionEncountered = true;
	if(mInFlexCollectionMode)
	{
		// while in flex, collect the parameters to the flex parameter
		LongList::const_iterator it = inOperandList.begin();

		// note a paculiarness for the 2nd pair of coordinates.
		// in type1 flex, the first 2 coordinates summed are the first edge coordinate
		if(mFlexParameters.size() == 2 && mIsFirst2Coordinates)
		{
			LongList::iterator itFlex = mFlexParameters.begin();
			*itFlex += *it;
			++it;
			++itFlex;
			*itFlex += *it;
            mIsFirst2Coordinates = false;
		}
		else
		{
			mFlexParameters.push_back(*it);
			++it;
			mFlexParameters.push_back(*it);
		}
		return eSuccess;
	}
	else
	{
		return RecordOperatorWithParameters(21,inOperandList);
	}
}

EStatusCode Type1ToType2Converter::Type1HMoveto(const LongList& inOperandList)
{
	if(inOperandList.size() < 1) {
		TRACE_LOG1("Type1ToType2Converter::Type1HMoveto, rmoveto expecting 1 argument, got %d. aborting.", inOperandList.size());
		return eFailure;
	}


	mFirstPathConstructionEncountered = true;
	return RecordOperatorWithParameters(22,inOperandList);
}

EStatusCode Type1ToType2Converter::Type1RLineto(const LongList& inOperandList)
{
	if(inOperandList.size() < 2) {
		TRACE_LOG1("Type1ToType2Converter::Type1RLineto, rlineto expecting 2 arguments, got %d. aborting.", inOperandList.size());
		return eFailure;
	}

	mFirstPathConstructionEncountered = true;
	return RecordOperatorWithParameters(5,inOperandList);
}

EStatusCode Type1ToType2Converter::Type1HLineto(const LongList& inOperandList)
{
	if(inOperandList.size() < 1) {
		TRACE_LOG1("Type1ToType2Converter::Type1HLineto, hlineto expecting 1 argument, got %d. aborting.", inOperandList.size());
		return eFailure;
	}

	mFirstPathConstructionEncountered = true;
	return RecordOperatorWithParameters(6,inOperandList);
}

EStatusCode Type1ToType2Converter::Type1VLineto(const LongList& inOperandList)
{
	if(inOperandList.size() < 1) {
		TRACE_LOG1("Type1ToType2Converter::Type1VLineto, vlineto expecting 1 argument, got %d. aborting.", inOperandList.size());
		return eFailure;
	}

	mFirstPathConstructionEncountered = true;
	return RecordOperatorWithParameters(7,inOperandList);
}

EStatusCode Type1ToType2Converter::Type1RRCurveto(const LongList& inOperandList)
{
	if(inOperandList.size() < 6) {
		TRACE_LOG1("Type1ToType2Converter::Type1RRCurveto, rrcurveto expecting 6 arguments, got %d. aborting.", inOperandList.size());
		return eFailure;
	}


	mFirstPathConstructionEncountered = true;
	return RecordOperatorWithParameters(8,inOperandList);
}

EStatusCode Type1ToType2Converter::Type1VHCurveto(const LongList& inOperandList)
{
	if(inOperandList.size() < 4) {
		TRACE_LOG1("Type1ToType2Converter::Type1VHCurveto, vhcurveto expecting 4 arguments, got %d. aborting.", inOperandList.size());
		return eFailure;
	}

	mFirstPathConstructionEncountered = true;
	return RecordOperatorWithParameters(30,inOperandList);
}

EStatusCode Type1ToType2Converter::Type1HVCurveto(const LongList& inOperandList)
{
	if(inOperandList.size() < 4) {
		TRACE_LOG1("Type1ToType2Converter::Type1HVCurveto, hvcurveto expecting 4 arguments, got %d. aborting.", inOperandList.size());
		return eFailure;
	}

	mFirstPathConstructionEncountered = true;
	return RecordOperatorWithParameters(31,inOperandList);
}

EStatusCode Type1ToType2Converter::Type1ClosePath(const LongList& inOperandList)
{
	// IMPORTANT - apparently closepath was removed for type 2. didn't notice it till now
        (void) inOperandList;
	return eSuccess;	
}

EStatusCode Type1ToType2Converter::Type1Endchar(const LongList& inOperandList)
{
	(void) inOperandList;
	RecordOperatorMarker(14);
	return eSuccess;	
}

EStatusCode Type1ToType2Converter::Type1Seac(const LongList& inOperandList)
{
	if(inOperandList.size() < 1) {
		TRACE_LOG1("Type1ToType2Converter::Type1Seac, seac expecting at least one argument. got %d. aborting", inOperandList.size());
		return eFailure;
	}
	// le'ts convert it already to the final EndChar...and stop any later recording
	
	// note that type2 endchar implementation avoids sidebearing
	LongList params;
	LongList::const_iterator it = inOperandList.begin();
	++it;
	for(; it != inOperandList.end();++it)
		params.push_back(*it);
	return RecordOperatorWithParameters(14,params);
}

static bool sStemSort(const Stem* inLeft, const Stem* inRight)
{
	return inLeft->mOrigin < inRight->mOrigin;
}


void Type1ToType2Converter::ConvertStems()
{


	if(mHStems.size() == 0 && mVStems.size() == 0)
		return;

	StemVector orderedHStems;
	StemVector orderedVStems;

	StemToSizeTMap::iterator it = mHStems.begin();
	for(; it != mHStems.end(); ++it)
		orderedHStems.push_back(&(it->first));
	it = mVStems.begin();
	for(; it != mVStems.end(); ++it)
		orderedVStems.push_back(&(it->first));

	// sort stems
	sort(orderedHStems.begin(),orderedHStems.end(),sStemSort);
	sort(orderedVStems.begin(),orderedVStems.end(),sStemSort);

	ConversionNodeList::iterator itProgramPosition = mConversionProgram.begin();


	// determine if hint mask is required.
	// two conditions:
	// 1. stem replacement encountered, then we need a new set of stems, which requires a mask
	// 2. no stem replacement, but hints were defined after first path construction. this means
	//    that initially there's a zero hint state, and only after some drawing there'll be a hint applied
	//    i think this is theoretical...but i guess it is possible.
	if(mHintReplacementEncountered || mHintAdditionEncountered)
	{
		// setup stem to index maps
		size_t i=0;
		for(; i < orderedHStems.size();++i)
			mHStems[*orderedHStems[i]] = i;
		for(i=0; i < orderedVStems.size();++i)
			mVStems[*orderedVStems[i]] = i+orderedHStems.size();

		// write initial hstemhm command
		if(orderedHStems.size() > 0)
		{
			itProgramPosition = InsertOperatorMarker(18,itProgramPosition);
			SetupStemHintsInNode(orderedHStems,mSideBearing[1],*itProgramPosition);
			++itProgramPosition;
		}

		// write vstemhm command
		if(orderedVStems.size() > 0)
		{
			itProgramPosition = InsertOperatorMarker(23,itProgramPosition);
			SetupStemHintsInNode(orderedVStems,mSideBearing[0],*itProgramPosition);
			++itProgramPosition;
		}

		// if in the target stem definiton is not the first thing, create an initial 0ing mask
		if(!IsStemHint(itProgramPosition->mMarkerType))
		{
			itProgramPosition = InsertOperatorMarker(19,itProgramPosition);
			itProgramPosition->mOperands.push_back(0);
			++itProgramPosition;
		}

		// now loop the nodes, converting each stem set to a call. 
		while(itProgramPosition != mConversionProgram.end())
		{
			if(IsStemHint(itProgramPosition->mMarkerType))
			{
				ConversionNodeList::iterator firstNonHint = CollectHintIndexesFromHere(itProgramPosition);

				// remove stem definitions
				itProgramPosition = mConversionProgram.erase(itProgramPosition,firstNonHint);

				// place hintmask as a replacement for the hint definitions
				itProgramPosition = InsertOperatorMarker(19,itProgramPosition);
				itProgramPosition->mOperands.push_back(GenerateHintMaskFromCollectedHints());
				++itProgramPosition;

			} 
			else if(0x0c10 == itProgramPosition->mMarkerType) 
			{
				// other, hints replacement here just clears the current hint collection
				mCurrentHints.clear();
				itProgramPosition = mConversionProgram.erase(itProgramPosition);
			}
			else
				++itProgramPosition;
		}
	}
	else
	{
		// if not hint mask, just write the hint definitions and go.
		// write initial hstem command
		if(orderedHStems.size() > 0)
		{
			itProgramPosition = InsertOperatorMarker(1,itProgramPosition);
			SetupStemHintsInNode(orderedHStems,mSideBearing[1],*itProgramPosition);
			++itProgramPosition;
		}

		// write vstem command
		if(orderedVStems.size() > 0)
		{
			itProgramPosition = InsertOperatorMarker(3,itProgramPosition);
			SetupStemHintsInNode(orderedVStems,mSideBearing[0],*itProgramPosition);
			++itProgramPosition;
		}

		// now erase all other stem hints
		while(itProgramPosition != mConversionProgram.end())
		{
			if(IsStemHint(itProgramPosition->mMarkerType))
				itProgramPosition = mConversionProgram.erase(itProgramPosition);
			else
				++itProgramPosition;
		}
	}
}

ConversionNodeList::iterator Type1ToType2Converter::InsertOperatorMarker(unsigned short inMarkerType,ConversionNodeList::iterator inInsertPosition)
{
	ConversionNode node;
	ConversionNodeList::iterator result = mConversionProgram.insert(inInsertPosition,node);
	result->mMarkerType = inMarkerType;	
	return result;
}

void Type1ToType2Converter::SetupStemHintsInNode(const StemVector& inStems,long inOffsetCoordinate,ConversionNode& refNode)
{
	StemVector::const_iterator it = inStems.begin();
	long lastCoordinate;
	
	refNode.mOperands.push_back((*it)->mOrigin + inOffsetCoordinate);
	refNode.mOperands.push_back((*it)->mExtent);
	lastCoordinate = (*it)->mOrigin + (*it)->mExtent;
	++it;

	for(; it != inStems.end();++it)
	{
		refNode.mOperands.push_back((*it)->mOrigin - lastCoordinate);
		refNode.mOperands.push_back((*it)->mExtent);
		lastCoordinate = (*it)->mOrigin + (*it)->mExtent;
	}
}

bool Type1ToType2Converter::IsStemHint(unsigned short inMarkerType)
{
	return 1 == inMarkerType  || 3 == inMarkerType || 0x0c01 == inMarkerType || 0x0c02 == inMarkerType;
}

ConversionNodeList::iterator Type1ToType2Converter::CollectHintIndexesFromHere(ConversionNodeList::iterator inFirstStemHint)
{
	ConversionNodeList::iterator it = inFirstStemHint;
	for(; it != mConversionProgram.end();++it)
	{
		// hstem
		if(1 == it->mMarkerType)
		{
			LongList::const_reverse_iterator itOperands = it->mOperands.rbegin();

			long extent = *itOperands;
			++itOperands;
			long origin = *itOperands;
			mCurrentHints.insert(mHStems[Stem(origin,extent)]);
			
		} // vstem
		else if(3 == it->mMarkerType)
		{
			LongList::const_reverse_iterator itOperands = it->mOperands.rbegin();

			long extent = *itOperands;
			++itOperands;
			long origin = *itOperands;
			mCurrentHints.insert(mVStems[Stem(origin,extent)]);
		}// vstem3
		else if(0x0c01 == it->mMarkerType)
		{
			LongList::const_reverse_iterator itOperands = it->mOperands.rbegin();

			long extent = *itOperands;
			++itOperands;
			long origin = *itOperands;
			mCurrentHints.insert(mVStems[Stem(origin,extent)]);
			++itOperands;
			extent = *itOperands;
			++itOperands;
			origin = *itOperands;
			mCurrentHints.insert(mVStems[Stem(origin,extent)]);
			++itOperands;
			extent = *itOperands;
			++itOperands;
			origin = *itOperands;
			mCurrentHints.insert(mVStems[Stem(origin,extent)]);
		}// hstem3
		else if(0x0c02 == it->mMarkerType)
		{
			LongList::const_reverse_iterator itOperands = it->mOperands.rbegin();

			long extent = *itOperands;
			++itOperands;
			long origin = *itOperands;
			mCurrentHints.insert(mHStems[Stem(origin,extent)]);
			++itOperands;
			extent = *itOperands;
			++itOperands;
			origin = *itOperands;
			mCurrentHints.insert(mHStems[Stem(origin,extent)]);
			++itOperands;
			extent = *itOperands;
			++itOperands;
			origin = *itOperands;
			mCurrentHints.insert(mHStems[Stem(origin,extent)]);
		}
		else // stop stem set
		{
			break;
		}
	}
	return it; // return first instruction out of this set
}

long Type1ToType2Converter::GenerateHintMaskFromCollectedHints()
{
	unsigned long hintMask = 0;
	size_t totalHints = mHStems.size() + mVStems.size();
	SizeTSet::iterator it = mCurrentHints.begin();
	unsigned long maskByteSize = (unsigned long)(totalHints/8 + (totalHints % 8 != 0 ? 1:0));

	for(; it != mCurrentHints.end();++it)
		hintMask = hintMask | (1 << (maskByteSize*8 - 1  - *it));

	return (long)hintMask;
}

static const unsigned long scMergeLimit = 40; // PDF implementation details show 48. i take a safety distance. just for kicks.

void Type1ToType2Converter::ConvertPathConsturction()
{
	// find the first removeto, and add sidebearing offset to it
	ConversionNodeList::iterator it = mConversionProgram.begin();

	for(; it != mConversionProgram.end(); ++it)
	{
		// either vmoveto, rmoveto or hmoveto	
		if(21 == it->mMarkerType || 22 == it->mMarkerType ||  4 == it->mMarkerType)
		{
			long x;
			long y;
			LongList::iterator itOperands = it->mOperands.begin();

			if(4 == it->mMarkerType) // vmoveto
			{
				x = mSideBearing[0];
				y = (*itOperands) + mSideBearing[1];

			}
			else if(22 == it->mMarkerType) // hmoveto
			{
				x = (*itOperands) + mSideBearing[0];
				y = mSideBearing[1];
			}
			else // rmoveto
			{
				x = (*itOperands) + mSideBearing[0];
				++itOperands;
				y = (*itOperands) + mSideBearing[1];
			}

			it->mOperands.clear();
			if(0 == x)
			{
				it->mMarkerType = 4; // vmoveto
				it->mOperands.push_back(y);
			}
			else if(0 == y)
			{
				it->mMarkerType = 22; // hmoveto
				it->mOperands.push_back(x);
			}
			else
			{
				it->mMarkerType = 21; // rmoveto
				it->mOperands.push_back(x);
				it->mOperands.push_back(y);
			}
			break;
		}
	}

	// now, convert some rrcurvetos into vvcurvetos and hhcurvetos, if there are any [should be at first *moveto position now]
	ConversionNodeList::iterator itFirstMoveto = it;
	for(;it != mConversionProgram.end();++it)
	{
		if(8 == it->mMarkerType)
		{
			// look for either horizontal curves, or vertical curves
			LongList::iterator itOperands = it->mOperands.begin();
			
			long operands[6];
			for(int i=0;i<6;++i,++itOperands)
				operands[i] = *itOperands;

			if(0 == operands[1] && 0 == operands[5])
			{
				it->mMarkerType = 27; // hhcurveto
				it->mOperands.clear();
				it->mOperands.push_back(operands[0]);
				it->mOperands.push_back(operands[2]);
				it->mOperands.push_back(operands[3]);
				it->mOperands.push_back(operands[4]);
			}
			else if(0 == operands[0] && 0 == operands[4])
			{
				it->mMarkerType = 26; // vvcurveto
				it->mOperands.clear();
				it->mOperands.push_back(operands[1]);
				it->mOperands.push_back(operands[2]);
				it->mOperands.push_back(operands[3]);
				it->mOperands.push_back(operands[5]);
			}
		}
	}

	it = itFirstMoveto; // reset iteration

	// optimize multiple calls into one. we got *linetos and *curvetos
	// start looking from the first moveto...as no path construction
	// can occur before it
	while(it != mConversionProgram.end())
	{
		switch(it->mMarkerType)
		{
			case(5): // rlineto
			case(8): // rrcurveto
			case(26): // vvcurveto [leaving aside optimizing prior rrcurveto here]
			case(27): // hhcurveto [leaving aside optimizing prior rrcurveto here]
				{	// see if there are more same nodes and merge them
					it = MergeSameOperators(it);
					break;
				}
			case(6): // hlineto
				{
					it = MergeAltenratingOperators(it,7);
					break;
				}
			case(7): // vlineto
				{
					it = MergeAltenratingOperators(it,6);
					break;
				}
			case(30): // vhcurveto
				{
					ConversionNodeList::iterator itStarter = it;
					
					it = MergeAltenratingOperators(it,31);
					
					// optionally also merge the next rrcurveto, if it's starting
					// point conforms to the operators list
					if(it != mConversionProgram.end() && 8 == it->mMarkerType && (itStarter->mOperands.size() + it->mOperands.size() - 1 < scMergeLimit))
					{
						if(itStarter->mOperands.size() % 8 == 0)
						{
							// this curve needs to start with vertical
							if(0 == *(it->mOperands.begin()))
							{
								LongList::iterator itOperands = it->mOperands.begin();
								++itOperands;
								itStarter->mOperands.insert(itStarter->mOperands.end(),itOperands,it->mOperands.end());
								it = mConversionProgram.erase(it);
							}
						}
						else
						{
							// this curve needs to start with horizontal
							LongList::iterator itOperands = it->mOperands.begin();
							++itOperands;
							if(0 == *itOperands)
							{
								++itOperands;
								itStarter->mOperands.insert(itStarter->mOperands.end(),*(it->mOperands.begin()));
								itStarter->mOperands.insert(itStarter->mOperands.end(),itOperands,it->mOperands.end());
								// need to switch the last two
								long dyf = itStarter->mOperands.back();
								itStarter->mOperands.pop_back();
								long dxf = itStarter->mOperands.back();
								itStarter->mOperands.pop_back();
								itStarter->mOperands.push_back(dyf);
								itStarter->mOperands.push_back(dxf);
								it = mConversionProgram.erase(it);
							}
						}
					}
					break;
				}
			case(31): // hvcurveto
				{
					ConversionNodeList::iterator itStarter = it;

					it = MergeAltenratingOperators(it,30);

					// optionally also merge the next rrcurveto, if it's starting
					// point conforms to the operators list
					if(it != mConversionProgram.end() && 8 == it->mMarkerType && (itStarter->mOperands.size() + it->mOperands.size() - 1 < scMergeLimit))
					{
						if(itStarter->mOperands.size() % 8 == 0)
						{
							// this curve needs to start with horizontal
							LongList::iterator itOperands = it->mOperands.begin();
							++itOperands;
							if(0 == *itOperands)
							{
								++itOperands;
								itStarter->mOperands.insert(itStarter->mOperands.end(),*(it->mOperands.begin()));
								itStarter->mOperands.insert(itStarter->mOperands.end(),itOperands,it->mOperands.end());
								// need to switch the last two
								long dyf = itStarter->mOperands.back();
								itStarter->mOperands.pop_back();
								long dxf = itStarter->mOperands.back();
								itStarter->mOperands.pop_back();
								itStarter->mOperands.push_back(dyf);
								itStarter->mOperands.push_back(dxf);
								it = mConversionProgram.erase(it);
							}
						}
						else
						{
							// this curve needs to start with vertical
							if(0 == *(it->mOperands.begin()))
							{
								LongList::iterator itOperands = it->mOperands.begin();
								++itOperands;
								itStarter->mOperands.insert(itStarter->mOperands.end(),itOperands,it->mOperands.end());
								it = mConversionProgram.erase(it);
							}
						}
					}
					break;
				}
			default:
				++it;
		}
	}
}

ConversionNodeList::iterator Type1ToType2Converter::MergeSameOperators(ConversionNodeList::iterator inStartingNode)
{
	return MergeSameOperators(inStartingNode,inStartingNode->mMarkerType);
}

ConversionNodeList::iterator Type1ToType2Converter::MergeSameOperators(ConversionNodeList::iterator inStartingNode,
																	   unsigned short inOpCode)
{
	ConversionNodeList::iterator itNext = inStartingNode;
	++itNext;

	while(inOpCode == itNext->mMarkerType && (inStartingNode->mOperands.size()  + itNext->mOperands.size() < scMergeLimit))
	{
		inStartingNode->mOperands.insert(inStartingNode->mOperands.end(),itNext->mOperands.begin(),itNext->mOperands.end());
		itNext = mConversionProgram.erase(itNext);
	}
	return itNext;
}

ConversionNodeList::iterator Type1ToType2Converter::MergeAltenratingOperators(ConversionNodeList::iterator inStartingNode,
																			  unsigned short inAlternatingOpcode)
{
	ConversionNodeList::iterator itNext = inStartingNode;
	++itNext;
	unsigned short currentMarker = inAlternatingOpcode;

	while(currentMarker == itNext->mMarkerType && (inStartingNode->mOperands.size()  + itNext->mOperands.size() < scMergeLimit))
	{
		inStartingNode->mOperands.insert(inStartingNode->mOperands.end(),itNext->mOperands.begin(),itNext->mOperands.end());
		itNext = mConversionProgram.erase(itNext);
		currentMarker = inStartingNode->mMarkerType == currentMarker ? inAlternatingOpcode:inStartingNode->mMarkerType;
	}
	return itNext;
}

void Type1ToType2Converter::AddInitialWidthParameter()
{
	// the first appearance of one of the following must have the width agrument added to it:
	// hstem, hstemhm, vstem, vstemhm, [cntrmask], [hintmask], hmoveto, vmoveto,
	// rmoveto, or endchar
	// [note theat cntrmask i'm not using and hintmask can't appear before stem hints...so not checking fo these]

	ConversionNodeList::iterator it = mConversionProgram.begin();

	for(; it != mConversionProgram.end();++it)
	{
		if(	1 == it->mMarkerType || // hstem 
			3 == it->mMarkerType || // vstem
			18 == it->mMarkerType || // hstemhm
			23 == it->mMarkerType || // vstemhm
			4 == it->mMarkerType || // vmoveto
			21 == it->mMarkerType || // rmoveto
			22 == it->mMarkerType || // hmoveto
			14 == it->mMarkerType) // endchar
		{
			it->mOperands.push_front(mWidth[0]);
			break;
		}
	}
}

EStatusCode Type1ToType2Converter::WriteProgramToStream(IByteWriter* inByteWriter)
{
	Type2CharStringWriter commandWriter(inByteWriter);
	EStatusCode status = eSuccess;

	ConversionNodeList::iterator it = mConversionProgram.begin();

	for(; it != mConversionProgram.end() && eSuccess == status; ++it)
	{
		LongList::iterator itOperands = it->mOperands.begin();
		
		if(19 == it->mMarkerType) // hintmask
		{
			if(itOperands == it->mOperands.end()) {
				TRACE_LOG("Type1ToType2Converter::WriteProgramToStream, no mask for hintmask. aborting");
				status = eFailure;
				break;
			}
			status = commandWriter.WriteHintMask((unsigned long)(*itOperands),(unsigned long)(mHStems.size() + mVStems.size()));
		}
		else
		{
			for(; itOperands != it->mOperands.end() && eSuccess == status;++itOperands)
				status = commandWriter.WriteIntegerOperand(*itOperands);
			
			if(eSuccess == status)
			{
				// if marker type is vstemhm, and next one is hintmask, no need to write vstemhm
				if(23 == it->mMarkerType)
				{
					ConversionNodeList::iterator itNext = it;
					++itNext;
					if(itNext != mConversionProgram.end() && itNext->mMarkerType != 19)
						status = commandWriter.WriteOperator(it->mMarkerType);
				}
				else
					status = commandWriter.WriteOperator(it->mMarkerType);

			}
		}
	}
	return status;	
}