
#include "model.h"

Model_B::Model_B()
{
	m_pInfo = nullptr;
}
Model_B::Model_B(dword offset,int size,const VERTEXINFO *info,dword infocount)
{
	Set(offset,size,info,infocount);
}
void Model_B::Set(dword offset,int size,const VERTEXINFO *info,dword infocount)
{
	m_uOffset=offset;

	if(info)
	{
		m_pInfo=new VERTEXINFO[infocount];
		memd::copy(m_pInfo,info,infocount*(sizeof(VERTEXINFO)/4));
		m_pLast=m_pInfo+infocount;
	}
}
void Model_B::Delete()
{
	delete m_pInfo;
}
void Model_B::_Draw(VertexBuffer &VB,dword vsize) const
{
	VB.Use(m_uOffset,vsize);
	for(register VERTEXINFO* p=m_pInfo;p<m_pLast;p++) g_pDevice->DrawPrimitive(p->tType,p->uOffset,p->uCount);
}
VERTEXINFO* Model_B::NewInfo(dword infocount)
{
	m_pInfo=new VERTEXINFO[infocount];
	m_pLast=m_pInfo+infocount;
	return m_pInfo;
}
bool Model_B::Live()
{
	return m_pInfo != nullptr;
}

CModelMapBase::CModelMapBase()
{
	m_vMap=nullptr;
}
void CModelMapBase::NoiseFill(const float2 &vOffset,PT ptSegment,float fSize,float fHeight,float fAmplitude,float fFrequency,dword repeat,Random &rand)
{
	/*
	sig(1,m) a^n = x
	sig(1,m) a^n - (sig(1,m) a^n * a) = x - x*a 
	a - a^(m+1)= x * (1 - a) 
	(a - a^(m+1)) / (1 - a) = x
	a / (1 - a)
	*/

	m_fSize=fSize;
	m_vOffset=vOffset;
						
	fHeight /= fAmplitude / ( 1 - fAmplitude);
	fHeight /= RAND_MAX;

	float randmap[16][16];
	for(int i=0;i<16;i++)
	{
		for(int j=0;j<16;j++)
		{
			randmap[i][j]=rand() * fHeight;
		}
	}

	float smulx= 4 / fSize / ptSegment.x;
	float smuly= 4 / fSize / ptSegment.y;
			
	_Allocate(ptSegment);

	float3 *pvb = m_vMap;
	for(int y=0;y<=ptSegment.y;y++)
	{
		float ypos= vOffset.y + y * fSize;
		for(int x=0;x<=ptSegment.x;x++)
		{
			float xpos= vOffset.x + x * fSize;
			float zpos= float2(xpos * smulx , ypos * smuly).noise(fAmplitude,fFrequency,repeat,randmap);

			pvb->set(xpos,ypos,zpos);
			pvb++;
		}
	}
}
void CModelMapBase::FlatFill(const float2 &vOffset,PT ptSegment,float fSize,float fHeight)
{
	m_fSize=fSize;
	m_vOffset=vOffset;
	_Allocate(ptSegment);
	float3 *pvb = m_vMap;
	for(int y=0;y <= ptSegment.y;y++)
	{
		float ypos= vOffset.y + y * fSize;
		for(int x=0;x <= ptSegment.x;x++)
		{
			float xpos= vOffset.x + x * fSize;
			pvb->set(xpos,ypos,fHeight);
			pvb++;
		}
	}
}
void CModelMapBase::Mountain(const PT &ptPos,float size,float wide,float smooth)
{
	float d=size/wide/wide *(1-smooth);

	float c1 = math::pi / wide;
	float c2 = smooth / 2 * size;

	auto lambda= [&](float &z,float len)
	{
		float x=len - wide;
		z += (math::cos(len * c1) + 1) *c2 + x*x*d;
	};
	CustomTool(ptPos,wide,lambda);
}
void CModelMapBase::MountainRange(const PT &ptStart,const PT &ptEnd,float size,float wide,float smooth,Random &rand)
{
	auto lambda = [&](PT pt)
	{
		float frand= rand(0.5f,1.f);
		float2 vAdd(rand(-0.8f,0.8f)*wide,rand(-0.8f,0.8f)*wide);
		Mountain(pt+(PT)vAdd,size * frand,wide * frand,smooth);
	};

	int nmin=fcast<int>(wide / 4);
	int nmax=nmin * 4 / 3;
	if(nmin == nmax) nmax=nmin+1;
	CustomZitter(ptStart,ptEnd,nmin,nmax,lambda,rand);
}
void CModelMapBase::Flatland(const PT &ptPos,float size,float strength,float smooth)
{
	Flatland(ptPos,m_vMap[ptPos.y * m_ptSize.x + ptPos.x].z,size,strength,smooth);
}
void CModelMapBase::Flatland(const PT &ptPos,float altitude,float size,float strength,float smooth)
{
	float margin= size * smooth;
	float c1= math::pi/(size - margin);

	auto lambda= [&](float &z,float len)
	{
		float str=strength;
		if(len > margin)
		{
			str *= math::cos((len - margin) *c1)/2 + 0.5f;
		}
		z = z * (1 - str) + altitude * str;
	};
	CustomTool(ptPos,size,lambda);
}
void CModelMapBase::Path(PT ptStart,PT ptEnd,float size,float smooth)
{
	if(ptStart.y > ptEnd.y) xchg(ptStart,ptEnd);
	ptEnd -= ptStart;
	float2 width;

	if(ptEnd.x > 0)
	{
		width=(float2)ptEnd.GetRotateCR();
		width.Normalize();
		width*=size;
			
		float fSkip=width.x / width.y;

		fixed leninc=ptEnd.x;
		leninc= leninc / ptEnd.y + (fixed)fSkip;

		TFixed<int,2> skip(-fSkip);
		skip+=m_ptSize.x;

		fixed skipinc = -leninc;

		ptStart+=(PT)width;

		_PolyTool(ptStart.y * m_ptSize.x +ptStart.x,fast_cast<int>(width.y*2),1,skip,leninc,skipinc,100,0,0);
	}
	else if(ptEnd.x < 0)
	{
		width=(float2)ptEnd.GetRotateR();
		width.Normalize();
		width*=size;
	}
}
float CModelMapBase::GetAltitude(float2 pt)
{
	pt -= m_vOffset;
		
	if(pt.x < 0) pt.x=0;
	if(pt.y < 0) pt.y=0;

	pt.x /= m_fSize;
	pt.y /= m_fSize;

	dword x,y;
	math::settofloor();
	x=fcast<int>(pt.x);
	y=fcast<int>(pt.y);
	math::settoround();
		
	float offx= pt.x - x;
	float offy= pt.y - y;

	int x2 = (x + 1);
	int y2 = (y + 1);
		
	if(x2 >= m_ptSize.x) x=x2= m_ptSize.x -1;
	if(y2 >= m_ptSize.y) y=y2= m_ptSize.y -1;
		
	y*=m_ptSize.x;
	y2*=m_ptSize.x;
	float3 &vLT = m_vMap[y + x];
	float3 &vRT = m_vMap[y + x2];
	float3 &vLB = m_vMap[y2 + x];
	float3 &vRB = m_vMap[y2 + x2];

	if(offx < offy)
	{
		return vLT.z * (1-offy) + (vLB.z * (1-offx) + vRB.z * offx) * offy;
	}
	else
	{
		return (vLT.z * (1-offx) + vRT.z * offx) * (1-offy) + vRB.z * offy;
	}
}
const float3 CModelMapBase::GetNormal(float2 pt)
{
	pt -= m_vOffset;
		
	if(pt.x < 0) pt.x=0;
	if(pt.y < 0) pt.y=0;

	pt.x /= m_fSize;
	pt.y /= m_fSize;

	dword x,y;
	math::settofloor();
	x=fcast<int>(pt.x);
	y=fcast<int>(pt.y);
	math::settoround();
		
	float offx= pt.x - x;
	float offy= pt.y - y;

	int x2 = (x + 1);
	int y2 = (y + 1);
		
	if(x2 >= m_ptSize.x) x=x2= m_ptSize.x -1;
	if(y2 >= m_ptSize.y) y=y2= m_ptSize.y -1;
		
	y*=m_ptSize.x;
	y2*=m_ptSize.x;
	float3 &vLT = m_vMap[y + x];
	float3 &vRT = m_vMap[y + x2];
	float3 &vLB = m_vMap[y2 + x];
	float3 &vRB = m_vMap[y2 + x2];

	if(offx < offy)
	{
		return triangle3(vLT,vRT,vRB).normal();
	}
	else
	{
		return triangle3(vRB,vLB,vLT).normal();
	}
}
float CModelMapBase::GetSegmentSize()
{
	return m_fSize;
}
void CModelMapBase::Delete()
{
	delete [] m_vMap;
}
rectf CModelMapBase::GetRect()
{
	return rectf(m_vOffset,m_vOffset + (float2)(m_ptSize - PT(1,1)) * m_fSize);
}
void CModelMapBase::_Allocate(const PT & ptSegments)
{
	m_ptSize = ptSegments + PT(1,1);
	m_vMap=new float3[m_ptSize.x * m_ptSize.y];
}
void CModelMapBase::_PolyTool(fixed pos, int hloop, fixed len, TFixed<int, 2> skip, fixed leninc, fixed skipinc, float value, float vinc, float vskip)
{
	TFixedPoint<float> map=&(m_vMap->z);
	map+= pos;

	fixed wloop;
	do
	{
		wloop=len;
		do
		{
			*map = value;
			value+=vinc;
			map+=3;
		}
		while((--wloop).value != 0);
		len += leninc;

		value+=vskip;
		map+=skip;
	}
	while((--hloop) != 0);
}

