#include "stdafx.h"
#include "uts.h"

#include "math.h"

#ifdef WIN32
#include <KR3/wl/windows.h>
#elif defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

#include <ctime>

using namespace kr;

inline UnixTimeStamp::TIMEZONE::TIMEZONE() noexcept
{
#ifdef WIN32
	TIME_ZONE_INFORMATION TimeZoneInfo;
	GetTimeZoneInformation(&TimeZoneInfo);
	if (TimeZoneInfo.Bias < 0)
	{
		cSign = '-';
		TimeZoneInfo.Bias = -TimeZoneInfo.Bias;
	}
	else
	{
		cSign = '+';
	}
	nHour = (byte)(TimeZoneInfo.Bias / 60);
	nMin = (byte)(TimeZoneInfo.Bias % 60);
#elif defined(__EMSCRIPTEN__)
	int minoffset = EM_ASM_INT_V({ return new Date().getTimezoneOffset(); });
	if (minoffset < 0)
	{
		cSign = '+';
		minoffset = -minoffset;
	}
	else
	{
		cSign = '-';
	}
	nHour = (byte)(minoffset / 60);
	nMin = (byte)(minoffset % 60);
#else
#error Need implement
#endif
}

UnixTimeStamp::TIMEZONE UnixTimeStamp::timeZone;

struct tmex:tm
{
	int tm_week;
};

template <>
Text _pri_::DATETEXT<char>::wkday[] ={ (Text)"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
template <>
Text _pri_::DATETEXT<char>::month[] ={ (Text)"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
template <>
Text16 _pri_::DATETEXT<char16>::wkday[] ={ (Text16)u"Sun", u"Mon", u"Tue", u"Wed", u"Thu", u"Fri", u"Sat" };
template <>
Text16 _pri_::DATETEXT<char16>::month[] ={ (Text16)u"Jan", u"Feb", u"Mar", u"Apr", u"May", u"Jun", u"Jul", u"Aug", u"Sep", u"Oct", u"Nov", u"Dec" };


template <typename C> struct UnixTimeStamp::TimeMethods
{
	using Text = View<C>;
	struct REALVALUE
	{
		dword value;
		dword cipher;
	
		REALVALUE(Text point) noexcept
		{
			if (point.size() > 10) point = point.cut(10);
			value = point.to_uint();
			cipher = (dword)math::pow((size_t)10, point.size());
		}
	};

	static void GetTime(tmex *t, Text hour, Text point) throws(InvalidSourceException, OutOfRangeException)
	{
		if(!hour.numberonly()) throw InvalidSourceException();
		if(!point.numberonly()) throw InvalidSourceException();
		t->tm_hour = hour.to_uint();
		if((uint)t->tm_hour > 24) throw OutOfRangeException();

		if(point != nullptr)
		{
			REALVALUE pt = point;
			t->tm_sec = pt.value * (60*60) / pt.cipher;
			t->tm_min = t->tm_sec / 60;
			t->tm_sec %= 60;
		}
	}
	static void GetTime(tmex *t, Text hour, Text min, Text point) throws(InvalidSourceException, OutOfRangeException)
	{
		if(!hour.numberonly()) throw InvalidSourceException();
		if(!min.numberonly()) throw InvalidSourceException();
		if(!point.numberonly()) throw InvalidSourceException();
		t->tm_hour = hour.to_uint();
		if((uint)t->tm_hour > 24) throw OutOfRangeException();
		t->tm_min = min.to_uint();
		if((uint)t->tm_min > 59) throw OutOfRangeException();

		if(point != nullptr)
		{
			REALVALUE pt = point;
			t->tm_sec = pt.value * 60 / pt.cipher;
		}
	}
	static void GetTime(tmex *t, Text hour, Text min, Text sec, Text point) throws(InvalidSourceException, OutOfRangeException)
	{
		if(!hour.numberonly()) throw InvalidSourceException();
		if(!min.numberonly()) throw InvalidSourceException();
		if(!sec.numberonly()) throw InvalidSourceException();
		if(!point.numberonly()) throw InvalidSourceException();
		t->tm_hour = hour.to_uint();
		if((uint)t->tm_hour > 24) throw OutOfRangeException();
		t->tm_min = min.to_uint();
		if((uint)t->tm_min > 59) throw OutOfRangeException();
		t->tm_sec = sec.to_uint();
		if((uint)t->tm_sec > 59) throw OutOfRangeException();
	}
	static void GetDate(tmex *t, Text year) throws(InvalidSourceException, OutOfRangeException)
	{
		if(!year.numberonly()) throw InvalidSourceException();
		t->tm_year = year.to_uint();

		if(t->tm_year < 1970) throw OutOfRangeException();
		t->tm_year -= 1900;
	}
	static void GetDate(tmex *t, Text year, Text month) throws(InvalidSourceException, OutOfRangeException)
	{
		if(!year.numberonly()) throw InvalidSourceException();
		if(!month.numberonly()) throw InvalidSourceException();
		t->tm_year = year.to_uint();
		if(t->tm_year < 1970) throw OutOfRangeException();
		t->tm_year -= 1900;
		t->tm_mon = month.to_uint()-1;
		if((uint)t->tm_mon >= 12) throw OutOfRangeException();
	}
	static void GetYDate(tmex *t, Text year, Text yday) throws(InvalidSourceException, OutOfRangeException)
	{
		if(!year.numberonly()) throw InvalidSourceException();
		if(!yday.numberonly()) throw InvalidSourceException();
		t->tm_year = year.to_uint();
		if(t->tm_year < 1970) throw OutOfRangeException();
		t->tm_year -= 1900;
		t->tm_yday = yday.to_uint();
		if((uint)t->tm_yday-1 > 365) throw OutOfRangeException();
	}
	static void GetWDate(tmex *t, Text year, Text week, Text wday) throws(InvalidSourceException, OutOfRangeException)
	{
		if(!year.numberonly()) throw InvalidSourceException();
		if(!week.numberonly()) throw InvalidSourceException();
		if(!wday.numberonly()) throw InvalidSourceException();
		t->tm_year = year.to_uint();
		if(t->tm_year < 1970) throw OutOfRangeException();
		t->tm_year -= 1900;
		t->tm_week = week.to_uint();
		t->tm_wday = wday.to_uint();
	}
	static void GetWDate(tmex *t, Text year, Text week) throws(InvalidSourceException, OutOfRangeException)
	{
		if(!year.numberonly()) throw InvalidSourceException();
		if(!week.numberonly()) throw InvalidSourceException();
		t->tm_year = year.to_uint();
		if(t->tm_year < 1970) throw OutOfRangeException();
		t->tm_year -= 1900;
		t->tm_week = week.to_uint();
		t->tm_wday = 0;
	}
	static void GetDate(tmex *t, Text year, Text month, Text date) throws(InvalidSourceException, OutOfRangeException)
	{
		if(!year.numberonly()) throw InvalidSourceException();
		if(!month.numberonly()) throw InvalidSourceException();
		if(!date.numberonly()) throw InvalidSourceException();
		t->tm_year = year.to_uint();
		if(t->tm_year < 1970) throw OutOfRangeException();
		t->tm_year -= 1900;
		t->tm_mon = month.to_uint()-1;
		if((uint)t->tm_mon >= 12) throw OutOfRangeException();
		t->tm_mday = date.to_uint();
	}
	static time_t ReadISO8601(Text str) throws(InvalidSourceException, OutOfRangeException)
	{
		Text ymd;
		tmex t;
		mema::zero(t);
		t.tm_mday = 1;
		t.tm_wday = -1;
		t.tm_yday = -1;

		// time test
		{
			Text time = Text(str.find('T'), str.end());
			if(time != nullptr)
			{
				ymd = str.cut(time.data());
				time++;

				if(time.size() < 2) throw InvalidSourceException();
				if(time.size() == 2)
				{
					GetTime(&t, (Text)time, (Text)nullptr);
				}
				else
				{
					const C* point = time.find_y({ ',', '.' });
					Text tmcut;
					if(point != nullptr)
					{
						tmcut = time.cut(point);
						point++;
					}
					else tmcut = time;

					if(time[2] != ':') // non ext
					{
						switch(tmcut.size())
						{
						case 2: GetTime(&t, tmcut, Text(point, time.end())); break;
						case 4: GetTime(&t, tmcut.subarr(0, 2), (Text)tmcut.subarr(2), Text(point, time.end())); break;
						case 6: GetTime(&t, tmcut.subarr(0, 2), tmcut.subarr(2, 2), tmcut.subarr(4), Text(point, time.end())); break;
						default: throw InvalidSourceException();
						}
					}
					else // ext
					{
						switch(tmcut.size())
						{
						case 5: GetTime(&t, time.cut(2), time.subarr(3), Text(point, time.end())); break;
						case 8:
							if(time[5] != ':') throw InvalidSourceException();
							GetTime(&t, time.cut(2), time.subarr(3, 2), time.subarr(6));
							break;
						default: throw InvalidSourceException();
						}
					}
				}
			}
			else ymd = str;
		}

		// year test
		if(ymd.size() < 4) throw InvalidSourceException();
		if(ymd.size() == 4)
		{
			GetDate(&t, ymd);
		}
		else if(ymd[4] == '-') // ext
		{
			if(ymd.size() < 7) throw InvalidSourceException();
			Text year = ymd.subarr(0, 4);
			if(ymd[5] == 'W') // week day
			{
				switch(ymd.size())
				{
				case 8: GetWDate(&t, year, (Text)ymd.subarr(6)); break;
				case 10:
					if(ymd[8] != '-') throw InvalidSourceException();
					GetWDate(&t, year, ymd.subarr(6, 2), (Text)ymd.subarr(9));
					break;
				}
			}
			else
			{
				switch(ymd.size()) // year/month day
				{
				case 7: GetDate(&t, year, (Text)ymd.subarr(5)); break;
				case 8: GetYDate(&t, year, (Text)ymd.subarr(5)); break;
				case 10:
					if(ymd[7] != '-') throw InvalidSourceException();
					GetDate(&t, year, ymd.subarr(5, 2), (Text)ymd.subarr(8));
					break;
				default: throw InvalidSourceException();
				}
			}
		}
		else // non ext
		{
			Text year = ymd.subarr(0, 4);
			if(ymd[4] == 'W') // week day
			{
				switch(ymd.size())
				{
				case 7: GetWDate(&t, year, (Text)ymd.subarr(5)); break;
				case 8: GetWDate(&t, year, ymd.subarr(5, 2), (Text)ymd.subarr(7)); break;
				}
			}
			else // year/month day
			{
				switch(ymd.size())
				{
				case 8: GetDate(&t, year, ymd.subarr(4, 2), (Text)ymd.subarr(6)); break;
				case 7: GetYDate(&t, year, (Text)ymd.subarr(4)); break;
				default: throw InvalidSourceException();
				}
			}
		}

		if(t.tm_yday != -1) // yday
		{
			int yday = t.tm_yday;
			time_t uts = mktime(&t);
			uts += (yday - t.tm_yday) * (60*60*24);
			return uts;
		}
		else if(t.tm_wday != -1) // wday
		{
			t.tm_week--;
			if((uint)t.tm_week > 52) throw OutOfRangeException();
			int wday = t.tm_wday-1;
			if((uint)wday > 6) throw OutOfRangeException();
			time_t uts = mktime(&t);
			uts += (t.tm_week * 7 + wday - t.tm_wday) * (60*60*24);
			return uts;
		}
		else // normal
		{
			if((uint)t.tm_mday-1 > 30) throw OutOfRangeException();
			return mktime(&t);
		}
	}
	static constexpr size_t lengthWebDate = 29;
	static time_t readWebDate(Text str) throws(InvalidSourceException)
	{
		//"wkd, DD MMM YYYY hh:mm:ss GMT"

		if(str.size() != lengthWebDate) throw InvalidSourceException();
		if(str[3] != ',') throw InvalidSourceException();
		if(str[4] != ' ') throw InvalidSourceException();
		if(str[7] != ' ') throw InvalidSourceException();
		if(str[11] != ' ') throw InvalidSourceException();
		if(str[16] != ' ') throw InvalidSourceException();
		if(str[19] != ':') throw InvalidSourceException();
		if(str[22] != ':') throw InvalidSourceException();
		if(str[25] != ' ') throw InvalidSourceException();
		if(str[26] != 'G') throw InvalidSourceException();
		if(str[27] != 'M') throw InvalidSourceException();
		if(str[28] != 'T') throw InvalidSourceException();

		tm t;
		mema::zero(t);

		{ // date
			Text strDate = str.subarr(5, 2);
			if(!strDate.numberonly()) return 0;
			t.tm_mday = strDate.to_uint();
			if((uint)(t.tm_mday-1) > 30) return 0;
		}

		{ // year
			Text strYear = str.subarr(12, 4);
			if(!strYear.numberonly()) return 0;
			t.tm_year = strYear.to_uint();
			if(t.tm_year < 1970) return 0;
			t.tm_year -= 1900;
		}

		{ // hour
			Text strHour = str.subarr(17, 2);
			if(!strHour.numberonly()) return 0;
			t.tm_hour = strHour.to_uint();
			if((uint)t.tm_hour >= 24) return 0;
		}

		{ // min
			Text strMin = str.subarr(20, 2);
			if(!strMin.numberonly()) return 0;
			t.tm_min = strMin.to_uint();
			if((uint)t.tm_min >= 60) return 0;
		}

		{ // sec
			Text strSec = str.subarr(23, 2);
			if(!strSec.numberonly()) return 0;
			t.tm_sec = strSec.to_uint();
			if((uint)t.tm_sec >= 60) return 0;
		}

		{ // month
			Text strMonth = str.subarr(8, 3);
			for(auto & str2 : _pri_::DATETEXT<C>::month)
			{
				if(strMonth.equals((Text&)str2)) goto __findmonth;
				t.tm_mon++;
			}
			return 0;
		__findmonth:;
		}

		// week
		int wkday=0;
		Text strWeek = str.subarr(0, 3);
		for(auto & str2 : _pri_::DATETEXT<C>::wkday)
		{
			if(strWeek.equals((Text&)str2)) goto __findweek;
			wkday++;
		}
		return 0;
	__findweek:

		time_t uts = mktime(&t);
		if(t.tm_wday != wkday) return 0;
		return uts;
	}
	static time_t FromDateString(Text str) throws(InvalidSourceException, OutOfRangeException)
	{
		C fchr = *str;
		if(fchr == '-')
		{
			throw OutOfRangeException();
		}
		if(fchr == '+')
		{
			throw InvalidSourceException();
		}

#ifndef NDEBUG
		try
		{
#endif

			if('0'<= fchr && fchr <= '9')
			{
				return ReadISO8601(str);
			}
			else
			{
				return readWebDate(str);
			}

#ifndef NDEBUG
		}
		catch(OutOfRangeException &e)
		{
			udout << u"Out Of Range Date AText, " << ansiToUtf16(str);
			throw e;
		}
		catch(InvalidSourceException &e)
		{
			udout << u"Out Of Range Date AText, " << ansiToUtf16(str);
			throw e;
		}
#endif
	}
};

UnixTimeStamp::UnixTimeStamp(time_t uts) noexcept
{
	m_uts = uts;
}
UnixTimeStamp::UnixTimeStamp(filetime_t filetime) noexcept
{
#ifdef WIN32
	m_uts = (filetime - 116444736000000000LL) / 10000000;
#else
	m_uts = filetime;
#endif
}
UnixTimeStamp::UnixTimeStamp(Text strGMT) throws(InvalidSourceException, OutOfRangeException)
{
	m_uts = TimeMethods<char>::FromDateString(strGMT);
}
UnixTimeStamp::UnixTimeStamp(Text16 strGMT) throws(InvalidSourceException, OutOfRangeException)
{
	m_uts = TimeMethods<char16>::FromDateString(strGMT);
}
time_t UnixTimeStamp::getUTS() noexcept
{
	return m_uts;
}
filetime_t UnixTimeStamp::getFileTime() noexcept
{
	return m_uts * 10000000 + 116444736000000000LL;;
}
UnixTimeStamp & UnixTimeStamp::operator =(time_t uts) noexcept
{
	m_uts = uts;
	return *this;
}
UnixTimeStamp & UnixTimeStamp::operator =(Text strGMT) throws(InvalidSourceException, OutOfRangeException)
{
	m_uts = TimeMethods<char>::FromDateString(strGMT);
	return *this;
}
UnixTimeStamp & UnixTimeStamp::operator =(Text16 strGMT) throws(InvalidSourceException, OutOfRangeException)
{
	m_uts = TimeMethods<char16>::FromDateString(strGMT);
	return *this;
}
UnixTimeStamp & UnixTimeStamp::operator += (stime_t dura) noexcept
{
	m_uts += dura;
	return *this;
}
const UnixTimeStamp UnixTimeStamp::operator + (stime_t dura) const noexcept
{
	return (time_t)(m_uts + dura);
}
stime_t UnixTimeStamp::operator - (const UnixTimeStamp &other) const noexcept
{
	return m_uts - other.m_uts;
}
bool UnixTimeStamp::operator <= (UnixTimeStamp uts) const noexcept
{
	 return m_uts <= uts.m_uts;
}
bool UnixTimeStamp::operator >= (UnixTimeStamp uts) const noexcept
{
	return m_uts >= uts.m_uts;
}
bool UnixTimeStamp::operator < (UnixTimeStamp uts) const noexcept
{
	return m_uts < uts.m_uts;
}
bool UnixTimeStamp::operator > (UnixTimeStamp uts) const noexcept
{
	return m_uts > uts.m_uts;
}
bool UnixTimeStamp::operator == (UnixTimeStamp uts) const noexcept
{
	return m_uts == uts.m_uts;
}
bool UnixTimeStamp::operator != (UnixTimeStamp uts) const noexcept
{
	return m_uts != uts.m_uts;
}
TimeInformation UnixTimeStamp::getInfo() const noexcept
{
	TimeInformation t;
#ifdef WIN32
	int localtime_s_result = localtime_s(&t, &m_uts);
	_assert(localtime_s_result == 0);
#else
	t = *localtime(&m_uts);
#endif
	return t;
}
UnixTimeStamp UnixTimeStamp::now() noexcept
{
	return time(nullptr);
}
