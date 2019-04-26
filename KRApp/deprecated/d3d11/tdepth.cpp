#define KRGL_PRIVATE
#include "tdepth.h"
#include "3d.h"

#include <unordered_map>

kr::gl::TempDepthTexture * kr::gl::TempDepthTexture::create(int w, int h, int ms) noexcept
{
	Resolution key = {w,h,ms};
	auto res = wnd->m_tempdepth.insert({key, nullptr});
	if (res.second)
	{
		TempDepthTexture * tex = new TempDepthTexture(key);
		tex->create(w,h,ms);
		res.first->second = tex;
	}
	return res.first->second;
}
void kr::gl::TempDepthTexture::AddRef() noexcept
{
	m_ref++;
}
void kr::gl::TempDepthTexture::Release() noexcept
{
	m_ref--;
	if (m_ref == 0) delete this;
}
kr::gl::TempDepthTexture::TempDepthTexture(const Resolution & resol) noexcept
	:DepthTexture(resol.w, resol.h, resol.ms)
{
	m_resol = resol;
	m_ref = 0;
}
kr::gl::TempDepthTexture::~TempDepthTexture() noexcept
{
	wnd->m_tempdepth.erase(m_resol);
}
