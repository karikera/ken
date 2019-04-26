#pragma once

#include <KR3/main.h>
#include <KRUtil/fs/path.h>
#include <KRThird/ken-res-loader/include/ken-back/image.h>
#include <KRThird/ken-res-loader/include/ken-back/sound.h>

namespace kr
{
	using WaveFormat = _krb_wave_format_t;
	namespace krb
	{
		using Extension = krb_extension_t;
		class File:public krb_file_t
		{
		public:
			File(const fchar_t* path) noexcept;
			~File() noexcept;

			template <typename Derived, typename Info>
			inline File(InStream<Derived, void, Info>* stream) noexcept
			{
				static krb_file_vtable_t vtable = {
					nullptr,
					[](krb_file_t * _this, void* data, size_t size)->size_t{
						return ((InStream<Derived, void, Info>*)(_this->param))->read(data, size);
					},
					nullptr,
					nullptr,
					[](krb_file_t * _this, uint64_t pos) {
						((InStream<Derived, void, Info>*)(_this->param))->skip(intactAuto(pos));
					},
					nullptr,
					[](krb_file_t * _this) {}
				};
				this->param = stream;
				this->vtable = &vtable;
			}

			template <typename Derived, typename Info>
			inline File(io::Streamable<Derived, Info>* stream) noexcept
				: File(stream)
			{
			}
		};

		template <typename T>
		krb_extension_t makeExtension(View<T> extension)
		{
			uint32_t v = 0;
			uint32_t offset = 0;
			for (;;)
			{
				if (extension.empty()) break;
				if (offset >= 32) return ExtensionInvalid;
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
					return ExtensionInvalid;
				}
				v |= (uint32_t)chr << offset;
				offset += 8;
			}
			return (krb_extension_t)v;
		}

		template <typename T>
		krb_extension_t makeExtensionFromPath(View<T> path)
		{
			View<T> ext = path_t<T>::extname(path);
			if (ext.empty()) return ExtensionInvalid;
			return makeExtension(ext+1);
		}

	}
}
