#pragma once

#include "../main.h"

namespace kr
{
	template <typename point_t, typename dura_t>
	class TimeType
	{
	public:
		class timepoint;

		class duration
		{
			friend timepoint;
		private:
			dura_t m_time; // ms

		public:
			inline duration()
			{
			}
			template <typename point_t2, typename dura_t2>
			explicit duration(const typename TimeType<point_t2, dura_t2>::duration &other) noexcept;
			constexpr duration(const duration & time) noexcept
				:m_time(time.m_time)
			{
			}
			constexpr explicit duration(dura_t time) noexcept
				:m_time(time)
			{
			}

			static const duration second;

			static duration fromRealTime(float realTime) noexcept;
			float getRealTime() const noexcept;
			constexpr inline dura_t value() const noexcept
			{
				return m_time;
			}
			inline duration operator -() const noexcept
			{
				return duration((dura_t)-m_time);
			}
			inline duration operator -(duration d) const noexcept
			{
				return duration(m_time - d.m_time);
			}
			inline duration operator +(duration d) const noexcept
			{
				return duration(m_time + d.m_time);
			}
			inline dura_t operator /(duration d) const noexcept
			{
				return m_time / d.m_time;
			}
			inline duration operator /(dura_t d) const noexcept
			{
				return duration(m_time / d);
			}
			inline duration operator *(dura_t mul) const noexcept
			{
				return duration(m_time * mul);
			}
			friend inline duration operator *(dura_t mul, duration dura) noexcept
			{
				return duration(mul * dura.m_time);
			}
			inline duration& operator -=(duration d) noexcept
			{
				m_time -= d.m_time;
				return *this;
			}
			inline duration& operator +=(duration d) noexcept
			{
				m_time += d.m_time;
				return *this;
			}
			inline duration& operator *=(dura_t mul) noexcept
			{
				m_time *= mul;
				return *this;
			}
			inline bool operator <(duration d) const noexcept
			{
				return m_time < d.m_time;
			}
			inline bool operator >(duration d) const noexcept
			{
				return m_time > d.m_time;
			}
			inline bool operator <=(duration d) const noexcept
			{
				return m_time <= d.m_time;
			}
			inline bool operator >=(duration d) const noexcept
			{
				return m_time >= d.m_time;
			}
			inline bool operator ==(duration d) const noexcept
			{
				return m_time == d.m_time;
			}
			inline bool operator !=(duration d) const noexcept
			{
				return m_time != d.m_time;
			}
		};

		class timepoint
		{
		private:
			point_t m_time; // ms

		protected:
			constexpr timepoint(point_t value) noexcept
				: m_time(value)
			{
			}

		public:
			static timepoint now() noexcept;
			inline timepoint() noexcept
			{
			}
			inline point_t value() const noexcept
			{
				return m_time;
			}
			inline duration operator -(timepoint p) const noexcept
			{
				return duration(m_time - p.m_time);
			}
			inline timepoint operator -(duration d) const noexcept
			{
				return timepoint(m_time - d.m_time);
			}
			inline timepoint operator +(duration d) const noexcept
			{
				return timepoint(m_time + d.m_time);
			}
			friend inline timepoint operator +(duration d, timepoint pt) noexcept
			{
				return timepoint(d.value() + pt.m_time);
			}
			inline timepoint& operator -=(duration d) noexcept
			{
				m_time -= d.m_time;
				return *this;
			}
			inline timepoint& operator +=(duration d) noexcept
			{
				m_time += d.m_time;
				return *this;
			}
			inline bool operator <(timepoint p) const noexcept
			{
				return (dura_t)(m_time - p.m_time) < 0;
			}
			inline bool operator >(timepoint p) const noexcept
			{
				return (dura_t)(m_time - p.m_time) > 0;
			}
			inline bool operator <=(timepoint p) const noexcept
			{
				return (dura_t)(m_time - p.m_time) <= 0;
			}
			inline bool operator >=(timepoint p) const noexcept
			{
				return (dura_t)(m_time - p.m_time) >= 0;
			}
			inline bool operator ==(timepoint p) const noexcept
			{
				return m_time == p.m_time;
			}
			inline bool operator !=(timepoint p) const noexcept
			{
				return m_time != p.m_time;
			}

		};

	};

	using MilliTime = TimeType<uint32_t, int32_t>;
	using DetailTime = TimeType<uint64_t, int64_t>;

	using timepoint = MilliTime::timepoint;
	using duration = MilliTime::duration;
	using timepoint_detail = DetailTime::timepoint;
	using duration_detail = DetailTime::duration;

	class TimeChecker
	{
	public:
		TimeChecker() noexcept;
		void axis() noexcept;
		float getDelta() noexcept;
		timepoint_detail getTime() noexcept;
		void initTime() noexcept;

	private:
		float m_delta;
		timepoint_detail m_time;
	};

	// push된 요소중 CASES 개수의 평균을 구해준다.
	template <size_t CASES>
	class Averager
	{
	public:
		Averager() noexcept;
		void push(float delta) noexcept;
		float getAverage() noexcept;

	private:
		float m_sum;
		float m_cases[32];
		size_t m_index;
	};

	template<size_t CASES>
	inline Averager<CASES>::Averager() noexcept
	{
		m_sum = 0.f;
		memset(m_cases, 0, sizeof(m_cases));
		m_index = 0;
	}
	template<size_t CASES>
	void Averager<CASES>::push(float delta) noexcept
	{
		m_sum -= m_cases[m_index];
		m_sum += delta;
		m_cases[m_index] = delta;
		m_index = (m_index + 1) % CASES;
	}
	template<size_t CASES>
	float Averager<CASES>::getAverage() noexcept
	{
		return m_sum / CASES;
	}

	template <> float duration_detail::getRealTime() const noexcept;
	template <> float duration::getRealTime() const noexcept;
	template <> timepoint_detail timepoint_detail::now() noexcept;
	template <> timepoint timepoint::now() noexcept;
}

inline kr::duration operator ""_ms(unsigned long long v) noexcept
{
	return kr::duration((int32_t)v);
}

inline kr::duration operator ""_s(unsigned long long v) noexcept
{
	return kr::duration((int32_t)(v * 1000));
}

inline kr::duration operator ""_min(unsigned long long v) noexcept
{
	return kr::duration((int32_t)(v * (1000 * 60)));
}

inline kr::duration operator ""_hours(unsigned long long v) noexcept
{
	return kr::duration((int32_t)(v * (1000 * 60 * 60)));
}
