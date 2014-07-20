/*
   Source File : PDFDate.h


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

#include <string>



class PDFDate
{
public:
	// enumerator describing the relation to UTC
	enum EUTCRelation
	{
		eEarlier,
		eLater,
		eSame,
		eUndefined
	};


	PDFDate(void); // sets all values to NULL
	~PDFDate(void);

	void SetTime(int inYear,
				 int inMonth = -1,
				 int inDay = -1,
				 int inHour = -1,
				 int inMinute = -1,
				 int inSecond = -1,
				 EUTCRelation inUTC = eUndefined,
				 int inHourFromUTC = -1,
				 int inMinuteFromUTC = -1);

	bool IsNull(); // null is signified by having -1 as the year.
	std::string ToString(); // writes only non-null values
    void ParseString(std::string inValue); // sets date from input string
    
	// set PDF Date to the current time
	void SetToCurrentTime();

	int Year;
	int Month;
	int Day;
	int Hour;
	int Minute;
	int Second;
	EUTCRelation UTC;
	int HourFromUTC;
	int MinuteFromUTC;

};
