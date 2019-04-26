#pragma once

#include <KRDirect3D/d3d.h>
#include <KR/math/geom.h>
#include <KR/math/fixedmatrix.h>
#include <KR/util/random.h>

namespace kr
{
	namespace d3d
	{
		bool colCube(movable3 &move, bool bContact);
		bool colWall(movable3 &move, bool bContact);
		bool colPlane(movable3 &move, bool bContact);
		bool selCube(movable3 &move, bool bContact);
		bool selWall(movable3 &move, bool bContact);
		bool selPlane(movable3 &move, bool bContact);

		struct VERTEXINFO
		{
			dword uOffset, uCount;
			D3DPRIMITIVETYPE tType;

			inline VERTEXINFO()
			{
			}
			inline VERTEXINFO(int offset, int count, D3DPRIMITIVETYPE type)
			{
				Set(offset, count, type);
			}
			inline void Set(int offset, int count, D3DPRIMITIVETYPE type)
			{
				uOffset=offset;
				uCount=count;
				tType=type;
			}

		};
		class Model_B
		{
			friend class CMap;
		protected:
			dword m_uOffset;
			VERTEXINFO *m_pInfo, *m_pLast;

			void _Draw(VertexBuffer &VB, dword vsize) const;
		public:
			Model_B();
			Model_B(dword offset, int size, const VERTEXINFO *info, dword infocount);
			void Set(dword offset, int size, const VERTEXINFO *info, dword infocount);
			void Delete();
			_Check_return_ VERTEXINFO* NewInfo(dword infocount);
			_Check_return_ bool Live();
		};
		template <class TVertex> class KModelT:public Model_B
		{
		private:
			float3 _CalcNormal(const float3 *src, dword w, dword h, dword x, dword y)
			{
				float3 n1, n2;
				int x1, x2, y1, y2;
				int xmul=3;
				int ymul=3;

				if(y == 0)
				{
					y1 = 0;
					ymul--;
				}
				else y1 = y-1;
				if(y == h)
				{
					y2 = h;
					ymul--;
				}
				else y2 = y+1;

				if(x == 0)
				{
					x1 = 0;
					xmul--;
				}
				else x1 = x-1;
				if(x == w)
				{
					x2 = w;
					xmul--;
				}
				else x2 = x+1;

				int pitch = w +1;
				float3 xsub, ysub;
				if(xmul != 0) xsub= (src[y * pitch + (x2)] - 	src[y * pitch + (x1)]) / (float)xmul;
				else xsub.set(1, 0, 0);
				if(ymul != 0) ysub= (src[y2 * pitch + (x)] - 	src[y1 * pitch + (x)]) / (float)ymul;
				else ysub.set(0, 1, 0);

				return cross(xsub, ysub).getnormalize();
			}

			dword _GetPlaneIC(const PT &seg)
			{
				return seg.y;
			}
			dword _GetBoxIC(const PT3 &seg)
			{
				int nSize;
				nSize = seg.y * 2;
				nSize += seg.z * 2;
				nSize += seg.x * 2;
				return nSize;
			}
			dword _GetCylinderIC(const PT &seg)
			{
				return seg.y;
			}
			void _FillPlane(TVertex *pstart, TVertex *&pvb, VERTEXINFO *&info, const float3 &xpos, const float3 &ypos, const float3 &off, PT seg)
			{
				dword nCount = GetPlaneSize(seg);

				float txinc = 1 / fcast<float>(seg.x);
				float tyinc = 1 / fcast<float>(seg.y);
				float3 xinc = xpos * txinc;
				float3 yinc = ypos * tyinc;

				float3 vpos = off;
				float2 tpos(0, 0);

				if(TVertex::IsUsed(D3DDECLUSAGE_NORMAL))
				{
					float3 vnorm=cross(xpos, ypos).getnormalize();
					for(dword i=0; i<nCount; i++)
					{
						pvb[i].SetNormal(vnorm);
					}
				}
				for(int y=0; y<seg.y; y++)
				{
					info->Set(pvb - pstart, seg.x * 2, D3DPT_TRIANGLESTRIP);
					info++;
					for(int x=0; x<=seg.x; x++)
					{
						pvb->SetPosition(vpos + yinc);
						pvb->SetTexCoord(tpos.x, tpos.y + tyinc);
						pvb++;

						pvb->SetPosition(vpos);
						pvb->SetTexCoord(tpos);
						pvb++;

						vpos += xinc;
					}

					vpos = off + yinc * fcast<float>(y);
				}
			}
		public:
			void Draw(VertexBuffer &vb) const
			{
				_Draw(vb, sizeof(TVertex));
			}
			void SetBuffer(VertexBuffer &vb, TVertex *&out, int count)
			{
				out=(TVertex*)vb.GetBuffer(count*sizeof(TVertex));
				Set(vb.GetUsedSize(), sizeof(TVertex), nullptr, 0);
			}
			static dword GetBoxSize(const PT3 &vSeg)
			{
				int nSize;
				nSize = (vSeg.x + 1) * 2 * vSeg.y * 2;
				nSize += (vSeg.y + 1) * 2 * vSeg.z * 2;
				nSize += (vSeg.z + 1) * 2 * vSeg.x * 2;
				return 24;
			}
			static dword GetPlaneSize(const PT &vSegments)
			{
				return (vSegments.x + 1)*2*vSegments.y;
			}
			static dword GetSphereSize(int h, int w)
			{
				return (w+1)*(h-1)*2 + 2;
			}
			static dword GetCylinderSize(const PT &vSegments)
			{
				return (vSegments.x + 1) * 2 * vSegments.y;
			}
			template <typename LS> void CreateBox(VertexBuffer &vb, const PT3 &vSeg, LS vs)
			{
				dword uCount = GetBoxSize(vSeg);

				TVertex *pvb, *pstart;
				SetBuffer(vb, pvb, uCount);
				pstart = pvb;

				VERTEXINFO* info=NewInfo(_GetBoxIC(vSeg));
				_FillPlane(pstart, pvb, info, float3(1, 0, 0), float3(0, -1, 0), float3(0, 1, 0), PT(vSeg.x, vSeg.y)); // ¹Ø ¶Ñ²±
				_FillPlane(pstart, pvb, info, float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1), PT(vSeg.x, vSeg.y)); // À­ ¶Ñ²±
				_FillPlane(pstart, pvb, info, float3(1, 0, 0), float3(0, 0, -1), float3(0, 1, 1), PT(vSeg.x, vSeg.z)); // ¾Æ·¹ 
				_FillPlane(pstart, pvb, info, float3(0, -1, 0), float3(0, 0, -1), float3(1, 1, 1), PT(vSeg.y, vSeg.z)); // ¿À¸¥
				_FillPlane(pstart, pvb, info, float3(-1, 0, 0), float3(0, 0, -1), float3(1, 0, 1), PT(vSeg.x, vSeg.z)); // À§
				_FillPlane(pstart, pvb, info, float3(0, 1, 0), float3(0, 0, -1), float3(0, 0, 1), PT(vSeg.y, vSeg.z)); // ¿Þ

				assert(pvb == pstart + uCount);
				assert(m_pLast == info);

				vb.Update(pstart, uCount, vs);
			}
			template <typename LS> void CreatePlane(VertexBuffer &vb, const PT &vSeg, LS vs)
			{
				dword uCount = GetPlaneSize(vSeg);

				TVertex *pvb, *pstart;
				SetBuffer(vb, pvb, nCount);
				pstart = pvb;

				VERTEXINFO * info= NewInfo(_GetPlaneIC(vSeg));
				_FillPlane(pvb, info, float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 0), vSeg);

				assert(pvb == pstart + uCount);
				assert(m_pLast == info);

				vb.Update(pstart, uCount, vs);
			}
			template <typename LS> void CreateSphere(VertexBuffer &vb, int h, int w, LS vs)
			{
				stack(float, _allocdir1, w+2);
				stack(float, _allocdir2, w+2);
				float *dir1=_allocdir1;
				float *dir2=_allocdir2;

				for(int i=0; i<=w+1; i++)
				{
					cos1[i]=math::cos((float)i*(math::pi*2)/w);
					cos2[i]=math::cos(((float)i+0.5f)*(math::pi*2)/w);
					sin1[i]=math::sin((float)i*(math::pi*2)/w);
					sin2[i]=math::sin(((float)i+0.5f)*(math::pi*2)/w);
				}

				float pside, psiden, paxis, paxisn;
				float side, siden, axis, axisn;
				float tv, ptv;

				dword count=GetSphereSize(h, w);
				dword offset=vb.GetUsedSize();

				TVertex::Index pvb, pstart;
				SetBuffer(vb, pvb, count);
				pstart=pvb;

				VERTEXINFO* info=NewInfo(h);

				// À­¶Ñ²±
				pvb->SetPosition(0, 0, 1);
				pvb->SetNormal(0, 0, 1);
				pvb->SetTexCoord(0.5f, 0);
				pvb++;
				axisn=math::cos(math::pi/h);
				siden=math::sin(math::pi/h);
				axis=axisn;
				side=siden;
				tv=1.f/h;
				for(int i=0; i<=w; i++)
				{
					pvb->SetPosition(side*cos1[i], side*sin1[i], axis);
					pvb->SetNormal(siden*cos1[i], siden*sin1[i], axisn);
					pvb->SetTexCoord((float)i/w, tv);
					pvb++;
				}
				info->Set(offset, w, D3DPT_TRIANGLEFAN);
				info++;
				offset+=w+2;

				//Å×µÎ¸®
				int lastj=h;
				int OffAng=0;
				float offu1=0, offu2=0.5f;
				for(int j=2; j<lastj; j++)
				{
					paxisn=axisn; psiden=siden; paxis=axis; pside=side; ptv=tv;
					axisn=math::cos(math::pi*j/h);
					siden=math::sin(math::pi*j/h);
					axis=axisn;
					side=siden;
					tv=(float)j/h;
					for(int i=0; i<=w; i++)
					{
						pvb->SetPosition(pside*cos1[i], pside*sin1[i], paxis);
						pvb->SetNormal(psiden*cos1[i], psiden*sin1[i], paxisn);
						pvb->SetTexCoord((i+offu1)/w, ptv);
						pvb++;
						pvb->SetPosition(side*cos2[i+OffAng], side*sin2[i+OffAng], axis);
						pvb->SetNormal(siden*cos2[i+OffAng], siden*sin2[i+OffAng], axisn);
						pvb->SetTexCoord((i+offu2)/w, tv);
						pvb++;
					}
					info->Set(offset, w*2, D3DPT_TRIANGLESTRIP);
					info++;
					offset+=w*2+2;
					xchg(cos1, cos2);
					xchg(sin1, sin2);
					xchg(offu1, offu2);
					OffAng=!OffAng;
				}

				// ¹Ø¶Ñ²±
				pvb->SetPosition(0, 0, -1);
				pvb->SetNormal(0, 0, -1);
				pvb->SetTexCoord(0.5f, 1);
				for(int i=w; i>=0; i--)
				{
					pvb->SetPosition(side*cos1[i], side*sin1[i], axis);
					pvb->SetNormal(siden*cos1[i], siden*sin1[i], axisn);
					pvb->SetTexCoord((float)(i+offu1)/w, tv);
					pvb++;
				}
				info->Set(offset, w, D3DPT_TRIANGLEFAN);

				assert(pvb == pstart + uCount);
				assert(m_pLast == info);

				vb.Update(pstart, count, vs);
			}
			template <typename LS> void CreateCylinder(VertexBuffer &vb, const PT vSeg, LS vs)
			{
				stack(float2, dir, vSeg.x + 1);

				dword uCount = GetCylinderSize(vSeg);
				VERTEXINFO *info = NewInfo(_GetCylinderIC(vSeg));

				TVertex *pvb, *pstart;
				SetBuffer(vb, pvb, uCount);
				pstart = pvb;

				for(int i=0; i<=vSeg.x; i++)
				{
					dir[i].set(math::cos((math::pi*2) * i / vSeg.x), math::sin((math::pi*2) * i / vSeg.y));
				}

				for(int y=0; y<vSeg.y; y++)
				{
					info->Set(pvb - pstart, vSeg.x * 2, D3DPT_TRIANGLESTRIP);
					info++;
					for(int x=0; x<=vSeg.x; x++)
					{
						float zpos = (float)y / vSeg.y;
						pvb->SetPosition(dir[x].x, dir[x].y, zpos);
						pvb->SetNormal(dir[x].x, dir[x].y, zpos);
						pvb->SetTexCoord((float)x / vSeg.x, zpos);
						pvb++;

						zpos = ((float)y + 1) / vSeg.y;
						pvb->SetPosition(dir[x].x, dir[x].y, zpos);
						pvb->SetNormal(dir[x].x, dir[x].y, zpos);
						pvb->SetTexCoord((float)x / vSeg.x, zpos);
						pvb++;
					}
				}

				assert(pvb == pstart + uCount);
				assert(m_pLast == info);

				vb.Update(pstart, uCount, vs);
			}
			template <typename LS> void CreateGround(VertexBuffer &vb, const float3 *src, const PT &vSegments, LS vs)
			{
				dword uCount = GetPlaneSize(vSegments);
				dword uPitch = vSegments.x + 1;

				TVertex *pvb, *pstart;
				SetBuffer(vb, pvb, uCount);
				pstart=pvb;

				VERTEXINFO *info = NewInfo(_GetPlaneIC(vSegments));

				float2 vInc;
				vInc.x = 1.f / vSegments.x;
				vInc.y = 1.f / vSegments.y;

				for(int y=0; y<vSegments.y; y++)
				{
					info->Set(pvb - pstart, vSegments.x * 2, D3DPT_TRIANGLESTRIP);
					info++;
					for(int x=0; x<=vSegments.x; x++)
					{
						pvb->SetTexCoord(float2((float)x*vInc.x, ((float)y+1)*vInc.y));
						pvb->SetNormal(_CalcNormal(src, vSegments.x, vSegments.y, x, y+1));
						pvb->SetPosition(src[(y+1) * uPitch + x]);
						pvb++;

						pvb->SetTexCoord(float2((float)x*vInc.x, (float)y*vInc.y));
						pvb->SetNormal(_CalcNormal(src, vSegments.x, vSegments.y, x, y));
						pvb->SetPosition(src[y * uPitch + x]);
						pvb++;
					}
				}
				assert(pvb == pstart + uCount);
				assert(m_pLast == info);

				vb.Update(pstart, uCount, vs);
			}
		};
		template <class TVertex> class KBufferedModelT:public KModelT<TVertex>
		{
		protected:
			VertexBuffer m_VB;
		public:
			void Delete()
			{
				m_VB.Delete();
				Model_B::Delete();
			}
			void Draw()
			{
				KModelT<TVertex>::Draw(m_VB);
			}
			void Lost()
			{
				m_VB.Lost();
			}
			void Reset()
			{
				m_VB.Reset(TVertex::GetFVF());
			}
			template <typename LS> void CreateBox(const PT3 &vSeg, LS vs)
			{
				m_VB.Create(GetBoxSize(vSeg) * sizeof(TVertex), TVertex::GetFVF());
				KModelT<TVertex>::CreateBox(m_VB, vSeg, vs);
			}
			template <typename LS> void CreatePlane(const PT &vSeg, LS vs)
			{
				m_VB.Create(GetPlaneSize(vSeg) * sizeof(TVertex), TVertex::GetFVF());
				KModelT<TVertex>::CreatePlane(m_VB, vSeg, vs);
			}
			template <typename LS> void CreateSphere(int h, int w, LS vs)
			{
				m_VB.Create(GetSphereSize(h, w) * sizeof(TVertex), TVertex::GetFVF());
				KModelT<TVertex>::CreateSphere(m_VB, h, w, vs);
			}
			template <typename LS> void CreateCylinder(const PT &vSeg, LS vs)
			{
				m_VB.Create(GetCylinderSize(vSeg) * sizeof(TVertex), TVertex::GetFVF());
				KModelT<TVertex>::CreateCylinder(m_VB, vSeg, vs);
			}
			template <typename LS> void CreateGround(const float3 *src, const PT &vSegments, LS vs)
			{
				m_VB.Create(GetPlaneSize(vSegments) * sizeof(TVertex), TVertex::GetFVF());
				KModelT<TVertex>::CreateGround(m_VB, src, vSegments, vs);
			}
		};

		class CModelMapBase
		{
		public:
			CModelMapBase();
			void NoiseFill(const float2 &vOffset, PT ptSegment, float fSize, float fHeight, float fAmplitude, float fFrequency, dword repeat, Random &rand=g_Random);
			void FlatFill(const float2 &vOffset, PT ptSegment, float fSize, float fHeigh);
			void Mountain(const PT &ptPos, float size, float wide, float smooth);
			void MountainRange(const PT &ptStart, const PT &ptEnd, float size, float wide, float smooth, Random &rand=g_Random);
			void Flatland(const PT &ptPos, float size, float strength, float smooth);
			void Flatland(const PT &ptPos, float altitude, float size, float strength, float smooth);
			void Path(PT ptStart, PT ptEnd, float size, float smooth);
			float GetAltitude(float2 pt);
			const float3 GetNormal(float2 pt);
			float GetSegmentSize();
			void Delete();
			rectf GetRect();

			template <typename T> bool Test(const line3 &ln, T lambda)
			{
				float2 vTFrom= (((float2)ln.from - m_vOffset) / m_fSize);
				float2 vTTo= (((float2)ln.to - m_vOffset) / m_fSize);
				float2 vTLen= vTTo - vTFrom;

				float fNextY;
				float fIncY;

				math::settofloor();
				int fromy=fcast<int>(vTFrom.y);
				int toy=fcast<int>(vTTo.y);

				toy=math::abs(toy - fromy) + 1;

				if(vTLen.y > 0)
				{
					fIncY=1;
					fNextY= math::floor(vTFrom.y);
					if(fNextY > vTTo.y) fNextY=vTTo.y;
				}
				else
				{
					fIncY=-1;
					fNextY= math::ceil(vTFrom.y);
					if(fNextY < vTTo.y) fNextY=vTTo.y;
				}

				int nXFrom=fcast<int>(vTFrom.x);
				int nXTo;

				float y=vTFrom.y;
				do
				{
					if(vTLen.y == 0) nXTo= fcast<int>(vTTo.x);
					else nXTo=fcast<int>(vTFrom.x+(fNextY - vTFrom.y)*vTLen.x/vTLen.y);
					if(nXFrom > nXTo) xchg(nXFrom, nXTo);
					int nY=fcast<int>(y);

					for(int i=nXFrom; i<=nXTo; i++)
					{
						if(_TestBlock(ln, PT(i, nY), lambda)) return true;
					}

					y=fNextY;
					fNextY += fIncY;
					nXFrom = nXTo+1;
				}
				while(--toy);

				math::settoround();
				return false;
			}
			template <typename T> void CustomTool(const PT &ptPos, float wide, T lambda)
			{
				int nWide=math::toceilint(wide);
				int starty=_max(ptPos.y-nWide, 0);
				int endy=_min(ptPos.y+nWide, m_ptSize.y);

				nWide*=nWide;

				for(int y=starty; y<endy; y++)
				{
					int ny=y-ptPos.y;
					int len = math::sqrt(nWide - ny * ny);
					int startx = _max(ptPos.x -len, 0);
					int endx = _min(ptPos.x + len + 1, m_ptSize.x);
					for(int x=startx; x<endx; x++)
					{
						int nx=x-ptPos.x;
						float flen=math::sqrt(fast_cast<float>(nx*nx+ny*ny));
						if(flen >= wide) continue;
						lambda(m_vMap[y * m_ptSize.x + x].z, flen);
					}
				}
			}
			template <typename T> void CustomZitter(const PT &ptStart, const PT &ptEnd, int randmin, int randmax, T lambda, Random &rand=g_Random)
			{
				PT ptRange=ptEnd - ptStart;
				int nLen=ptRange.length();
				randmax-=randmin;
				for(int i=0; i<=nLen; i++)
				{
					i+=rand(randmax)+randmin;
					if(i > nLen) i=nLen;
					lambda(ptStart+(ptRange * i / nLen));
				}
			}
		protected:

			float3 *m_vMap;
			float2 m_vOffset;
			float m_fSize;
			PT m_ptSize;

			void _Allocate(const PT & ptSegments);
			void _PolyTool(fixed pos, int hloop, fixed len, TFixed<int, 2> skip, fixed leninc, fixed skipinc, float value, float vinc, float vskip);
			template <typename T> bool _TestBlock(const line3 &ln, PT ptBlock, T lambda)
			{
				int v=ptBlock.y * (m_ptSegment.x + 1);
				int v2 = v + m_ptSegment.x + 1;
				int h = ptBlock.x;
				int h2 = ptBlock.x+1;

				float3 p1= m_vMap[v + h];
				float3 p2= m_vMap[v + h2];
				float3 p3= m_vMap[v2 + h];
				float3 p4= m_vMap[v2 + h2];

				if(lambda(ln, triangle3(p1, p2, p3))) return true;
				return lambda(ln, triangle3(p4, p3, p2));
			}
		};

		template <class TVertex> class KModelMapT:public CModelMapBase
		{
		protected:
			KModelT<TVertex> m_mdMap;

		public:
			dword GetSize(const PT &ptSegments)
			{
				return m_mdMap.GetPlaneSize(ptSegments);
			}
			void Delete()
			{
				m_mdMap.Delete();
				CModelMapBase::Delete();
			}
			bool Live()
			{
				return m_mdMap.Live();
			}
			void Update(VertexBuffer &vb, const float2 &vTile)
			{
				auto ls = [vTile](TVertex& vb)
				{
					vb.TexCoord() *= vTile;
				};
				m_mdMap.CreateGround(vb, m_vMap, m_ptSize - PT(1, 1), ls);
			}
			void Draw(VertexBuffer &vb)
			{
				m_mdMap.Draw(vb);
			}

		};

	}
}