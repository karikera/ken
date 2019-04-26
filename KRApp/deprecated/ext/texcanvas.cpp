#include "texcanvas.h"
#include <KR3/mem/text.h>
#include <cstdlib>
#include <cstdint>

kr::gl::TextureCanvas::TextureCanvas(int width, int height) noexcept
	:ImageCanvas(width, height), m_texture(kr::gl::Texture::RGBA, width, height)
{
	m_width = width;
	m_height = height;
	m_texture.setSampleMode(SamplerState::Linear | SamplerState::Clamp);
}
void kr::gl::TextureCanvas::use(int slot) noexcept
{
	void * dest = m_texture.lock();
	if (dest != nullptr)
	{
		getBits(dest);
		m_texture.unlock();
	}
	m_texture.use(slot);
}
