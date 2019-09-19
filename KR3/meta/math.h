#pragma once

#include "boolidx.h"

namespace kr
{
	namespace meta
	{
		template <typename T> constexpr T pow(T a, unsigned int b)
		{
			return b == 0 ? 1 : pow(a*a, b>>1)*((b & 1) ? a : 1);
		}

		template <typename T>
		constexpr T mint(T v) noexcept
		{
			return v;
		}
		template <typename T, typename T2, typename ... ARGS>
		constexpr T mint(T v, T2 v2, ARGS ... vs) noexcept
		{
			return (T)mint(v2, vs ...) < v ? (T)mint(v2, vs ...) : v;
		}

		template <typename T>
		constexpr T maxt(T v) noexcept
		{
			return v;
		}
		template <typename T, typename T2, typename ... ARGS>
		constexpr T maxt(T v, T2 v2, ARGS ... vs) noexcept
		{
			return (T)maxt(v2, vs ...) > v ? (T)maxt(v2, vs ...) : v;
		}

		/// sqrt
		namespace _pri_
		{
			template <int n, int res, bool end> struct isqrt_impl;
			template <int n, int res> struct isqrt_impl<n, res, true>:isqrt_impl<n-res, res+2, n >= 0>
			{
			};;
			template <int n, int res> struct isqrt_impl<n, res, false>
			{
				constexpr static size_t value = (res-4) >> 1;
			};;
			template <int n, int offset> struct isqrt_offset:isqrt_impl<n-offset*offset, 1+offset*2, true>
			{
			};;

			template <int n, size_t index> struct isqrt_switch:isqrt_offset<n, 1200 - index*60>
			{
			};
			template <int n> struct isqrt_switch<n, 20>:isqrt_impl<n, 1, true>{};;
		}

		template <int n> struct isqrt: _pri_::isqrt_switch<n, bool_index<
			(n > 1200*1200), (n > 1140*1140), (n > 1080*1080), (n > 1020*1020), (n > 990*990), (n > 930*930),
			(n > 900*900), (n > 840*840), (n > 780*780), (n > 720*720), (n > 690*690), (n > 630*630),
			(n > 600*600), (n > 540*540), (n > 480*480), (n > 420*420), (n > 390*390), (n > 330*330),
			(n > 300*300), (n > 240*240), (n > 180*180), (n > 160*160), (n > 120*120), (n > 60*60),
			true>::value>
		{
		};;
	}
}
