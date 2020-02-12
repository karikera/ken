#include <KR3/main.h>
#include <KR3/util/parameter.h>
#include <KR3/http/fetch.h>
#include <KR3/msg/pump.h>

using namespace kr;


EventPump* pump;

int CT_CDECL wmain(int argn, char16** args)
{
	if (argn < 3)
	{
		cerr << "Need more parameters." << endl;
		return EINVAL;
	}
	
	pump = EventPump::getInstance();

	setlocale(LC_ALL, nullptr);

	Text16 src = (Text16)args[1];
	Text16 dest = (Text16)args[2];
	fetchAsFile(src, dest)->then([] {
		PostQuitMessage(0);
	});

	return pump->messageLoop();
}