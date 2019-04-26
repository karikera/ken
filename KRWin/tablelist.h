#pragma once

#include <KR3/main.h>
#include "winx.h"
#include "gdi.h"

namespace kr
{
	enum class ColumnType :byte
	{
		String,
		Unsigned,
		Byte,
		Boolean,
		Custom,
	};

	struct FColumnFlags
	{
		ColumnType columnType : 3;
		bool readOnly : 1;
	};

	class TableList : public WindowProgram
	{
	public:
		class Edit :public SubClassingProgram
		{
		public:
			Edit(win::Window* pParent);
			~Edit();
			virtual void wndProc(win::Window* pWindow, uint Msg, WPARAM wParam, LPARAM lParam) override;
			void open(const irect &rc, Text16 strText);
			void close();
		};

		struct ColumnHeader
		{
			Text16 name;
			int right;
			int width;

			ColumnHeader();
		};
		struct Column
		{
			urectwh pos;
			FColumnFlags flags;

			Column();
		};
		struct ItemInfo
		{
			size_t id;
			bool readOnly;
		};
		enum HitTest
		{
			HTHeaderResize,
			HTHeader,
			HTCell,
			HTNone
		};

		TableList(int style, const irectwh& rc, win::Window * pParent) noexcept;
		
		int getItemY(size_t id) noexcept;
		const irect getItemRect(size_t id) noexcept;
		const irect getSubItemRect(upvec2 pos) noexcept;
		uint getColumnByX(int x) noexcept;
		upvec2 getCursor() noexcept;

		HitTest hitTest(ivec2 mouse, upvec2 * out) noexcept;
		bool setCursor(upvec2 cursor) noexcept;
		void upCursor() noexcept;
		void downCursor() noexcept;
		void moveCursor(upvec2 cursor) noexcept;
		void clearCursor() noexcept;
		HitTest click(ivec2 mouse, int buttonIndex) noexcept;
		void previousCursor() noexcept;
		void nextCursor() noexcept;

		void resize(ivec2 sz) noexcept;
		void setItemCount(size_t count);
		void setColumnHeader(uint id, Text16 text, int width) noexcept;
		void setColumn(uint id, urectwh pos, ColumnType type, bool readonly) noexcept;
		void setColumnCount(size_t sz) noexcept;

		void updateColumn() noexcept;
		bool openEditBox() noexcept;
		void closeEditBox() noexcept;
		void drawItem(win::DrawContext * dc, size_t id) noexcept;
		void draw(win::PaintStruct &dc) noexcept;

		virtual void onGetItemInfo(ItemInfo * info) noexcept;
		virtual void onGetItemText(upvec2 pos, Writer16 * writer) noexcept;
		virtual void onPreviousCursor() noexcept;
		virtual void onNextCursor() noexcept;
		virtual void onHeaderClick(uintptr_t column, int sortdir) noexcept;
		virtual void onSelect(upvec2 cursor) noexcept;
		virtual void onChange(upvec2 cursor, Text16 text) noexcept;
		virtual void onChangeBoolean(upvec2 cursor, bool v) noexcept;
		virtual void onRButtonClick(upvec2 cursor, ivec2 mouse) noexcept;
		void wndProc(win::Window* pWindow, uint Msg, WPARAM wParam, LPARAM lParam) override;

	private:
		void _updateScrollWidth() noexcept;
		void _updateScrollHeight() noexcept;

		Edit m_edit;
		upvec2 m_cursor;

		Array<ColumnHeader> m_headers;
		Array<Column> m_columns;

		int m_fontHeight;
		int m_headerHeight;
		int m_itemRowCount;
		int m_itemHeight;
		int m_rowHeight;
		size_t m_itemCount;

		size_t m_columnDrag;
		bool m_columnResize;
		int m_nSortDir;
		uintptr_t m_nSortColumn;

		bool m_bEdit;

		Keep<win::Brush> m_pReadOnlySelected;
		Keep<win::Brush> m_pReadOnlyNormal;

		Keep<win::Brush> m_pWritableSelected;
		Keep<win::Brush> m_pWritableNormal;

	};

}
