/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


 #include <windows.h>
 #include <tchar.h>
 #include <time.h>

 /* globals */
 static const __int64 _onesec_in100ns = (__int64)10000000;
 static int   timezone_WCE, _timezone_WCE, altzone_WCE;
 static int   daylight;
 static char *tzname[2];


 /* __int64 <--> FILETIME */
 static __int64 wce_FILETIME2int64(FILETIME f)
 {
         __int64 t;

         t = f.dwHighDateTime;
         t <<= 32;
         t |= f.dwLowDateTime;
         return t;
 }

 static FILETIME wce_int642FILETIME(__int64 t)
 {
         FILETIME f;

         f.dwHighDateTime = (DWORD)((t >> 32) & 0x00000000FFFFFFFF);
         f.dwLowDateTime  = (DWORD)( t        & 0x00000000FFFFFFFF);
         return f;
 }

 /* FILETIME utility */
 static FILETIME wce_getFILETIMEFromYear(WORD year)
 {
         SYSTEMTIME s={0};
         FILETIME f;

         s.wYear      = year;
         s.wMonth     = 1;
         s.wDayOfWeek = 1;
         s.wDay       = 1;

         SystemTimeToFileTime( &s, &f );
         return f;
 }

 static time_t wce_getYdayFromSYSTEMTIME(const SYSTEMTIME* s)
 {
         __int64 t;
         FILETIME f1, f2;

         f1 = wce_getFILETIMEFromYear( s->wYear );
         SystemTimeToFileTime( s, &f2 );

         t = wce_FILETIME2int64(f2)-wce_FILETIME2int64(f1);

         return (time_t)((t/_onesec_in100ns)/(60*60*24));
 }

 /* tm <--> SYSTEMTIME */
 static SYSTEMTIME wce_tm2SYSTEMTIME(struct tm *t)
 {
         SYSTEMTIME s;

         s.wYear      = t->tm_year + 1900;
         s.wMonth     = t->tm_mon  + 1;
         s.wDayOfWeek = t->tm_wday;
         s.wDay       = t->tm_mday;
         s.wHour      = t->tm_hour;
         s.wMinute    = t->tm_min;
         s.wSecond    = t->tm_sec;
         s.wMilliseconds = 0;

         return s;
 }

 static struct tm wce_SYSTEMTIME2tm(SYSTEMTIME *s)
 {
         struct tm t;

         t.tm_year  = s->wYear - 1900;
         t.tm_mon   = s->wMonth- 1;
         t.tm_wday  = s->wDayOfWeek;
         t.tm_mday  = s->wDay;
         t.tm_yday  = wce_getYdayFromSYSTEMTIME(s);
         t.tm_hour  = s->wHour;
         t.tm_min   = s->wMinute;
         t.tm_sec   = s->wSecond;
         t.tm_isdst = 0;

         return t;
 }

 /* FILETIME <--> time_t */
 time_t wce_FILETIME2time_t(const FILETIME* f)
 {
         FILETIME f1601, f1970;
         __int64 t, offset;

         f1601 = wce_getFILETIMEFromYear(1601);
         f1970 = wce_getFILETIMEFromYear(1970);

         offset = wce_FILETIME2int64(f1970) - wce_FILETIME2int64(f1601);

         t = wce_FILETIME2int64(*f);

         t -= offset;
         return (time_t)(t / _onesec_in100ns);
 }

 FILETIME wce_time_t2FILETIME(const time_t t)
 {
         FILETIME f, f1970;
         __int64 time;

         f1970 = wce_getFILETIMEFromYear(1970);

         time = t;
         time *= _onesec_in100ns;
         time += wce_FILETIME2int64(f1970);

         f = wce_int642FILETIME(time);

         return f;
 }

 /* time.h difinition */
 time_t time( time_t *timer )
 {
         SYSTEMTIME s;
         FILETIME   f;

         if( timer==NULL ) return 0;

         GetSystemTime( &s );

         SystemTimeToFileTime( &s, &f );

         *timer = wce_FILETIME2time_t(&f);
         return *timer;
 }

 struct tm *localtime( const time_t *timer )
 {
         SYSTEMTIME ss, ls, s;
         FILETIME   sf, lf, f;
         __int64 t, diff;
         static struct tm tms;

         GetSystemTime(&ss);
         GetLocalTime(&ls);

         SystemTimeToFileTime( &ss, &sf );
         SystemTimeToFileTime( &ls, &lf );

         diff = wce_FILETIME2int64(sf) - wce_FILETIME2int64(lf);

         f = wce_time_t2FILETIME(*timer);
         t = wce_FILETIME2int64(f) - diff;
         f = wce_int642FILETIME(t);

         FileTimeToSystemTime( &f, &s );

         tms = wce_SYSTEMTIME2tm(&s);

         return &tms;
 }

 time_t mktime(struct tm* pt)
 {
         SYSTEMTIME ss, ls, s;
         FILETIME   sf, lf, f;
         __int64 diff;

         GetSystemTime(&ss);
         GetLocalTime(&ls);
         SystemTimeToFileTime( &ss, &sf );
         SystemTimeToFileTime( &ls, &lf );

         diff = (wce_FILETIME2int64(lf)-wce_FILETIME2int64(sf))/_onesec_in100ns;

         s = wce_tm2SYSTEMTIME(pt);
         SystemTimeToFileTime( &s, &f );
         return wce_FILETIME2time_t(&f) - (time_t)diff;
 }

 struct tm *gmtime(const time_t *t)
 {
         FILETIME f;
         SYSTEMTIME s;
         static struct tm tms;

         f = wce_time_t2FILETIME(*t);
         FileTimeToSystemTime(&f, &s);
         tms = wce_SYSTEMTIME2tm(&s);
         return &tms;
 }

 char* ctime( const time_t *t )
 {
         // Wed Jan 02 02:03:55 1980\n\0
         static char buf[30]={0};
         char week[] = "Sun Mon Tue Wed Thr Fri Sat ";
         char month[]= "Jan Feb Mar Apl May Jun Jul Aug Sep Oct Nov Dec ";
         struct tm tms;

         tms = *localtime(t);

         strncpy( buf,    week+tms.tm_wday*4, 4 );
         strncpy( buf+4,  month+tms.tm_mon*4, 4 );
         sprintf( buf+8,  "%02d ", tms.tm_mday );
         sprintf( buf+11, "%02d:%02d:%02d %d\n",
                 tms.tm_hour, tms.tm_min, tms.tm_sec, tms.tm_year+1900 );
         return buf;
 }

 char *asctime(const struct tm *pt)
 {
         static char buf[30]={0};
         char week[] = "Sun Mon Tue Wed Thr Fri Sat ";
         char month[]= "Jan Feb Mar Apl May Jun Jul Aug Sep Oct Nov Dec ";

         strncpy( buf,    week+pt->tm_wday*4, 4 );
         strncpy( buf+4,  month+pt->tm_mon*4, 4 );
         sprintf( buf+8,  "%02d ", pt->tm_mday );
         sprintf( buf+11, "%02d:%02d:%02d %d\n",
                 pt->tm_hour, pt->tm_min, pt->tm_sec, pt->tm_year+1900 );
         return buf;
 }

 void tzset()
 {
         daylight = 1;
         _timezone_WCE = 28800;
         timezone_WCE = 28800;
 }


