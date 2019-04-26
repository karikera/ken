#pragma once

#include "texture.h"

namespace kr
{
	COM_CLASS_METHODS(ID3D11RenderTargetView)
	{
		void create(ID3D11Texture2D * texture);
	};

	COM_CLASS_METHODS(ID3D11DepthStencilView)
	{
		void create(ID3D11Texture2D * texture);
	};

}