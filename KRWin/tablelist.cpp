#include "stdafx.h"
#include "tablelist.h"
#include "handle.h"
#include <KR3/util/scopeobserver.h>

using namespace kr;
using namespace win;

namespace
{
	const ivec2 ITEM_TEXT_OFFSET = { 2, 2 };
	constexpr int DEFAULT_FONT_HEIGHT = 18;

	const char16 * getTableListClassName()
	{
		static const char16 g_szListClassName[] = u"CCS_TABLELIST";
		static kr_scope([]{
			WindowProgram::registerClass(g_szListClassName, 0, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS);
		}, []{
			WindowClass::unregister(g_szListClassName);
		});
		
		return g_szListClassName;
	}
}

TableList::Edit::Edit(win::Window* pParent) :
	SubClassingProgram(win::EditBox::create(WS_CHILD, irectwh({ { 0, 0 },{ 0, 0 } }), pParent))
{
}
TableList::Edit::~Edit()
{
}
void TableList::Edit::wndProc(win::Window* pWindow, uint Msg, WPARAM wParam, LPARAM lParam)
{
	TableList* list;
	switch (Msg)
	{
	case WM_CHAR:
		switch (wParam)
		{
		case VK_ESCAPE:
			list = ((TableList*)m_window->getParent()->getProgram());
			list->closeEditBox();
			throw (LRESULT)0;
		case VK_SPACE:
			if (!(pWindow->getStyle() & ES_NUMBER)) break;
		case VK_TAB:
		case VK_RETURN:
			list = ((TableList*)m_window->getParent()->getProgram());
			list->closeEditBox();
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				list->previousCursor();
			}
			else
			{
				list->nextCursor();
			}
			throw (LRESULT)0;
		}
		break;
	}
}
void TableList::Edit::open(const irect &rc, Text16 strText)
{
	irectwh nrc;
	nrc.x = rc.left + 2;
	nrc.y = rc.top + 2;
	nrc.width = rc.right - nrc.x;
	nrc.height = rc.bottom - nrc.y;
	m_window->setText(TSZ16() << strText);
	((win::EditBox*)(win::Window*)m_window)->setSelect(0, strText.size());
	m_window->move(nrc, true);
	m_window->show(SW_SHOW);
	m_window->setFocus();
}
void TableList::Edit::close()
{
	m_window->show(SW_HIDE);
}

TableList::ColumnHeader::ColumnHeader()
{
	right = 0;
	width = 0;
}
TableList::Column::Column()
{
	pos.x = 0;
	pos.y = 0;
	pos.width = 0;
	pos.height = 0;
}

TableList::TableList(int style, const irectwh& rc, win::Window * pParent) noexcept
	: WindowProgram(win::Window::create(getTableListClassName(), u"", style, rc, pParent))
	, m_edit(getWindow())
{
	m_pReadOnlySelected = win::Brush::create(RGB(215, 215, 215));
	m_pReadOnlyNormal = win::Brush::create(RGB(235, 235, 235));

	m_pWritableSelected = win::Brush::create(RGB(225, 240, 255));
	m_pWritableNormal = win::Brush::create(RGB(255, 255, 255));

	m_itemCount = 0;
	m_cursor = upvec2(-1, -1);
	m_bEdit = false;

	int fontheight = g_defaultFont->getHeight();
	m_fontHeight = fontheight;
	m_headerHeight = m_fontHeight + 10;
	m_rowHeight = m_fontHeight + 5;
	m_itemRowCount = 1;
	m_itemHeight = m_rowHeight;

	m_nSortColumn = 0;
	m_nSortDir = 1;

	m_columnResize = false;
	m_columnDrag = -1;

	m_columns.reserve(20);
}

int TableList::getItemY(size_t id) noexcept
{
	return (int)(-m_window->getScrollY() + m_itemHeight * id + m_headerHeight);
}
const irect TableList::getItemRect(size_t id) noexcept
{
	int y = getItemY(id);
	return{ { 0 ,y } ,{ m_window->getClientRect().right , y + m_itemHeight } };
}
const irect TableList::getSubItemRect(upvec2 pos) noexcept
{
	if (pos.x == -1) return{ { 0,0 },{ 0,0 } };
	int y = getItemY(pos.y);

	Column & column = m_columns[pos.x];
	irect out;
	out.top = y + column.pos.y * m_rowHeight;
	out.bottom = out.top + m_rowHeight * column.pos.height;
	if (column.pos.x > 0) out.left = m_headers[column.pos.x - 1].right;
	else out.left = 0;
	out.right = m_headers[column.pos.right() - 1].right;

	int scrollX = m_window->getScrollX();
	out.left -= scrollX;
	out.right -= scrollX;

	return out;
}
uint TableList::getColumnByX(int x) noexcept
{
	int relx = (x + m_window->getScrollX());
	if (relx < 0) return -1;
	for (uint outx = 0; outx < m_headers.size(); outx++)
	{
		if (relx >= m_headers[outx].right) continue;
		return outx;
	}
	return -1;
}
upvec2 TableList::getCursor() noexcept
{
	return m_cursor;
}

TableList::HitTest TableList::hitTest(ivec2 mouse, upvec2 * out) noexcept
{
	if (mouse.y < m_headerHeight)
	{
		constexpr int GAP = 5;

		closeEditBox();
		int relx = mouse.x + m_window->getScrollX();
		if (relx < 0) return HTNone;

		for (uintptr_t outx = 0; outx < m_headers.size(); outx++)
		{
			int headerR = m_headers[outx].right;
			if (relx >= headerR + GAP) continue;
			out->x = outx;
			if (relx >= headerR - GAP) return HTHeaderResize;
			return HTHeader;
		}
	}

	uvec2 columnpos;
	int rely = (mouse.y + m_window->getScrollY()) - m_headerHeight;
	if (rely < 0) goto _outofregion;

	out->y = rely / m_itemHeight;
	columnpos.y = (rely % m_itemHeight) / m_rowHeight;
	if (out->y >= m_itemCount) goto _outofregion;

	columnpos.x = getColumnByX(mouse.x);
	if (columnpos.x == -1) goto _outofregion;

	for (out->x = 0; out->x < m_columns.size(); out->x++)
	{
		Column & column = m_columns[out->x];
		if (column.pos.contains(columnpos))
		{
			return HTCell;
		}
	}

_outofregion:
	return HTNone;
}
bool TableList::setCursor(upvec2 cursor) noexcept
{
	if (cursor.x >= m_columns.size()) return false;
	if (cursor.y >= m_itemCount) return false;
	if (cursor == m_cursor) return false;

	bool redrawall;
	int scrollto = intact<int>(cursor.y * m_itemHeight);
	int scrollnow = m_window->getScrollY();
	if (scrollto < scrollnow)
	{
		m_window->setScrollY(scrollto, true);
		redrawall = true;
	}
	else
	{
		scrollto += m_itemHeight - m_window->getScrollYPage();
		if (scrollto > scrollnow)
		{
			m_window->setScrollY(scrollto, true);
			redrawall = true;
		}
		else redrawall = false;
	}

	closeEditBox();
	onSelect(cursor);
	if (!redrawall)
	{
		m_window->invalidateRect(getItemRect(m_cursor.y));
		m_window->invalidateRect(getItemRect(cursor.y));
	}
	else
	{
		m_window->invalidate();
	}
	m_cursor = cursor;
	return true;
}
void TableList::upCursor() noexcept
{
	Column & column = m_columns[m_cursor.x];

	auto find = [&](uintptr_t axis)->uintptr_t {
		if(axis == 0) return -1;
		axis --;
		do
		{
			Column & c = m_columns[axis];
			if (c.pos.x == column.pos.x) return axis;
			axis--;
		}
		while(axis != 0);
		return -1;
	};

	upvec2 cursor;
	cursor.x = find(m_cursor.x);
	cursor.y = m_cursor.y;
	if(cursor.x == -1)
	{
		cursor.x = find(m_columns.size());
		cursor.y --;
	}

	setCursor(cursor);
}
void TableList::downCursor() noexcept
{
	Column & column = m_columns[m_cursor.x];

	auto find = [&](uintptr_t axis)->uintptr_t {
		axis++;
		if(axis >= m_columns.size()) return -1;
		do
		{
			Column & c = m_columns[axis];
			if (c.pos.x == column.pos.x) return axis;
			axis++;
		}
		while (axis != m_columns.size());
		return -1;
	};

	upvec2 cursor;
	cursor.x = find(m_cursor.x);
	cursor.y = m_cursor.y;
	if (cursor.x == -1)
	{
		cursor.x = find(-1);
		cursor.y++;
	}

	setCursor(cursor);
}
void TableList::moveCursor(upvec2 cursor) noexcept
{
	cursor += m_cursor;

	size_t mx = m_columns.size();
	if ((int)cursor.x < 0) cursor.x = 0;
	else if (cursor.x >= mx) cursor.x = mx - 1;

	size_t my = m_itemCount;
	if ((int)cursor.y < 0 && m_cursor.y != -1) cursor.y = 0;
	else if (cursor.y == -2) cursor.y = my - 1;
	else if (cursor.y >= my) cursor.y = my - 1;

	setCursor(cursor);
}
void TableList::clearCursor() noexcept
{
	if (m_cursor.y == -1) return;
	closeEditBox();
	irect rc = getItemRect(m_cursor.y);
	m_cursor.y = -1;
	onSelect(m_cursor);
	m_window->invalidateRect(rc);
}
TableList::HitTest TableList::click(ivec2 mouse, int buttonIndex) noexcept
{
	constexpr int GAP = 5;

	m_columnResize = false;
	m_columnDrag = -1;
	m_window->setCapture();
	m_window->setFocus();

	upvec2 cursor;
	HitTest target = hitTest(mouse, &cursor);
	switch (target)
	{
	case HTCell:
		setCursor(cursor);
		break;
	case HTHeader:
	{
		closeEditBox();
		if (m_nSortColumn == cursor.x)
		{
			m_nSortDir = -m_nSortDir;
		}
		else
		{
			m_nSortColumn = cursor.x;
		}
		onHeaderClick(cursor.x, m_nSortDir);
		break;
	}
	case HTHeaderResize:
		m_columnResize = true;
		m_columnDrag = cursor.x;
		break;
	case HTNone:
		clearCursor();
		break;
	}
	return target;
}
void TableList::previousCursor() noexcept
{
	onPreviousCursor();
}
void TableList::nextCursor() noexcept
{
	onNextCursor();
}

void TableList::resize(ivec2 sz) noexcept
{
	m_window->setScrollYPage(sz.y - m_headerHeight);
	m_window->setSize(sz);
}
void TableList::setItemCount(size_t count)
{
	m_itemCount = count;
	_updateScrollHeight();
}
void TableList::setColumnHeader(uint id, Text16 text, int width) noexcept
{
	if (id >= m_headers.size())
	{
		m_headers.resize(id + 1);
	}
	ColumnHeader & header = m_headers[id];
	header.name = text;
	header.width = width * m_fontHeight / DEFAULT_FONT_HEIGHT;
}
void TableList::setColumn(uint id, urectwh pos, ColumnType type, bool readonly) noexcept
{
	if (id >= m_columns.size())
	{
		m_columns.resize(id + 1);
	}
	Column & column = m_columns[id];
	column.pos = pos;
	column.flags.columnType = type;
	column.flags.readOnly = readonly;
}
void TableList::setColumnCount(size_t sz) noexcept
{
	m_headers.resize(sz);
	m_columns.resize(sz);
}
void TableList::updateColumn() noexcept
{
	uint x = 0;
	uint y = 0;
	for (ColumnHeader & c : m_headers)
	{
		x += c.width;
		c.right = x;
	}
	for (Column & c : m_columns)
	{
		if (y < c.pos.y) y = c.pos.y;
	}

	m_itemRowCount = y+1;
	m_itemHeight = m_rowHeight * m_itemRowCount;
	_updateScrollHeight();
	_updateScrollWidth();
}

bool TableList::openEditBox() noexcept
{
	if (m_cursor.y == -1) return false;

	FColumnFlags flags = m_columns[m_cursor.x].flags;
	if (flags.readOnly) return false;

	char16 temp[1024];
	Writer16 writer = temp;
	onGetItemText(m_cursor, &writer);

	switch (flags.columnType)
	{
	case ColumnType::Boolean:
	case ColumnType::String:
		m_edit.getWindow()->removeStyle(ES_NUMBER);
		break;
	case ColumnType::Byte:
	case ColumnType::Unsigned:
		m_edit.getWindow()->addStyle(ES_NUMBER);
		break;
	}

	writer << nullterm;
	m_edit.open(getSubItemRect(m_cursor), writer.flip(temp));
	m_bEdit = true;
	return true;
}
void TableList::closeEditBox() noexcept
{
	if (!m_bEdit) return;
	m_bEdit = false;
	m_edit.close();

	onChange(m_cursor, TSZ16() << m_edit.getWindow()->text());
}
void TableList::drawItem(win::DrawContext * dc, size_t id) noexcept
{
	irect rc = getItemRect(id);

	Brush * pSelected;
	Brush * pNormal;

	ItemInfo ii;
	ii.id = id;
	ii.readOnly = false;
	onGetItemInfo(&ii);

	if (ii.readOnly)
	{
		pSelected = m_pReadOnlySelected;
		pNormal = m_pReadOnlyNormal;
	}
	else
	{
		pSelected = m_pWritableSelected;
		pNormal = m_pWritableNormal;
	}

	if (m_cursor.y == id)
	{
		dc->fillRect(rc, pSelected);
	}
	else
	{
		dc->fillRect(rc, pNormal);
	}

	char16 text[1024];

	if (id == m_cursor.y)
	{
		Writer16 writer = text;
		onGetItemText(m_cursor, &writer);

		irect focusRect = getSubItemRect(m_cursor);
		if (!m_bEdit)
		{
			focusRect += { {1, 1}, { -1,-1 }};
			dc->drawFocusRect(focusRect);
			focusRect -= { {1, 1}, { -1,-1 }};
		}
		else
		{
			dc->fillRect(focusRect, Brush::getStock(WHITE_BRUSH));
		}
		focusRect.from += ITEM_TEXT_OFFSET;
		dc->drawText(writer.flip(text), &focusRect, DT_SINGLELINE | DT_VCENTER);
	}

	size_t ccount = m_columns.size();
	for (uint i = 0; i<ccount; i++)
	{
		if (i == m_cursor.x && id == m_cursor.y) continue;

		Writer16 writer = text;
		onGetItemText({ i, id }, &writer);

		irect rect = getSubItemRect({ i, id });
		rect.from += ITEM_TEXT_OFFSET;
		dc->drawText(writer.flip(text), &rect, DT_SINGLELINE | DT_VCENTER);
	}
}
void TableList::draw(win::PaintStruct &dc) noexcept
{
	win::Window * pWindow = getWindow();
	ObjectSelector _fontsel = dc->use(g_defaultFont);

	irect clientRect = m_window->getClientRect();
	irect rc = (irect&)dc.rcPaint;
	dc->setBkMode(TRANSPARENT);

	int scrollX = m_window->getScrollX();
	int scrollY = pWindow->getScrollY();
	clientRect.top = m_headerHeight;

	Keep<RGN> rgnrect(RGN::createRect(clientRect));
	dc->clip(rgnrect);

	int y = m_headerHeight - scrollY;
	size_t id;
	if (y < rc.top)
	{
		id = (rc.top - y) / m_itemHeight;
	}
	else id = 0;
	y += (int)(id * m_itemHeight);

	Keep<Pen> graypen = Pen::create(0xcccccc);
	ObjectSelector _pensel_o(dc, graypen);

	while (y < rc.bottom)
	{
		if (id >= m_itemCount) break;
		drawItem(dc, id);

		dc->moveTo({ clientRect.left, y });
		dc->lineTo({ clientRect.right, y });

		y += m_itemHeight;
		id++;
	}
	dc->clip(nullptr);


	if (rc.top < m_headerHeight)
	{
		Keep<Pen> ltgraypen = Pen::create(0xeeeeee);
		ObjectSelector _pensel(dc, ltgraypen);
		ObjectSelector _brushsel(dc, Brush::getStock(BLACK_BRUSH));
		dc->fillRect({ { 0,0 },{ clientRect.right, 10 } }, Brush::getStock(WHITE_BRUSH));
		dc->gradientRectV({ 0,10 }, { clientRect.right,m_headerHeight }, 0xfff4f4f4, 0xffeeeeee);
		int prevx = -scrollX;

		for (size_t i = 0; i<m_headers.size(); i++)
		{
			ColumnHeader & column = m_headers[i];
			int nowx = column.right - scrollX;

			if (i == m_nSortColumn)
			{
				ObjectSelector _pensel2(dc, Pen::getStock(NULL_PEN));
				dc->gradientRectV({ prevx,10 }, { nowx,m_headerHeight }, 0xfff4eeee, 0xffeedddd);

				if (m_nSortDir == 1)
				{
					dc->polygon({
						{ prevx + 4, 15 },
						{ prevx + 12, 15 },
						{ prevx + 8, 22 },
					});
				}
				else
				{
					dc->polygon({
						{ prevx + 4, 22 },
						{ prevx + 12, 22 },
						{ prevx + 8, 15 },
					});
				}
			}

			irect columnrc = { { prevx + 12, 3 },{ nowx, m_headerHeight } };
			dc->drawText(column.name, &columnrc, DT_SINGLELINE);

			int x = nowx;
			{
				dc->moveTo({ x, 0 });
				dc->lineTo({ x, m_headerHeight });
			}
			x--;
			{
				ObjectSelector _pensel2(dc, Pen::getStock(WHITE_PEN));
				dc->moveTo({ x, 0 });
				dc->lineTo({ x, m_headerHeight });
			}
			prevx = nowx;
		}
	}

	{
		ObjectSelector _pensel(dc, graypen);
		int lineBottom = (int)(m_itemHeight * m_itemCount - scrollY + m_headerHeight);
		if (lineBottom > clientRect.top)
		{
			if(lineBottom > clientRect.bottom) lineBottom = clientRect.bottom;
			for (ColumnHeader& column : m_headers)
			{
				int x = column.right - scrollX;
				dc->moveTo({ x, clientRect.top });
				dc->lineTo({ x, lineBottom });
			}
		}
		else lineBottom = clientRect.top;


		clientRect.right--;
		clientRect.bottom--;
		dc->moveTo({ clientRect.right , lineBottom });
		dc->lineTo({ clientRect.right , clientRect.top });
		dc->lineTo({ clientRect.left, clientRect.top });
		dc->lineTo({ clientRect.left, lineBottom });
		dc->lineTo({ clientRect.right, lineBottom });
	}
}

void TableList::onGetItemInfo(ItemInfo * info) noexcept
{
}
void TableList::onGetItemText(upvec2 pos, Writer16 * writer) noexcept
{
}
void TableList::onPreviousCursor() noexcept
{
}
void TableList::onNextCursor() noexcept
{
}
void TableList::onHeaderClick(uintptr_t column, int sortdir) noexcept
{
}
void TableList::onSelect(upvec2 cursor) noexcept
{
}
void TableList::onChange(upvec2 cursor, Text16 text) noexcept
{
}
void TableList::onRButtonClick(upvec2 cursor, ivec2 mouse) noexcept
{
}
void TableList::onChangeBoolean(kr::upvec2 cursor, bool v) noexcept
{
}
void TableList::wndProc(win::Window* pWindow, uint Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_SETCURSOR:
	{
		constexpr int GAP = 5;
		ivec2 mouse = m_window->getCursorPos();
		upvec2 out;
		HitTest hit = hitTest(mouse, &out);
		if (hit == HitTest::HTHeaderResize)
		{
			win::Cursor::current = win::Cursor::load(IDC_SIZEWE);
			throw (LRESULT)0;
		}
		break;
	}
	case WM_MOUSEMOVE:
	{
		constexpr int MINWIDTH = 20;
		if(!m_window->isCaptured()) return;
		if (!m_columnResize) return;
		if (m_columnDrag >= m_headers.size()) return;
		int scrollX = m_window->getScrollX();
		int x = ((svec2&)lParam).x + scrollX;
		irect invrect = m_window->getClientRect();
		ColumnHeader &header = m_headers[m_columnDrag];
		invrect.left = (m_columnDrag > 0) ? m_headers[m_columnDrag - 1].right+1 : 0;

		int rightX = x;
		for (size_t i = m_columnDrag + 1; i < m_headers.size(); i++)
		{
			ColumnHeader &rightHeader = m_headers[i];
			rightX += rightHeader.width;
			rightHeader.right = rightX;
		}
		int limitL = invrect.left + MINWIDTH;
		if (x < limitL) x = limitL;
		if(header.right == x) break;
		header.width = x - invrect.left;
		header.right = x;
		invrect.left -= scrollX;
		m_window->invalidateRect(invrect);
		break;
	}
	case WM_LBUTTONUP:
		m_window->releaseCapture();
		throw (LRESULT)0;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		click((ivec2)(svec2&)lParam, Msg == WM_LBUTTONDOWN ? 0 : 1);
		throw (LRESULT)0;
	case WM_LBUTTONDBLCLK:
	{
		ivec2 mouse = (ivec2)(svec2&)lParam;
		switch (click(mouse, 0))
		{
		case HTCell:
			openEditBox();
			break;
		}
		throw (LRESULT)0;
	}
	case WM_MOUSEWHEEL:
		if (pWindow->moveScrollY(-GET_WHEEL_DELTA_WPARAM(wParam) * m_itemHeight / 40, true))
			pWindow->invalidate();
		break;
	case WM_HSCROLL:
	{
		int pagesz;
		switch (LOWORD(wParam))
		{
		case SB_LINERIGHT:
			if (!pWindow->moveScrollX(30)) return;
			break;
		case SB_LINELEFT:
			if (!pWindow->moveScrollX(-30)) return;
			break;
		case SB_PAGERIGHT:
			pagesz = pWindow->getClientRect().right - 60;
			if (!pWindow->moveScrollX(tmax(pagesz, 20))) return;
			break;
		case SB_PAGELEFT:
			pagesz = pWindow->getClientRect().right - 60;
			if (!pWindow->moveScrollX(-tmax(pagesz, 20))) return;
			break;
		case SB_THUMBPOSITION:
			pWindow->setScrollX(HIWORD(wParam));
			break;
		case SB_THUMBTRACK:
			pWindow->setScrollX(HIWORD(wParam));
			break;
			//case SB_TOP: break;
			//case SB_BOTTOM: break;
			//case SB_ENDSCROLL: break;
		default: return;
		}
		pWindow->invalidate();
		break;
	}
	case WM_VSCROLL:
	{
		int pagesz;
		switch (LOWORD(wParam))
		{
		case SB_LINEDOWN:
			if (!pWindow->moveScrollY(m_itemHeight)) return;
			break;
		case SB_LINEUP:
			if (!pWindow->moveScrollY(-m_itemHeight)) return;
			break;
		case SB_PAGEDOWN:
			pagesz = pWindow->getClientRect().bottom - m_itemHeight * 2;
			if (!pWindow->moveScrollY(tmax(pagesz, 20))) return;
			break;
		case SB_PAGEUP:
			pagesz = pWindow->getClientRect().bottom - m_itemHeight * 2;
			if (!pWindow->moveScrollY(-tmax(pagesz, 20))) return;
			break;
		case SB_THUMBPOSITION:
			pWindow->setScrollY(HIWORD(wParam));
			break;
		case SB_THUMBTRACK:
			pWindow->setScrollY(HIWORD(wParam));
			break;
			//case SB_TOP: break;
			//case SB_BOTTOM: break;
			//case SB_ENDSCROLL: break;
		default: return;
		}
		pWindow->invalidate();
		break;
	}
	case WM_RBUTTONUP:
	{
		ivec2 screen = getCursorPos();
		ivec2 client = screen;
		m_window->screenToClient(&client);
		if (client.y < m_headerHeight) return;

		click(client, Msg == WM_LBUTTONDOWN ? 0 : 1);
		onRButtonClick(m_cursor, screen);
		break;
	}
	case WM_CHAR:
		if(GetKeyState(VK_CONTROL) & 0x8000) break;
		switch (wParam)
		{
		case VK_RETURN:
		case VK_SPACE:
			break;
		default:
			if (m_cursor.y == -1) break;
			if (m_columns[m_cursor.x].flags.columnType == ColumnType::Boolean)
			{
				bool bValue;
				switch (wParam)
				{
				case '0':
				case 'o':
				case 'O': bValue = true; break;
				case 'x':
				case 'X': bValue = false; break;
				default: goto _skipboolean;
				}
				upvec2 cursor = m_cursor;
				onChangeBoolean(cursor, bValue);
				cursor.y++;
				if (!setCursor(cursor))
				{
					m_window->invalidateRect(getItemRect(m_cursor.y));
				}
			}
			else
			{
				if (!openEditBox()) break;
				m_edit.getWindow()->postMessage(WM_CHAR, wParam, lParam);
			}
		_skipboolean:
			break;
		}
		throw (LRESULT)0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_DELETE:
			if (openEditBox())
			{
				m_edit.getWindow()->setText(u"");
			}
			throw (LRESULT)0;
		case VK_RETURN:
		case VK_SPACE:
			openEditBox();
			throw (LRESULT)0;
		case VK_NEXT: moveCursor(upvec2(0, 25)); break;
		case VK_PRIOR: moveCursor(upvec2(0, -25)); break;
		case VK_HOME: setCursor(upvec2(0, m_cursor.y)); break;
		case VK_END: setCursor(upvec2(m_columns.size() - 1, m_cursor.y)); break;
		case VK_UP: upCursor(); break;
		case VK_DOWN: downCursor(); break;
		case VK_LEFT: moveCursor(upvec2(-1, 0)); break;
		case VK_RIGHT: moveCursor(upvec2(1, 0)); break;
		}
		throw (LRESULT)0;
	case WM_KEYUP:
		throw (LRESULT)0;
	case WM_PAINT:
		draw(PaintStruct(pWindow));
		break;
	}
}

void kr::TableList::_updateScrollWidth() noexcept
{
	m_window->setScroll(SB_HORZ, m_window->getClientRect().right, m_headers.back().right);
}
void kr::TableList::_updateScrollHeight() noexcept
{
	m_window->setScroll(SB_VERT, m_window->getClientRect().bottom - m_headerHeight, intact<int>(m_itemCount * m_itemHeight));
}
