#define KRGL_PRIVATE
#define INITGUID 
#include "shader.h"
#include "3d.h"
#include "include_d3d11.h"

#include <cstddef>
#include <new>

template<>
kr::gl::Shader<kr::gl::ShaderType::Vertex>::Shader(const void * data, std::size_t size) noexcept
{
	m_data = data;
	m_size = size;
	if (hrshouldbe(wnd->m_device->CreateVertexShader(data, size, nullptr, &m_shader)))
	{
		hrshouldbe(D3DReflect(data, size, IID_ID3D11ShaderReflection, (void**)&m_reflect));
	}
}
template<>
kr::gl::Shader<kr::gl::ShaderType::Fragment>::Shader(const void * data, std::size_t size) noexcept
{
	m_data = data;
	m_size = size;
	if (hrshouldbe(wnd->m_device->CreatePixelShader(data, size, nullptr, &m_shader)))
	{
		hrshouldbe(D3DReflect(data, size, IID_ID3D11ShaderReflection, (void**)&m_reflect));
	}
}
template<kr::gl::ShaderType type>
kr::gl::Shader<type>::~Shader() noexcept
{
}
template<kr::gl::ShaderType type>
kr::gl::Shader<type>::Shader(const Shader & _copy) noexcept
{
	m_data = _copy.m_data;
	m_size = _copy.m_size;
	m_reflect = _copy.m_reflect;
	m_shader = _copy.m_shader;
}
template<kr::gl::ShaderType type>
kr::gl::Shader<type>::Shader(Shader && _move) noexcept
{
	m_data = _move.m_data;
	m_size = _move.m_size;
	m_reflect = std::move(_move.m_reflect);
	m_shader = std::move(_move.m_shader);
}
template<kr::gl::ShaderType type>
kr::gl::Shader<type>& kr::gl::Shader<type>::operator =(const Shader & _copy) noexcept
{
	this->~Shader();
	new(this) Shader(_copy);
	return *this;
}
template<kr::gl::ShaderType type>
kr::gl::Shader<type>& kr::gl::Shader<type>::operator =(Shader && _move) noexcept
{
	this->~Shader();
	new(this) Shader(std::move(_move));
	return *this;
}

template class kr::gl::Shader<kr::gl::ShaderType::Vertex>;
template class kr::gl::Shader<kr::gl::ShaderType::Fragment>;

//void RenderDevice::InitShaders( const char * vertex_shader, const char * pixel_shader, ShaderSet ** pShaders, ID3D11InputLayout ** pVertexIL,
//				  D3D11_INPUT_ELEMENT_DESC * DistortionMeshVertexDesc, int num_elements)
//{
//    ID3D10Blob* vsData = CompileShader("vs_4_0", vertex_shader);
//
//    Ptr<VertexShader> vtxShader = *new VertexShader(this, vsData);
//
//	ID3D11InputLayout** objRef   = pVertexIL;
//
//	HRESULT validate = Device->CreateInputLayout(
//        DistortionMeshVertexDesc, num_elements,
//        vsData->GetBufferPointer(), vsData->GetBufferSize(), objRef);
//	if(FAILED(validate)) OVR_ASSERT(false);
//    
//	(*pShaders) = CreateShaderSet();
//	(*pShaders)->SetShader(vtxShader);
// 
//    ID3D10Blob *pShader = CompileShader("ps_4_0", pixel_shader);
//    Ptr<PixelShader> ps  = *new PixelShader(this, pShader);
//
//    (*pShaders)->SetShader(ps);
//}
//


 //   ID3D10Blob* shader;
 //   ID3D10Blob* errors;
 //   HRESULT hr = D3DCompile(src, strlen(src), nullptr, nullptr, nullptr, mainName, profile,
 //                           0, 0, &shader, &errors);
	//if (errors)
	//{
 //       OutputDebugStringA((char*)errors->GetBufferPointer());
	//	errors->Release();
	//}
	//assert(SUCCEEDED(hr));
//m_data = shader;
