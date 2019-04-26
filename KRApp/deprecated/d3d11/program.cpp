#define KRGL_PRIVATE
#include "program.h"
#include "include_d3d11.h"
#include "3d.h"

namespace
{
	const char * const semanticNames[]=
	{
		"POSITION",
		"POSITION1",
		"POSITION2",
		"NORMAL",
		"NORMAL1",
		"NORMAL2",
		"TEXCOORD0",
		"TEXCOORD1",
		"TEXCOORD2",
	};

	const size_t formatSize[] =
	{
		sizeof(float) * 1,
		sizeof(float) * 2,
		sizeof(float) * 3,
		sizeof(float) * 4,
	};

	const DXGI_FORMAT formatDXGI[] =
	{
		DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_R32G32_FLOAT,
		DXGI_FORMAT_R32G32B32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R8_UNORM,
		DXGI_FORMAT_R8G8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM
	};
}

kr::gl::Program::Program(VertexShader * vs, FragmentShader * fs, initializer_list<const AttributeData> layers) noexcept
{
	ID3D11ShaderReflection * test =  vs->m_reflect;
	m_vs = vs;
	m_fs = fs;
	
	Array<D3D11_INPUT_ELEMENT_DESC> temp(layers.size());

	D3D11_INPUT_ELEMENT_DESC* ptr = temp.begin();
	size_t offset = 0;
	for (const AttributeData & data : layers)
	{
		assert(offset <= 0xffffffff);
		assert((size_t)data.semantic <= (size_t)Semantic::Texcoord3);
		assert((size_t)data.format <= (size_t)Format::Float32x4);
		ptr->SemanticName = semanticNames[(size_t)data.semantic];
		ptr->SemanticIndex = 0;
		ptr->AlignedByteOffset = (UINT)offset;
		ptr->InputSlot = 0;
		ptr->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		ptr->InstanceDataStepRate = 0;
		switch (data.format)
		{
		case Format::Float32x2:
			ptr->Format = DXGI_FORMAT_R32G32_FLOAT;
			break;
		case Format::Float32x3:
			ptr->Format = DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case Format::Float32x4:
			ptr->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		default: assert(!"Unsupported"); return;
		}
		offset+= formatSize[(size_t)data.format];
		ptr++;
	}
	hrshouldbe(wnd->m_device->CreateInputLayout(temp.begin(), (UINT)temp.size(), 
		m_vs->m_data, 
		m_vs->m_size, &m_layer));
}
kr::gl::Program::~Program() noexcept
{
	if (wnd->m_curProgram == this)
	{
		wnd->m_curProgram = nullptr;
		ID3D11DeviceContext * ctx = wnd->m_ctx;
		ctx->PSSetConstantBuffers(0, 0, nullptr);
		ctx->PSSetShader(nullptr, nullptr, 0);
		ctx->VSSetConstantBuffers(0, 0, nullptr);
		ctx->VSSetShader(nullptr, nullptr, 0);
	}
}
void kr::gl::Program::use() noexcept
{
	if (wnd->m_curProgram == this) return;
	wnd->m_curProgram = this;
	wnd->m_isProgramG = false;
	ID3D11DeviceContext * ctx = wnd->m_ctx;
	ctx->PSSetShader(m_fs->m_shader, nullptr, 0);
	ctx->GSSetShader(nullptr, nullptr, 0);
	ctx->VSSetShader(m_vs->m_shader, nullptr, 0);
	wnd->m_ctx->IASetInputLayout(m_layer);
}

kr::gl::ProgramG::ProgramG(VertexShader * vs, GeometryShader * gs, FragmentShader * fs, initializer_list<const AttributeData> layers) noexcept
	:Program(vs, fs, layers), m_gs(gs)
{
}
kr::gl::ProgramG::~ProgramG() noexcept
{
}
void kr::gl::ProgramG::use() noexcept
{
	if (wnd->m_curProgram == this) return;
	wnd->m_curProgram = this;
	wnd->m_isProgramG = true;
	ID3D11DeviceContext * ctx = wnd->m_ctx;
	ctx->PSSetShader(m_fs->m_shader, nullptr, 0);
	ctx->GSSetShader(m_gs->m_shader, nullptr, 0);
	ctx->VSSetShader(m_vs->m_shader, nullptr, 0);
	wnd->m_ctx->IASetInputLayout(m_layer);
}

kr::gl::UniformImpl::UniformImpl(Program * prog, const char * name) noexcept
{
	m_location = 0;

	D3D11_SHADER_VARIABLE_DESC desc;
	if (SUCCEEDED(prog->m_vs->m_reflect->GetVariableByName(name)->GetDesc(&desc)))
	{
		m_location = desc.StartOffset;
		return;
	}
	if (SUCCEEDED(prog->m_fs->m_reflect->GetVariableByName(name)->GetDesc(&desc)))
	{
		m_location = ~desc.StartOffset;
		return;
	}
	
	std::cerr << "Shader get variable failed: " << name << std::endl;
}

template <typename T>
kr::gl::Uniform<T>& kr::gl::Uniform<T>::operator =(const T &value) noexcept
{
	if (wnd->m_curProgram == nullptr)
	{
		std::cerr << "Program not setted" << std::endl;
		return *this;
	}
	Array<byte> * arr;
	uint32_t nloc;
	if (m_location < 0)
	{
		nloc = ~m_location;
		arr = &wnd->m_curProgram->m_fsUniform;
	}
	else
	{
		nloc = m_location;
		arr = &wnd->m_curProgram->m_vsUniform;
	}
	size_t fullsize = ((nloc + sizeof(T) + 0xf) & ~0xf);
	if(arr->size() < fullsize) arr->resize(fullsize);
	arr->subcopy((byte*)&value, sizeof(T), nloc);
	return *this;
}

template kr::gl::UniformMatrix;
template kr::gl::Uniform4f;
template kr::gl::Uniform3f;
template kr::gl::Uniform2f;
template kr::gl::Uniform1f;
