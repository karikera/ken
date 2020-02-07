#pragma once

#include <KR3/main.h>
#include <KR3/util/path.h>
#include <KRThird/ken-res-loader/include/ken-back/image.h>
#include <KRThird/ken-res-loader/include/ken-back/sound.h>

namespace kr
{
	using WaveFormat = KrbWaveFormat;
	namespace krb
	{
		using Extension = KrbExtension;
		class File:public KrbFile
		{
		public:
			File(const fchar_t* path) noexcept;
			~File() noexcept;

			template <typename Derived, typename Info>
			inline File(InStream<Derived, void, Info>* stream) noexcept
			{
				static KrbFileVFTable vftable = {
					nullptr,
					[](KrbFile * _this, void* data, size_t size)->size_t{
						return ((InStream<Derived, void, Info>*)(_this->param))->read(data, size);
					},
					nullptr,
					nullptr,
					[](KrbFile* _this, uint64_t pos) {
						((InStream<Derived, void, Info>*)(_this->param))->skip(intactAuto(pos));
					},
					nullptr,
					[](KrbFile* _this) {}
				};
				this->param = stream;
				this->vftable = &vftable;
			}

			template <typename Derived, typename Info>
			inline File(io::StreamCastable<Derived, Info>* stream) noexcept
				: File(stream)
			{
			}
		};

		template <typename T>
		KrbExtension makeExtension(View<T> extension)
		{
			uint32_t v = 0;
			uint32_t offset = 0;
			for (;;)
			{
				if (extension.empty()) break;
				if (offset >= 32) return KrbExtension::Invalid;
				T chr = *extension++;
				if (u'a' <= chr && chr <= u'z')
				{
					chr += u'A' - u'a';
				}
				else if (u'A' <= chr && chr <= u'Z')
				{
				}
				else if (u'0' <= chr && chr <= u'9')
				{
				}
				else
				{
					return KrbExtension::Invalid;
				}
				v |= (uint32_t)chr << offset;
				offset += 8;
			}
			return (KrbExtension)v;
		}

		template <typename T>
		KrbExtension makeExtensionFromPath(View<T> path)
		{
			View<T> ext = path_t<T>::extname(path);
			if (ext.empty()) return KrbExtension::Invalid;
			return makeExtension(ext+1);
		}

	}
}
