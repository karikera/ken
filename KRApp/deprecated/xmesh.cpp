#include "stdafx.h"

#include "rtypes.h"
#include "d3d.h"
#include "xmesh.h"
#include "fvf.h"

#include "private/_kr_d3dcontainer.h"

namespace kr
{
	namespace d3d
	{
		extern LPDIRECT3DDEVICE9 g_pDevice;
		extern dword dwMeshFlags;
	}
}

using namespace kr;
using namespace d3d;
	
void XAEffect::CreateFromResource(HINSTANCE hModule,dword uID)
{
	Effect::CreateFromResource(hModule,uID);
	_GetParameter();
}
void XAEffect::CreateFromFile(pcwstr strFileName)
{
	Effect::CreateFromFile(strFileName);
	_GetParameter();
}
void XAEffect::CreateFromSource(RStringA strCode)
{
	Effect::CreateFromSource(strCode);
	_GetParameter();
}

void XAEffect::_UpdateBone(const DWORD *index,const matrix4 *offset,const matrix4 **bone, dword nCount,const Camera &cam)
{
	stack(matrix4, mBoneMatrix,nCount);
	for( dword j = 0; j < nCount; ++j )
	{
		int nMatrixIndex = index[j];
		if( nMatrixIndex == UINT_MAX ) continue;
		mBoneMatrix[j]= offset[nMatrixIndex] * (*bone[nMatrixIndex]) * cam.GetView();
	}
	(*this)[m_mWorldMatrixArray].SetArray( mBoneMatrix,nCount );
}
void XAEffect::_SetDiffuse(const float4 &color)
{
	(*this)[m_MaterialDiffuse] = color;
}
void XAEffect::_SetAmbient(const float4 &ambient)
{
	(*this)[m_MaterialAmbient] = ambient;
}
void XAEffect::_SetBoneNumber(dword n)
{
	(*this)[m_CurNumBones] = n;
}
void XAEffect::_GetParameter()
{
	m_mWorldMatrixArray= GetParameter("mWorldMatrixArray");
	m_MaterialDiffuse= GetParameter("MaterialDiffuse");
	m_MaterialAmbient= GetParameter("MaterialAmbient");
	m_CurNumBones= GetParameter("CurNumBones");
	m_mViewProj= GetParameter("mViewProj");
	m_lhtDir = GetParameter("lhtDir");
}
void XAEffect::UpdateView(const Camera &cam)
{
	CFVFDevice::UpdateView(cam);
}
void XAEffect::UpdateProjection(const Camera &cam)
{
	(*this)[m_mViewProj] = cam.GetProjection();
	CFVFDevice::UpdateProjection(cam);
}
void XAEffect::UpdateLight(const float4 &vLight)
{
	(*this)[m_lhtDir] = vLight;
	CFVFDevice::UpdateLight(vLight);
}
void XAEffect::SetMaterial(const D3DMATERIAL &material)
{
	_SetAmbient((float4&)(material.Ambient));
	_SetDiffuse((float4&)(material.Diffuse));
}
	
XAMesh_B::XAMesh_B()
{
	m_pFrameRoot=nullptr;
	m_pControl=nullptr;
}
XAMesh_B::XAMesh_B(LPD3DXFRAME frame,LPD3DXANIMATIONCONTROLLER control)
{
	m_pFrameRoot=frame;
	m_pControl=control;
}
bool XAMesh_B::Live()
{
	assert(this != nullptr);
	return m_pControl != nullptr;
}
void XAMesh_B::Draw(const matrix4 &mat,XAEffect &eff,const Camera &cam)
{
	scast<CAnimationFrame*>(m_pFrameRoot)->Update(mat);
	scast<CAnimationFrame*>(m_pFrameRoot)->Draw(eff,cam);
}
void XAMesh_B::AdvanceTime(double TimeDelta)
{
	m_pControl->AdvanceTime(TimeDelta,nullptr);
}
void XAMesh_B::ResetTime()
{
	m_pControl->ResetTime();
}
void XAMesh_B::ChangeTrack(int n)
{
	LPD3DXANIMATIONSET AnimSet = nullptr;
	m_pControl ->GetAnimationSet(n, &AnimSet);
	m_pControl ->SetTrackAnimationSet(0, AnimSet);
	m_pControl ->ResetTime();
	AnimSet->Release();
}
LPD3DXANIMATIONCONTROLLER XAMesh_B::GetAnimationController()
{
	m_pControl->AddRef();
	return m_pControl;
}
LPD3DXFRAME XAMesh_B::GetFrameRoot()
{
	return m_pFrameRoot;
}
	
XAMeshC::XAMeshC()
{
}
XAMeshC::XAMeshC(LPD3DXFRAME frame,LPD3DXANIMATIONCONTROLLER control):XAMesh_B(frame,control)
{
}
void XAMeshC::Delete()
{
	TestRelease(m_pControl);
	m_pFrameRoot=nullptr;
}
void XAMesh::Load(RString strPath,RString strFileName)
{
	CHierarchyAllocator Allocator(strPath);

	krrcom(D3DXLoadMeshHierarchyFromXW(Allocator.GetFullPath(strFileName),D3DXMESH_SYSTEMMEM,g_pDevice,&Allocator, nullptr, &m_pFrameRoot, &m_pControl ));

	((CAnimationFrame*)m_pFrameRoot)->SetupBoneMatrixPointers(m_pFrameRoot);

	//float3 vec;
	//float fObjRadius;
	//D3DXFrameCalculateBoundingSphere( (LPD3DXFRAME)m_pFrameRoot, (D3DXVECTOR3*)&vec, &fObjRadius );
}
void XAMesh::Close()
{
	if(m_pFrameRoot != nullptr)
	{
		CHierarchyDestructor Destruct;
		D3DXFrameDestroy( (LPD3DXFRAME)m_pFrameRoot, &Destruct );
	}
	TestRelease(m_pControl);
}
XAMeshC XAMesh::Clone()
{
	assert(this != nullptr);
	LPD3DXANIMATIONCONTROLLER pControl;
	m_pControl->CloneAnimationController(
		m_pControl->GetMaxNumAnimationOutputs(),
		m_pControl->GetMaxNumAnimationSets(),
		m_pControl->GetMaxNumTracks(),
		m_pControl->GetMaxNumEvents(),
		&pControl);
	return XAMeshC(m_pFrameRoot,pControl);
}

XMesh::XMesh()
{
	m_pMesh = nullptr;
}
bool XMesh::Live()
{
	return (m_pMesh != nullptr);
}
HRESULT XMesh::Load(RString strPath,RString strFileName)
{
	HRESULT hr;

	TString strFilePath;
	~strFilePath<<strPath<<L'\\';

	TIndex strFileDest=strFilePath+strFilePath.length;

	LPD3DXBUFFER pEffectBuffer;
	~strFileDest<<strFileName<<nullend;

	if(FAILED( hr = D3DXLoadMeshFromXW(strFilePath.string,dwMeshFlags,g_pDevice,nullptr,&m_pMaterialBuffer,&pEffectBuffer,(LPDWORD)&m_uMaterial,&m_pMesh))) return hr;
	if(m_uMaterial == 0)
	{
		m_pMesh->Release();
		m_pMesh = nullptr;
		return E_INVALIDARG;
	}

	m_pMaterials= ( D3DXMATERIAL* )m_pMaterialBuffer->GetBufferPointer();
	m_pTexture= new Texture[m_uMaterial];
		
	for( dword i = 0; i < m_uMaterial; i++ )
	{
		if(m_pMaterials[i].pTextureFilename == nullptr) continue;

		RStringA strTexture = (RStringA)m_pMaterials[i].pTextureFilename;
		if(strTexture.length <= 0 ) continue;

		~strFileDest<<strTexture<<nullend;

		m_pTexture[i].Load(strFilePath.string);
	}

	pEffectBuffer->Release(); // do not use
	return S_OK;
}
void XMesh::Close()
{
	assert(m_pMesh != nullptr);
	if(m_uMaterial != 0)
	{
		for(dword i=0;i<m_uMaterial;i++)
		{
			SafeRelease(m_pTexture[i]);
		}
		delete [] m_pTexture;
		m_pMaterialBuffer->Release();
	}
	m_pMesh->Release();
}
void XMesh::Draw()
{
	for( dword i = 0; i < m_uMaterial; i++ )
	{
		m_pTexture[i].Use();
		g_pDevice->SetMaterial(&(m_pMaterials[i].MatD3D));
		m_pMesh->DrawSubset( i );
	}
}
LPD3DXMESH XMesh::GetMesh()
{
	m_pMesh->AddRef();
	return m_pMesh;
}
	
CollisionXMesh::CollisionXMesh()
{
	m_pMesh=nullptr;
}
CollisionXMesh::CollisionXMesh(XMesh &mesh)
{
	krrcom(mesh.m_pMesh->CloneMeshFVF(D3DXMESH_MANAGED,D3DFVF_XYZ,g_pDevice,&m_pMesh));

	m_uFace=m_pMesh->GetNumFaces();
	krrcom(m_pMesh->LockIndexBuffer(0,(ptr*)m_pIndex));
	krrcom(m_pMesh->LockVertexBuffer(0,(ptr*)m_pVertex));
}
void CollisionXMesh::Delete()
{
	m_pMesh->UnlockVertexBuffer();
	m_pMesh->UnlockIndexBuffer();
	m_pMesh->Release();
}
bool CollisionXMesh::Live()
{
	return m_pMesh != nullptr;
}

bool CollisionXMesh::SetSlip(movable3 &move)
{
	word* index=m_pIndex;
	int n=m_uFace;
	while(n--)
	{
		triangle3 tri((m_pVertex[*(index++)]) , (m_pVertex[*(index++)]) , (m_pVertex[*(index++)]));
		if( tri.slip(move) ) goto __stick;
	}
	return false;
__stick:
	while(n--)
	{
		triangle3 tri((m_pVertex[*(index++)]) , (m_pVertex[*(index++)]) , (m_pVertex[*(index++)]));
		tri.stick(move);
	}
	return true;
}
bool CollisionXMesh::SetStick(movable3 &move)
{
	word* index=m_pIndex;
	int n=m_uFace;
	while(n--)
	{
		triangle3 tri((m_pVertex[*(index++)]) , (m_pVertex[*(index++)]) , (m_pVertex[*(index++)]));
		if( tri.stick(move) ) goto __stick;
	}
	return false;
__stick:
	while(n--)
	{
		triangle3 tri((m_pVertex[*(index++)]) , (m_pVertex[*(index++)]) , (m_pVertex[*(index++)]));
		tri.stick(move);
	}
	return true;
}
bool CollisionXMesh::Test(const movable3 &move)
{
	word* index=m_pIndex;
	int n=m_uFace;
	while(n--)
	{
		triangle3 tri((m_pVertex[*(index++)]) , (m_pVertex[*(index++)]) , (m_pVertex[*(index++)]));
		if(tri.test(move)) return true;
	}
	return false;
}
