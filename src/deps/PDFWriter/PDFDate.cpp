/*
   Source File : PDFDate.cpp


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
#include "PDFDate.h"
#include "SafeBufferMacrosDefs.h"
#include "Trace.h"
#include "BoxingBase.h"
#if defined (__MWERKS__)
	// MAC OSX methods for providing UTC difference
	#include <CoreFoundation/CFDate.h>
	#include <CoreFoundation/CFTimeZone.h>
#endif

#include <ctime>
#include <math.h>
#include <stdlib.h>

PDFDate::PDFDate(void)
{
	SetTime(-1);
}

PDFDate::~PDFDate(void)
{
}

void PDFDate::SetTime(	int inYear,
						int inMonth,
						int inDay,
						int inHour,
						int inMinute,
						int inSecond,
						EUTCRelation inUTC,
						int inHourFromUTC,
						int inMinuteFromUTC)
{
	Year = inYear;
	Month = inMonth;
	Day = inDay;
	Hour = inHour;
	Minute = inMinute;
	Second = inSecond;
	UTC = inUTC;
	HourFromUTC = inHourFromUTC;
	MinuteFromUTC = inMinuteFromUTC;
}

bool PDFDate::IsNull()
{
	return -1 == Year;	
}

/*
	write a string represetnation of the date. undefined parameters will not
	have values in the string unless later values are defined. in that
	case the PDF specs defaults are used [See section 3.8.3 of PDF Reference, V1.7]

*/

static const std::string scEmpty = "";
std::string PDFDate::ToString()
{
	if(IsNull())
		return scEmpty;

	char buffer[24];
	bool wroteSomethingLater = false;

	// UTC info
	if(UTC != PDFDate::eUndefined)
	{
		if(PDFDate::eSame == UTC)
		{
			buffer[17] = '\0';
			buffer[16] = 'Z';
		}
		else
		{
			buffer[23] = '\0';
			buffer[22] = '\'';
			buffer[21] = '0' + ((-1 == MinuteFromUTC ? 0:MinuteFromUTC) % 10);
			buffer[20] = '0' + ((-1 == MinuteFromUTC ? 0:MinuteFromUTC) / 10);
			buffer[19] = '\'';
			buffer[18] = '0' + ((-1 == HourFromUTC ? 0:HourFromUTC) % 10);
			buffer[17] = '0' + ((-1 == HourFromUTC ? 0:HourFromUTC) / 10);
			buffer[16] = (PDFDate::eLater == UTC )? '+':'-';
		}
		wroteSomethingLater = true;
	}

	// Second
	if(Second != -1 || wroteSomethingLater)
	{
		if(!wroteSomethingLater)
		{
			wroteSomethingLater = true;
			buffer[16] = '\0';
		}
		buffer[15] = '0' + ((-1 == Second ? 0:Second) % 10);
		buffer[14] = '0' + ((-1 == Second ? 0:Second) / 10);
	}

	// Minute
	if(Minute != -1 || wroteSomethingLater)
	{
		if(!wroteSomethingLater)
		{
			wroteSomethingLater = true;
			buffer[14] = '\0';
		}
		buffer[13] = '0' + ((-1 == Minute ? 0:Minute) % 10);
		buffer[12] = '0' + ((-1 == Minute ? 0:Minute) / 10);
	}

	// Hour
	if(Hour != -1 || wroteSomethingLater)
	{
		if(!wroteSomethingLater)
		{
			wroteSomethingLater = true;
			buffer[12] = '\0';
		}
		buffer[11] = '0' + ((-1 == Hour ? 0:Hour) % 10);
		buffer[10] = '0' + ((-1 == Hour ? 0:Hour) / 10);
	}

	// Day
	if(Day != -1 || wroteSomethingLater)
	{
		if(!wroteSomethingLater)
		{
			wroteSomethingLater = true;
			buffer[10] = '\0';
		}
		buffer[9] = '0' + ((-1 == Day ? 1:Day) % 10);
		buffer[8] = '0' + ((-1 == Day ? 1:Day) / 10);
	}

	// Month
	if(Month != -1 || wroteSomethingLater)
	{
		if(!wroteSomethingLater)
		{
			wroteSomethingLater = true;
			buffer[8] = '\0';
		}
		buffer[7] = '0' + ((-1 == Month ? 1:Month) % 10);
		buffer[6] = '0' + ((-1 == Month ? 1:Month) / 10);
	}	

	// Year [bug 10K waiting to happen!!!!111]
	if(!wroteSomethingLater)
		buffer[6] = '\0';
	buffer[5] = '0' + Year % 10;
	buffer[4] = '0' + (Year/10) % 10;
	buffer[3] = '0' + (Year/100) % 10;
	buffer[2] = '0' + (Year/1000) % 10;
	buffer[1] = ':';
	buffer[0] = 'D';

	return std::string(buffer);
}

void PDFDate::SetToCurrentTime()
{
	time_t currentTime;
	tm structuredLocalTime;
	long timeZoneSecondsDifference;

	time(&currentTime);
	SAFE_LOCAL_TIME(structuredLocalTime,currentTime);

	Year = structuredLocalTime.tm_year + 1900;
	Month = structuredLocalTime.tm_mon + 1;
	Day = structuredLocalTime.tm_mday;
	Hour = structuredLocalTime.tm_hour;
	Minute = structuredLocalTime.tm_min;
	Second = structuredLocalTime.tm_sec;

	// if unsuccesful or method unknown don't provide UTC info (currently only knows for WIN32 and OSX
#if defined (__MWERKS__) || defined (__GNUC__)  || defined(_AIX32) || defined(WIN32)
	int status;
#if !defined(__MWERKS__) // (using c methods)
	struct tm *gmTime;

	time_t localEpoch, gmEpoch;

	/*First get local epoch time*/
	localEpoch = time(NULL);

	/* Using local time epoch get the GM Time */
	gmTime = gmtime(&localEpoch);
	gmTime->tm_isdst = -1;
	/* Convert gm time in to epoch format */
	gmEpoch = mktime(gmTime);

	timeZoneSecondsDifference =difftime(gmEpoch, localEpoch);
	status = 0;
#else // __MWERKS__ (using OSX methods)
	CFTimeZoneRef tzRef = ::CFTimeZoneCopySystem();
	if (tzRef)
	{
		CFTimeInterval intervalFromGMT = ::CFTimeZoneGetSecondsFromGMT(tzRef, currentTime);
		::CFRelease(tzRef);
		timeZoneSecondsDifference = intervalFromGMT;
		status = 0;
	}
	else
		status = -1;
#endif

	if(0 == status)
	{
		if(0 == timeZoneSecondsDifference)
		{
			UTC = eSame;
		}
		else
		{
			UTC = timeZoneSecondsDifference > 0 ? eEarlier : eLater;
			HourFromUTC = (int)(labs(timeZoneSecondsDifference) / 3600);
			MinuteFromUTC = (int)((labs(timeZoneSecondsDifference) - (labs(timeZoneSecondsDifference) / 3600)*3600) / 60);
		}
	}
	else
	{
		UTC = eUndefined;
		TRACE_LOG("PDFDate::SetToCurrentTime, Couldn't get UTC.");
	}

#else
	UTC = eUndefined;
#endif
}

void PDFDate::ParseString(std::string inValue)
{
    if(inValue.length() < 2 || inValue[0] != 'D' || inValue[1] != ':')
    {
        Year = -1;
        return;
    }
    
    Year = Int(inValue.substr(2,4));
    Month = -1;
    Day = -1;
    Hour = -1;
    Minute = -1;
    Second = -1;
    UTC = eUndefined;
    HourFromUTC = -1;
    MinuteFromUTC = -1;
    
    if(inValue.length() < 7)
        return;
    
    Month = Int(inValue.substr(6,2));

    if(inValue.length() < 9)
        return;
    
    Day = Int(inValue.substr(8,2));
    
    if(inValue.length() < 11)
        return;
    
    Hour = Int(inValue.substr(10,2));
    
    if(inValue.length() < 13)
        return;
    
    Minute = Int(inValue.substr(12,2));
    
    if(inValue.length() < 15)
        return;
    
    Second = Int(inValue.substr(14,2));

    if(inValue.length() < 17)
        return;
    
    if(inValue[16] == 'Z')
    {
        UTC = eSame;
    }
    else if(inValue[16] == '-' || inValue[16] == '+')
    {
        UTC = (inValue[16] == '-') ? eEarlier:eLater;
        
        if(inValue.length() < 18)
            return;
        
        HourFromUTC = Int(inValue.substr(17,2));
        
        if(inValue.length() < 21) // skipping '
            return;
        
        MinuteFromUTC = Int(inValue.substr(20,2));
    }
    
}