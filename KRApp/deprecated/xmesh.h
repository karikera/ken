#pragma once

#include <KRDirect3D/shader.h>
#include <KR/math/geom.h>

namespace kr
{
	namespace d3d
	{
		class XAEffect:public Effect
		{
		protected:
			EffectParameter<matrix4>	m_mWorldMatrixArray;
			EffectParameter<float4>	m_MaterialDiffuse;
			EffectParameter<float4>	m_MaterialAmbient;
			EffectParameter<int>		m_CurNumBones;
			EffectParameter<matrix4>	m_mViewProj;
			EffectParameter<float4>	m_lhtDir;

		private:
			friend class CMeshContainer;

			void _SetDiffuse(const float4 &color);
			void _SetAmbient(const float4 &ambient);
			void _SetBoneNumber(dword n);
			void _UpdateBone(const DWORD *index,const matrix4 *offset,const matrix4 **bone, dword nCount,const Camera &cam);
			void _GetParameter();

		public:
			void CreateFromResource(HINSTANCE hModule,dword uID);
			void CreateFromFile(pcwstr strFileName);
			void CreateFromSource(RStringA strCode);
		
			void UpdateView(const Camera &cam);
			void UpdateProjection(const Camera &cam);
			void UpdateLight(const float4 &vLight);
			void SetMaterial(const D3DMATERIAL &material);

		};
		class XAMesh_B
		{
		protected:
			LPD3DXFRAME					m_pFrameRoot;
			LPD3DXANIMATIONCONTROLLER	m_pControl;
		
			XAMesh_B(LPD3DXFRAME frame,LPD3DXANIMATIONCONTROLLER control);
			XAMesh_B();

		public:

			bool Live();
			void Draw(const matrix4 &mat,XAEffect &eff,const Camera &cam);
			void AdvanceTime(double TimeDelta);
			void ResetTime();
			void ChangeTrack(int n);
			bool SetSlip(movable3 &move);
			bool SetStick(movable3 &move);
			bool Test(const movable3 &move);
			LPD3DXANIMATIONCONTROLLER GetAnimationController();
			LPD3DXFRAME GetFrameRoot();
		};
		class XAMeshC:public XAMesh_B
		{
			friend class XAMesh;
		protected:
			XAMeshC(LPD3DXFRAME frame,LPD3DXANIMATIONCONTROLLER control);

		public:
			XAMeshC();
			void Delete();
		};
		class XAMesh:public XAMesh_B
		{
		public:
			void Load(RString strPath,RString strFileName);
			void Close();
			XAMeshC Clone();

		};
		class XMesh
		{
			friend class CollisionXMesh;
		public:
			XMesh();
			bool Live();
			HRESULT Load(RString strPath,RString strFileName);
		
			void Close();
			void Draw();
			LPD3DXMESH GetMesh();

		protected:
			LPD3DXMESH	m_pMesh;
			LPD3DXBUFFER m_pMaterialBuffer;
			D3DXMATERIAL *m_pMaterials;
			dword m_uMaterial;

			Texture *m_pTexture;

		public:
			template <class T> void Draw(T &effect)
			{
				for( dword i = 0; i < m_uMaterial; i++ )
				{
					m_pTexture[i].set();
					effect.SetMaterial(m_pMaterials[i].MatD3D);
					_DrawSubset(i);
				}
			}
		};
		class CAnimatedCollisionMesh
		{
		public:
			bool SetSlip(movable3 &move);
			bool SetStick(movable3 &move);
			bool Test(const movable3 &move);

		protected:
		};
		class CollisionXMesh
		{
		public:
			CollisionXMesh();
			CollisionXMesh(XMesh &mesh);
			void Delete();
			bool Live();

			bool SetSlip(movable3 &move);
			bool SetStick(movable3 &move);
			bool Test(const movable3 &move);
		
		protected:
			LPD3DXMESH	m_pMesh;
			word* m_pIndex;
			float3* m_pVertex;
			dword m_uFace;
		};
	}


}
