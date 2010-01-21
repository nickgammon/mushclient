/////////////////////////////////////////////////////////////////////////////
// CmcDateTime class    - MUSHclient date/time
//    like COleDateTime but with double support for seconds

class CmcDateTimeSpan;

#ifndef AFX_OLE_DATETIME_HALFSECOND
  #define AFX_OLE_DATETIME_HALFSECOND (1.0 / (2.0 * (60.0 * 60.0 * 24.0)))
#endif

class CmcDateTime
{
// Constructors
public:
  static CmcDateTime GetTimeNow ();

	CmcDateTime() 
      { m_dt = 0; SetStatus(valid); }

	CmcDateTime(const CmcDateTime& dateSrc) 
      { m_dt = dateSrc.m_dt; m_status = dateSrc.m_status; }

	CmcDateTime(const double dtSrc) 
    { m_dt = dtSrc; SetStatus(valid); }

	CmcDateTime(int nYear, int nMonth, int nDay,
		int nHour, int nMin, double nSec)
      { SetDateTime(nYear, nMonth, nDay, nHour, nMin, nSec); }


// Attributes
public:
	enum DateTimeStatus
	{
		valid = 0,
		invalid = 1,    // Invalid date (out of range, etc.)
		null = 2,       // Literally has no value
	};

	double m_dt;
	DateTimeStatus m_status;

	void SetStatus(DateTimeStatus status) { m_status = status; }

	DateTimeStatus GetStatus() const { return m_status; }

	int GetYear() const;
	int GetMonth() const;       // month of year (1 = Jan)
	int GetDay() const;         // day of month (0-31)
	int GetHour() const;        // hour in day (0-23)
	int GetMinute() const;      // minute in hour (0-59)
	int GetSecond() const;      // second in minute (0-59)
	int GetDayOfWeek() const;   // 1=Sun, 2=Mon, ..., 7=Sat
	int GetDayOfYear() const;   // days since start of year, Jan 1 = 1

  const double GetTime () const { return m_dt; }

// Operations
public:
	const CmcDateTime& operator=(const CmcDateTime& dateSrc)
    { m_dt = dateSrc.m_dt; m_status = dateSrc.m_status; return *this; }

	bool operator==(const CmcDateTime& date) const
    { return (m_status == date.m_status && m_dt == date.m_dt); }

	bool operator!=(const CmcDateTime& date) const
    { return (m_status != date.m_status || m_dt != date.m_dt); }

	bool operator<(const CmcDateTime& date) const
    { return m_dt < date.m_dt; }

	bool operator>(const CmcDateTime& date) const
    { return m_dt > date.m_dt; }

	bool operator<=(const CmcDateTime& date) const
    { return m_dt <= date.m_dt; }

	bool operator>=(const CmcDateTime& date) const
    { return m_dt >= date.m_dt; }

	// DateTime math
	CmcDateTime operator+(const CmcDateTimeSpan& dateSpan) const;

  CmcDateTime operator-(const CmcDateTimeSpan& dateSpan) const;

	const CmcDateTime& operator+=(const CmcDateTimeSpan dateSpan);
	const CmcDateTime& operator-=(const CmcDateTimeSpan dateSpan);

	// DateTimeSpan math
	CmcDateTimeSpan operator-(const CmcDateTime& date) const;

	int SetDateTime(int nYear, int nMonth, int nDay,
		int nHour, int nMin, double nSec);
	int SetDate(int nYear, int nMonth, int nDay);
	int SetTime(int nHour, int nMin, double nSec);

// Implementation
protected:
	void CheckRange();
	friend CmcDateTimeSpan;
};


/////////////////////////////////////////////////////////////////////////////
// CmcDateTimeSpan class
class CmcDateTimeSpan
{
// Constructors
public:
	CmcDateTimeSpan()
  	{ m_span = 0; SetStatus(valid); }


	CmcDateTimeSpan(double dblSpanSrc)
	  { m_span = dblSpanSrc; SetStatus(valid); }

	CmcDateTimeSpan(const CmcDateTimeSpan& dateSpanSrc)
  	{ m_span = dateSpanSrc.m_span; m_status = dateSpanSrc.m_status; }

	CmcDateTimeSpan(long lDays, int nHours, int nMins, double nSecs)
  	{ SetDateTimeSpan(lDays, nHours, nMins, nSecs); }

// Attributes
public:
	enum DateTimeSpanStatus
	{
		valid = 0,
		invalid = 1,    // Invalid span (out of range, etc.)
		null = 2,       // Literally has no value
	};

	double m_span;
	DateTimeSpanStatus m_status;

	void SetStatus(DateTimeSpanStatus status)
  	{ m_status = status; }

	DateTimeSpanStatus GetStatus() const
  	{ return m_status; }
  
	double GetTotalDays() const    // span in days (about -3.65e6 to 3.65e6)
  	{ ASSERT(GetStatus() == valid); return m_span; }
	double GetTotalHours() const   // span in hours (about -8.77e7 to 8.77e6)
  	{ ASSERT(GetStatus() == valid);
		long lReturns = (long)(m_span * 24 + AFX_OLE_DATETIME_HALFSECOND);
		return lReturns;
	  }

	double GetTotalMinutes() const // span in minutes (about -5.26e9 to 5.26e9)
	  { ASSERT(GetStatus() == valid);
		long lReturns = (long)(m_span * 24 * 60 + AFX_OLE_DATETIME_HALFSECOND);
		return lReturns;
	  }

  double GetTotalSeconds() const // span in seconds (about -3.16e11 to 3.16e11)
  	{ ASSERT(GetStatus() == valid);
		return m_span * (double) (24 * 60 * 60);
	  }


	long GetDays() const       // component days in span
  	{ ASSERT(GetStatus() == valid); return (long)m_span; }
	long GetHours() const;      // component hours in span (-23 to 23)
	long GetMinutes() const;    // component minutes in span (-59 to 59)
	double GetSeconds() const;    // component seconds in span (-59 to 59)

// Operations
public:
	const CmcDateTimeSpan& operator=(double dblSpanSrc);
	const CmcDateTimeSpan& operator=(const CmcDateTimeSpan& dateSpanSrc);

	bool operator==(const CmcDateTimeSpan& dateSpan) const
  	{ return (m_status == dateSpan.m_status &&
	  	m_span == dateSpan.m_span); }

	bool operator!=(const CmcDateTimeSpan& dateSpan) const
  	{ return (m_status != dateSpan.m_status ||
	  	m_span != dateSpan.m_span); }

	bool operator<(const CmcDateTimeSpan& dateSpan) const
  	{ ASSERT(GetStatus() == valid);
	  	ASSERT(dateSpan.GetStatus() == valid);
		  return m_span < dateSpan.m_span; }

	bool operator>(const CmcDateTimeSpan& dateSpan) const
  	{ ASSERT(GetStatus() == valid);
		  ASSERT(dateSpan.GetStatus() == valid);
		  return m_span > dateSpan.m_span; }

	bool operator<=(const CmcDateTimeSpan& dateSpan) const
  	{ ASSERT(GetStatus() == valid);
	  	ASSERT(dateSpan.GetStatus() == valid);
		  return m_span <= dateSpan.m_span; }

	bool operator>=(const CmcDateTimeSpan& dateSpan) const
	  { ASSERT(GetStatus() == valid);
		  ASSERT(dateSpan.GetStatus() == valid);
		  return m_span >= dateSpan.m_span; }


	// DateTimeSpan math
	CmcDateTimeSpan operator+(const CmcDateTimeSpan& dateSpan) const;
	CmcDateTimeSpan operator-(const CmcDateTimeSpan& dateSpan) const;
	const CmcDateTimeSpan& operator+=(const CmcDateTimeSpan dateSpan);
	const CmcDateTimeSpan& operator-=(const CmcDateTimeSpan dateSpan);
	CmcDateTimeSpan operator-() const;

	operator double() const;

	void SetDateTimeSpan(long lDays, int nHours, int nMins, double nSecs);

// Implementation
public:
	void CheckRange();
	friend CmcDateTime;
};
