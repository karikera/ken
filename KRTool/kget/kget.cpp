#include <KR3/main.h>
#include <KR3/util/parameter.h>
#include <KR3/http/fetch.h>
#include <KR3/msg/pump.h>

using namespace kr;

EventPump* pump = EventPump::getInstance();
bool setQuit;

int CT_CDECL wmain(int argn, char16** args)
{
	if (argn < 3)
	{
		cerr << "Need more parameters." << endl;
		return EINVAL;
	}
	
	setlocale(LC_ALL, nullptr);

	Text16 src = (Text16)args[1];
	Text16 dest = (Text16)args[2];
	AtomicProgress prog;

	cout << "download: " << toAnsi(src) << endl;

	fetchAsFile(src, dest, &prog)->then([] {
		PostQuitMessage(0);
		setQuit = true;
	});

	
	pump->post(1_s, [&](TimerEvent* timer){
		if (setQuit) return;

		if (prog.downloadTotal != 0)
		{
			uint downloadPercent = (int)((prog.downloadNow * 2000 / prog.downloadTotal + 1) / 2);
			uint decv = downloadPercent % 10;
			uint intv = downloadPercent / 10;
			cout << decf(intv, 3, ' ') << '.' << decv << '%' << '\r';
		}
		timer->addTime(1_s);
		pump->attach(timer);
		});

	return pump->messageLoop();
}