//===- WinCE/TimeValue.cpp - Win32 TimeValue Implementation -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides the Win32 implementation of the TimeValue class.
//
//===----------------------------------------------------------------------===//

#include "Win32.h"
#include <time.h>

namespace llvm {
using namespace sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only Win32 specific code.
//===----------------------------------------------------------------------===//



TimeValue TimeValue::now() {
  uint64_t ft = 0;
  SYSTEMTIME systemtime;
  GetSystemTime(&systemtime);
  SystemTimeToFileTime(&systemtime, reinterpret_cast<FILETIME *>(&ft));

  TimeValue t(0, 0);
  t.fromWin32Time(ft);
  return t;
}

char *week_short[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *mon_short[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

const int SecOfOneDay = 86400 ; // 24*60*60

const short month[2][13]={
           365,31,28,31,30,31,30,31,31,30,31,30,31,
           366,31,29,31,30,31,30,31,31,30,31,30,31
};


int IsLeapYear(int year)
{
    return (0==year%4 && (0!=year%100 || 0==year%400) );
}

void Second2Date(unsigned long* seconds , struct tm* Date )
{
	long totalsec = *(seconds) ;
	int leapyear;

	for(int i=0 ; ; i++)
	{
		leapyear = IsLeapYear(1970+i);
		totalsec -= (long) (month[leapyear][0] * SecOfOneDay);
		if( totalsec < 0 )
		{
			Date->tm_year = 70+i;          // 1970+i-1900 as standard C
			totalsec += (long) (month[leapyear][0] * SecOfOneDay);
			break;
		}
    }


	int weekday = 1900+Date->tm_year-1;       // the 1900 , do not forget !!!!!
	weekday = weekday + (int)(weekday/4) - (int)(weekday/100) + (int)(weekday/400) + (int)(totalsec/SecOfOneDay)+1;
	Date->tm_wday = weekday%7;


	for(int i=1; i<=12 ; i++ )
	{
		totalsec -= (long) (month[leapyear][i] * SecOfOneDay);
        if(totalsec < 0)
        {
			Date->tm_mon = i-1;           // the define of tm_mon (0--11)
			totalsec += (long) (month[leapyear][i] * SecOfOneDay);
			break;
		}
	}

	Date->tm_mday = (int)(totalsec/SecOfOneDay+1) ;

	int oddsec = (int)(totalsec%SecOfOneDay);
	Date->tm_hour = (int)(oddsec/3600);

	oddsec = (int)(oddsec%3600);
	Date->tm_min       = (int)(oddsec/60);

	oddsec = (int)(oddsec%60);
	Date->tm_sec = oddsec;

	Date->tm_year += 1900;

	return;
}

struct tm* gettime(struct tm* Date, unsigned long* seconds)
{
	Second2Date(seconds, Date);
	return Date;
}

std::string TimeValue::str() const {
	unsigned long seconds = this->toEpochTime() + 28800;
	struct tm Date;
	memset(&Date,0,sizeof(Date));
	struct tm *lt = gettime(&Date, &seconds);

	char buffer[25];
	sprintf(buffer, "%s %s %02d %02d:%02d:%02d %d", week_short[lt->tm_wday], mon_short[lt->tm_mon], lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, lt->tm_year);
	return std::string(buffer);
}

}
