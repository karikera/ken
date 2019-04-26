#pragma once

#include <KR3/main.h>
#include <KR3/math/coord.h>
#include <KRUtil/wl/com.h>
#include <KRWin/handle.h>

#include "gi.h"
#include "dxcommon.h"
#include "wic.h"

#define WS_EX_NOREDIRECTIONBITMAP 0x00200000L

struct ID2D1RenderTarget;
struct ID2D1DCRenderTarget;
struct ID2D1HwndRenderTarget;
struct ID3D11Texture2D;
struct IWICBitmapSource;

struct ID2D1Brush;
struct ID2D1SolidColorBrush;
struct ID2D1BitmapBrush;
struct ID2D1LinearGradientBrush;
struct ID2D1Geometry;
struct ID2D1PathGeometry;
struct ID2D1GeometrySink;
struct ID2D1Layer;
struct ID2D1Bitmap;
struct IDWriteTextFormat;
struct IDXGISurface;
struct ID3D11Texture2D;
#ifdef USE_D2D2
struct ID2D1Bitmap1;
struct ID2D1Effect;
#endif
struct ID2D1Image;

namespace kr
{
	COM_INHERIT(ID2D1HwndRenderTarget, ID2D1RenderTarget);
	COM_INHERIT(ID2D1DCRenderTarget, ID2D1RenderTarget);
	COM_INHERIT(ID2D1SolidColorBrush, ID2D1Brush);
	COM_INHERIT(ID2D1LinearGradientBrush, ID2D1Brush);
	COM_INHERIT(ID2D1PathGeometry, ID2D1Geometry);
#ifdef USE_D2D2
	COM_CLASS_DECLARE(ID2D1Effect);
	COM_INHERIT(ID2D1DeviceContext, ID2D1RenderTarget);
	COM_INHERIT(ID2D1Bitmap1, ID2D1Bitmap);
#endif

	namespace d2d
	{
		using WindowRenderTarget = Com<ID2D1HwndRenderTarget>;
		using DCRenderTarget = Com<ID2D1DCRenderTarget>;
		using RenderTarget = Com<ID2D1RenderTarget>;
		using Brush = Com<ID2D1Brush>;
		using SolidBrush = Com<ID2D1SolidColorBrush>;
		using BitmapBrush = Com<ID2D1BitmapBrush>;
		using LinearGradientBrush = Com<ID2D1LinearGradientBrush>;
		using Font = Com<IDWriteTextFormat>;
		using Path = Com<ID2D1PathGeometry>;
		using PathMake = Com<ID2D1GeometrySink>;
		using Layer = Com<ID2D1Layer>;
		using Bitmap = Com<ID2D1Bitmap>;
#ifdef USE_D2D2
		using DeviceContext = Com<ID2D1DeviceContext>;
		using CompositionTarget = Com<IDCompositionTarget>;
		using CompositionDevice = Com<IDCompositionDevice>;
		using Bitmap1 = Com<ID2D1Bitmap1>;
		using Effect = Com<ID2D1Effect>;
#endif

		struct Direct2D
		{
			struct Init
			{
				Init() noexcept;
				~Init() noexcept;
			};
		};

		struct GradientSet
		{
			float position;
			vec4 color;
		};

		class PushedTransform
		{
		public:
			ID2D1RenderTarget * d2d;
			mat2p transform;

			PushedTransform(ID2D1RenderTarget * d2d, const mat2p & transform) noexcept;
			~PushedTransform() noexcept;
		};

		void setDeviceContext() noexcept;

		class EffectColorMatrix;
	}
	
#ifdef USE_D2D2
	COM_CLASS_METHODS(IDCompositionTarget)
	{
	};
#endif

	COM_CLASS_METHODS(ID2D1RenderTarget)
	{
		void create(IDXGISurface * surface) throws(ErrorCode);
		Com<ID3D11Texture2D> createD3D11Texture(uint width, uint height) throws(ErrorCode);
		void createFromD3D11Texture(ID3D11Texture2D * texture) throws(ErrorCode);
		void createFromSurface(IDXGISurface * surface) throws(ErrorCode);
		
		void beginDraw() noexcept;
		void endDraw() noexcept;
		void clear(const vec4 & color) noexcept;

		void drawLine(vec2 from, vec2 to, ID2D1Brush * brush, float lineWidth = 1.f) noexcept;
		void strokeRect(const frect & rect, ID2D1Brush * brush, float lineWidth = 1.f) noexcept;
		void strokeEllipse(const frectwh & rect, ID2D1Brush * brush) noexcept;
		void fillRect(const frect & rect, ID2D1Brush * brush) noexcept;
		void fillEllipse(const frectwh & rect, ID2D1Brush * brush) noexcept;
		void fill(ID2D1Geometry * path, ID2D1Brush * brush, ID2D1Brush * opacityBrush = nullptr) noexcept;
		void stroke(ID2D1Geometry * path, ID2D1Brush * brush, float strokeWidth = 1.f) noexcept;

		void fillText(Text16 text, IDWriteTextFormat * font, vec2 pos, ID2D1Brush * brush) noexcept;
		void fillText(Text16 text, IDWriteTextFormat * font, const frect & rect, ID2D1Brush * brush) noexcept;
		float measureText(Text16 text, IDWriteTextFormat * font) noexcept;
		vec2 measureText(Text16 text, IDWriteTextFormat * font, float width, float height = FLT_MAX) noexcept;

		d2d::SolidBrush createSolidBrush(const vec4 & color) noexcept;
		d2d::BitmapBrush createBitmapBrush(ID2D1Bitmap * bitmap) noexcept;
		d2d::LinearGradientBrush createLinearGradient(const vec2 &start, const vec2 &end, View<d2d::GradientSet> colors) noexcept;
		d2d::Layer createLayer() throws(ErrorCode);

		void pushRectClip(const frect & rect) noexcept;
		void popRectClip() noexcept;
		void pushLayer(ID2D1Layer * layer, ID2D1Geometry * path, const math::mat2p & matrix = math::mat2p::identity(), float opacity = 1.f, ID2D1Brush * opacityBrush = nullptr) noexcept;
		void pushLayer(ID2D1Layer * layer, float opacity, ID2D1Brush * opacityBrush = nullptr) noexcept;
		void popLayer() noexcept;

		void setTransform(const math::mat2p & matrix) noexcept;
		void getTransform(math::mat2p * matrix) noexcept;
		const math::mat2p getTransform() noexcept;
		d2d::PushedTransform save() noexcept;
		void translate(vec2 pos) noexcept;
		void scale(float scale) noexcept;
		void scale(vec2 scale) noexcept;
		void rotate(float angle) noexcept;

		void drawImage(ID2D1Bitmap * image) noexcept;
		void drawImage(ID2D1Bitmap * image, const frect & rect) noexcept;
		d2d::Bitmap createSharedBitmap(IDXGISurface * surface) throws(ErrorCode);
		d2d::Bitmap loadImage(pcstr16 filename) noexcept;
		d2d::Bitmap loadImage(IWICBitmapSource* source) noexcept;
	};

	COM_CLASS_METHODS(ID2D1DCRenderTarget)
	{
		void create(win::DrawContext * dc, const irect & subrect) throws(ErrorCode);
	};

	COM_CLASS_METHODS(ID2D1HwndRenderTarget)
	{
		void create(win::Window * win) throws(ErrorCode);
	};

#ifdef USE_D2D2
	COM_CLASS_METHODS(ID2D1DeviceContext)
	{
		void create() throws(ErrorCode);
		d2d::Bitmap createBitmapFromSurface(IDXGISurface * surface) throws(ErrorCode);
		d2d::EffectColorMatrix createEffectColorMatrix() throws(ErrorCode);
		d2d::Effect createEffectComposite() throws(ErrorCode);
	};

	COM_CLASS_METHODS(IDCompositionDevice)
	{
		void create() throws(ErrorCode);
		d2d::CompositionTarget createTarget(win::Window * wnd, bool topmost = false);
		void setContent(IDCompositionTarget * compTarget, IDXGISwapChain1 * swapChain);
	};
#endif

	COM_CLASS_METHODS(ID2D1SolidColorBrush)
	{
		void setColor(const vec4 & color) noexcept;
		const vec4 getColor() noexcept;
	};

	COM_CLASS_METHODS(IDWriteTextFormat)
	{
		enum class Weight
		{
			Thin = 100,
			ExtraLight = 200,
			Light = 300,
			SemiLight = 350,
			Normal = 400,
			Medium = 500,
			SemiBold = 600,
			Bold = 700,
			ExtraBold = 800,
			Black = 900,
			ExtraBlack = 950,
		};
		enum class Style
		{
			Normal,
			Oblique,
			Italic
		};
		enum class Stretch
		{
			Undefined = 0,
			UltraCondensed = 1,
			ExtraCondensed = 2,
			Condensed = 3,
			SemiCondensed = 4,
			Normal = 5,
			SemiExpanded = 6,
			Expanded = 7,
			ExtraExpanded = 8,
			UltraExpanded = 9
		};

		enum class Align
		{
			Leading,
			Trailing,
			Center
		};
		enum class Paragraph
		{
			Near,
			Far,
			Center
		};

		ComMethod() noexcept;
		ComMethod(pcstr16 familyName, float fontSize, Weight weight = Weight::Normal, Style style = Style::Normal, Stretch stretch = Stretch::Normal, pcstr16 locale = u"ko-kr") noexcept;

		void create(pcstr16 familyName, float fontSize, Weight weight = Weight::Normal, Style style = Style::Normal, Stretch stretch = Stretch::Normal, pcstr16 locale = u"ko-kr") noexcept;
		void setTextAlign(Align align) noexcept;
		void setParagraphAlign(Paragraph paragraph) noexcept;
		float getHeight() noexcept;
	};
	
	COM_CLASS_METHODS(ID2D1PathGeometry)
	{
		void create() noexcept;
		d2d::PathMake open() noexcept;
	};

	COM_CLASS_METHODS(ID2D1GeometrySink)
	{
		struct Bezier
		{
			vec2 p1;
			vec2 p2;
			vec2 to;
		};
		struct Quadratic
		{
			vec2 p;
			vec2 to;
		};

		void begin(vec2 pos, bool hollow = false) noexcept;
		void lineTo(vec2 pos) noexcept;
		void lineTo(View<vec2> pos) noexcept;
		void arcTo(const vec2 & pos, const vec2 & radius, float rotationAngle = 0.f, bool clockWise = true, bool large = false) noexcept;
		void arcTo(const vec2& center, float r, float from, float sweep) noexcept;
		void bezierTo(const vec2& p1, const vec2& p2, const vec2& to) noexcept;
		void bezierTo(const Bezier& bezier) noexcept;
		void beziers(View<Bezier> beziers) noexcept;
		void quadraticTo(const vec2& p, const vec2& to) noexcept;
		void quadraticTo(const Quadratic& bezier) noexcept;
		void quadratics(View<Quadratic> beziers) noexcept;
		void end(bool close) noexcept;
		void rect(const frect & rect) noexcept;
		void text(Text16 text, IDWriteTextFormat * font) noexcept;
		void close() noexcept;
	};

	COM_CLASS_METHODS(ID2D1Bitmap)
	{
		const vec2 getSize() noexcept;
		void setValue() throws(ErrorCode);
	};

#ifdef USE_D2D2
	enum class CompositeMode
	{
		SOURCE_OVER,		// O = S + (1 SA) * D				Union of source and destination bitmaps
		DESTINATION_OVER,	// O = (1 DA) * S + D				Union of source and destination bitmaps
		SOURCE_IN,			// O = DA * S						Intersection of source and destination bitmaps
		DESTINATION_IN,		// O = SA * D						Intersection of source and destination bitmaps
		SOURCE_OUT,			// O = (1 - DA) * S					Region of the source bitmap
		DESTINATION_OUT,	// O = (1 - SA) * D					Region of the destination bitmap
		SOURCE_ATOP,		// O = DA * S + (1 - SA) * D		Region of the destination bitmap
		DESTINATION_ATOP,	// O = (1 - DA) * S + SA * D		Region of the source bitmap
		XOR,				// O = (1 - DA) * S + (1 - SA) * D	Union of source and destination bitmaps
		PLUS,				// O = S + D						Union of source and destination bitmaps
		SOURCE_COPY,		// O = S							Region of the source bitmap
		BOUNDED_SOURCE_COPY, // O = S(only where source exists)	Union of source and destination bitmaps.Destination is not overwritten where the source doesn't exist.
		MASK_INVERT,		// O = (1 D) * S + (1 SA) * D		Union of source and destination bitmaps.The alpha values are unchanged.
	};

	COM_CLASS_METHODS(ID2D1Effect)
	{
		void setInput(uint32_t index, ID2D1Image * image, bool invalidate = true) noexcept;
	};
	namespace d2d
	{
		class EffectColorMatrix :public Effect
		{
		public:
			void setColorMatrix(float matrix[20]) noexcept;
		};
	}
#endif

}
