#pragma once

#include "app.h"
#include "glutil.h"
#include "globject.h"
#include "glshader.h"
#include "gltexture.h"
#include "glrenderprog.h"
#include "glmodel.h"

namespace kr
{
	class WebCanvasGL: public Application
	{
	public:
		WebCanvasGL() noexcept;
		~WebCanvasGL() noexcept;
		void create(int width, int height) noexcept;

		void onResize(int width, int height) noexcept override;

	protected:
		void _flush() noexcept override;

	private:
		GLContext m_ctx;
	};

}
