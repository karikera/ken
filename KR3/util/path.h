#pragma once

#include <KR3/main.h>

namespace kr
{

#ifndef NO_USE_FILESYSTEM

	class CurrentDirectory final : public Bufferable<CurrentDirectory, BufferInfo<AutoComponent, method::CopyTo, true, true>>
	{
	public:
		static constexpr size_t PREPARE = 260;

		template <typename CHR>
		bool set(const CHR* text) const noexcept;
		template <typename CHR>
		size_t $copyTo(CHR* dest) const noexcept;
		template <typename CHR>
		size_t $sizeAs() const noexcept;
	};


	class CurrentApplicationPath final : public Bufferable<CurrentApplicationPath, BufferInfo<AutoComponent, method::WriteTo, true, true>>
	{
	public:
		CurrentApplicationPath() noexcept;

		template <typename _Derived, typename C, typename _Info>
		void $writeTo(OutStream<_Derived, C, _Info>* os) const throws(...)
		{
			using OS = OutStream<_Derived, C, _Info>;
			WriteLock<OS, CurrentDirectory::PREPARE> lock;
			C* dest = lock.lock(os);
			size_t size = copyTo(dest);
			lock.unlock(os, size);
		}

		template <typename CHR>
		size_t copyTo(CHR* dest) const noexcept;

	private:
		void* m_module;
	};

	static constexpr const CurrentDirectory currentDirectory = CurrentDirectory();

#endif

	class Path
	{
	public:
		constexpr static size_t MAX_LEN = 260;

		Path() noexcept;
		Text16 get(Text16 filename) throws(NotEnoughSpaceException);
		pcstr16 getsz(Text16 filename) throws(NotEnoughSpaceException);
		char16 * enter(Text16 name) throws(NotEnoughSpaceException);
		void leave(char16 * path) noexcept;
		pcstr16 getCurrentDirectorySz() noexcept;

	private:
		char16 * m_path;
		char16 m_buffer[MAX_LEN];
	};

	template <typename C>
	class path_t final
	{
	private:
#ifdef WIN32
		static const C SEPERATOR[3];
#endif
	public:
#ifdef WIN32
		static constexpr C sep = (C)'\\';
#else
		static constexpr C sep = (C)'/';
#endif

		using Text = View<C>;
		using TSZ = TempSzText<C>;

		// '/' -> true
		// '\\' -> true (if win32)
		static bool isSeperator(C chr) noexcept
		{
			switch (chr)
			{
#ifdef WIN32
			case '\\': return true;
#endif
			case '/': return true;
			default: return false;
			}
		}

		// "/dirname" -> true
		// "\\dirname" -> true (if win32)
		// "dirname" -> false
		static bool startsWithSeperator(Text text) noexcept
		{
			if (text.empty()) return false;
			return isSeperator(text.front());
		}

		// "dirname/" -> true
		// "dirname\\" -> true (if win32)
		// "dirname" -> false
		static bool endsWithSeperator(Text text) noexcept
		{
			if (text.empty()) return false;
			return isSeperator(text.back());
		}

		// "dir/name/basename.ext" -> "dir/name/"
		// "basename.ext" -> ""
		static Text dirname(Text text) noexcept
		{
			return text.cut(basename(text).data());
		}

		// "dir/name/basename.ext.name" -> "basename"
		// "dir/name/.ext.name" -> ""
		static Text filenameOnly(Text text) noexcept
		{
			Text name = basename(text);
			const C* find = text.find_e('.');
			return name.cut(find);
		}

		// "dir/name/basename.ext" -> "basename.ext"
		// "basename.ext" -> "basename.ext"
		static Text basename(Text text) noexcept
		{
			size_t pos;
#ifdef WIN32
			pos = text.pos_ry(SEPERATOR);
#else
			pos = text.pos_r(sep);
#endif
			return text.subarr(pos + 1);
		}

		// "dirname/basename.ext.name" -> ".ext.name"
		// "dirname/basename" -> ""
		// "http://dirname/basename" -> ""
		static Text extname(Text text) noexcept
		{
			Text name = basename(text);
			return name.subarr(name.find_e((C)'.'));
		}

		struct ProtocolInfo
		{
			Text protocol;
			bool isAbsolute;
			bool hasSeperator;
		};

		static ProtocolInfo getProtocolInfo(Text path) noexcept
		{
			ProtocolInfo out;
			const C* seppos;
#ifdef WIN32
			seppos = path.find_y(SEPERATOR);
#else
			seppos = path.find(sep);
#endif
			const C* protocol;
			if (seppos != nullptr)
			{
				if (seppos == path.begin())
				{
					out.protocol = nullptr;
					out.isAbsolute = true;
					return out;
				}
				else
				{
					protocol = path.cut(seppos).find(':');
				}
			}
			else
			{
				protocol = path.find(':');
			}
			if (protocol != nullptr)
			{
				out.isAbsolute = true;
				protocol++;
				if (protocol != path.end())
				{
#ifdef WIN32
					if (*protocol == '\\')
					{
						protocol++;
						out.protocol = path.cut(protocol);
						out.hasSeperator = true;
						return out;
					}
#endif
					if (*protocol == '/')
					{
						out.hasSeperator = true;
						protocol++;
						if (protocol != path.end() && *protocol == '/')
						{
							protocol++;
						}
					}
					else
					{
						out.hasSeperator = false;
					}
				}
				out.protocol = path.cut(protocol);
			}
			else
			{
				out.isAbsolute = false;
				out.protocol = nullptr;
			}
			return out;
		}
		
		template <typename T>
		static void joinEx(T * dest, View<Text> texts, bool resolve, C seper = sep) noexcept
		{
			dest->clear();
			if (texts.empty()) return;

			bool skip_sep;

			const Text* iter;
			Text text;

			const Text* begin = texts.begin();
			const Text* rend = begin - 1;
			const Text* end = texts.end();
			const Text* riter = end - 1;

			for (;riter != rend; riter--)
			{
				text = *riter;
				auto info = getProtocolInfo(text);
				if (!info.isAbsolute) continue;
				
				if (info.protocol != nullptr)
				{
					*dest << info.protocol;
#ifdef WIN32
					if (seper != '/') dest->change('/', seper);
#endif

					text.setBegin(info.protocol.end());
					iter = riter;
					skip_sep = info.hasSeperator;
					goto _find_rpath;
				}
				else
				{
					text++;
					iter = riter;
					riter--;
					goto _find_protocol;
				}
			}
			iter = begin;
			goto _find_start;
		_find_protocol:
			for (;riter != rend; riter--)
			{
				Text tx = *riter;
				auto info = getProtocolInfo(tx);
				if (info.protocol == nullptr) continue;

				*dest << info.protocol;
#ifdef WIN32
				if (seper != '/') dest->change('/', seper);
#endif
				tx.setBegin(info.protocol.end());
				skip_sep = info.hasSeperator;
				goto _find_rpath;
			}
			skip_sep = false;
			goto _find_rpath;
		_find_start:
			if (resolve)
			{
				*dest << currentDirectory;
				skip_sep = false;
			}
			else
			{
				skip_sep = true;
			}

			for (;iter != end; iter++)
			{
				text = *iter;
			_find_rpath:
				
				for (;;)
				{
#ifdef WIN32
					Text name = text.readwith_y(SEPERATOR);
#else
					Text name = text.readwith(seper);
#endif
					if (name != nullptr)
					{
						if (!name.empty())
						{
							if (*name == (C)'.')
							{
								switch (name.size())
								{
								case 1:
									continue;
								case 2: {
									if (name[1] != (C)'.') break;
									if (dest->empty())
									{
										*dest << name;
										skip_sep = false;
										continue;
									}
									if (dest->endsWith(sep))
									{
										if (dest->size() == 1)
										{
											*dest << name;
											skip_sep = false;
											continue;
										}
										dest->pop();
									}
									switch (dest->size())
									{
									case 1:
										if (dest->get(0) == '.')
										{
											*dest << (C)'.';
											skip_sep = false;
											continue;
										}
										break;
									case 2:
										if (dest->get(0) == '.' && dest->get(1) == '.')
										{
											*dest << seper;
											*dest << (C)'.';
											*dest << (C)'.';
											skip_sep = false;
											continue;
										}
										break;
									default:
										const C* end = dest->end();
										if (end[-1] == '.' && end[-2] == '.' && end[-3] == seper)
										{
											*dest << seper;
											*dest << (C)'.';
											*dest << (C)'.';
											skip_sep = false;
											continue;
										}
										break;
									}
									const C* findend;
									findend = dest->find_r(seper);
									if (findend == nullptr)
									{
										dest->clear();
										*dest << (C)'.';
										skip_sep = false;
									}
									else
									{
										dest->cut_self(findend+1);
										skip_sep = true;
									}
									continue;
								}
								default:
									break;
								}
							}
							if (skip_sep) skip_sep = false;
							else *dest << seper;
							*dest << name;
						}
					}
					else
					{
						if (!text.empty())
						{
							if (skip_sep) skip_sep = false;
							else *dest << seper;
							*dest << text;
						}
						else
						{
							if (!skip_sep)
							{
								*dest << seper;
								skip_sep = true;
							}
						}
						break;
					}
				}
			}
		}

		// ("dir", "name" ,"nextdir") -> "dir/name/nextdir"
		// ("dir", "name" ,"nextdir/") -> "dir/name/nextdir/"
		// ("dir/", "name/" ,"nextdir/") -> "dir/name/nextdir/"
		// ("/dir", "name" ,"nextdir") -> "/dir/name/nextdir"
		// ("dir", "/name" ,"nextdir") -> "/name/nextdir"
		// ("/dir", "/name" ,"nextdir") -> "/name/nextdir"
		static TSZ join(View<Text> texts, C seper = sep) noexcept
		{
			TSZ dest;
			joinEx(&dest, texts, false, seper);
			return dest;
		}

		// ("dir", "name" ,"nextdir") -> "dir/name/nextdir"
		// ("dir", "name" ,"nextdir/") -> "dir/name/nextdir/"
		// ("dir/", "name/" ,"nextdir/") -> "dir/name/nextdir/"
		// ("/dir", "name" ,"nextdir") -> "/dir/name/nextdir"
		// ("dir", "/name" ,"nextdir") -> "/name/nextdir"
		// ("/dir", "/name" ,"nextdir") -> "/name/nextdir"
		static TSZ join(Text text, C seper = sep) noexcept
		{
			TSZ dest;
			joinEx(&dest, { text }, false, seper);
			return dest;
		}

#ifndef NO_USE_FILESYSTEM
		// "dirname/../../basename.ext" -> "/absolute/path/basename.ext"
		static TSZ resolve(Text path, C seper = sep) noexcept
		{
			TSZ dest;
			joinEx(&dest, { path }, true, seper);
			return dest;
		}

		static TSZ resolve(View<Text> paths, C seper = sep) noexcept
		{
			TSZ dest;
			joinEx(&dest, paths, true, seper);
			return dest;
		}
#endif
	};

#ifdef WIN32
	template <typename C>
	const C path_t<C>::SEPERATOR[3] = { (C)'\\', (C)'/', (C)'\0' };
#endif

	extern template class path_t<char>;
	extern template class path_t<char16>;
	static constexpr const path_t<char> path = path_t<char>();
	static constexpr const path_t<char16> path16 = path_t<char16>();
}

