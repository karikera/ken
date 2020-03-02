#pragma once

#include <ctime>
#include <KR3/main.h>

namespace kr
{
	using filetime_t = uint64_t;

	namespace _pri_
	{
		
		template <typename C> struct DATETEXT
		{
			static View<C> wkday[];
			static View<C> month[];
		};

	}

	class TimeInformation :public Printable<TimeInformation, AutoComponent, std::tm>
	{
	public:

		template <class _Derived, typename C, class _Info>
		void writeTo(OutStream<_Derived, C, _Info> *str) const throws(NotEnoughSpaceException)
		{
			// Web Standard "wkd, DD MMM YYYY hh:mm:ss GMT"
			*str << _pri_::DATETEXT<C>::wkday[tm_wday] << (C)',' << (C)' '
				<< decf(tm_mday, 2) << (C)' '
				<< _pri_::DATETEXT<C>::month[tm_mon] << (C)' '
				<< decf(tm_year + 1900, 4) << (C)' '
				<< decf(tm_hour, 2) << (C)':'
				<< decf(tm_min, 2) << (C)':'
				<< decf(tm_sec, 2) << (C)' ' << (C)'G' << (C)'M' << (C)'T';
		}

		TimeInformation & operator = (const std::tm & t) noexcept
		{
			*static_cast<std::tm*>(this) = t;
			return *this;
		}

	};

	using stime_t = std::make_signed_t<time_t>;

	class UnixTimeStamp
	{
	public:
		UnixTimeStamp() = default;
		UnixTimeStamp(time_t uts) noexcept;
		UnixTimeStamp(filetime_t filetime) noexcept;
		UnixTimeStamp(Text strGMT) throws(InvalidSourceException, OutOfRangeException);
		UnixTimeStamp(Text16 strGMT) throws(InvalidSourceException, OutOfRangeException);
		time_t getUTS() noexcept;
		filetime_t getFileTime() noexcept;
		UnixTimeStamp & operator =(time_t uts) noexcept;
		UnixTimeStamp & operator =(Text strGMT) throws(InvalidSourceException, OutOfRangeException);
		UnixTimeStamp & operator =(Text16 strGMT) throws(InvalidSourceException, OutOfRangeException);

		UnixTimeStamp & operator += (stime_t dura) noexcept;
		const UnixTimeStamp operator + (stime_t dura) const noexcept;
		stime_t operator - (const UnixTimeStamp &other) const noexcept;

		bool operator <= (UnixTimeStamp uts) const noexcept;
		bool operator >= (UnixTimeStamp uts) const noexcept;
		bool operator < (UnixTimeStamp uts) const noexcept;
		bool operator > (UnixTimeStamp uts) const noexcept;
		bool operator == (UnixTimeStamp uts) const noexcept;
		bool operator != (UnixTimeStamp uts) const noexcept;

		TimeInformation getInfo() const noexcept;
		template <typename C>
		void copyTo(C* dest) const noexcept;
		size_t size() const noexcept;

		static struct TIMEZONE
		{
			char cSign;
			byte nHour;
			byte nMin;

			TIMEZONE() noexcept;
		} timeZone;

		static UnixTimeStamp now() noexcept;
		
	private:
		time_t m_uts;
		
		
		template <typename C> struct TimeMethods;

	};

}