#include "image.h"

#include <SDL/SDL_image.h>

using namespace kr;

int WebVariable::s_lastIndex = -1;

WebVariable::WebVariable() noexcept
{
	m_index = -1;
}
WebVariable::~WebVariable() noexcept
{
}

void WebVariable::_allocateIndex() noexcept
{
	if (m_index != -1) return;
	if (!s_freed.empty())
	{
		m_index = s_freed.popGet();
	}
	else
	{
		m_index = ++s_lastIndex;
	}
}
void WebVariable::_freeIndex() noexcept
{
	if (s_lastIndex == m_index)
	{
		m_index--;
		s_lastIndex = -1;
	}
	else
	{
//		s_freed.push(m_index);
	}
}

WebImage * WebImage::load(const char * name)
{
	SDL_Surface* surface = IMG_Load(name);
	if (surface == nullptr)
	{
		throw ErrorMessage(IMG_GetError());
	}
	return (WebImage*)surface;
}
void WebImage::operator delete(void * ptr) noexcept
{
	SDL_FreeSurface((SDL_Surface*)ptr);
}