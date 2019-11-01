#pragma once

#include <KR3/mt/thread.h>
#include <KR3/mt/criticalsection.h>
#include <KRWin/winx.h>
#include <KR3/msg/promise.h>
#include <KR3/msg/pool.h>

namespace kr
{
	class MCISound;

	class MCISystem :private WndProcedure, public EventThread
	{
	public:
		MCISystem(WindowProgram * window) noexcept;
		~MCISystem() noexcept;
		void wndProc(win::Window* pWindow, uint Msg, WPARAM wParam, LPARAM lParam) override;

	private:
		WindowProgram * m_window;
	};
	
	class MCISound
	{
	public:
		MCISound() noexcept;
		~MCISound() noexcept;

		MCISound(const MCISound&) = delete;

		void play() noexcept;
		Promise<void> * playAnd() noexcept;
		Promise<void> * open(AText16 filename) noexcept;
		void close() noexcept;
		uintptr_t duration() noexcept;

	private:
		uint m_device;
		uintptr_t m_duration;
	};
}
