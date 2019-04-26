#pragma once

#include "math.h"

namespace kr
{
	struct Interpolate
	{
		template <typename T>
		static const T bilinear(const T * arr, int W, int H, int _pitch, float x, float y) noexcept
		{
			float tx = fmodf(x, (float)W);
			float ty = fmodf(y, (float)H);

			int nx = math::suspectedFloor(tx);
			int ny = math::suspectedFloor(ty);

			float offx = tx - nx;
			float offy = ty - ny;
			float invx = 1 - offx;
			float invy = 1 - offy;

			int nx2 = (nx + 1) % _pitch;
			int ny2 = ((ny + 1) % H) * _pitch;
			ny *= _pitch;

			return (T)((arr[nx + ny] * invx + arr[nx2 + ny] * offx) * invy +
				(arr[nx + ny2] * invx + arr[nx2 + ny2] * offx) * offy);
		};
		template <typename T>
		static const T coslinear(const T * arr, int W, int H, int _pitch, float x, float y) noexcept
		{
			float tx = fmodf(x, (float)W);
			float ty = fmodf(y, (float)H);

			int nx = math::suspectedFloor(tx);
			int ny = math::suspectedFloor(ty);

			float invx = math::cos((tx - nx) * math::pi) / 2 + 0.5f;
			float invy = math::cos((ty - ny) * math::pi) / 2 + 0.5f;
			float offx = 1 - invx;
			float offy = 1 - invy;

			int nx2 = (nx + 1) % _pitch;
			int ny2 = ((ny + 1) % H) * _pitch;
			ny *= _pitch;

			return (T)((arr[nx + ny] * invx + arr[nx2 + ny] * offx) * invy +
				(arr[nx + ny2] * invx + arr[nx2 + ny2] * offx) * offy);
		}
		template <typename T, uintptr_t W, uintptr_t H> 
		static const T bilinear(const T(&arr)[H][W], float x, float y) noexcept
		{
			return bilinear(arr[0], W, H, W, x, y);
		};
		template <typename T, uintptr_t W, uintptr_t H>
		static const T coslinear(const T(&arr)[H][W], float x, float y) noexcept
		{
			return coslinear(arr[0], W, H, W, x, y);
		};
		template <uintptr_t W, uintptr_t H>
		static float noise(float fAmplitude, float fFrequency, uint repeat, const float(&arr)[H][W], float x, float y) noexcept
		{
			float amplitude = 1;
			float frequency = 1;
			float fSum = bilinear(arr, x, y);
			for (uint i = 1; i<repeat; i++)
			{
				amplitude *= fAmplitude;
				frequency *= fFrequency;
				fSum += coslinear(arr, x * frequency, y * frequency, x, y) * amplitude;
			}
			return fSum;
		}
	};
}
