#include "timerTest.h"

#include <KRMessage/pump.h>
#include <KRMessage/message.h>

void timerTest() noexcept
{
	EventPump * pump = EventPump::getInstance();

	timepoint start = timepoint::now();
	for (int i = 0; i < 10; i++)
	{
		timepoint at = start + duration(rand() % 5000);
		pump->postL(at, [&, i, at](EventPump::Timer * node) {
			dout << i << ".hello " << (at - start).value() << endl;
		});
	}
	pump->postL(start+6000_ms, [&](EventPump::Timer * node) {
		pump->quit(0);
	});

	pump->messageLoop();
}
