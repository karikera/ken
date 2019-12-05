#pragma once

#include <KR3/main.h>
#include <KR3/wl/com.h>
#include "dxcommon.h"

struct IDXGISwapChain;
struct IDXGISwapChain1;
struct IDXGISurface;
struct IDXGISurface1;
struct ID3D11Texture2D;
struct IDXGIAdapter;
struct IDXGIOutput;
struct IDXGIDevice;

namespace kr
{
	COM_CLASS_DECLARE(ID3D11Texture2D);
	COM_CLASS_DECLARE(IDXGIAdapter);
	COM_INHERIT(IDXGISwapChain1, IDXGISwapChain);
	COM_INHERIT(IDXGISurface1, IDXGISurface);

	namespace dxgi
	{
		using SwapChain = Com<IDXGISwapChain>;
		using SwapChain1 = Com<IDXGISwapChain1>;
		using Surface = Com<IDXGISurface>;
		using Surface1 = Com<IDXGISurface1>;
		using Adapter = Com<IDXGIAdapter>;
		using Output = Com<IDXGIOutput>;

		IDXGIDevice * getDevice() noexcept;
	}

	COM_CLASS_METHODS(IDXGIAdapter)
	{
		struct Iterator
		{
			const dxgi::Adapter * adapter;
			dxgi::Output output;
			UINT index;

			dxgi::Output& operator *() noexcept;
			Iterator & operator ++() throws(ErrorCode);
			Iterator & operator --() throws(ErrorCode);
			bool operator ==(const Iterator & iter) noexcept;
			bool operator !=(const Iterator & iter) noexcept;
		};

		Iterator begin() const noexcept;
		Iterator end() const noexcept;
		dxgi::Output operator [](UINT idx) const throws(ErrorCode);
	};

	namespace dxgi
	{
		struct Adapters final
		{
			struct Iterator
			{
				Adapter adapter;
				UINT index;

				Adapter& operator *() noexcept;
				Iterator & operator ++() throws(ErrorCode);
				Iterator & operator --() throws(ErrorCode);
				bool operator ==(const Iterator & iter) noexcept;
				bool operator !=(const Iterator & iter) noexcept;
			};
			Iterator begin() const noexcept;
			Iterator end() const noexcept;
			Adapter operator [](UINT idx) const throws(ErrorCode);
		};

		constexpr const Adapters adapters = Adapters();
	}

	COM_CLASS_METHODS(IDXGISwapChain)
	{
#ifdef USE_D2D2
		void setTarget(ID2D1DeviceContext * target);
#endif
		void present() noexcept;
		Com<ID3D11Texture2D> getTexture();
	};

	COM_CLASS_METHODS(IDXGISwapChain1)
	{
#ifdef USE_D2D2
		void createForComposition(int width, int height);
#endif
	};

	COM_CLASS_METHODS(IDXGISurface)
	{
		void setFrom(ID3D11Texture2D * texture);
	};

#ifdef USE_D2D2
	COM_CLASS_METHODS(IDXGISurface1)
	{
		void setFrom(ID3D11Texture2D * texture);
	};
#endif

}