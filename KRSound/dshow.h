#pragma once

#include <KRMessage/promise.h>
#include <KR3/wl/com.h>
#include <KRMessage/eventdispatcher.h>

struct IGraphBuilder;
struct IMediaControl;
struct IBasicAudio;
struct IMediaEvent;
struct ISampleGrabber;

namespace kr
{
	class DShowFilterGraph
	{
	public:
		DShowFilterGraph() noexcept;
		~DShowFilterGraph() noexcept;

	protected:
		bool _renderFile(pcstr16 path) noexcept;

		Com<IGraphBuilder> m_graph;
	};
	class DShowSound:public DShowFilterGraph
	{
	public:
		DShowSound() noexcept;
		~DShowSound() noexcept;
		bool play(pcstr16 path) noexcept;
		Promise<void>* playAnd(pcstr16 path) noexcept;

		// volume: -10000 ~ 0
		//		100 times of dB
		//		ex) -5000 == -50dB
		void setVolume(long volume) noexcept;
		
		// balance: -10000 ~ 10000
		void setBalance(long balance) noexcept;

		// volume: -10000 ~ 0
		//		100 times of dB
		//		ex) -5000 == -50dB
		long getVolume() noexcept;

		// balance: -10000 ~ 10000
		long getBalance() noexcept;

	protected:
		Com<IMediaControl> m_control;
		Com<IBasicAudio> m_basicAudio;
		Com<IMediaEvent> m_event;
		DispatchedEvent * m_eventHandler;
	};
}
