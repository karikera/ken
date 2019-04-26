#include <KRUtil/fetch.h>
#include <KRUtil/fs/path.h>

#include "gltexture.h"
#include <K2D/canvas.h>

using namespace kr;

gl::Texture::Texture(nullptr_t) noexcept
{
	m_id = 0;
}
void gl::Texture::generate() noexcept
{
	glGenTextures(1, &m_id);
}
void gl::Texture::remove() noexcept
{
	glDeleteTextures(1, &m_id);
}
Promise<void>* gl::Texture::load(Text16 filename) noexcept
{
	krb::Extension ext = krb::makeExtensionFromPath(filename);
	return fetchAsText(filename)->then([this, ext](AText & data) {
		kr::Buffer databuf = data.cast<void>();
		image::ImageData image;
		if (image.load(&databuf, ext, nullptr))
		{
			generate();
			texture2D = *this;
			texture2D.image2D(&image);
			texture2D.minFilter(GL_LINEAR);
			texture2D.magFilter(GL_LINEAR);
			image.free();
		}
	});
}

void gl::krTexImage2D(GLenum target, const image::ImageData * img) noexcept
{
	image::ImageData temp = nullptr;
	image::ImageData temp2 = nullptr;

	const image::ImageData * result;
	GLint iformat;
	GLint format;
	GLint formatType;

	GLint max;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);

	int w = img->getWidth();
	int h = img->getHeight();
	int nw = math::pow2ceil(w);
	if (nw > max) nw = max;
	int nh = math::pow2ceil(h);
	if (nh > max) nh = max;

	image::PixelFormat pf;

	if (image::getFormatInfo(img->getPixelFormat())->hasAlpha)
	{
		pf = PixelFormatABGR8;
		result = image::reformat(&temp, img, pf);
		iformat = GL_RGBA;
		format = GL_RGBA;
		formatType = GL_UNSIGNED_BYTE;
	}
	else
	{
		pf = PixelFormatBGR8;
		int pitchBytes = img->getWidth() * 3;
		result = image::reformat(&temp, img, pf, ((pitchBytes - 1) | 0x3) + 1);
		iformat = GL_RGB;
		format = GL_RGB;
		formatType = GL_UNSIGNED_BYTE;
	}

	if (nw != w || nh != h)
	{
		temp2.allocate(pf, nw, nh);
		temp2.copyStretch(result);
		result = &temp2;
	}
	
	glTexImage2D(target, 0, iformat, result->getWidth(), result->getHeight(), 0, format, formatType, result->getBits());
	temp.free();
	temp2.free();
}
