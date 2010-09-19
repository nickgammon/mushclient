#include "stdafx.h"

#define MIN_DATE                0  // year 1800
#define MAX_DATE                2958465L    // about year 9999

// Half a second, expressed in days
#define HALF_SECOND  (1.0/172800.0)

#define SECS_IN_DAY (60.0 * 60.0 * 24.0)

	// DateTime math
CmcDateTime CmcDateTime::operator+(const CmcDateTimeSpan& dateSpan) const
  {
	  CmcDateTime dateResult;    // Initializes m_status to valid

	  // If either operand NULL, result NULL
	  if (GetStatus() == null || dateSpan.GetStatus() == null)
	    {
		  dateResult.SetStatus(null);
		  return dateResult;
	    }

	  // If either operand invalid, result invalid
	  if (GetStatus() == invalid || dateSpan.GetStatus() == invalid)
	    {
		  dateResult.SetStatus(invalid);
		  return dateResult;
	    }

	  // Compute the actual date difference by adding underlying dates
	  dateResult = m_dt + dateSpan.m_span;

	  // Validate within range
//	    dateResult.CheckRange();

	  return dateResult;
  }  // end of CmcDateTime CmcDateTime::operator+

CmcDateTime CmcDateTime::operator-(const CmcDateTimeSpan& dateSpan) const
  {
	  CmcDateTime dateResult;    // Initializes m_status to valid

	  // If either operand NULL, result NULL
	  if (GetStatus() == null || dateSpan.GetStatus() == null)
	    {
		  dateResult.SetStatus(null);
		  return dateResult;
	    }

	  // If either operand invalid, result invalid
	  if (GetStatus() == invalid || dateSpan.GetStatus() == invalid)
	    {
		  dateResult.SetStatus(invalid);
		  return dateResult;
	    }

	  // Compute the actual date difference by subtracting underlying dates
	  dateResult = m_dt - dateSpan.m_span;

	  // Validate within range
//	    dateResult.CheckRange();

	  return dateResult;
  }  // end of CmcDateTime CmcDateTime::operator-


const CmcDateTime& CmcDateTime::operator+=(const CmcDateTimeSpan dateSpan)
  { 
  *this = *this + dateSpan; 
  return *this; 
  }   // end of CmcDateTime& CmcDateTime::operator+=

const CmcDateTime& CmcDateTime::operator-=(const CmcDateTimeSpan dateSpan)
  { 
  *this = *this - dateSpan; 
  return *this; 
  }   //  end of CmcDateTime& CmcDateTime::operator-=

void CmcDateTime::CheckRange()
{
	if (m_dt > MAX_DATE || m_dt < MIN_DATE) // about year 100 to about 9999
		SetStatus(invalid);
}


// DateTimeSpan math
CmcDateTimeSpan CmcDateTime::operator-(const CmcDateTime& date) const
  {
	COleDateTimeSpan spanResult;

	// If either operand NULL, result NULL
	if (GetStatus() == null || date.GetStatus() == null)
	{
		spanResult.SetStatus(COleDateTimeSpan::null);
		return (double)spanResult;
	}

	// If either operand invalid, result invalid
	if (GetStatus() == invalid || date.GetStatus() == invalid)
	{
		spanResult.SetStatus(COleDateTimeSpan::invalid);
		return (double)spanResult;
	}

	// Return result (span can't be invalid, so don't check range)
	return m_dt - date.m_dt;

  }


int CmcDateTime::GetYear() const
  {
  COleDateTime t (m_dt);
  return t.GetYear ();
  }

int CmcDateTime::GetMonth() const       // month of year (1 = Jan)
  {
  COleDateTime t (m_dt);
  return t.GetMonth ();
  }

int CmcDateTime::GetDay() const         // day of month (0-31)
  {
  COleDateTime t (m_dt);
  return t.GetDay ();
  }

int CmcDateTime::GetHour() const        // hour in day (0-23)
  {
  COleDateTime t (m_dt);
  return t.GetHour ();
  }

int CmcDateTime::GetMinute() const      // minute in hour (0-59)
  {
  COleDateTime t (m_dt);
  return t.GetMinute ();
  }

int CmcDateTime::GetSecond() const      // second in minute (0-59)
  {
  COleDateTime t (m_dt);
  return t.GetSecond ();
  }

int CmcDateTime::GetDayOfWeek() const   // 1=Sun, 2=Mon, ..., 7=Sat
  {
  COleDateTime t (m_dt);
  return t.GetDayOfWeek ();
  }

int CmcDateTime::GetDayOfYear() const   // days since start of year, Jan 1 = 1
  {
  COleDateTime t (m_dt);
  return t.GetDayOfYear ();
  }

// One-based array of days in year at month start
static int MonthDays[13] =
	{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};


static BOOL  mcDateFromTm(WORD wYear, WORD wMonth, WORD wDay,
	WORD wHour, WORD wMinute, WORD wSecond, DATE& dtDest)
{
	// Validate year and month (ignore day of week and milliseconds)
	if (wYear > 9999 || wMonth < 1 || wMonth > 12)
		return FALSE;

	//  Check for leap year and set the number of days in the month
	BOOL bLeapYear = ((wYear & 3) == 0) &&
		((wYear % 100) != 0 || (wYear % 400) == 0);

	int nDaysInMonth =
		MonthDays[wMonth] - MonthDays[wMonth-1] +
		((bLeapYear && wDay == 29 && wMonth == 2) ? 1 : 0);

	// Finish validating the date
	if (wDay < 1 || wDay > nDaysInMonth ||
		wHour > 23 || wMinute > 59 ||
		wSecond > 59)
	{
		return FALSE;
	}

	// Cache the date in days and time in fractional days
	long nDate;
	double dblTime;

	//It is a valid date; make Jan 1, 1AD be 1
	nDate = wYear*365L + wYear/4 - wYear/100 + wYear/400 +
		MonthDays[wMonth-1] + wDay;

	//  If leap year and it's before March, subtract 1:
	if (wMonth <= 2 && bLeapYear)
		--nDate;

	//  Offset so that 12/30/1899 is 0
	nDate -= 693959L;

	dblTime = (((long)wHour * 3600L) +  // hrs in seconds
		((long)wMinute * 60L) +  // mins in seconds
		  wSecond) / SECS_IN_DAY;

	dtDest = (double) nDate + ((nDate >= 0) ? dblTime : -dblTime);

	return TRUE;
}  // end of mcDateFromTm

int CmcDateTime::SetDateTime(int nYear, int nMonth, int nDay,
	int nHour, int nMin, double nSec)
{
	return m_status = mcDateFromTm((WORD)nYear, (WORD)nMonth,
		(WORD)nDay, (WORD)nHour, (WORD)nMin, nSec, m_dt) ?
		valid : invalid;
}


/*

  This considerable amount of mucking around is because the date/time routines
  do not seem to return a time to more than 1-second granularity, regardless of
  the fact that GetSystemTime is supposed to return time to the millisecond.

  I guess this is because the real-time clocks are only updated every second.

  So, what I am doing here is taking the 'real' time every RESYNC_EVERY_SECS
  seconds, and then using the high-performance counter to find the number of
  seconds (fractions of seconds) that elapsed since. This is then added to the
  last "real" time to find the actual time, effectively with much higher
  granularity.

  */

#define RESYNC_EVERY_SECS 60

CmcDateTime CmcDateTime::GetTimeNow ()
  {
static bool bFirstTime = true;

static CmcDateTime last_date;
static LARGE_INTEGER last_time;
static LONGLONG iCounterFrequency = 0;

  if (bFirstTime)
    {
    bFirstTime = false;
    LARGE_INTEGER large_int_frequency;
    if (QueryPerformanceFrequency (&large_int_frequency))
      {
      iCounterFrequency = large_int_frequency.QuadPart;
      QueryPerformanceCounter (&last_time);
      }
    }

  double secs = 0;
  LARGE_INTEGER time_now;

  if (iCounterFrequency)
    {
    QueryPerformanceCounter (&time_now);

    LONGLONG offset = time_now.QuadPart - last_time.QuadPart;
    secs = (double) offset / (double) iCounterFrequency;
//    TRACE ("Secs = %10.4f\n", secs);
    }
  else
    {
    time_now.QuadPart = 0;
    time_now.QuadPart = 0;
    }


  // if no high-performance counter, just query the time each time
  if (iCounterFrequency == 0 || 
      last_date.m_dt == 0 ||
      secs > RESYNC_EVERY_SECS)
    {
    SYSTEMTIME systime;
    GetLocalTime (&systime);
 	  last_date = CmcDateTime(systime.wYear, systime.wMonth,
		                  systime.wDay, systime.wHour, systime.wMinute,
		                  (double) systime.wSecond + ( (double) systime.wMilliseconds / 1000.0) );
    secs = 0;
    last_time = time_now;
    }

  CmcDateTime this_date (last_date);

  this_date.m_dt += secs / SECS_IN_DAY;

  // ---- debugging
#if 0
    {
    SYSTEMTIME systime;
    GetSystemTime (&systime);
 	  CmcDateTime test = CmcDateTime(systime.wYear, systime.wMonth,
		                  systime.wDay, systime.wHour, systime.wMinute,
		                  (double) systime.wSecond + ( (double) systime.wMilliseconds / 1000.0) );
    double diff = test.m_dt - this_date.m_dt;

    TRACE1 ("Time difference = %10.8f\n", diff);
    }

#endif
  // --- end debugging

  return this_date; 

  }

void CmcDateTimeSpan::SetDateTimeSpan(
	long lDays, int nHours, int nMins, double nSecs)
{
	// Set date span by breaking into fractional days (all input ranges valid)
	m_span = lDays + ((double)nHours)/24 + ((double)nMins)/(24*60) +
		(nSecs)/(24*60*60);

	SetStatus(valid);
}


const CmcDateTimeSpan& CmcDateTimeSpan::operator=(double dblSpanSrc)
{
	m_span = dblSpanSrc;
	SetStatus(valid);
	return *this;
}

const CmcDateTimeSpan& CmcDateTimeSpan::operator=(const CmcDateTimeSpan& dateSpanSrc)
{
	m_span = dateSpanSrc.m_span;
	m_status = dateSpanSrc.m_status;
	return *this;
}


CmcDateTimeSpan CmcDateTimeSpan::operator+(const CmcDateTimeSpan& dateSpan) const
  {
	CmcDateTimeSpan dateSpanTemp;

	// If either operand Null, result Null
	if (GetStatus() == null || dateSpan.GetStatus() == null)
	{
		dateSpanTemp.SetStatus(null);
		return dateSpanTemp;
	}

	// If either operand Invalid, result Invalid
	if (GetStatus() == invalid || dateSpan.GetStatus() == invalid)
	{
		dateSpanTemp.SetStatus(invalid);
		return dateSpanTemp;
	}

	// Add spans and validate within legal range
	dateSpanTemp.m_span = m_span + dateSpan.m_span;
//	dateSpanTemp.CheckRange();

	return dateSpanTemp;
  }

CmcDateTimeSpan CmcDateTimeSpan::operator-(const CmcDateTimeSpan& dateSpan) const
  {
	CmcDateTimeSpan dateSpanTemp;

	// If either operand Null, result Null
	if (GetStatus() == null || dateSpan.GetStatus() == null)
	{
		dateSpanTemp.SetStatus(null);
		return dateSpanTemp;
	}

	// If either operand Invalid, result Invalid
	if (GetStatus() == invalid || dateSpan.GetStatus() == invalid)
	{
		dateSpanTemp.SetStatus(invalid);
		return dateSpanTemp;
	}

	// Subtract spans and validate within legal range
	dateSpanTemp.m_span = m_span - dateSpan.m_span;
//	dateSpanTemp.CheckRange();

	return dateSpanTemp;
  }

const CmcDateTimeSpan& CmcDateTimeSpan::operator+=(const CmcDateTimeSpan dateSpan)
	{ *this = *this + dateSpan; return *this; }

const CmcDateTimeSpan& CmcDateTimeSpan::operator-=(const CmcDateTimeSpan dateSpan)
	{ *this = *this - dateSpan; return *this; }
CmcDateTimeSpan CmcDateTimeSpan::operator-() const
	{ return -this->m_span; }


