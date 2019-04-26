//
//#include <KR3/main.h>
//#include <KR3/wl/windows.h>
//
//#include <KRDirect3D/rtypes.h>
//#include "textures.h"
//#include "resource.h"
//#include <KRWin/winx.h>
//#include <KRDirect3D/fvf.h>
//#include <KRDirect3D/model.h>
//#include <KR3/math/math.h>
//#include <KRMovie/movie.h>
//#include <KR3/mem/encoder.h>
//#include <KR3/initializer.h>
//
////#include <KR/math/color.h>
//
//using namespace kr;
//
//using namespace d3d;
//using math::pi;
//
//CDirect3D				g_d3d;
//VertexBuffer			g_pVB = nullptr;
//dshow::MovieTexture    g_movie;
//
//const int nGrid = 50;
//
//BYTE bkRed = 50;
//BYTE bkGrn = 100;
//BYTE bkBlu = 150;
//
//HRESULT InitGeometry();
//VOID SetupMatrices();
//void CalculateShades();
//void UpgradeGeometry(float tuW,float tuH);
//HRESULT InitDShowTextureRenderer();
//
//SETVERTEX_FVF(VideoVertex,D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
//
//const char16 g_szClassName[] = u"DirectShow Texture3D9 Sample";
//
//#define TIMER_ID    100
//#define TIMER_RATE  20      // milliseconds
//
//void CDirect3D::SetRenderState()
//{
//	D3DSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
//	D3DSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
//
//	// Add filtering
//	D3DSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
//	D3DSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
//
//	// set maximum ambient light
//	D3DRenderState(D3DRS_AMBIENT,RGB(255,255,255));
//
//	// Turn off culling
//	D3DRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
//
//	// Turn off D3D lighting
//	D3DRenderState(D3DRS_LIGHTING, FALSE );
//
//	// Turn on the zbuffer
//	D3DRenderState( D3DRS_ZENABLE, TRUE );
//
//	// Set texture states
//	D3DTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
//	D3DTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
//	D3DTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
//	D3DTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
//}
//CDirect3D::CDirect3D()
//{
//}
//void CDirect3D::Create()
//{
//	D3DCreate(false);
//	SetRenderState();
//	InitGeometry();
//	InitDShowTextureRenderer();
//}
//void CDirect3D::Delete()
//{
//	D3DDelete();
//}
//void CDirect3D::OnLost()
//{
//	g_movie.OnLost();
//}
//void CDirect3D::OnReset()
//{
//	SetRenderState();
//	g_movie.OnReset();
//}
//void CDirect3D::Render()
//{
//	D3DClear(color3(bkRed,bkGrn,bkBlu));
//	if(!D3DBegin()) return;
//	
//	if(g_movie.Exists())
//	{
//		SetupMatrices();
//		CalculateShades();
//		g_movie.Use();
//		CFVFDevice::Use<VideoVertex>();
//		g_pVB.Use(0,sizeof(VideoVertex));
//		D3DDraw( D3DPT_TRIANGLESTRIP,0,2*nGrid-2);
//	}
//
//	D3DEnd();
//	D3DPresent();
//	if(g_movie.Exists())
//	{
//		g_movie.CheckStatus();
//	}
//}
//
//HRESULT InitGeometry()
//{
//	g_pVB.Create(nGrid*2*sizeof(VideoVertex),VideoVertex::GetFVF());
//	
//	VideoVertex * pVertices;
//	if(SUCCEEDED(g_pVB->Lock(0,0,(void**)&pVertices,0)))
//	{
//		for( DWORD i=0; i<nGrid; i++ )
//		{
//			FLOAT theta = (2*pi*i)/(nGrid-1) + (FLOAT)(pi/2.f);
//
//			pVertices->Position3()= float3( sinf(theta),-1.0f, cosf(theta) );
//			pVertices->Diffuse()	= 0xffffffff;
//			pVertices->TexCoord()	= float2(((FLOAT)i)/((FLOAT)nGrid-1.f) ,1.0f);
//			pVertices++;
//
//			pVertices->Position3()= float3( sinf(theta), 1.0f, cosf(theta) );
//			pVertices->Diffuse()	= 0xffffffff;
//			pVertices->TexCoord()	= float2(((FLOAT)i)/((FLOAT)nGrid-1.f),0.0f);
//			pVertices++;
//		}
//
//		g_pVB->Unlock();
//	}
//
//    return S_OK;
//}
//void UpgradeGeometry(float tuW,float tvH)
//{
//    VideoVertex* pVertices;
//    if ( SUCCEEDED( g_pVB->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
//    {
//		for( DWORD i=0; i<nGrid; i++ )
//		{
//			FLOAT theta = (2*D3DX_PI*i)/(nGrid-1) + (FLOAT)(D3DX_PI/2.f);
//
//			pVertices->TexCoord() = float2(tuW * ((FLOAT)i)/((FLOAT)nGrid-1.f) , tvH);
//			pVertices++;
//			pVertices->TexCoord() = float2(tuW * ((FLOAT)i)/((FLOAT)nGrid-1.f) , 0.0f);
//			pVertices++;
//		}
//
//		g_pVB->Unlock();
//    }
//}
//VOID SetupMatrices()
//{
//    matrix4 mWorld;
//	mWorld.RotationX((FLOAT)(GetTickCount()/2000.0f + D3DX_PI/2.0));
//	
//	Camera cam;
//	RC rc = g_pMainWindow->GetClientRect();
//    cam.SetProjection((float)rc.r,(float)rc.b, D3DX_PI/4.f, 1.0f, 100.0f );
//	cam.SetView(float3( 0.0f, 3.0f,-3.0f ),float3( 0.0f, -1.0f, 1.0f ),float3( 0.0f, 1.0f, 0.0f ));
//	CFVFDevice::UpdateProjection(cam);
//	CFVFDevice::UpdateView(cam);
//	CFVFDevice::Transform(cam,mWorld);
//}
//void CalculateShades()
//{
//    float cost = (float)cos(GetTickCount()/2000.0f);
//
//    static BYTE btRed1, btGrn1, btBlu1;
//    static BYTE btRed2, btGrn2, btBlu2;
//
//    VideoVertex* pVertices = 0;
//    if ( SUCCEEDED( g_pVB->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
//    {
//        {
//            btRed1 = (BYTE)(((0xFF-bkRed)*cost + (0xFF+bkRed))/2);
//            btGrn1 = (BYTE)(((0xFF-bkGrn)*cost + (0xFF+bkGrn))/2);
//            btBlu1 = (BYTE)(((0xFF-bkBlu)*cost + (0xFF+bkBlu))/2);
//        }
//
//        {
//            btRed2 = (BYTE)(((bkRed-0xFF)*cost + (0xFF+bkRed))/2);
//            btGrn2 = (BYTE)(((bkGrn-0xFF)*cost + (0xFF+bkGrn))/2);
//            btBlu2 = (BYTE)(((bkBlu-0xFF)*cost + (0xFF+bkBlu))/2);
//        }
//
//        for( DWORD i=0; i<nGrid; i++ )
//        {
//			pVertices[2*i+0].Diffuse()    = D3DCOLOR_XRGB(btRed1, btGrn1, btBlu1);
//            pVertices[2*i+1].Diffuse()    = D3DCOLOR_XRGB(btRed2, btGrn2, btBlu2);
//        }
//        g_pVB->Unlock();
//    }
//}
//HRESULT InitDShowTextureRenderer()
//{
//#define FILE_FILTER_TEXT \
//    L"Video Files (*.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v)\0*.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v\0"\
//    L"Audio files (*.wav; *.mpa; *.mp2; *.mp3; *.au; *.aif; *.aiff; *.snd)\0*.wav; *.mpa; *.mp2; *.mp3; *.au; *.aif; *.aiff; *.snd\0"\
//    L"MIDI Files (*.mid, *.midi, *.rmi)\0*.mid; *.midi; *.rmi\0"\
//    L"Image Files (*.jpg, *.bmp, *.gif, *.tga)\0*.jpg; *.bmp; *.gif; *.tga\0"\
//    L"All Files (*.*)\0*.*;\0\0"
//
//    WCHAR strFileName[MAX_PATH] = {0};
//	if(!OpenFileName(strFileName, L"Open Media File", L"*", FILE_FILTER_TEXT))
//	{
//        return E_FAIL;
//	}
//
//	HRESULT hr;
//	if(FAILED(hr = g_movie.Create())) return hr;
//	if(FAILED(hr = g_movie.Render((RString)strFileName))) return hr;
//	g_movie.SetVolume(-10000);
//	if(FAILED(hr = g_movie.Run())) return hr;
//
//	float2 rate = g_movie.GetSizeRate();
//	UpgradeGeometry(rate.x,rate.y);
//
//    return S_OK;
//}
//
//LRESULT WINAPI MsgProc( Window* pWindow, UINT msg, WPARAM wParam, LPARAM lParam )
//{
//    switch( msg )
//    {
//        case WM_DESTROY:
//            PostQuitMessage( 0 );
//            return 0;
//
//        case WM_TIMER:
//        case WM_PAINT:
//            g_d3d.Render();       // Update the main window when needed
//            break;
//
//        case WM_DISPLAYCHANGE:
//            g_d3d.Delete();
//			g_d3d.Create();
//            break;
//
//		case WM_SIZE:
//			g_d3d.D3DResize();
//			break;
//        case WM_CHAR:
//        {
//            if (wParam == VK_ESCAPE)
//            {
//				pWindow->PostMessage(WM_CLOSE,0,0);
//            }
//            else if(wParam == VK_SPACE)
//            {
//                bkRed = g_Random(0xFF);
//                bkGrn = g_Random(0xFF);
//                bkBlu = g_Random(0xFF);
//            }
//            else if(wParam == VK_RETURN )
//            {
//                bkRed = bkGrn = bkBlu = 0x0;
//            }
//        }
//        break;
//    }
//
//	return DefWindowProcW(pWindow, msg, wParam, lParam);
//}
//
//#include <KR/encoder.h>
//
//#include <KR3/mem/text.h>
//#include <KR3/mem/idmap.h>
//#include <KR3/mem/bufferqueue.h>
//
//
//using namespace kr;
//
//struct Test
//{
//	int a;
//	int b;
//	int c;
//};
//template <> struct Key<Test>
//{
//	static int getKey(const Test & t)
//	{
//		return t.a;
//	}
//};
//
//#include <KR3/io/file.h>
//#include <KR3/io/bufferedstream.h>
//
//int CALLBACK WinMain(HINSTANCE, HINSTANCE,LPSTR,int)
//{
//#ifdef _DEBUG
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//	//_crtBreakAlloc = 707;
//#endif
//	{
//		AText text;
//		text << "asd";
//	}
//	clearTempBuffer();
//#ifdef _DEBUG
//	_CrtDumpMemoryLeaks();
//#endif
//	return 0;
//	Finally([]()
//	{
//		g_pMainWindow->KillTimer(TIMER_ID);
//		g_movie.Delete();
//		g_d3d.Delete();
//		UnregisterClassW(g_szClassName, g_hInstance);
//	});
//	WindowClass wc(g_szClassName, MsgProc, IDI_TEXTURES, 0);
//	wc.Register();
//
//	Window::CreatePrimary(g_szClassName, L"DS - Texture Video", WS_VISIBLE | WS_OVERLAPPEDWINDOW, 1024, 768);
//	g_d3d.Create();
//	g_pMainWindow->SetTimer(TIMER_ID, TIMER_RATE, nullptr);
//	return MessageStruct().DefLoop();
//}