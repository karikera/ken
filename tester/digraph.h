#pragma once

#include <KR3/main.h>
#include <KR3/math/coord.h>
#include <KR3/data/idmap.h>

class ICanvas
{
public:
	virtual void text(kr::Text16 str, kr::vec2 p) noexcept = 0;
	virtual void moveTo(kr::vec2 p) noexcept = 0;
	virtual void lineTo(kr::vec2 p) noexcept = 0;

	void drawCross(kr::vec2 p, float size) noexcept;
};

class DiGraph
{
	struct VertexId__;

public:
	using VertexId = VertexId__ *;
	struct Vertex;
	struct Path;

	struct Arc
	{
		VertexId from;
		VertexId to;
		float weight;
	};
	struct Vertex
	{
		kr::vec2 pos;
		kr::Array<Arc> arcs;
	};

	struct Path:public kr::Referencable<Path>
	{
		DiGraph::Vertex * pos;
		Arc * arc;
		kr::Keep<Path> prev;
		float wsum;
		float weight;

		void draw(ICanvas * draw) noexcept;
		float getKey() const noexcept;
		float compare(const DiGraph::Path * b) const noexcept;
	};
	
	size_t size() noexcept;
	void generateRandomGraph(kr::Random & rnd, kr::frectwh rc, size_t cnt) noexcept;
	VertexId getRandomVertex() noexcept;
	Vertex * get(VertexId id) noexcept;
	VertexId addVertex(kr::vec2 pos) noexcept;
	void addArc(VertexId from, VertexId to) noexcept;
	void draw(ICanvas * draw) noexcept;
	kr::Keep<Path> astar(VertexId from, VertexId to) noexcept;

private:
	kr::Array<Vertex> m_vertex;
};
