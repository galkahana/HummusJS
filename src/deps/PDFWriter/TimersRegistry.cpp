/*
   Source File : TimersRegistry.cpp


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
#include "TimersRegistry.h"
#include "Trace.h"

TimersRegistry::TimersRegistry(void)
{
}

TimersRegistry::~TimersRegistry(void)
{
}

void TimersRegistry::StartMeasure(const std::string& inTimerName)
{
	StringToTimerMap::iterator it = mTimers.find(inTimerName);
	if(it == mTimers.end())
		it = mTimers.insert(StringToTimerMap::value_type(inTimerName,Timer())).first;

	it->second.StartMeasure();
}

void TimersRegistry::StopMeasureAndAccumulate(const std::string& inTimerName)
{
	StringToTimerMap::iterator it = mTimers.find(inTimerName);
	if(it == mTimers.end())
		it = mTimers.insert(StringToTimerMap::value_type(inTimerName,Timer())).first;

	it->second.StopMeasureAndAccumulate();
}

double TimersRegistry::GetTotalMiliSeconds(const std::string& inTimerName)
{
	StringToTimerMap::iterator it = mTimers.find(inTimerName);
	if(it == mTimers.end())
		it = mTimers.insert(StringToTimerMap::value_type(inTimerName,Timer())).first;

	return it->second.GetTotalMiliSeconds();
}

Timer& TimersRegistry::GetTimer(const std::string& inTimerName)
{
	StringToTimerMap::iterator it = mTimers.find(inTimerName);
	if(it == mTimers.end())
		it = mTimers.insert(StringToTimerMap::value_type(inTimerName,Timer())).first;

	return it->second;
}

void TimersRegistry::ReleaseAll()
{
	mTimers.clear();
}

void TimersRegistry::TraceAll()
{
	StringToTimerMap::iterator it = mTimers.begin();
	
	TRACE_LOG("Start Tracing Timers");
	for(; it != mTimers.end(); ++it)
	{
		long hours = (long)it->second.GetTotalMiliSeconds()/3600000;
		long minutes = ((long)it->second.GetTotalMiliSeconds()%3600000) / 60000;
		long seconds = ((long)it->second.GetTotalMiliSeconds()%60000) / 1000;
		long miliseconds = (long)it->second.GetTotalMiliSeconds()%1000;
		TRACE_LOG5("Tracing Timer %s. total time [h:m:s:ms] = %ld:%ld:%ld:%ld",
						it->first.c_str(),hours,minutes,seconds,miliseconds);
	}
	TRACE_LOG("End Tracing Timers");
}

void TimersRegistry::TraceAndReleaseAll()
{
	TraceAll();
	ReleaseAll();
}
