#pragma once

#include <KR3/wl/windows.h>
#include <CommCtrl.h>
#include "handle.h"

struct _IMAGELIST {};

namespace kr
{
	namespace win
	{
		class ProgressControl :public Window
		{
		public:
			uint setProgressRange(uint range) noexcept;
			uint setProgressRange(uint rmin, uint rmax) noexcept;
			uint getProgress() noexcept;
			uint setProgress(uint progress) noexcept;
			uint addProgress(uint progress) noexcept;
			uint setStep(uint stepSize) noexcept;
			uint stepIt() noexcept;
		};
		class TabControl :public Window
		{
		public:
			static const char16 CLASS_NAME[];
			static TabControl* createEx(dword nExStyle, dword nStyle, irectwh rc, Window* pParent, int nID = 0) noexcept;
			static inline TabControl* create(int nStyle, irectwh rc, Window* pParent, int nID = 0) noexcept
			{
				return createEx(0, nStyle, rc, pParent, nID);
			}

			int setCursorSelect(int sel);
			int getCursorSelect() noexcept;
			int getItemCount() noexcept;
			bool deleteAllItems() noexcept;
			int setItem(int iItem, const char16 * strName, int iImage = 0) noexcept;
			int insertItem(int iItem, const char16 * strName, int iImage = 0) noexcept;
			bool deleteItem(int iItem) noexcept;
			int adjustRect(bool bLarger, irect* pRect) noexcept;
			win::Brush* getBackgroundBrush() noexcept;
		};

		class HeaderControl : public Window
		{
		public:
			int getItemCount() noexcept;
		};

		class ImageList:public Handle<_IMAGELIST>
		{
		public:
			static ImageList * create(int cx, int cy, UINT flags, int cInitial, int cGrow) noexcept;
			void operator delete(void * p) noexcept;
			int add(HBITMAP hbmImage, HBITMAP hbmMask = nullptr) noexcept;
			int addIcon(HICON hicon) noexcept;
			int replace(int i, HBITMAP hbmImage, HBITMAP hbmMask = nullptr) noexcept;
			int replaceIcon(int i, HICON hicon) noexcept;
		};

		class ListView :public Window
		{
		public:
			static const char16 CLASS_NAME[];
			static ListView* createEx(int nExStyle, int nStyle, irectwh rc, Window* pParent, int nID = 0) noexcept;
			static inline ListView* create(int nStyle, irectwh rc, Window* pParent, int nID = 0) noexcept
			{
				return createEx(0, nStyle, rc, pParent, nID);
			}

			static Writer16 getWriter(LPARAM lParam) noexcept;
			HeaderControl * getHeader() noexcept;
			int getColumnCount() noexcept;
			dword setExStyle(dword nExStyle, dword dwMask = 0) noexcept;
			void addColumn(int nIndex, const char16 * strName, int nWidth) noexcept;
			bool removeColumn(int nIndex) noexcept;
			void addItem(int nIndex, const char16 * strName, int iImage = 0) noexcept;
			void setItem(int nIndex, int nSub, const char16 * strName, int iImage = 0) noexcept;
			void setItem(int nIndex, int nSub, pcstr strName, int iImage = 0) noexcept;
			void getItem(int nIndex, int nSub, pstr16 strName, int nLimit) noexcept;
			void setItemState(int nIndex, int nState, int nMask) noexcept;
			int getItemState(int nIndex, int nMask) noexcept;
			irect getItemRect(int nIndex, int nFlags) noexcept;
			irect getSubItemRect(int nIndex, int nSub, int nFlags) noexcept;
			void drawItemText(DRAWITEMSTRUCT * dis, int nSub, Text16 text) noexcept;
			int getNextItem(int iPos, uint flags) noexcept;
			int setItemCount(int nCount) noexcept;
			HIMAGELIST setImageList(HIMAGELIST himl, int iImageList) noexcept;
			HIMAGELIST setIconListSmall(HIMAGELIST himl) noexcept;
			HIMAGELIST setIconListNormal(HIMAGELIST himl) noexcept;
			void check(int nIndex) noexcept;
			void uncheck(int nIndex) noexcept;
			BOOL isChecked(int nIndex) noexcept;
			LVHITTESTINFO hitTest(ivec2 pt) noexcept;
			LVHITTESTINFO hitTest() noexcept;
			LVHITTESTINFO subItemHitTest(ivec2 pt) noexcept;
			LVHITTESTINFO subItemHitTest() noexcept;
		};

	}
}