#include <KR3/main.h>
#include <KR3/io/selfbufferedstream.h>
#include <KR3/io/bufferedstream.h>
#include <KR3/data/switch.h>
#include <KR3/data/map.h>
#include <KR3/fs/file.h>
#include <KR3/wl/windows.h>

#include <unordered_map>
#include <iostream>

constexpr char16 EXTRA_COMMAND = u'\x11';

using namespace kr;

class MyFOStream;

struct Column
{
	MyFOStream * output;
};
struct StringTableInfo
{
	AText name;
	Array<AText> identities;
};
enum class IdentityMode
{
	disable,
	text,
	number,
	enumerator
};
class MyFOStream :public OutStream<MyFOStream, char, StreamInfo<false, io::FOStream<char, false>>>
{
	using Super = OutStream<MyFOStream, char, StreamInfo<false, io::FOStream<char, false>>>;
public:
	MyFOStream() noexcept;
	MyFOStream(Text16 fn);
	~MyFOStream() noexcept;
	void $write(const char * str, size_t sz) noexcept;

	void flush() noexcept;
	void open(Text16 filename);
	void close() noexcept;
	void writeUtf8(char16 uc) noexcept;
	void writeWord(Text16 tx) noexcept;
	void writeText(Text16 tx) noexcept;
};
struct IdentityInfo
{
	StringTableInfo * table;
	size_t nameIdx;
};
class Main
{
public:
	Main() noexcept;
	~Main() noexcept;
	int main(int argn, const wchar_t ** args) noexcept;
	void flushVar() noexcept;
	void flushSource() noexcept;
	Text getColumn(Text & line) throws(EofException);
	void warning(Text16 message) noexcept;
	File * createFile(Text16 text) throws(Error);
	void modifyTest(Text16 filename) noexcept;


	Text16 filename;
	int lineNumber;
	MyFOStream src_h;
	AText gvarname;
	Array<StringTableInfo*> tables;
	Array<Column> columns;

	StringTableInfo * curtable;

	std::unordered_map<AText, IdentityInfo> identityInfo;
	size_t regressionIndex;
	bool regressed = false;

	AText identityParam;
	filetime_t srcModifiedTime;

	IdentityMode identity = IdentityMode::disable;

	enum class FileState
	{
		Writed,
		Skipped,
	};

	std::unordered_map<char16, AText16> s_extraCommand;
	std::unordered_map <AText16, FileState> s_usedFile;
};

Main * g_main;

int CT_CDECL wmain(int argn, const wchar_t ** args)
{
	{
		Main main;
		g_main = &main;
		main.main(argn, args);
	}
	return 0;
}

MyFOStream::MyFOStream() noexcept
	: Super(nullptr)
{
}
MyFOStream::MyFOStream(Text16 fn)
	: Super(g_main->createFile(fn))
{
}
MyFOStream::~MyFOStream() noexcept
{
	close();
}

void MyFOStream::$write(const char * str, size_t sz) noexcept
{
	if (base() == nullptr)
		return;
	return Super::$write(str, sz);
}
void MyFOStream::flush() noexcept
{
	if (base() == nullptr)
		return;
	Super::flush();
}
void MyFOStream::open(Text16 filename)
{
	close();
	resetStream(g_main->createFile(filename));
}
void MyFOStream::close() noexcept
{
	File * file = base();
	if (file == nullptr)
		return;
	delete file;
	resetStream(nullptr);
	file = nullptr;
}
void MyFOStream::writeUtf8(char16 uc) noexcept
{
	if (uc <= 0x7f)
	{
		write((char)uc);
	}
	else if (uc <= 0x7ff)
	{
		write((char)(0xc0 + uc / (0x01 << 6)));
		write((char)(0x80 + uc % (0x01 << 6)));
	}
	else if (uc <= 0xffff)
	{
		write((char)(0xe0 + uc / (0x01 << 12)));
		write((char)(0x80 + uc / (0x01 << 6) % (0x01 << 6)));
		write((char)(0x80 + uc % (0x01 << 6)));
	}
}
void MyFOStream::writeWord(Text16 tx) noexcept
{
	try
	{
		for (;;)
		{
			char16 uc = tx.read();
			if (uc != '\\')
			{
				writeUtf8(uc);
				continue;
			}

			uc = tx.read();
			auto iter = g_main->s_extraCommand.find(uc);
			if (iter != g_main->s_extraCommand.end())
			{
				writeWord(iter->second);
				continue;
			}

			if (uc == 'c')
			{
				Text16 number = tx.readto_eL([](char16 chr) { return ('0' > chr || chr > '9') && ('a' > chr || chr > 'f') && ('A' > chr || chr > 'F'); });

				uint num = number.to_uint(16);
				if (num >= 30000) // keruch: UTF8로 변환시, 2바이트를 전부 사용해도 되는지 잘 모르겠다.
				{
					g_main->warning(u"Too large extra value");
					continue;
				}
				if (num == 0)
				{
					g_main->warning(u"Cannot use null character");
					num = 1;
				}

				writeUtf8(EXTRA_COMMAND);
				writeUtf8((char16)num);
				continue;
			}

			g_main->warning(u"Unknown escape character");
			uc = '?';
		}
	}
	catch (EofException&)
	{
	}
}
void MyFOStream::writeText(Text16 tx) noexcept
{
	writeWord(tx);
	write('\0');
}

Main::Main() noexcept
{
	curtable = nullptr;
	lineNumber = 0;
}
Main::~Main() noexcept
{
	delete curtable;
}
int Main::main(int argn, const wchar_t ** args) noexcept
{
	setlocale(LC_ALL, nullptr);

	if (argn < 2)
	{
		ucerr << u"Need file path parameter.\n";
		return EINVAL;
	}

	s_extraCommand[u'/'] = u"/";
	s_extraCommand[u'r'] = u"\r";
	s_extraCommand[u'n'] = u"\n";
	s_extraCommand[u'\\'] = u"\\";
	s_extraCommand[u't'] = u"\t";

	curtable = _new StringTableInfo;

	try
	{
		pcstr16 * args16 = unwide(args);
		filename = (Text16)args16[1];
		io::FIStream<char> fis = File::open(args16[1]);

		srcModifiedTime = fis->getLastModifiedTime();

		for(;;)
		{
			Text line = fis.readLine();
			lineNumber++;

			// remove comment
			{
				pcstr comment = line.find("//");
				if (comment != nullptr) line.cut_self(comment);
			}

			// skip empty line
			for (char chr : line)
			{
				if (chr == ' ') continue;
				if (chr == '\t') continue;
				goto _no_empty_line;
			}
			continue;
		_no_empty_line:

			try
			{
				Text order = getColumn(line);
				if (!order.empty() && order[0] == '#')
				{
					// 특수 커맨드 처리
					order++;

					static TextSwitch txswitch = {
						"header",
						"table",
						"identity",
						"output",
						"define_extra",
					};

					switch (txswitch[order])
					{
					case 0: // header
						try
						{
							flushSource();

							src_h.open(TSZ16() << (Utf8ToUtf16)getColumn(line));
							gvarname.copy(getColumn(line));
							src_h << "#pragma once\r\n";
						}
						catch (Error&)
						{
							warning(u"Cannot create file");
						}
						break;
					case 1: // table
						try
						{
							flushVar();
							curtable->name.copy(getColumn(line));
						}
						catch (Error&)
						{
							warning(u"Cannot create file");
						}
						break;
					case 2: // identity
						order = getColumn(line);
						if (order == "disable") identity = IdentityMode::disable;
						else if (order == "text") identity = IdentityMode::text;
						else if (order == "number") identity = IdentityMode::number;
						else if (order == "enum")
						{
							identity = IdentityMode::enumerator;
							identityParam.clear();
							identityParam << getColumn(line);
						}
						else warning(u"Must set disable or text or number");
						break;
					case 3: // output
						regressionIndex = 0;

						for (Column& column : columns)
						{
							column.output->flush();
							delete column.output;
						}

						try
						{
							columns.clear();
							for (;;)
							{
								MyFOStream * output = _new MyFOStream(TSZ16() << (Utf8ToUtf16)getColumn(line));
								Column * column = columns.prepare(1);
								column->output = output;
							}
						}
						catch (Error&)
						{
							warning(u"Cannot create file");
						}
						catch (EofException&)
						{
						}
						break;
					case 4: // define_extra
					{
						Text from = getColumn(line);
						if (from.empty())
						{
							warning(u"empty parameter");
							break;
						}
						if (from[0] != '\\')
						{
							warning(u"define_extra parameters must started with \\");
						}
						else
						{
							from++;
						}
						if (from.empty())
						{
							warning(u"empty parameter");
							break;
						}
						if (from.size() != 1)
						{
							warning(u"define_extra parameter must be one character");
						}
						Text to = getColumn(line);
						if (to.empty())
						{
							warning(u"ignored: too short extra command");
							break;
						}
						s_extraCommand[*from] = (Utf8ToUtf16)to;
						break;
					}
					default:
						warning(u"Invalid command");
						break;
					}
					continue;
				}
				else
				{
					// 일반 텍스트 처리
					if (regressed)
					{
					_useRegression:
						if (regressionIndex >= curtable->identities.size())
						{
							warning(u"Regression overflow");
						}
						else
						{
							Text oldname = curtable->identities[regressionIndex];
							Text newname = order;
							if (oldname != newname)
							{
								if (oldname.empty())
								{
									warning(TSZ16() << u"Regression name unmatch (previous: empty)");
								}
								else
								{
									warning(TSZ16() << u"Regression name unmatch (previous: \""
										<< (Utf8ToUtf16)oldname << u"\")");
								}
							}
							regressionIndex++;
						}
					}
					else
					{
						if (order.empty())
						{
							curtable->identities.push(nullptr);
						}
						else
						{
							AText aorder = order;
							IdentityInfo info;
							info.table = curtable;

							info.nameIdx = curtable->identities.size();
							auto res = identityInfo.insert({ aorder, info });
							if (res.second)
							{
								curtable->identities.push(move(aorder));
							}
							else
							{
								IdentityInfo & info = res.first->second;
								if (identity == IdentityMode::text)
								{
									warning(u"Already used identifier");
									curtable->identities.push(nullptr);
								}
								else if (info.table == curtable)
								{
									warning(u"Already used identifier");
									curtable->identities.push(nullptr);
								}
								else
								{
									if (!curtable->identities.empty())
									{
										warning(u"Something already defined in regression table");
									}
									curtable->identities = info.table->identities;
									regressionIndex = info.nameIdx;
									regressed = true;
									goto _useRegression;
								}
							}
						}
					}

					Column * fend = columns.end();
					Column * fiter = columns.begin();
					TSZ16 firstcolumn;

					try
					{
						if (fiter != fend)
						{
							Text column = getColumn(line);
							firstcolumn << (Utf8ToUtf16)column;
							fiter->output->writeText(firstcolumn);
							fiter++;
							for (; fiter != fend; fiter++)
							{
								fiter->output->writeText(TSZ16() << (Utf8ToUtf16)getColumn(line));
							}
						}

						try
						{
							getColumn(line);
							warning(u"Too many parameter");
						}
						catch (EofException&)
						{
						}
					}
					catch (EofException&)
					{
						do
						{
							fiter->output->writeText(firstcolumn);
							fiter++;
						} while (fiter != fend);
					}
				}
			}
			catch (EofException&)
			{
				warning(u"Too few parameter");
			}
		}
	}
	catch (EofException &)
	{
		flushSource();
		for (Column & column : columns)
		{
			column.output->flush();
			delete column.output;
		}

		src_h.close();
		for (auto files : s_usedFile)
		{
			if (files.second == FileState::Skipped)
				continue;
			modifyTest(files.first);
		}
		File::saveFromArray(TSZ16() << filename << u".log", (kr::Text)"succeeded");
		return 0;
	}
	catch (Error &)
	{
		warning(u"File path problem");
		return EBADF;
	}
}
void Main::flushVar() noexcept
{
	if (curtable->identities.empty()) 
		return;

	if (regressed)
	{
		if(regressionIndex != curtable->identities.size())
			warning(u"Not enough regression text");
		regressed = false;
	}
	else
	{
		size_t counter = 0;
		switch (identity)
		{
		case IdentityMode::disable:
			break;

		case IdentityMode::number:
			src_h << "\r\n";
			for (Text id : curtable->identities)
			{
				if (!id.empty())
				{
					src_h << "constexpr int " << id << " = " << counter << ";\r\n";
				}
				counter++;
			}
			break;

		case IdentityMode::text:
			src_h << "\r\n";
			for (Text id : curtable->identities)
			{
				if (!id.empty())
				{
					src_h << "#define " << id << " (" << gvarname << '.' << curtable->name << '[' << counter << "])\r\n";
				}
				counter++;
			}
			break;

		case IdentityMode::enumerator:
			src_h << "\r\nenum " << identityParam << "\r\n"
				"{\r\n";
			bool skipped = false;
			for (Text id : curtable->identities)
			{
				if (!id.empty())
				{
					if (skipped)
					{
						src_h << "\t " << id << " = " << counter << ",\r\n";
						skipped = false;
					}
					else
					{
						src_h << "\t " << id << ",\r\n";
					}
				}
				else
				{
					skipped = true;
				}
				counter++;
			}
			src_h << "};\r\n";
			src_h << "constexpr size_t " << identityParam << "Count = " << curtable->identities.size() << ";\r\n";
			break;
		}
	}

	tables.push(curtable);
	curtable = _new StringTableInfo;
}
void Main::flushSource() noexcept
{
	flushVar();
	if (tables.empty()) 
		return;
	
	src_h <<
		"\r\nstruct " << gvarname << "_t:StringTableData\r\n"
		"{\r\n";
	for (StringTableInfo * tbl : tables)
	{
		src_h << "\tStringTable<" << tbl->identities.size() << "> " << tbl->name << ";\r\n";
	}
	src_h << "\tinline void load(kr::Text texts) noexcept\r\n"
		"\t{\r\n"
		"\t\tStringTableData::load(texts);\r\n"
		"\t\tread(&";
	bool first = true;
	for (StringTableInfo * tbl : tables)
	{
		if (first) first = false;
		else src_h << ", &";
		src_h << tbl->name;
	}
	src_h <<
		");\r\n"
		"\t}\r\n";
	src_h <<
		"};\r\n"
		"\r\n"
		"\r\nextern " << gvarname << "_t " << gvarname << ";\r\n";

	for (StringTableInfo * info : tables)
	{
		delete info;
	}
	tables.clear();
	src_h.flush();
}
Text Main::getColumn(Text & line) throws(EofException)
{
	if (line.empty())
		throw EofException();
	pcstr next = line.find('\t');
	if (next == nullptr)
	{
		Text out = line;
		line = line.endIndex();
		return out;
	}
	Text out = line.readto_p(next);
	line = line.readto_n('\t');
	return out;
}
void Main::warning(Text16 message) noexcept
{
	ucerr << filename << u"(" << lineNumber << u"): warning: " << message << endl;
}
File * Main::createFile(Text16 text) throws(Error)
{
	if (text == u"null")
		return nullptr;

	TSZ16 ttext;
	ttext << text;

	try
	{
		File * file;
		auto iter = s_usedFile.insert({ text, FileState::Skipped });
		if (iter.second)
		{
			filetime_t passedTime;
			try
			{
				filetime_t old = File::getLastModifiedTime(ttext);
				if (old >= srcModifiedTime)
				{
					return nullptr;
				}

				passedTime = (srcModifiedTime - old);
			}
			catch (Error&)
			{
				passedTime = 0;
			}
			iter.first->second = FileState::Writed;
			ucout << ttext << u"\n";

			pcstr16 folderCut = ttext.find_r(u'\\');
			if (folderCut != nullptr)
				File::createFullDirectory(ttext.cut(folderCut));

			file = File::create(ttext << u".new");
		}
		else
		{
			switch (iter.first->second)
			{
			case FileState::Writed:
				file = File::openWrite(ttext << u".new");
				file->movePointerToEnd(0);
				break;
			case FileState::Skipped:
				return nullptr;
			default:
				return nullptr;
			}
		}

		return file;
	}
	catch (Error&)
	{
		ucout << ttext << u": Generate  failed\n";
		throw;
	}
}
void Main::modifyTest(Text16 filename) noexcept
{
	constexpr size_t SIZE = 8192;
	TmpArray<char> buffer1(SIZE);
	TmpArray<char> buffer2(SIZE);

	TSZ16 oldfile;
	oldfile << filename << nullterm;

	TSZ16 newfile;
	newfile << filename << u".new" << nullterm;
	try
	{
		Must<File> file1 = File::open(oldfile);
		Must<File> file2 = File::open(newfile);
		if (file1->size() != file2->size())
			goto _change;

		try
		{
			for (;;)
			{
				size_t sz1 = file1->$read(buffer1.begin(), SIZE);
				size_t sz2 = file2->$read(buffer2.begin(), SIZE);
				if (sz1 != sz2)
					goto _change;
				if (memcmp(buffer1.begin(), buffer2.begin(), sz1) != 0)
					goto _change;
			}
		}
		catch (EofException&)
		{
		}
	}
	catch (Error&)
	{
		goto _change;
	}

	File::remove(newfile);
	ucout << oldfile << u": Nothing changed\n";
	return;
_change:
	File::remove(oldfile);
	if (!File::move(oldfile, newfile))
		warning(TSZ16() << oldfile << u": Cannot delete file");
}
