#pragma once

#include "common.h"
#include "digraph.h"
#include <KRApp/2d.h>

class Wnd : public WebCanvas2D, public ICanvas
{
private:
	DiGraph m_graph;
	Array<Keep<DiGraph::Path>> m_path;
	float m_profile;
	float m_sum[3];
	float m_count[3];


public:

	Wnd() noexcept;
	~Wnd() noexcept;
	void clearPath() noexcept;

	void render() noexcept;
	void text(Text16 tx, vec2 pos) noexcept override;
	void moveTo(vec2 pos) noexcept override;
	void lineTo(vec2 pos) noexcept override;
	void onKeyDown(int key, bool repeat) noexcept override;
};
