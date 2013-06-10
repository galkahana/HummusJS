/*
   Source File : TimersRegistry.h


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
#pragma once

#include "Timer.h"

#include <string>
#include <map>



typedef std::map<std::string,Timer> StringToTimerMap;

class TimersRegistry
{
public:
	TimersRegistry(void);
	~TimersRegistry(void);

	void StartMeasure(const std::string& inTimerName);
	void StopMeasureAndAccumulate(const std::string& inTimerName);

	double GetTotalMiliSeconds(const std::string& inTimerName);

	Timer& GetTimer(const std::string& inTimerName);

	void ReleaseAll();
	void TraceAll();
	void TraceAndReleaseAll();

private:
	StringToTimerMap mTimers;
};
