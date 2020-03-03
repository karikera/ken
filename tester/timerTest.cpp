#include "timerTest.h"

#include <KR3/msg/pump.h>

void timerTest() noexcept
{
	EventPump * pump = EventPump::getInstance();

	timepoint start = timepoint::now();
	for (int i = 0; i < 10; i++)
	{
		timepoint at = start + duration(rand() % 5000);
		pump->post(at, [&, i, at](TimerEvent * node) {
			dout << i << ".hello " << (at - start).value() << endl;
		});
	}
	pump->post(start+6000_ms, [&](TimerEvent * node) {
		pump->quit(0);
	});

	pump->messageLoop();
}
