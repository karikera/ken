#include "stdafx.h"
#include "ctrlx.h"
#include "gdi.h"
#pragma comment(lib,"Comctl32.lib")

#include <Uxtheme.h>
#include <Vsstyle.h>
#include <Vssym32.h>
#pragma comment(lib, "UxTheme.lib")

using namespace kr;
using namespace win;

const char16 TabControl::CLASS_NAME[] = TOUTF16(WC_TABCONTROLA);

#pragma warning(disable:4800)

uint ProgressControl::setProgressRange(uint range) noexcept
{
	return (UINT)sendMessage(PBM_SETRANGE32, 0, range);
}
uint ProgressControl::setProgressRange(uint rmin, uint rmax) noexcept
{
	return (UINT)sendMessage(PBM_SETRANGE32, rmin, rmax);
}
uint ProgressControl::getProgress() noexcept
{
	return (UINT)sendMessage(PBM_GETPOS, 0, 0);
}
uint ProgressControl::setProgress(uint progress) noexcept
{
	return (UINT)sendMessage(PBM_SETPOS, progress, 0);
}
uint ProgressControl::addProgress(uint progress) noexcept
{
	return (UINT)sendMessage(PBM_DELTAPOS, progress, 0);
}
uint ProgressControl::setStep(uint stepSize) noexcept
{
	return (UINT)sendMessage(PBM_SETSTEP, stepSize, 0);
}
uint ProgressControl::stepIt() noexcept
{
	return (UINT)sendMessage(PBM_STEPIT, 0, 0);
}

TabControl* TabControl::createEx(dword nExStyle, dword nStyle, irectwh rc, Window* pParent, int nID) noexcept
{
	TabControl* pTab = (TabControl*)Window::createEx(nExStyle, CLASS_NAME, zeroptr, nStyle, rc, pParent, nID);
	pTab->setFont(win::g_defaultFont);
	return pTab;
}
int TabControl::setCursorSelect(int sel)
{
	return (int)sendMessage(TCM_SETCURSEL, sel, 0);
}
int TabControl::getCursorSelect() noexcept
{
	return (int)sendMessage(TCM_GETCURSEL,0 ,0);
}
int TabControl::getItemCount() noexcept
{
	return (int)sendMessage(TCM_GETITEMCOUNT, 0, 0);
}
bool TabControl::deleteAllItems() noexcept
{
	return (bool)sendMessage(TCM_DELETEALLITEMS, 0, 0);
}
int TabControl::setItem(int iItem, const char16 * strName, int iImage) noexcept
{
	TCITEMW tie;
	tie.mask = TCIF_TEXT;
	tie.iImage = iImage;
	tie.pszText = (LPWSTR)strName;
	return (int)sendMessage(TCM_SETITEMW, iItem, (LPARAM)&tie);
}
int TabControl::insertItem(int iItem, const char16 * strName, int iImage) noexcept
{
	TCITEMW tie;
	tie.mask = TCIF_TEXT;
	tie.iImage = iImage;
	tie.pszText = (LPWSTR)strName;
	return (int)sendMessage(TCM_INSERTITEMW, iItem, (LPARAM)&tie);
}
bool TabControl::deleteItem(int iItem) noexcept
{
	return (bool)sendMessage(TCM_DELETEITEM, iItem, 0);
}
int TabControl::adjustRect(bool bLarger, irect* pRect) noexcept
{
	return (int)sendMessage(TCM_ADJUSTRECT, (WPARAM)bLarger, (LPARAM)pRect);
}
win::Brush* TabControl::getBackgroundBrush() noexcept
{
	HBRUSH hbrTabBackground = nullptr;
	//Open the theme data for "Tab" class
	HTHEME hTheme = OpenThemeData(this, L"Tab");

	//create pattern brush from tab control theme body part
	if (hTheme)
	{
		HDC hdc = GetDC(nullptr);
		if (hdc)
		{
			HDC hdcMem = CreateCompatibleDC(hdc);
			if (hdcMem)
			{
				INT iBgType;

				if (SUCCEEDED(GetThemeEnumValue(hTheme, TABP_BODY, 0, TMT_BGTYPE, &iBgType)))
				{
					if (iBgType == BT_IMAGEFILE)
					{
						SIZE size;

						if (SUCCEEDED(GetThemePartSize(hTheme, hdcMem, TABP_BODY, 0, nullptr, TS_TRUE, &size)))
						{
							HBITMAP hbmpMem = CreateCompatibleBitmap(hdc, size.cx, size.cy);
							if (hbmpMem)
							{
								RECT rc;

								SetRect(&rc, 0, 0, size.cx, size.cy);
								SelectObject(hdcMem, hbmpMem);

								if (SUCCEEDED(DrawThemeBackground(hTheme, hdcMem, TABP_BODY, 0, &rc, nullptr)))
								{
									hbrTabBackground = CreatePatternBrush(hbmpMem);
								}

								DeleteObject(hbmpMem);
							}
						}
					}
					else if (iBgType == BT_BORDERFILL)
					{
						COLORREF clr;

						if (SUCCEEDED(GetThemeColor(hTheme, TABP_BODY, 0, TMT_FILLCOLOR, &clr)))
						{
							hbrTabBackground = CreateSolidBrush(clr);
						}
					}
				}
				DeleteDC(hdcMem);
			}
			ReleaseDC(nullptr, hdc);
		}
		CloseThemeData(hTheme);
	}
	else
	{
		hbrTabBackground = (HBRUSH)GetClassLongPtr(this, GCLP_HBRBACKGROUND);
	}

	return (win::Brush*)hbrTabBackground;
}

int HeaderControl::getItemCount() noexcept
{
	return (int)sendMessage(HDM_GETITEMCOUNT, 0,0);
}

const char16 ListView::CLASS_NAME[] = TOUTF16(WC_LISTVIEWA);

ListView* ListView::createEx(int nExStyle, int nStyle, irectwh rc, Window* pParent, int nID) noexcept
{
	return (ListView*)Window::createEx(nExStyle, CLASS_NAME, zeroptr, nStyle, rc, pParent, nID);
}
Writer16 ListView::getWriter(LPARAM lParam) noexcept
{
	NMLVDISPINFOW* pi = (NMLVDISPINFOW*)lParam;
	return Writer16(unwide(pi->item.pszText), pi->item.cchTextMax);
}
HeaderControl * ListView::getHeader() noexcept 
{
	return (HeaderControl*)sendMessage(LVM_GETHEADER, 0, 0);
}
int ListView::getColumnCount() noexcept
{
	return getHeader()->getItemCount();
}
dword ListView::setExStyle(dword nExStyle, dword dwMask) noexcept
{
	return (dword)sendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, dwMask, nExStyle);
}
void ListView::addColumn(int nIndex, const char16 * strName, int nWidth) noexcept
{
	LVCOLUMNW c;
	c.mask = LVCF_TEXT | LVCF_WIDTH;
	c.pszText = (LPWSTR)strName;
	c.cx = nWidth;
	c.iImage = 0;
	sendMessage(LVM_INSERTCOLUMNW, nIndex, (LPARAM)&c);
}
bool ListView::removeColumn(int nIndex) noexcept
{
	return (bool)sendMessage(LVM_DELETECOLUMN, nIndex, 0);
}
void ListView::addItem(int nIndex, const char16 * strName, int iImage) noexcept
{
	LVITEMW i;
	i.mask = LVIF_TEXT;
	i.iImage = iImage;
	i.iItem = nIndex;
	i.iSubItem = 0;
	i.pszText = (LPWSTR)strName;
	sendMessage(LVM_INSERTITEMW, 0, (LPARAM)&i);
}
void ListView::setItem(int nIndex, int nSub, const char16 * strName, int iImage) noexcept
{
	LVITEMW i;
	i.mask = LVIF_TEXT;
	i.iImage = iImage;
	i.iItem = nIndex;
	i.iSubItem = nSub;
	i.pszText = (LPWSTR)strName;
	sendMessage(LVM_SETITEMW, 0, (LPARAM)&i);
}
void ListView::setItem(int nIndex, int nSub, pcstr strName, int iImage) noexcept
{
	LVITEMA i;
	i.mask = LVIF_TEXT;
	i.iImage = iImage;
	i.iItem = nIndex;
	i.iSubItem = nSub;
	i.pszText = (LPSTR)strName;
	sendMessageA(LVM_SETITEMA, 0, (LPARAM)&i);
}
void ListView::getItem(int nIndex, int nSub, pstr16 strName, int nLimit) noexcept
{
	LVITEMW i;
	i.cchTextMax = nLimit;
	i.pszText = wide(strName);
	i.iSubItem = nSub;
	sendMessage(LVM_GETITEMTEXTW, nIndex, (LPARAM)&i);
}
irect ListView::getItemRect(int nIndex, int nFlags) noexcept
{
	RECT rc;
	rc.left = nFlags;
	if(!sendMessage(LVM_GETITEMRECT, nIndex, (LPARAM)&rc))
	{
		mema::zero(rc);
	}
	return (irect&)rc;
}
irect ListView::getSubItemRect(int nIndex, int nSub, int nFlags) noexcept
{
	
	RECT rc;
	rc.left = nFlags;
	rc.top = nSub;
	if(!sendMessage(LVM_GETSUBITEMRECT, nIndex, (LPARAM)&rc))
	{
		mema::zero(rc);
	}
	return (irect&)rc;
}
void ListView::setItemState(int nIndex, int nState, int nMask) noexcept
{
	LVITEMW state;
	state.stateMask = nMask;
	state.state = nState;
	sendMessage(LVM_SETITEMSTATE, nIndex, (LPARAM)&state);
}
int ListView::getItemState(int nIndex, int nMask) noexcept
{
	return (int)sendMessage(LVM_GETITEMSTATE, nIndex, nMask);
}
void ListView::drawItemText(DRAWITEMSTRUCT * dis, int nSub, Text16 text) noexcept
{
	irect rc = getSubItemRect(dis->itemID, nSub, LVIR_LABEL);
	rc.left += 3;
	((win::DrawContext*)dis->hDC)->drawText(text, &rc, DT_SINGLELINE | DT_VCENTER);
}

int ListView::getNextItem(int iPos, uint flags) noexcept
{
	return (int)sendMessage(LVM_GETNEXTITEM, (WPARAM)iPos, MAKELPARAM((flags), 0));
}
int ListView::setItemCount(int nCount) noexcept
{
	return (int)sendMessage(LVM_SETITEMCOUNT, (WPARAM)(nCount), 0);
}
HIMAGELIST ListView::setImageList(HIMAGELIST himl, int iImageList) noexcept
{
	return ListView_SetImageList(this, himl, iImageList);
}
HIMAGELIST ListView::setIconListSmall(HIMAGELIST himl) noexcept
{
	return setImageList(himl, LVSIL_SMALL);
}
HIMAGELIST ListView::setIconListNormal(HIMAGELIST himl) noexcept
{
	return setImageList(himl, LVSIL_NORMAL);
}
void ListView::check(int nIndex) noexcept
{
	LVITEMW _macro_lvi;
	_macro_lvi.stateMask = LVIS_STATEIMAGEMASK;
	_macro_lvi.state =  UINT(2 << 12);
	sendMessage(LVM_SETITEMSTATE, (WPARAM)(nIndex), (LPARAM)&_macro_lvi);
}
void ListView::uncheck(int nIndex) noexcept
{
	LVITEMW _macro_lvi;
	_macro_lvi.stateMask = LVIS_STATEIMAGEMASK;
	_macro_lvi.state =  UINT(1 << 12);
	sendMessage(LVM_SETITEMSTATE, (WPARAM)(nIndex), (LPARAM)&_macro_lvi);
}
BOOL ListView::isChecked(int nIndex) noexcept
{
	return (BOOL)((sendMessage(LVM_GETITEMSTATE, (WPARAM)(nIndex), (LPARAM)(LVIS_STATEIMAGEMASK)) & LVIS_STATEIMAGEMASK) >> 13);
}
LVHITTESTINFO ListView::hitTest(ivec2 pt) noexcept
{
	Window* pHeader = getHeader();
	LVHITTESTINFO ht;

	if(pHeader->getClientRect().contains(pt))
	{
		mema::zero(ht);
		ht.iItem = -1;
	}
	else
	{
		ht.pt = (POINT&)pt;
		sendMessage(LVM_HITTEST, 0, (LPARAM)&ht);
	}

	return ht;
}
LVHITTESTINFO ListView::hitTest() noexcept
{
	return hitTest(getCursorPos());
}
LVHITTESTINFO ListView::subItemHitTest(ivec2 pt) noexcept
{
	Window* pHeader = getHeader();
	LVHITTESTINFO ht;

	if(pHeader->getClientRect().contains(pt))
	{
		mema::zero(ht);
		ht.iItem = -1;
	}
	else
	{
		ht.pt = (POINT&)pt;
		sendMessage(LVM_SUBITEMHITTEST, 0, (LPARAM)&ht);
	}

	return ht;
}
LVHITTESTINFO ListView::subItemHitTest() noexcept
{
	return subItemHitTest(getCursorPos());
}

ImageList * ImageList::create(int cx, int cy, UINT flags, int cInitial, int cGrow) noexcept
{
	return static_cast<ImageList*>(ImageList_Create(cx, cy, flags, cInitial, cGrow));
}
void ImageList::operator delete(void * p) noexcept
{
	ImageList_Destroy((HIMAGELIST)p);
}
int ImageList::add(HBITMAP hbmImage, HBITMAP hbmMask) noexcept
{
	return ImageList_Add(this, hbmImage, hbmMask);
}
int ImageList::addIcon(HICON hicon) noexcept
{
	return ImageList_AddIcon(this, hicon);
}
int ImageList::replace(int i, HBITMAP hbmImage, HBITMAP hbmMask) noexcept
{
	return ImageList_Replace(this, i, hbmImage, hbmMask);
}
int ImageList::replaceIcon(int i, HICON hicon) noexcept
{
	return ImageList_ReplaceIcon(this, i, hicon);
}