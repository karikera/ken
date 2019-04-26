#include "stdafx.h"
#include "dshow.h"
using namespace kr;

#include <dshow.h>
#pragma comment(lib, "strmiids.lib")

DShowFilterGraph::DShowFilterGraph() noexcept
{
	HRESULT hr = ::CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void**)& m_graph);
	if (FAILED(hr)) return;
}
DShowFilterGraph::~DShowFilterGraph() noexcept
{
}
bool DShowFilterGraph::_renderFile(pcstr16 path) noexcept
{
	if (m_graph == nullptr) return false;
	// Build the graph.
	HRESULT hr = m_graph->RenderFile(wide(path), nullptr);
	return SUCCEEDED(hr);
}

DShowSound::DShowSound() noexcept
{
	m_control = m_graph.as<IMediaControl>();
	m_basicAudio = m_graph.as<IBasicAudio>();
	m_event = m_graph.as<IMediaEvent>();
	m_eventHandler = nullptr;
}
DShowSound::~DShowSound() noexcept
{
	if (m_eventHandler)
	{
		m_eventHandler->detach();
	}
}
bool DShowSound::play(pcstr16 path) noexcept
{
	if (!_renderFile(path)) return false;
	// Run the graph.
	HRESULT hr = m_control->Run();
	return SUCCEEDED(hr);
}
Promise<void>* DShowSound::playAnd(pcstr16 path) noexcept
{
	if (!play(path)) return Promise<void>::reject(UnsupportedException());
	if (m_event == nullptr) return Promise<void>::reject(UnsupportedException());

	if (m_eventHandler)
	{
		m_eventHandler->detach();
		m_eventHandler = nullptr;
	}

	OAEVENT handle;
	HRESULT hr = m_event->GetEventHandle(&handle);
	if (FAILED(hr)) return Promise<void>::reject(ErrorCode(hr));

	DeferredPromise<void> * prom = _new DeferredPromise<void>();
	m_eventHandler = EventDispatcher::registThreaded((EventHandle*)handle, [this, prom](DispatchedEvent * ev){
		long code;
		LONG_PTR param1;
		LONG_PTR param2;
		while (m_event->GetEvent(&code, &param1, &param2, 0) == S_OK)
		{
			if (code == EC_COMPLETE)
			{
				prom->resolve();
				if (m_eventHandler)
				{
					m_eventHandler->detach();
					m_eventHandler = nullptr;
				}
			}
			m_event->FreeEventParams(code, param1, param2);
		}
	});
	return prom;
}
void DShowSound::setVolume(long volume) noexcept
{
	m_basicAudio->put_Volume(volume);
}
void DShowSound::setBalance(long balance) noexcept
{
	m_basicAudio->put_Balance(balance);
}

long DShowSound::getVolume() noexcept
{
	long volume;
	if (FAILED(m_basicAudio->get_Volume(&volume))) return 0;
	return volume;
}
long DShowSound::getBalance() noexcept
{
	long balance;
	if (FAILED(m_basicAudio->get_Balance(&balance))) return 0;
	return balance;
}
