#include "stdafx.h"
#define WIN32_UNICODE

#include "browser.h"
#include <KR3/wl/com.h>

#include <stdio.h>
#include <string>
#include <Exdisp.h>
#include <comdef.h> // for variant_t

using kr::ComImplement;
using kr::EmbeddedBrowser;
using kr::Keep;
using kr::win::Window;
using kr::MessageStruct;
using kr::Text16;
using kr::irect;
using kr::wide;

namespace
{

	class EmbeddedBrowserImpl : public ComImplement<EmbeddedBrowserImpl, IOleClientSite, IOleInPlaceSite, IStorage>, public EmbeddedBrowser
	{
	protected:
		Keep<IOleInPlaceObject> m_oleInPlaceObject;
		Keep<IOleObject> m_oleObject;
		Keep<IWebBrowser2> m_webBrowser;
		Keep<IOleInPlaceActiveObject> m_pIOIPAO;
		Window * m_mainWindow;
		RECT m_objectRect;

	public:
		bool initialize(Window* _mainWindow) throws(kr::ErrorCode);

		virtual ~EmbeddedBrowserImpl() override;

		bool _Translate(LPMSG pMsg);

		virtual bool translate(const MessageStruct* pMsg) override;

		virtual void navigate(Text16 _url) override;

		virtual void setRect(const irect& _rc) override;

		// ---------- IUnknown ----------

		virtual ULONG STDMETHODCALLTYPE Release() override;

		// ---------- IOleWindow ----------

		virtual /* [input_sync] */ HRESULT STDMETHODCALLTYPE GetWindow(
			/* [out] */ __RPC__deref_out_opt HWND *phwnd) override;

		virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(
			/* [in] */ BOOL fEnterMode) override;

		// ---------- IOleInPlaceSite ----------

		virtual HRESULT STDMETHODCALLTYPE CanInPlaceActivate(void) override;

		virtual HRESULT STDMETHODCALLTYPE OnInPlaceActivate(void) override;

		virtual HRESULT STDMETHODCALLTYPE OnUIActivate(void) override;

		virtual HRESULT STDMETHODCALLTYPE GetWindowContext(
			/* [out] */ __RPC__deref_out_opt IOleInPlaceFrame **ppFrame,
			/* [out] */ __RPC__deref_out_opt IOleInPlaceUIWindow **ppDoc,
			/* [out] */ __RPC__out LPRECT lprcPosRect,
			/* [out] */ __RPC__out LPRECT lprcClipRect,
			/* [out][in] */ __RPC__inout LPOLEINPLACEFRAMEINFO lpFrameInfo) override;

		virtual HRESULT STDMETHODCALLTYPE Scroll(
			/* [in] */ SIZE scrollExtant) override;

		virtual HRESULT STDMETHODCALLTYPE OnUIDeactivate(
			/* [in] */ BOOL fUndoable) override;

		virtual HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate(void) override;

		virtual HRESULT STDMETHODCALLTYPE DiscardUndoState(void) override;

		virtual HRESULT STDMETHODCALLTYPE DeactivateAndUndo(void) override;

		virtual HRESULT STDMETHODCALLTYPE OnPosRectChange(
			/* [in] */ __RPC__in LPCRECT lprcPosRect) override;

		// ---------- IOleClientSite ----------

		virtual HRESULT STDMETHODCALLTYPE SaveObject(void) override;

		virtual HRESULT STDMETHODCALLTYPE GetMoniker(
			/* [in] */ DWORD dwAssign,
			/* [in] */ DWORD dwWhichMoniker,
			/* [out] */ __RPC__deref_out_opt IMoniker **ppmk) override;

		virtual HRESULT STDMETHODCALLTYPE GetContainer(
			/* [out] */ __RPC__deref_out_opt IOleContainer **ppContainer) override;

		virtual HRESULT STDMETHODCALLTYPE ShowObject(void) override;

		virtual HRESULT STDMETHODCALLTYPE OnShowWindow(
			/* [in] */ BOOL fShow) override;

		virtual HRESULT STDMETHODCALLTYPE RequestNewObjectLayout(void) override;

		// ----- IStorage -----

		virtual HRESULT STDMETHODCALLTYPE CreateStream(
			/* [string][in] */ __RPC__in_string const OLECHAR *pwcsName,
			/* [in] */ DWORD grfMode,
			/* [in] */ DWORD reserved1,
			/* [in] */ DWORD reserved2,
			/* [out] */ __RPC__deref_out_opt IStream **ppstm) override;

		virtual /* [local] */ HRESULT STDMETHODCALLTYPE OpenStream(
			/* [string][in] */ const OLECHAR *pwcsName,
			/* [unique][in] */ void *reserved1,
			/* [in] */ DWORD grfMode,
			/* [in] */ DWORD reserved2,
			/* [out] */ IStream **ppstm) override;

		virtual HRESULT STDMETHODCALLTYPE CreateStorage(
			/* [string][in] */ __RPC__in_string const OLECHAR *pwcsName,
			/* [in] */ DWORD grfMode,
			/* [in] */ DWORD reserved1,
			/* [in] */ DWORD reserved2,
			/* [out] */ __RPC__deref_out_opt IStorage **ppstg) override;

		virtual HRESULT STDMETHODCALLTYPE OpenStorage(
			/* [string][unique][in] */ __RPC__in_opt_string const OLECHAR *pwcsName,
			/* [unique][in] */ __RPC__in_opt IStorage *pstgPriority,
			/* [in] */ DWORD grfMode,
			/* [unique][in] */ __RPC__deref_opt_in_opt SNB snbExclude,
			/* [in] */ DWORD reserved,
			/* [out] */ __RPC__deref_out_opt IStorage **ppstg) override;

		virtual /* [local] */ HRESULT STDMETHODCALLTYPE CopyTo(
			/* [in] */ DWORD ciidExclude,
			/* [size_is][unique][in] */ const IID *rgiidExclude,
			/* [annotation][unique][in] */
			__RPC__in_opt  SNB snbExclude,
			/* [unique][in] */ IStorage *pstgDest) override;

		virtual HRESULT STDMETHODCALLTYPE MoveElementTo(
			/* [string][in] */ __RPC__in_string const OLECHAR *pwcsName,
			/* [unique][in] */ __RPC__in_opt IStorage *pstgDest,
			/* [string][in] */ __RPC__in_string const OLECHAR *pwcsNewName,
			/* [in] */ DWORD grfFlags) override;

		virtual HRESULT STDMETHODCALLTYPE Commit(
			/* [in] */ DWORD grfCommitFlags) override;

		virtual HRESULT STDMETHODCALLTYPE Revert(void) override;

		virtual /* [local] */ HRESULT STDMETHODCALLTYPE EnumElements(
			/* [in] */ DWORD reserved1,
			/* [size_is][unique][in] */ void *reserved2,
			/* [in] */ DWORD reserved3,
			/* [out] */ IEnumSTATSTG **ppenum) override;

		virtual HRESULT STDMETHODCALLTYPE DestroyElement(
			/* [string][in] */ __RPC__in_string const OLECHAR *pwcsName) override;

		virtual HRESULT STDMETHODCALLTYPE RenameElement(
			/* [string][in] */ __RPC__in_string const OLECHAR *pwcsOldName,
			/* [string][in] */ __RPC__in_string const OLECHAR *pwcsNewName) override;

		virtual HRESULT STDMETHODCALLTYPE SetElementTimes(
			/* [string][unique][in] */ __RPC__in_opt_string const OLECHAR *pwcsName,
			/* [unique][in] */ __RPC__in_opt const FILETIME *pctime,
			/* [unique][in] */ __RPC__in_opt const FILETIME *patime,
			/* [unique][in] */ __RPC__in_opt const FILETIME *pmtime) override;

		virtual HRESULT STDMETHODCALLTYPE SetClass(
			/* [in] */ __RPC__in REFCLSID clsid) override;

		virtual HRESULT STDMETHODCALLTYPE SetStateBits(
			/* [in] */ DWORD grfStateBits,
			/* [in] */ DWORD grfMask) override;

		virtual HRESULT STDMETHODCALLTYPE Stat(
			/* [out] */ __RPC__out STATSTG *pstatstg,
			/* [in] */ DWORD grfStatFlag) override;

	};

	bool EmbeddedBrowserImpl::initialize(Window * _mainWindow) throws(kr::ErrorCode)
	{
		//IInputObject

		m_oleObject = nullptr;
		m_webBrowser = nullptr;
		m_oleInPlaceObject = nullptr;

		::SetRect(&m_objectRect, -300, -300, 300, 300);
		m_mainWindow = _mainWindow;

		hrexcept(::OleCreate(CLSID_WebBrowser, IID_IOleObject, OLERENDER_DRAW, 0, this, this, (void**)&m_oleObject));
		hrexcept(m_oleObject->SetClientSite(this));
		hrexcept(OleSetContainedObject(m_oleObject, TRUE));

		RECT posRect;
		::SetRect(&posRect, -300, -300, 300, 300);
		hrexcept(m_oleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, nullptr, this, -1, m_mainWindow, &posRect));

		hrexcept(m_oleObject->QueryInterface((IWebBrowser2**)&m_webBrowser));
		hrexcept(m_webBrowser->QueryInterface((IOleInPlaceActiveObject**)&m_pIOIPAO));

		HWND hControl;
		m_oleInPlaceObject->GetWindow(&hControl);
		::ShowWindow(hControl, SW_SHOW);

		m_webBrowser->Navigate(const_cast<BSTR>(L"about:blank"), &variant_t((UINT)0), nullptr, nullptr, nullptr);
		return true;
	}
	EmbeddedBrowserImpl::~EmbeddedBrowserImpl()
	{
		m_pIOIPAO = nullptr;
		if (m_webBrowser != nullptr) m_webBrowser->Quit();
		if (m_oleObject != nullptr) m_oleObject->Close(0);
	}
	bool EmbeddedBrowserImpl::_Translate(LPMSG pMsg)
	{
		if (m_pIOIPAO == nullptr) return false;
		switch (pMsg->message)
		{
		case WM_KEYUP:
		case WM_KEYDOWN:
			switch (pMsg->wParam)
			{
			case VK_BACK:
			case VK_LEFT:
			case VK_RIGHT:
			case VK_UP:
			case VK_DOWN:
				return false;
			}
			break;
		}
		HRESULT hr = m_pIOIPAO->TranslateAccelerator((LPMSG)pMsg);
		return false;
	}
	bool EmbeddedBrowserImpl::translate(const MessageStruct * pMsg)
	{
		return _Translate((LPMSG)pMsg);
	}
	void EmbeddedBrowserImpl::navigate(Text16 _url)
	{
		BSTR url = SysAllocStringLen(wide(_url.data()), kr::intact<UINT>(_url.size()));
		HRESULT hr = m_webBrowser->Navigate(url, &variant_t(0x02u), nullptr, nullptr, nullptr);
		SysFreeString(url);
	}
	void EmbeddedBrowserImpl::setRect(const irect & _rc)
	{
		m_objectRect = (RECT&)_rc;

		{
			RECT hiMetricRect;
			{
				static bool s_initialized = false;
				static int s_pixelsPerInchX, s_pixelsPerInchY;
				if (!s_initialized)
				{
					HDC hdc = ::GetDC(nullptr);
					s_pixelsPerInchX = ::GetDeviceCaps(hdc, LOGPIXELSX);
					s_pixelsPerInchY = ::GetDeviceCaps(hdc, LOGPIXELSY);
					::ReleaseDC(nullptr, hdc);
					s_initialized = true;
				}

				hiMetricRect.left = MulDiv(2540, m_objectRect.left, s_pixelsPerInchX);
				hiMetricRect.top = MulDiv(2540, m_objectRect.top, s_pixelsPerInchY);
				hiMetricRect.right = MulDiv(2540, m_objectRect.right, s_pixelsPerInchX);
				hiMetricRect.bottom = MulDiv(2540, m_objectRect.bottom, s_pixelsPerInchY);
			}

			SIZEL sz;
			sz.cx = hiMetricRect.right - hiMetricRect.left;
			sz.cy = hiMetricRect.bottom - hiMetricRect.top;
			m_oleObject->SetExtent(DVASPECT_CONTENT, &sz);
		}

		if (m_oleInPlaceObject != nullptr)
		{
			m_oleInPlaceObject->SetObjectRects(&m_objectRect, &m_objectRect);
		}
	}
	ULONG EmbeddedBrowserImpl::Release()
	{
		return InterlockedDecrement(&m_nReference);
	}
	HRESULT EmbeddedBrowserImpl::GetWindow(HWND * phwnd)
	{
		(*phwnd) = m_mainWindow;
		return S_OK;
	}
	HRESULT EmbeddedBrowserImpl::ContextSensitiveHelp(BOOL fEnterMode)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::CanInPlaceActivate(void)
	{
		return S_OK;
	}
	HRESULT EmbeddedBrowserImpl::OnInPlaceActivate(void)
	{
		OleLockRunning(m_oleObject, TRUE, FALSE);
		m_oleObject->QueryInterface((IOleInPlaceObject**)&m_oleInPlaceObject);
		m_oleInPlaceObject->SetObjectRects(&m_objectRect, &m_objectRect);

		return S_OK;
	}
	HRESULT EmbeddedBrowserImpl::OnUIActivate(void)
	{
		return S_OK;
	}
	HRESULT EmbeddedBrowserImpl::GetWindowContext(IOleInPlaceFrame ** ppFrame, IOleInPlaceUIWindow ** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
	{
		(*ppFrame) = nullptr;
		(*ppDoc) = nullptr;
		(*lprcPosRect).left = m_objectRect.left;
		(*lprcPosRect).top = m_objectRect.top;
		(*lprcPosRect).right = m_objectRect.right;
		(*lprcPosRect).bottom = m_objectRect.bottom;
		*lprcClipRect = *lprcPosRect;

		lpFrameInfo->fMDIApp = false;
		lpFrameInfo->hwndFrame = m_mainWindow;
		lpFrameInfo->haccel = nullptr;
		lpFrameInfo->cAccelEntries = 0;

		return S_OK;
	}
	HRESULT EmbeddedBrowserImpl::Scroll(SIZE scrollExtant)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::OnUIDeactivate(BOOL fUndoable)
	{
		return S_OK;
	}
	HRESULT EmbeddedBrowserImpl::OnInPlaceDeactivate(void)
	{
		m_oleInPlaceObject = nullptr;
		return S_OK;
	}
	HRESULT EmbeddedBrowserImpl::DiscardUndoState(void)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::DeactivateAndUndo(void)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::OnPosRectChange(LPCRECT lprcPosRect)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::SaveObject(void)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker ** ppmk)
	{
		if ((dwAssign == OLEGETMONIKER_ONLYIFTHERE) &&
			(dwWhichMoniker == OLEWHICHMK_CONTAINER))
			return E_FAIL;

		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::GetContainer(IOleContainer ** ppContainer)
	{
		return E_NOINTERFACE;
	}
	HRESULT EmbeddedBrowserImpl::ShowObject(void)
	{
		return S_OK;
	}
	HRESULT EmbeddedBrowserImpl::OnShowWindow(BOOL fShow)
	{
		return S_OK;
	}
	HRESULT EmbeddedBrowserImpl::RequestNewObjectLayout(void)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::CreateStream(const OLECHAR * pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream ** ppstm)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::OpenStream(const OLECHAR * pwcsName, void * reserved1, DWORD grfMode, DWORD reserved2, IStream ** ppstm)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::CreateStorage(const OLECHAR * pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStorage ** ppstg)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::OpenStorage(const OLECHAR * pwcsName, IStorage * pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage ** ppstg)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::CopyTo(DWORD ciidExclude, const IID * rgiidExclude, SNB snbExclude, IStorage * pstgDest)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::MoveElementTo(const OLECHAR * pwcsName, IStorage * pstgDest, const OLECHAR * pwcsNewName, DWORD grfFlags)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::Commit(DWORD grfCommitFlags)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::Revert(void)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::EnumElements(DWORD reserved1, void * reserved2, DWORD reserved3, IEnumSTATSTG ** ppenum)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::DestroyElement(const OLECHAR * pwcsName)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::RenameElement(const OLECHAR * pwcsOldName, const OLECHAR * pwcsNewName)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::SetElementTimes(const OLECHAR * pwcsName, const FILETIME * pctime, const FILETIME * patime, const FILETIME * pmtime)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::SetClass(REFCLSID clsid)
	{
		return S_OK;
		//return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::SetStateBits(DWORD grfStateBits, DWORD grfMask)
	{
		return E_NOTIMPL;
	}
	HRESULT EmbeddedBrowserImpl::Stat(STATSTG * pstatstg, DWORD grfStatFlag)
	{
		return E_NOTIMPL;
	}

}

EmbeddedBrowser::~EmbeddedBrowser()
{
}
EmbeddedBrowser* EmbeddedBrowser::newInstance(Window * pWindow) throws(kr::ErrorCode)
{
	Keep<EmbeddedBrowserImpl> pBrowser = _new EmbeddedBrowserImpl;
	if (!pBrowser->initialize(pWindow))
	{
		return nullptr;
	}
	return pBrowser.detach();
}

