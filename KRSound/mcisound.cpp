#include "stdafx.h"
#include "mcisound.h"

#include <KR3/util/wide.h>
#include <KR3/util/callable.h>
#include <KR3/data/map.h>

#include <mmsystem.h>
#include <Digitalv.h>

#pragma comment(lib, "Winmm.lib")


using namespace kr;

class MCIPromise : public Promise<void>
{
public:
	void resolve() noexcept
	{
		_resolve();
	}
};

namespace
{
	Map<MCIDEVICEID, MCIPromise*> s_prommap;
	MCISystem * s_system;
}

MCISystem::MCISystem(WindowProgram * window) noexcept
	:m_window(window)
{
	window->procedures.attach(this);
	s_system = this;
}
MCISystem::~MCISystem() noexcept
{
	m_window->procedures.detach(this);
	quit(0);
}
void MCISystem::wndProc(win::Window* pWindow, uint Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case MM_MCINOTIFY:
	{
		WPARAM flags = wParam;
		MCIDEVICEID device = (MCIDEVICEID)lParam;
		auto iter = s_prommap.find(device);
		if (iter != s_prommap.end())
		{
			MCIPromise * prom = iter->second;
			s_prommap.erase(iter);
			prom->resolve();
		}
		break;
	}
	}
}

MCISound::MCISound() noexcept
{
	m_device = 0;
	m_duration = 0;
}
MCISound::~MCISound() noexcept
{
	close();
}
void MCISound::play() noexcept
{
	if (m_device == 0) return;
	(*s_system)->post([this](void*){
		MCI_PLAY_PARMS pp;
		pp.dwCallback = 0;
		pp.dwFrom = 0;
		pp.dwTo = 0;
		mciSendCommandW(m_device, MCI_PLAY, MCI_FROM | MCI_NOTIFY, (DWORD_PTR)&pp);
	});
}
Promise<void>* MCISound::playAnd() noexcept
{
	if (m_device == 0) return Promise<void>::resolve();

	auto res = s_prommap.insert({ m_device, nullptr });
	if (!res.second) return nullptr;
	MCIPromise * prom = res.first->second = _new MCIPromise();

	uint deviceId = m_device;
	(*s_system)->post([deviceId](void*) {
		MCI_PLAY_PARMS pp;
		pp.dwCallback = (DWORD_PTR)win::g_mainWindow;
		pp.dwFrom = 0;
		mciSendCommandW(deviceId, MCI_PLAY, MCI_NOTIFY | MCI_FROM, (DWORD_PTR)&pp);
	});
	return prom;
}
Promise<void> * MCISound::open(AText16 filename) noexcept
{
	close();

	MCIPromise * prom = _new MCIPromise;
	EventPump * pump = EventPump::getInstance();

	(*s_system)->post([this, prom, pump = (Must<EventPump>)pump, filename = move(filename)](void*) mutable{
		{
			MCI_OPEN_PARMSW op;
			op.dwCallback = 0;
			op.wDeviceID = 0;
			op.lpstrDeviceType = L"mpegvideo";// (LPWSTR)MCI_ALL_DEVICE_ID;
			op.lpstrElementName = wide(filename.c_str());
			TText16 alias = hexf((uintptr_t)this, sizeof(uintptr_t)*2);
			op.lpstrAlias = wide(alias.c_str());

			MCIERROR err = mciSendCommandW(NULL, MCI_OPEN,
				MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID | MCI_OPEN_TYPE | MCI_WAIT, (DWORD_PTR)&op);
			if (err)
			{
				throw MciError(err);
			}
			m_device = op.wDeviceID;
		}

		{
			MCI_STATUS_PARMS sp;
			sp.dwCallback = 0;
			sp.dwReturn = 0;
			sp.dwItem = MCI_STATUS_LENGTH;
			sp.dwTrack = 0;
			MCIERROR err = mciSendCommandW(m_device, MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM, (DWORD_PTR)&sp);
			if (err)
			{
				throw MciError(err);
			}
			m_duration = sp.dwReturn;
		}

		pump->post([prom](void*){
			prom->resolve();
		});
	});

	return prom;
}
void MCISound::close() noexcept
{
	if (m_device == 0) return;
	uint deviceId = m_device;
	m_device = 0;

	(*s_system)->post([deviceId](void*){
		mciSendCommandW(deviceId, MCI_CLOSE, 0, (DWORD_PTR)nullptr);
	});
}
uintptr_t MCISound::duration() noexcept
{
	return m_duration;
}
