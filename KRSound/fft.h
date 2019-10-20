#pragma once

#include "complex.h"
#include "sound.h"

typedef struct fftwf_plan_s * fftwf_plan;

namespace kr
{
	namespace sound
	{
		class SoundCaptureSampler :private SoundCapture
		{
		public:
			SoundCaptureSampler() noexcept;

			void create(word channel, uint samplingSize, uint delta) noexcept;
			void remove() noexcept;

			const sample * read() noexcept;

		private:
			Array<byte> m_buffer;
			uint m_samplingSize;
			Locked m_locked;
			uint m_lockRead;
			uint m_delta;
			uint m_deltaLeft;
		};

		class DFT
		{
		public:
			DFT() noexcept;
			~DFT() noexcept;

			void create(uint samplingCount) noexcept;
			void remove() noexcept;

			const complex* process(const sample * input) noexcept;

		private:
			complex * m_result;
			uint m_samplingCount;

		};

		class FFT
		{
		public:
			FFT() noexcept;
			~FFT() noexcept;

			void create(uint samplingCount) noexcept;
			void remove() noexcept;

			void put(const sample* input) noexcept;
			const complex * fft() noexcept;

		private:
			byte * m_buffer;
			complex * m_trifunc;
			complex * m_result;
			complex * m_temp;
			uint m_rounds;
			uint m_samplingCount;

			void _split(complex * even, complex * odd, const complex * src, uint count) noexcept;
			void _fft(complex * src, uint count) noexcept;

		};

		class FFTWPlane
		{
		public:
			FFTWPlane() noexcept;
			~FFTWPlane() noexcept;
			void remove() noexcept;
			void zero() noexcept;
			void create_r2c_1d(uint samplingCount) noexcept;
			void create_1d(uint samplingCount) noexcept;
			void create_1d_inverse(uint samplingCount) noexcept;
			const void* execute() noexcept;
			const void* get() noexcept;
			void* getInput() noexcept;
			uint getSamplingCount() noexcept;

		protected:
			void _alloc(uint samplingCount, uint isize, uint osize) noexcept;

			fftwf_plan m_plan;
			uint8_t* m_buffer;
			void* m_in;
			void* m_out;
			uint m_samplingCount;
		};

		template <typename IT, typename OT>
		class FFTW :public FFTWPlane
		{
		public:
			void shift(size_t count) noexcept;
			void put(const void* input, int bits, size_t stride, size_t destOffset, size_t count) noexcept;
			const OT* execute() noexcept;
			const OT* scale(float scale) noexcept;
			const OT* get() noexcept;
			IT* getInput() noexcept;
			template <typename T>
			void put(const T* input, size_t stride, size_t destOffset, size_t count) noexcept;
			template <>
			void put<IT>(const IT* input, size_t stride, size_t destOffset, size_t count) noexcept
			{
				_assert(destOffset + count <= m_samplingCount);
				IT * dest = (IT*)m_in + destOffset;
				IT * destend = dest + count;
				while (dest != destend)
				{
					*dest = *input;
					(uint8_t * &)input += stride;
					dest++;
				}
			}
			template <typename T>
			void put(const T* input) noexcept;
		};
		
		class STFTCapture
		{
		public:
			STFTCapture() noexcept;
			~STFTCapture() noexcept;

			void create(uint samplingCount, uint delta) noexcept;
			void remove() noexcept;
			bool skip() noexcept;
			const complex * process() noexcept;

		private:
			SoundCaptureSampler m_sampler;
			FFTW<complex, complex> m_fft;
		};
		template <typename IT, typename OT>
		void FFTW<IT, OT>::shift(size_t count) noexcept
		{
			memmove(m_in, (IT*)m_in + count, (m_samplingCount - count) * sizeof(IT));
		}
		template <typename IT, typename OT>
		void FFTW<IT, OT>::put(const void* input, int bits, size_t stride, size_t destOffset, size_t count) noexcept
		{
			switch (bits)
			{
			case 8:
				put((uint8_t*)input, stride, destOffset, count);
				break;
			case 16:
				put((int16_t*)input, stride, destOffset, count);
				break;
			case 32:
				put((int32_t*)input, stride, destOffset, count);
				break;
			default:
				_assert(!"Unsupported");
			}
		}
		template <typename IT, typename OT>
		const OT* FFTW<IT, OT>::scale(float scale) noexcept
		{
			OT* dest = m_out;
			OT* dest_end = dest + m_samplingCount;
			while (dest != dest_end)
			{
				*dest++ *= scale;
			}
			return m_out;
		}
		template <typename IT, typename OT>
		const OT* FFTW<IT, OT>::get() noexcept
		{
			return (OT*)m_out;
		}
		template <typename IT, typename OT>
		IT* FFTW<IT, OT>::getInput() noexcept
		{
			return (IT*)m_in;
		}
		template <typename IT, typename OT>
		const OT* FFTW<IT, OT>::execute() noexcept
		{
			return (OT*)FFTWPlane::execute();
		}
		template <typename IT, typename OT>
		template <typename T>
		void FFTW<IT, OT>::put(const T* input, size_t stride, size_t destOffset, size_t count) noexcept
		{
			_assert(destOffset + count <= m_samplingCount);
			IT* dest = (IT*)m_in + destOffset;
			IT* destend = dest + count;
			while (dest != destend)
			{
				if (std::is_same_v<IT, T>)
				{
					*dest = (IT)*input;
				}
				else if (std::is_floating_point_v<T>)
				{
					*dest = (IT)((float)* input);
				}
				else
				{
					if (std::is_unsigned_v<T>)
					{
						constexpr T half = (maxof(T) / 2 + 1);
						*dest = (IT)((float)(*input - half) / (float)half);
					}
					else
					{
						*dest = (IT)((float)(*input) / (float)maxof(T));
					}
				}
				(uint8_t * &)input += stride;
				dest++;
			}
		}
		template <typename IT, typename OT>
		template <typename T>
		void FFTW<IT, OT>::put(const T* input) noexcept
		{
			put(input, sizeof(T), 0, m_samplingCount);
		}

	}
}
