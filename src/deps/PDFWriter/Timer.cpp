/*
   Source File : Timer.cpp


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
#include "Timer.h"

Timer::Timer(void)
{
	mStartTime = 0;
	mTotal = 0;
}

Timer::~Timer(void)
{
}

void Timer::Reset()
{
	mStartTime = 0;
	mTotal = 0;
}

void Timer::StartMeasure()
{
	mStartTime = clock();
}

void Timer::StopMeasureAndAccumulate()
{
	mTotal += (double)(clock() - mStartTime)*1000/(CLOCKS_PER_SEC);
	mStartTime = 0;
}

double Timer::GetTotalMiliSeconds()
{
	return mTotal;
}	
