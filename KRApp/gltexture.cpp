#include <KR3/http/fetch.h>
#include <KR3/util/path.h>

#include "gltexture.h"
#include "glutil.h"
#include <K2D/canvas.h>
#include <KR3/msg/pool.h>

using namespace kr;

gl::Texture::Texture(nullptr_t) noexcept {
	m_id = 0;
}
void gl::Texture::generate() noexcept {
	glCheck();
	glGenTextures(1, &m_id);
	glCheck();
}
void gl::Texture::remove() noexcept {
	glCheck();
	glDeleteTextures(1, &m_id);
	glCheck();
}
Promise<void>* gl::Texture::load(Text16 filename, bool potResize) noexcept {
	return TextureData::load(filename, potResize)->then([this](const TextureData& image) {
		load(image);
	});
}
void gl::Texture::load(const image::ImageData& image) noexcept {
	generate();
	texture2D = *this;
	texture2D.image2D(image);
	texture2D.minFilter(GL_LINEAR);
	texture2D.magFilter(GL_LINEAR);
}
void gl::Texture::load(const TextureData& image) noexcept {
	generate();
	texture2D = *this;
	texture2D.image2D(image);
	texture2D.minFilter(GL_LINEAR);
	texture2D.magFilter(GL_LINEAR);
}

void gl::TextureData::image2D(GLenum target) const noexcept {
	glCheck();
	glTexImage2D(target, 0, format, getWidth(), getHeight(), 0, format, GL_UNSIGNED_BYTE, getBits());
	glCheck();
}
void gl::TextureData::subImage2D(GLenum target, int xoffset, int yoffset) const noexcept {
	glCheck();
	glTexSubImage2D(target, 0, xoffset, yoffset, getWidth(), getHeight(), format, GL_UNSIGNED_BYTE, getBits());
	glCheck();
}
void gl::TextureData::free() noexcept {
	delete[](byte*)m_image;
	m_image = nullptr;
}
gl::TextureData gl::TextureData::subimage(int _x, int _y, int _w, int _h) const noexcept {
	return gl::TextureData(ImageData::subimage(_x, _y, _w, _h), false);
}
Promise<gl::TextureData>* gl::TextureData::load(Text16 filename, bool potResize) noexcept {
	GLContext* ctx = GLContext::getInstance();

	return threading([filenamea = (AText16)filename, potResize, ctx]{
		ctx->shareContant();
		image::ImageData image;
		if (!image.loadFileSync(filenamea, nullptr)) {
			image = nullptr;
		}
		return TextureData(move(image), potResize);
	});
}

gl::TextureData::TextureData() noexcept
	:image::ImageData(nullptr){
	m_allocated = false;
}
gl::TextureData::TextureData(const image::ImageData& image, bool potResize) noexcept
	:TextureData(image, image::getFormatInfo(image.getPixelFormat())->hasAlpha, potResize){
}
gl::TextureData::TextureData(const image::ImageData& image, bool useAlpha, bool potResize) noexcept {
	image::ImageData temp = nullptr;
	const image::ImageData* result;

	int max = GLContext::getInstance()->maxTextureSize;
	int w = image.getWidth();
	int h = image.getHeight();

	bool needToResize = potResize;
	if (needToResize) {
		int nw = math::pow2ceil(w);
		if (nw > max) nw = max;
		int nh = math::pow2ceil(h);
		if (nh > max) nh = max;
		needToResize = nw != w || nh != h;
		w = nw;
		h = nh;
	}

	image::PixelFormat pf;
	image::ImageData* dest = needToResize ? &temp : this;

	int pitchBytes;
	if (useAlpha) {
		pf = PixelFormatABGR8;
		pitchBytes = image.getWidth() * 4;
		format = GL_RGBA;
	}
	else {
		pf = PixelFormatBGR8;
		pitchBytes = image.getWidth() * 3;
		pitchBytes = ((pitchBytes - 1) | 0x3) + 1;
		format = GL_RGB;
	}

	result = image::reformat(dest, &image, pf, pitchBytes);
	if (needToResize) {
		allocate(pf, w, h);
		copyStretch(result);
		m_allocated = true;
	}
	else if (result == dest) { // reformatted
		if (result != this) { // move from temp
			*this = *result;
			temp = nullptr;
		}
		m_allocated = true;
	}
	else { // same format
		*this = *result;
		m_allocated = false;
	}
	temp.free();
}
gl::TextureData::TextureData(image::ImageData&& image, bool resize) noexcept
	:TextureData(image, resize){
	if (m_allocated) {
		image.free();
	}
	image = nullptr;
}
gl::TextureData::~TextureData() noexcept {
	if (m_allocated) {
		free();
	}
}
gl::TextureData::TextureData(TextureData&& data) noexcept {
	memcpy(this, &data, sizeof(TextureData));
	data = nullptr;
}
