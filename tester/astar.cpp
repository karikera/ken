#include "astar.h"
#include <KR3/util/time.h>

Wnd::Wnd() noexcept
	: WebCanvas2D(1024, 768)
{
	Random rand(3646835);
	m_graph.generateRandomGraph(rand,
		frectwh(0, 0, (float)getWidth(), (float)getHeight()), 
		200);
	render();
	mema::zero(m_sum);
	mema::zero(m_count);
}
Wnd::~Wnd() noexcept
{
	clearPath();
}
void Wnd::clearPath() noexcept
{
	m_path.clear();
}

void Wnd::onKeyDown(int key, bool repeat) noexcept
{
	clearPath();
	timepoint_detail starttime = timepoint_detail::now();
	switch (key)
	{
	case '1':
		while ((timepoint_detail::now() - starttime).getRealTime() < 0.1f)
		{
			size_t sz = m_graph.size();
			Keep<DiGraph::Path> path = m_graph.astar(m_graph.getRandomVertex(), m_graph.getRandomVertex());
			m_path.push(path);
		}
		break;
	case '2':
		//while ((timepoint_detail::now() - starttime).getRealTime() < 0.1f)
		//{
		//	size_t sz = m_graph.size();
		//	DiGraph::Path * path = m_graph.astarWithoutReturn(m_graph.getRandomVertex(), m_graph.getRandomVertex());
		//	m_path.push(path);
		//}
		break;
	default:
		return;
	}
	size_t idx = key - '1';
	m_profile = (timepoint_detail::now() - starttime).getRealTime();
	m_sum[idx] += m_profile;
	m_count[idx] += (float)m_path.size();
	render();
}

void Wnd::render() noexcept
{
	clearRect(0, 0, (float)getWidth(), (float)getHeight());
	beginPath();
	m_graph.draw(this);
	strokeStyle = 0xff000000;
	stroke();

	Random random(15368914);
	for (DiGraph::Path * path : m_path)
	{
		beginPath();
		path->draw(this);
		strokeStyle = (random.getDword() & 0xffffff) | 0xff000000;
		stroke();
	}

	fillText(TSZ16() << u"시간: " << m_profile, 0, 30);
	fillText(TSZ16() << u"횟수: " << m_path.size(), 0, 60);
	fillText(TSZ16() << u"일반 A-Star: " << m_count[0] / m_sum[0], 0, 90);
	fillText(TSZ16() << u"중복 탐색 방지 A-Star: " << m_count[1] / m_sum[1], 0, 120);
}
void Wnd::text(Text16 tx, vec2 pos) noexcept
{
	WebCanvas2D::fillText(tx, pos.x, pos.y);
}
void Wnd::moveTo(vec2 pos) noexcept
{
	WebCanvas2D::moveTo(pos.x, pos.y);
}
void Wnd::lineTo(vec2 pos) noexcept
{
	WebCanvas2D::lineTo(pos.x, pos.y);
}
