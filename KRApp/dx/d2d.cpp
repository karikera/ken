#ifdef WIN32
#include "dxcommon.h"
#ifdef USE_D2D2 
#undef WINVER
#undef _WIN32_WINNT
#define WINVER 0x602
#define _WIN32_WINNT WINVER
#endif

#include "d2d.h"
#include <KR3/win/com.h>
#include <KRWin/gdi.h>

#include <KR3/win/windows.h>
#ifdef USE_D2D2
#include <wrl.h>
#endif
#include DXGI_HEADER
#include D2D_HEADER

#include <d3d11.h>
#include <dwrite.h>
#ifdef USE_D2D2
#include <dcomp.h>
#endif
// #include <d3d10_1.h>

#include "d3d11/internal.h"
#include "d3d11/texture.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "Windowscodecs.lib")

using namespace kr;
using namespace d2d;

namespace
{
//	Com<ID3D10Device1> s_d3d10;

#ifdef USE_D2D2
	Com<ID2D1Device1> s_d2d;
#endif
	Com<ID2D1FactoryT> s_d2dFactory;
	Com<IDWriteFactory> s_dwriteFactory;

	class TextStroker:public IDWriteTextRenderer
	{
	private:
		unsigned long m_refCount;

	public:
		
		TextStroker():
			m_refCount(0)
		{
		}

		~TextStroker() noexcept 
		{
		}

		STDMETHOD(DrawGlyphRun)(
			void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			DWRITE_MEASURING_MODE measuringMode,
			_In_ DWRITE_GLYPH_RUN const* glyphRun,
			_In_ DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
			IUnknown* clientDrawingEffect
		) override
		{
			IDWriteFontFace * face = glyphRun->fontFace;
			face->GetGlyphRunOutline(
				glyphRun->fontEmSize,
				glyphRun->glyphIndices,
				glyphRun->glyphAdvances,
				glyphRun->glyphOffsets,
				glyphRun->glyphCount,
				glyphRun->isSideways,
				glyphRun->bidiLevel % 2,
				(ID2D1GeometrySink*)clientDrawingContext);
			return S_OK;
		}

		STDMETHOD(DrawUnderline)(
			void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			_In_ DWRITE_UNDERLINE const* underline,
			IUnknown* clientDrawingEffect
			) override
		{
			return S_OK;
		}

		STDMETHOD(DrawStrikethrough)(
			void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			_In_ DWRITE_STRIKETHROUGH const* strikethrough,
			IUnknown* clientDrawingEffect
		) override
		{
			return S_OK;
		}

		STDMETHOD(DrawInlineObject)(
			void* clientDrawingContext,
			FLOAT originX,
			FLOAT originY,
			_In_ IDWriteInlineObject* inlineObject,
			BOOL isSideways,
			BOOL isRightToLeft,
			IUnknown* clientDrawingEffect
		) override
		{
			return S_OK;
		}

		STDMETHOD_(unsigned long,AddRef)() override
		{
			return InterlockedIncrement(&m_refCount);
		}

		STDMETHOD_(unsigned long,Release)() override
		{
			unsigned long newCount = InterlockedDecrement(&m_refCount);
			if (newCount == 0)
			{
				delete this;
				return 0;
			}

			return newCount;
		}

		STDMETHOD(IsPixelSnappingDisabled)(
			void* clientDrawingContext,
			BOOL* isDisabled
		) override
		{
			*isDisabled = FALSE;
			return S_OK;
		}

		STDMETHOD(GetCurrentTransform)(
			void* clientDrawingContext,
			DWRITE_MATRIX* transform
		) override
		{
			// forward the render target's transform
			transform->m11 = 1;
			transform->m12 = 0;
			transform->m21 = 0;
			transform->m22 = 1;
			transform->dx = 0;
			transform->dy = 0;
			return S_OK;
		}
		
		STDMETHOD(GetPixelsPerDip)(
			void* clientDrawingContext,
			FLOAT* pixelsPerDip
		) override
		{
			*pixelsPerDip = 1.f;
			return S_OK;
		}

		STDMETHOD(QueryInterface)(
			IID const& riid,
			void** ppvObject
		) override
		{
			if (__uuidof(IDWriteTextRenderer) == riid)
			{
				*ppvObject = this;
			}
			else if (__uuidof(IDWritePixelSnapping) == riid)
			{
				*ppvObject = this;
			}
			else if (__uuidof(IUnknown) == riid)
			{
				*ppvObject = this;
			}
			else
			{
				*ppvObject = nullptr;
				return E_FAIL;
			}

			this->AddRef();

			return S_OK;
		}

	}; 
}

void kr::d2d::setDeviceContext() noexcept
{
	
}

Direct2D::Init::Init() noexcept
{
	//hrmustbe(D3D10CreateDevice1(d3d11::s_adapter, D3D10_DRIVER_TYPE_HARDWARE, NULL,
	//	D3D10_CREATE_DEVICE_BGRA_SUPPORT | D3D10_CREATE_DEVICE_DEBUG, D3D10_FEATURE_LEVEL_10_1,
	//	D3D10_1_SDK_VERSION, &s_d3d10));
	hrmustbe(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, (ID2D1FactoryT**)&s_d2dFactory));

#ifdef USE_D2D2
	// Create the Direct2D device that links back to the Direct3D device
	hrmustbe(s_d2dFactory->CreateDevice(d3d11::s_dxgiDevice, &s_d2d));
#endif


	// dwrite factory
	hrmustbe(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &s_dwriteFactory));

}
Direct2D::Init::~Init() noexcept
{
	s_dwriteFactory = nullptr;
#ifdef USE_D2D2
	s_d2d = nullptr;
#endif
	s_d2dFactory = nullptr;
}

PushedTransform::PushedTransform(ID2D1RenderTarget * d2d, const mat2p & transform) noexcept
	:d2d(d2d), transform(transform)
{
}
PushedTransform::~PushedTransform() noexcept
{
	d2d->SetTransform((D2D1_MATRIX_3X2_F&)transform);
}

#ifdef USE_D2D2
void ComMethod<IDCompositionDevice>::create() throws(ErrorCode)
{
	hrexcept(DCompositionCreateDevice(
		d3d11::s_dxgiDevice,
		__uuidof(IDCompositionDevice),
		(void**)&ptr()));
}
CompositionTarget ComMethod<IDCompositionDevice>::createTarget(win::Window * wnd, bool topmost)
{
	CompositionTarget target;
	hrexcept(ptr()->CreateTargetForHwnd(wnd,
		topmost,
		&target));
	return target;
}
void ComMethod<IDCompositionDevice>::setContent(IDCompositionTarget * compTarget, IDXGISwapChain1 * swapChain)
{
	Com<IDCompositionVisual> visual;
	hrexcept(ptr()->CreateVisual(&visual));
	hrexcept(visual->SetContent(swapChain));
	hrexcept(compTarget->SetRoot(visual));
	hrexcept(ptr()->Commit());
}
#endif

void ComMethod<ID2D1RenderTarget>::create(IDXGISurface * surface) throws(ErrorCode)
{
#pragma warning(push)
#pragma warning(disable: 4996)
	FLOAT dpiX;
	FLOAT dpiY;
#pragma warning(disable: 4996)
	s_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_IGNORE),
		dpiX, dpiY
	);
#pragma warning(pop)

	hrexcept(s_d2dFactory->CreateDxgiSurfaceRenderTarget(surface, &props, &ptr()));
}
Com<ID3D11Texture2D> ComMethod<ID2D1RenderTarget>::createD3D11Texture(uint width, uint height) throws(ErrorCode)
{
	d3d11::Texture2D d3d11texture(
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 
		d3d11::Usage::Default, DXGI_FORMAT_B8G8R8A8_UNORM, 
		width, height, 1);

	createFromD3D11Texture(d3d11texture);
	return d3d11texture;
	
	//D3D10_TEXTURE2D_DESC tDesc;
	//tDesc.Width = width;
	//tDesc.Height = height;
	//tDesc.MipLevels = 1;
	//tDesc.ArraySize = 1;
	//tDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//tDesc.SampleDesc.Count = 1;
	//tDesc.SampleDesc.Quality = 0;
	//tDesc.Usage = D3D10_USAGE_DEFAULT;

	////EVEN IF YOU WON'T USE AS SHADER RESOURCE, SET THIS BIND FLAGS:
	//tDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
	//tDesc.CPUAccessFlags = 0;
	//tDesc.MiscFlags = D3D10_RESOURCE_MISC_SHARED;

	//Com<ID3D10Texture2D> shareTexture;
	//hrexcept(s_d3d10->CreateTexture2D(&tDesc, NULL, &shareTexture));

	//HANDLE share;
	//{
	//	Com<IDXGISurface> surface;
	//	shareTexture->QueryInterface<IDXGISurface>(&surface);

	//	Com<IDXGIResource> res;
	//	surface->QueryInterface<IDXGIResource>(&res);

	//	res->GetSharedHandle(&share);
	//}

	//{
	//	Com<ID3D11Resource> res;
	//	s_d3d11->OpenSharedResource(share, __uuidof(ID3D11Resource), &res);
	//	res->QueryInterface<ID3D11Texture2D>(texture);
	//}

	//Com<IDXGISurface1> surface;
	//shareTexture->QueryInterface<IDXGISurface1>(&surface);

	//return create(surface);
}
void ComMethod<ID2D1RenderTarget>::createFromD3D11Texture(ID3D11Texture2D * texture) throws(ErrorCode)
{
	Com<IDXGISurface1> surface;
	hrexcept(texture->QueryInterface<IDXGISurface1>(&surface));
	createFromSurface(surface);
}
void ComMethod<ID2D1RenderTarget>::createFromSurface(IDXGISurface * surface) throws(ErrorCode)
{
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		96, 96);
	hrexcept(s_d2dFactory->CreateDxgiSurfaceRenderTarget(surface, &props, &ptr()));
}

void ComMethod<ID2D1RenderTarget>::beginDraw() noexcept
{
	ptr()->BeginDraw();
}
void ComMethod<ID2D1RenderTarget>::endDraw() noexcept
{
	ptr()->EndDraw();
}
void ComMethod<ID2D1RenderTarget>::clear(const vec4 & color) noexcept
{
	ptr()->Clear((D2D1_COLOR_F&)color);
}

void ComMethod<ID2D1RenderTarget>::drawLine(vec2 from, vec2 to, ID2D1Brush * brush, float lineWidth) noexcept
{
	ptr()->DrawLine((D2D1_POINT_2F&)from, (D2D1_POINT_2F&)to, brush, lineWidth);
}
void ComMethod<ID2D1RenderTarget>::strokeRect(const frect & rect, ID2D1Brush * brush, float lineWidth) noexcept
{
	ptr()->DrawRectangle((D2D1_RECT_F&)rect, brush, lineWidth);
}
void ComMethod<ID2D1RenderTarget>::strokeEllipse(const frectwh & rect, ID2D1Brush * brush) noexcept
{
	ptr()->DrawEllipse((D2D1_ELLIPSE&)rect, brush);
}
void ComMethod<ID2D1RenderTarget>::fillRect(const frect & rect, ID2D1Brush * brush) noexcept
{
	ptr()->FillRectangle((D2D1_RECT_F&)rect, brush);
}
void ComMethod<ID2D1RenderTarget>::fillEllipse(const frectwh & rect, ID2D1Brush * brush) noexcept
{
	ptr()->FillEllipse((D2D1_ELLIPSE&)rect, brush);
}
void ComMethod<ID2D1RenderTarget>::fill(ID2D1Geometry * path, ID2D1Brush * brush, ID2D1Brush * opacityBrush) noexcept
{
	ptr()->FillGeometry(path, brush, opacityBrush);
}
void ComMethod<ID2D1RenderTarget>::stroke(ID2D1Geometry * path, ID2D1Brush * brush, float strokeWidth) noexcept
{
	ptr()->DrawGeometry(path, brush, strokeWidth);
}

void ComMethod<ID2D1RenderTarget>::fillText(Text16 text, IDWriteTextFormat * font, vec2 pos, ID2D1Brush * brush) noexcept
{
	fillText(text, font, { pos.x, pos.y, FLT_MAX, FLT_MAX }, brush);
}
void ComMethod<ID2D1RenderTarget>::fillText(Text16 text, IDWriteTextFormat * font, const frect & rect, ID2D1Brush * brush) noexcept
{
	ptr()->DrawTextW(
		wide(text.data()),				// The string to render.
		intact<UINT>(text.size()),		// The string's length.
		font,							// The text format.
		(D2D1_RECT_F&)rect,				// The region of the window where the text will be rendered.
		brush,							// The brush used to draw the text.
#ifdef USE_D2D2
		D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
#else
		D2D1_DRAW_TEXT_OPTIONS_NONE
#endif
	);
}
float ComMethod<ID2D1RenderTarget>::measureText(Text16 text, IDWriteTextFormat * font) noexcept
{
	float textWidth = 0;
	IDWriteTextLayout * layout;
	if (SUCCEEDED(s_dwriteFactory->CreateTextLayout(
		wide(text.data()),			// The string to be laid out and formatted.
		intact<UINT>(text.size()),	// The length of the string.
		font,						// The text format to apply to the string (contains font information, etc).
		FLT_MAX,						// The width of the layout box.
		FLT_MAX,						// The height of the layout box.
		&layout						// The IDWriteTextLayout interface pointer.
	)))
	{
		DWRITE_TEXT_METRICS metrics;
		layout->GetMetrics(&metrics);
		textWidth = metrics.widthIncludingTrailingWhitespace;
		// layout->DetermineMinWidth(&textWidth);
		layout->Release();
	}
	return textWidth;
}
vec2 ComMethod<ID2D1RenderTarget>::measureText(Text16 text, IDWriteTextFormat * font, float width, float height) noexcept
{
	vec2 size = {0, 0};
	IDWriteTextLayout * layout;
	if (SUCCEEDED(s_dwriteFactory->CreateTextLayout(
		wide(text.data()),			// The string to be laid out and formatted.
		intact<UINT>(text.size()),	// The length of the string.
		font,						// The text format to apply to the string (contains font information, etc).
		width,						// The width of the layout box.
		height,						// The height of the layout box.
		&layout						// The IDWriteTextLayout interface pointer.
	)))
	{
		DWRITE_TEXT_METRICS metrics;
		layout->GetMetrics(&metrics);
		layout->Release();
		size.x = metrics.widthIncludingTrailingWhitespace;
		size.y = metrics.height;
	}
	return size;
}
d2d::SolidBrush ComMethod<ID2D1RenderTarget>::createSolidBrush(const vec4 & color) noexcept
{
	SolidBrush brush;
	ptr()->CreateSolidColorBrush(
		(D2D1_COLOR_F&)color,
		&brush);
	return brush;
}
d2d::BitmapBrush ComMethod<ID2D1RenderTarget>::createBitmapBrush(ID2D1Bitmap * bitmap) noexcept
{
	d2d::BitmapBrush brush;
	ptr()->CreateBitmapBrush(
		bitmap,
		&brush);
	return brush;
}
LinearGradientBrush ComMethod<ID2D1RenderTarget>::createLinearGradient(const vec2 &start, const vec2 &end, View<GradientSet> colors) noexcept
{
	static_assert(sizeof(GradientSet) == sizeof(D2D1_GRADIENT_STOP), "gradient stop size unmatch");

	LinearGradientBrush brush;

	// Create an array of gradient stops to put in the gradient stop
	// collection that will be used in the gradient brush.
	Com<ID2D1GradientStopCollection> collection;

	// Create the ID2D1GradientStopCollection from a previously
	// declared array of D2D1_GRADIENT_STOP structs.
	if (FAILED(ptr()->CreateGradientStopCollection(
		(D2D1_GRADIENT_STOP*)colors.data(),
		intact<UINT32>(colors.size()),
		D2D1_GAMMA_2_2,
		D2D1_EXTEND_MODE_CLAMP,
		&collection
	))) return brush;

	// The line that determines the direction of the gradient starts at
	// the upper-left corner of the square and ends at the lower-right corner.
	ptr()->CreateLinearGradientBrush(
		D2D1::LinearGradientBrushProperties(
			(D2D1_POINT_2F&)start,
			(D2D1_POINT_2F&)end),
		collection,
		(ID2D1LinearGradientBrush**)&brush
	);
	return brush;
}
Layer ComMethod<ID2D1RenderTarget>::createLayer() throws(ErrorCode)
{
	Layer layer;
	hrexcept(ptr()->CreateLayer(&layer));
	return layer;
}

void ComMethod<ID2D1RenderTarget>::pushRectClip(const frect & rect) noexcept
{
	ptr()->PushAxisAlignedClip((D2D1_RECT_F&)rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}
void ComMethod<ID2D1RenderTarget>::popRectClip() noexcept
{
	ptr()->PopAxisAlignedClip();
}
void ComMethod<ID2D1RenderTarget>::pushLayer(ID2D1Layer * layer, ID2D1Geometry * path, const math::mat2p & matrix, float opacity, ID2D1Brush * opacityBrush) noexcept
{
	ptr()->PushLayer(
		D2D1::LayerParameters(D2D1::InfiniteRect(), path, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, (D2D1_MATRIX_3X2_F&)matrix, opacity, opacityBrush),
		layer
	);
}
void ComMethod<ID2D1RenderTarget>::pushLayer(ID2D1Layer * layer, float opacity, ID2D1Brush * opacityBrush) noexcept
{
	ptr()->PushLayer(
		D2D1::LayerParameters(D2D1::InfiniteRect(), nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::IdentityMatrix(), opacity, opacityBrush),
		layer
	);
}
void ComMethod<ID2D1RenderTarget>::popLayer() noexcept
{
	ptr()->PopLayer();
}

void ComMethod<ID2D1RenderTarget>::setTransform(const math::mat2p & matrix) noexcept
{
	ptr()->SetTransform((D2D1_MATRIX_3X2_F&)matrix);
}
void ComMethod<ID2D1RenderTarget>::getTransform(math::mat2p * matrix) noexcept
{
	ptr()->GetTransform((D2D1_MATRIX_3X2_F*)matrix);
}
const math::mat2p ComMethod<ID2D1RenderTarget>::getTransform() noexcept
{
	math::mat2p matrix;
	getTransform(&matrix);
	return matrix;
}
d2d::PushedTransform ComMethod<ID2D1RenderTarget>::save() noexcept
{
	return { ptr(), getTransform() };
}
void ComMethod<ID2D1RenderTarget>::translate(vec2 pos) noexcept
{
	mat2p transform = getTransform();
	setTransform(transform.preTranslate(pos));
}
void ComMethod<ID2D1RenderTarget>::scale(float scale) noexcept
{
	mat2p transform = getTransform();
	setTransform(transform.preScale((vec2)scale));
}
void ComMethod<ID2D1RenderTarget>::scale(vec2 scale) noexcept
{
	mat2p transform = getTransform();
	setTransform(transform.preScale(scale));
}
void ComMethod<ID2D1RenderTarget>::rotate(float angle) noexcept
{
	mat2p transform = getTransform();
	setTransform(transform.preRotate(angle));
}
void ComMethod<ID2D1RenderTarget>::drawImage(ID2D1Bitmap * image) noexcept
{
	if (image == nullptr) return;
	ptr()->DrawBitmap(image);
}
void ComMethod<ID2D1RenderTarget>::drawImage(ID2D1Bitmap * image, const frect & rect) noexcept
{
	if (image == nullptr) return;
	ptr()->DrawBitmap(image, (D2D1_RECT_F&)rect);
}
d2d::Bitmap ComMethod<ID2D1RenderTarget>::createSharedBitmap(IDXGISurface * surface) throws(ErrorCode)
{
	d2d::Bitmap bitmap;

	D2D1_BITMAP_PROPERTIES properties = {};
	properties.dpiX = 96;
	properties.dpiY = 96;
	properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	hrexcept(ptr()->CreateSharedBitmap(__uuidof(IDXGISurface), surface, &properties, &bitmap));
	return bitmap;
}
Bitmap ComMethod<ID2D1RenderTarget>::loadImage(pcstr16 filename) noexcept
{
	try
	{
		WICBitmapDecoder decoder(filename);
		return loadImage(decoder.getFrame(0));
	}
	catch (ErrorCode &err)
	{
		udout << err.getMessage<char16>() << endl;
		udout.flush();
		debug();
		return Bitmap();
	}
}
d2d::Bitmap ComMethod<ID2D1RenderTarget>::loadImage(IWICBitmapSource* source) noexcept
{
	Bitmap bitmap;
	try
	{
		hrexcept(ptr()->CreateBitmapFromWicBitmap(source, nullptr, &bitmap));
	}
	catch (ErrorCode &err)
	{
		udout << err.getMessage<char16>() << endl;
		udout.flush();
		debug();
	}
	return bitmap;
}

void ComMethod<ID2D1HwndRenderTarget>::create(win::Window * win) throws(ErrorCode)
{
	remove();

	irect size = win->getClientRect();

	hrexcept(s_d2dFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(win, { (UINT32)size.right, (UINT32)size.bottom }),
		&ptr()
	));
}
void ComMethod<ID2D1DCRenderTarget>::create(win::DrawContext * dc, const irect & subrect) throws(ErrorCode)
{
	remove();

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(
			DXGI_FORMAT_B8G8R8A8_UNORM,
			D2D1_ALPHA_MODE_IGNORE),
		0,
		0,
		D2D1_RENDER_TARGET_USAGE_NONE,
		D2D1_FEATURE_LEVEL_DEFAULT
	);
	hrexcept(s_d2dFactory->CreateDCRenderTarget(
		&props,
		&ptr()
	));
	ptr()->BindDC(dc, (RECT*)&subrect);
}
#ifdef USE_D2D2
void ComMethod<ID2D1DeviceContext>::create() throws(ErrorCode)
{
	// Create the Direct2D device context that is the actual render target
	// and exposes drawing commands
	hrexcept(s_d2d->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &ptr()));
}
d2d::Bitmap ComMethod<ID2D1DeviceContext>::createBitmapFromSurface(IDXGISurface * surface) throws(ErrorCode)
{
	Bitmap1 bitmap;
	hrexcept(ptr()->CreateBitmapFromDxgiSurface(surface, nullptr, &bitmap));
	return bitmap;
}
d2d::EffectColorMatrix ComMethod<ID2D1DeviceContext>::createEffectColorMatrix() throws(ErrorCode)
{
	EffectColorMatrix effect;
	hrexcept(ptr()->CreateEffect(CLSID_D2D1ColorMatrix, &effect));
	return effect;
}
d2d::Effect ComMethod<ID2D1DeviceContext>::createEffectComposite() throws(ErrorCode)
{
	Effect effect;
	hrexcept(ptr()->CreateEffect(CLSID_D2D1Composite, &effect));
	return effect;
}
#endif

void ComMethod<ID2D1SolidColorBrush>::setColor(const vec4 & color) noexcept
{
	ptr()->SetColor((D2D1_COLOR_F&)color);
}
const vec4 ComMethod<ID2D1SolidColorBrush>::getColor() noexcept
{
	return (vec4&)ptr()->GetColor();
}

ComMethod<IDWriteTextFormat>::ComMethod() noexcept
{
}
ComMethod<IDWriteTextFormat>::ComMethod(pcstr16 familyName, float fontSize, Weight weight, Style style, Stretch stretch, pcstr16 locale) noexcept
{
	ptr() = nullptr;
	create(familyName, fontSize, weight, style, stretch, locale);
}

void ComMethod<IDWriteTextFormat>::create(pcstr16 familyName, float fontSize, Weight weight, Style style, Stretch stretch, pcstr16 locale) noexcept
{
	remove();
	s_dwriteFactory->CreateTextFormat(
		wide(familyName),                // Font family name.
		NULL,                       // Font collection (NULL sets it to use the system font collection).
		(DWRITE_FONT_WEIGHT)weight,
		(DWRITE_FONT_STYLE)DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		fontSize,
		wide(locale),
		&ptr()
	);
}
void ComMethod<IDWriteTextFormat>::setTextAlign(Align align) noexcept
{
	static_assert((DWRITE_TEXT_ALIGNMENT)Align::Leading == DWRITE_TEXT_ALIGNMENT_LEADING, "text align const unmatch");
	static_assert((DWRITE_TEXT_ALIGNMENT)Align::Trailing == DWRITE_TEXT_ALIGNMENT_TRAILING, "text align const unmatch");
	static_assert((DWRITE_TEXT_ALIGNMENT)Align::Center == DWRITE_TEXT_ALIGNMENT_CENTER, "text align const unmatch");

	if (ptr())
	{
		ptr()->SetTextAlignment((DWRITE_TEXT_ALIGNMENT)align);
	}
}
void ComMethod<IDWriteTextFormat>::setParagraphAlign(Paragraph paragraph) noexcept
{
	static_assert((DWRITE_PARAGRAPH_ALIGNMENT)Paragraph::Near == DWRITE_PARAGRAPH_ALIGNMENT_NEAR, "paragraph align const unmatch");
	static_assert((DWRITE_PARAGRAPH_ALIGNMENT)Paragraph::Far == DWRITE_PARAGRAPH_ALIGNMENT_FAR, "paragraph align const unmatch");
	static_assert((DWRITE_PARAGRAPH_ALIGNMENT)Paragraph::Center == DWRITE_PARAGRAPH_ALIGNMENT_CENTER, "paragraph align const unmatch");

	if (ptr())
	{
		ptr()->SetParagraphAlignment((DWRITE_PARAGRAPH_ALIGNMENT)paragraph);
	}
}
float ComMethod<IDWriteTextFormat>::getHeight() noexcept
{
	return ptr()->GetFontSize();
}

void ComMethod<ID2D1PathGeometry>::create()  noexcept
{
	remove();
	s_d2dFactory->CreatePathGeometry(&ptr());
}
PathMake ComMethod<ID2D1PathGeometry>::open() noexcept
{
	PathMake make;
	if (ptr())
	{
		ptr()->Open(&make);
	}
	return make;
}
void ComMethod<ID2D1GeometrySink>::begin(vec2 pos, bool hollow) noexcept
{
	if (ptr() == nullptr) return;
	ptr()->BeginFigure((D2D1_POINT_2F&)pos, (D2D1_FIGURE_BEGIN)hollow);
}
void ComMethod<ID2D1GeometrySink>::lineTo(vec2 pos) noexcept
{
	if (ptr() == nullptr) return;
	ptr()->AddLine((D2D1_POINT_2F&)pos);
}
void ComMethod<ID2D1GeometrySink>::lineTo(View<vec2> pos) noexcept
{
	if (ptr() == nullptr) return;
	ptr()->AddLines((const D2D1_POINT_2F*)pos.data(), intact<UINT>(pos.size()));
}
void ComMethod<ID2D1GeometrySink>::arcTo(const vec2 & pos, const vec2 & radius, float rotationAngle, bool clockWise, bool large) noexcept
{
	if (ptr() == nullptr) return;
	D2D1_ARC_SEGMENT arc;
	(vec2&)arc.point = pos;
	(vec2&)arc.size = radius;
	arc.rotationAngle = rotationAngle;
	arc.sweepDirection = (D2D1_SWEEP_DIRECTION)clockWise;
	arc.arcSize = (D2D1_ARC_SIZE)large;
	ptr()->AddArc(&arc);
}
void ComMethod<ID2D1GeometrySink>::arcTo(const vec2& center, float r, float from, float sweep) noexcept
{
	if (ptr() == nullptr) return;
	if (sweep == 0.f) return;

	D2D1_ARC_SEGMENT arc;
	arc.size.width = r;
	arc.size.height = r;
	arc.rotationAngle = 0.f;
	arc.sweepDirection = D2D1_SWEEP_DIRECTION_CLOCKWISE;
	arc.arcSize = D2D1_ARC_SIZE_SMALL;
	
	vec2 radius = { r, r };
	from = math::radianmod(from);
	if (sweep >= math::tau || sweep <= -math::tau)
	{
		arc.point.y = center.y;
		vec2 arcTo = vec2::direction_z(from) * r + center;

		if (from < 0.f)
		{
			arc.point.x = center.x + r;
			ptr()->AddArc(&arc);
			if (arcTo.y < center.y)
			{
				arc.arcSize = D2D1_ARC_SIZE_LARGE;
			}
		}
		else
		{
			arc.point.x = center.x - r;
			ptr()->AddArc(&arc);
			if (arcTo.y > center.y)
			{
				arc.arcSize = D2D1_ARC_SIZE_LARGE;
			}
		}
		(vec2&)arc.point = arcTo;
		ptr()->AddArc(&arc);
	}
	else
	{
		bool clockwise = sweep > 0.f;
		sweep += from;
		vec2 arcTo = vec2::direction_z(sweep) * r + center;
		if (clockwise)
		{
			if (from < 0.f)
			{
				if (sweep > 0)
				{
					arc.point.x = center.x + r;
					ptr()->AddArc(&arc);
					if (arcTo.y < center.y)
					{
						arc.arcSize = D2D1_ARC_SIZE_LARGE;
					}
				}
			}
			else
			{
				if (sweep > math::pi)
				{
					arc.point.x = center.x - r;
					ptr()->AddArc(&arc);
					if (arcTo.y > center.y)
					{
						arc.arcSize = D2D1_ARC_SIZE_LARGE;
					}
				}
			}
		}
		else
		{
			if (from < 0.f)
			{
				if (sweep < -math::pi)
				{
					arc.point.x = center.x - r;
					ptr()->AddArc(&arc);
					if (arcTo.y < center.y)
					{
						arc.arcSize = D2D1_ARC_SIZE_LARGE;
					}
				}
			}
			else
			{
				if (sweep < 0)
				{
					arc.point.x = center.x + r;
					ptr()->AddArc(&arc);
					if (arcTo.y > center.y)
					{
						arc.arcSize = D2D1_ARC_SIZE_LARGE;
					}
				}
			}
		}

		(vec2&)arc.point = arcTo;
		ptr()->AddArc(&arc);
	}
}
void ComMethod<ID2D1GeometrySink>::bezierTo(const vec2& p1, const vec2& p2, const vec2& to) noexcept
{
	bezierTo({p1, p2, to});
}
void ComMethod<ID2D1GeometrySink>::bezierTo(const Bezier & bezier) noexcept
{
	static_assert(sizeof(D2D1_BEZIER_SEGMENT) == sizeof(Bezier), "Bezier segment size unmatch");
	ptr()->AddBezier((D2D1_BEZIER_SEGMENT&)bezier);
}
void ComMethod<ID2D1GeometrySink>::beziers(View<Bezier> beziers) noexcept
{
	ptr()->AddBeziers((D2D1_BEZIER_SEGMENT*)beziers.data(), intact<UINT>(beziers.size()));
}
void ComMethod<ID2D1GeometrySink>::quadraticTo(const vec2& p, const vec2& to) noexcept
{
	quadraticTo({p, to});
}
void ComMethod<ID2D1GeometrySink>::quadraticTo(const Quadratic& bezier) noexcept
{
	static_assert(sizeof(D2D1_QUADRATIC_BEZIER_SEGMENT) == sizeof(Quadratic), "Quadratic segment size unmatch");
	ptr()->AddQuadraticBezier((D2D1_QUADRATIC_BEZIER_SEGMENT&)bezier);
}
void ComMethod<ID2D1GeometrySink>::quadratics(View<Quadratic> beziers) noexcept
{
	ptr()->AddQuadraticBeziers((D2D1_QUADRATIC_BEZIER_SEGMENT*)beziers.data(), intact<UINT>(beziers.size()));
}
void ComMethod<ID2D1GeometrySink>::end(bool close) noexcept
{
	if (ptr() == nullptr) return;
	ptr()->EndFigure((D2D1_FIGURE_END)close);
}
void ComMethod<ID2D1GeometrySink>::rect(const frect & rect) noexcept
{
	begin(rect.from);
	lineTo({
		{ rect.right, rect.top },
		rect.to,
		{ rect.left, rect.bottom }
	});
	end(true);
}
void ComMethod<ID2D1GeometrySink>::text(Text16 text, IDWriteTextFormat * font) noexcept
{
	static_assert(sizeof(char32) == sizeof(UINT), "character size unmatch");

	IDWriteTextLayout * layout;
	if (SUCCEEDED(s_dwriteFactory->CreateTextLayout(
		wide(text.data()),
		intact<UINT>(text.size()),
		font,
		FLT_MAX,
		FLT_MAX,
		&layout
	)))
	{
		TextStroker stroker;
		stroker.AddRef();
		layout->Draw(ptr(), &stroker, 0, 0);
		layout->Release();
	}
}

void ComMethod<ID2D1GeometrySink>::close() noexcept
{
	if (ptr() == nullptr) return;
	ptr()->Close();
}

const vec2 ComMethod<ID2D1Bitmap>::getSize() noexcept
{
	if (ptr() == nullptr) return { 1, 1 };
	return (vec2&)ptr()->GetSize();
}

#ifdef USE_D2D2
void ComMethod<ID2D1Effect>::setInput(uint32_t index, ID2D1Image * image, bool invalidate) noexcept
{
	ptr()->SetInput(index, image, invalidate);
}
void EffectColorMatrix::setColorMatrix(float matrix[20]) noexcept
{
	HRESULT hr = ptr()->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, *(D2D1_MATRIX_5X4_F*)matrix);
	_assert(SUCCEEDED(hr));
}
#endif

#endif