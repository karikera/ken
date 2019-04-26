#include "digraph.h"
#include <KR3/data/binarray.h>

using namespace kr;


void ICanvas::drawCross(kr::vec2 p, float size) noexcept
{
	moveTo(p - vec2(size, size));
	lineTo(p + vec2(size, size));
	moveTo(p + vec2(-size, size));
	lineTo(p + vec2(size, -size));
}

float DiGraph::Path::getKey() const noexcept
{
	return weight;
}
float DiGraph::Path::compare(const DiGraph::Path * b) const noexcept
{
	return weight - b->weight;
}

size_t DiGraph::size() noexcept
{
	return m_vertex.size();
}
void DiGraph::generateRandomGraph(Random & rnd, frectwh rc, size_t cnt) noexcept
{
	m_vertex.reserve(cnt);
	Vertex * beg = m_vertex.begin();

	while (cnt--)
	{
		vec2 pos;
		pos.x = rnd.get<float>() * rc.width + rc.x;
		pos.y = rnd.get<float>() * rc.height + rc.y;
		addVertex(pos);
	}
	_assert(m_vertex.begin() == beg);

	for (Vertex& v : m_vertex)
	{
		float mins[4] = { INFINITY, INFINITY , INFINITY , INFINITY };
		Vertex * minsv[4] = { 0 };
		for (Vertex & p : m_vertex)
		{
			if (&v == &p) continue;

			vec2 sub = v.pos - p.pos;
			size_t did = ((sub.y < 0) << 1) | (sub.x < 0);
			float & now = mins[did];
			float dist = sub.length_sq();
			if (dist >= now) continue;
			now = dist;
			minsv[did] = &p;
		}

		float * mindist = mins;
		VertexId vid = (VertexId)(&v - beg);
		for (Vertex * p : minsv)
		{
			if (p == nullptr)
			{
				mindist++;
				continue;
			}

			VertexId pid = (VertexId)(p - beg);
			float dist = sqrtf(*mindist);
			_assert((size_t)pid < m_vertex.size());
			_assert((size_t)vid < m_vertex.size());

			for (Arc & a : v.arcs)
			{
				if (a.to == pid) goto _skip1;
			}
			v.arcs.push(Arc{ vid, pid, dist });
		_skip1:
			for (Arc & a : p->arcs)
			{
				if (a.to == vid) goto _skip2;
			}
			p->arcs.push(Arc{ pid, vid, dist });
		_skip2:
			mindist++;
		}
	}
}
DiGraph::VertexId DiGraph::getRandomVertex() noexcept
{
	return (VertexId)g_random.get<size_t>(m_vertex.size());
}
DiGraph::Vertex * DiGraph::get(VertexId id) noexcept
{
	return &m_vertex[(size_t)id];
}
DiGraph::VertexId DiGraph::addVertex(vec2 pos) noexcept
{
	VertexId id = (VertexId)m_vertex.size();
	Vertex* v = m_vertex.prepare(1);
	v->pos = pos;
	return id;
}
void DiGraph::addArc(VertexId from, VertexId to) noexcept
{
	Vertex* f = get(from);
	Vertex* t = get(to);

	float weight = (t->pos - f->pos).length();
	f->arcs.push(Arc{ from, to , weight });
	t->arcs.push(Arc{ to, from , weight });
}
void DiGraph::draw(ICanvas * draw) noexcept
{
	for (Vertex& v : m_vertex)
	{
		for (Arc& a : v.arcs)
		{
			vec2 fromp = v.pos;
			vec2 top = get(a.to)->pos;
			draw->moveTo(fromp);
			draw->lineTo(top);
			draw->text(TSZ16() << a.weight, (fromp + top) * 0.5 + vec2(-10, -7));
		}
	}
}
void DiGraph::Path::draw(ICanvas * draw) noexcept
{
	Path * p = this;
	Path * last = this;
	draw->drawCross(pos->pos, 5);
	draw->moveTo(pos->pos);
	for (;;)
	{
		last = p;
		if (p->arc == nullptr) break;
		vec2 ppos = p->pos->pos;
		p = p->prev;
		if (p == nullptr) break;
		vec2 pos = p->pos->pos;
		draw->lineTo(pos);
	}

	draw->drawCross(last->pos->pos, 10);
}

Keep<DiGraph::Path> DiGraph::astar(VertexId from, VertexId to) noexcept
{
	Vertex * f = get(from);
	Vertex * t = get(to);

	Keep<Path> first = _new Path;
	first->pos = f;
	first->arc = nullptr;
	first->weight = 0;
	first->wsum = 0;
	if (from == to)
		return first;

	BinArray boolmap(m_vertex.size());
	boolmap.disableAll();

	vec2 dest = t->pos;
	SortedArray<Keep<Path>> arr;
	arr.insert(first);

	boolmap.enable((size_t)from);

	while (!arr.empty())
	{
		Keep<Path> front = arr.removeByIndexGet(0);
		for (Arc &arc : front->pos->arcs)
		{
			if (boolmap.get((size_t)arc.to))
				continue;
			boolmap.enable((size_t)arc.to);

			Keep<Path> npath = _new Path;
			Vertex * now = get(arc.to);
			npath->pos = now;
			npath->prev = front;
			npath->arc = &arc;
			npath->wsum = arc.weight + front->wsum;
			npath->weight = npath->wsum + (now->pos - dest).length();

			if (arc.to == to)
			{
				return npath;
			}

			arr.insert(npath);
		}
	}
	return nullptr;
}
