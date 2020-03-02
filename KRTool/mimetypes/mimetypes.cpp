
#include <KR3/main.h>
#include <KR3/http/fetch.h>
#include <KR3/msg/pump.h>
#include <KR3/io/bufferedstream.h>
#include <KR3/fs/file.h>

using namespace kr;

EventPump* pump = EventPump::getInstance();

int main()
{
	fetchAsTextFromWeb("https://gist.githubusercontent.com/mahizsas/5999837/raw/3fade8d1cf8d02581ff92c0e0a3c001582352cd2/mimetypes")->then([](Text data){

		File::createDirectory(u"gen");
		io::FOStream<char> fout = File::create(u"gen/mime.h");

		fout << "#include <KR3/main.h>\r\n"
			"#include <KR3/data/map.h>\r\n"
			"namespace kr{\r\n"
			"namespace gen{\r\n"
			"void initMimeMap(Map<Text, AText> * map) noexcept\r\n"
			"{\r\n";
		
		for (Text line : data.splitIterable('\n'))
		{
			line = line.readto_e('#').trim();
			if (line.empty()) continue;

			Text mime = line.readwith_ye(Text::WHITE_SPACE);
			line.skipspace();
			while (!line.empty())
			{
				Text ext = line.readwith_ye(Text::WHITE_SPACE);
				line.skipspace();
				fout << "\t(*map)[\"" << ext << "\"] = \"" << mime << "\";\r\n";
			}
		}
		fout << "}\r\n"
			"}\r\n"
			"}\r\n";

		pump->quit(0);
		})->katch([](exception_ptr&){
			cerr << "unknown error" << endl;
			pump->quit(-1);
			});

	return pump->messageLoop();
}