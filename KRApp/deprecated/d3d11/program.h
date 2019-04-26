#pragma once

#include <KR3/main.h>
#include "shader.h"
#include "vertex.h"

#include <initializer_list>

struct ID3D11InputLayout;
struct ID3D11Buffer;

namespace kr
{
	namespace gl
	{

		class Program;
	
		class UniformImpl
		{
		public:
			UniformImpl(Program * prog, const char * name) noexcept;

		protected:
			int32_t m_location;
		};

		template <typename T> class Uniform:public UniformImpl
		{
		public:
			using UniformImpl::UniformImpl;

			Uniform<T>& operator =(const T &value) noexcept;
		private:
		};
	
		class Program
		{
		public:
			Program(VertexShader * vs, FragmentShader * fs, initializer_list<const AttributeData> layers) noexcept;
			template <typename V>
			Program(VertexShader * vs, FragmentShader * fs) noexcept
				:Program(vs, fs, { V::attributes, V::attributes + countof(V::attributes) })
			{
			}
			~Program() noexcept;
			void use() noexcept;

	#ifndef KRGL_PRIVATE
		private:
	#endif
			Keep<ID3D11InputLayout> m_layer;
			VertexShader *m_vs;
			FragmentShader *m_fs;
			Array<byte> m_vsUniform;
			Array<byte> m_fsUniform;
		};
	
		class ProgramG:public Program
		{
		public:
			ProgramG(VertexShader * vs, GeometryShader * gs, FragmentShader * fs, initializer_list<const AttributeData> layers) noexcept;
			template <typename V>
			ProgramG(VertexShader * vs, GeometryShader * gs, FragmentShader * fs) noexcept
				:Program(vs, fs, { V::attributes, V::attributes + countof(V::attributes) })
			{
			}
			~ProgramG() noexcept;
			void use() noexcept;

	#ifndef KRGL_PRIVATE
		private:
	#endif
			GeometryShader *m_gs;
			Array<byte> m_gsUniform;
		};

		using UniformMatrix = Uniform<mat4>;
		using Uniform4f = Uniform<vec4>;
		using Uniform3f = Uniform<vec3>;
		using Uniform2f = Uniform<vec2>;
		using Uniform1f = Uniform<float>;
	
		extern template UniformMatrix;
		extern template Uniform4f;
		extern template Uniform3f;
		extern template Uniform2f;
		extern template Uniform1f;

	}
}