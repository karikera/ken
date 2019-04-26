#include "stdafx.h"
/*
    {
        // Read in length of extra bytes.
        WORD cbExtraBytes = 0L;
        if( mmioRead( m_hmmio, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD) )
            return DXTRACE_ERR( TEXT("mmioRead"), E_FAIL );

        m_pwfx = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) + cbExtraBytes ];
        if( NULL == m_pwfx )
            return DXTRACE_ERR( TEXT("new"), E_FAIL );

        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = cbExtraBytes;

        // Now, read those extra bytes into the structure, if cbExtraAlloc != 0.
        if( mmioRead( m_hmmio, (CHAR*)(((BYTE*)&(m_pwfx->cbSize))+sizeof(WORD)),
                      cbExtraBytes ) != cbExtraBytes )
        {
            SAFE_DELETE( m_pwfx );
            return DXTRACE_ERR( TEXT("mmioRead"), E_FAIL );
        }


#include<dmusicc.h>
#include<dmusici.h>
#include"K_DirectMusic.h"

IDirectMusicPerformance8** Performance;
IDirectMusicSegment8** Segment;
IDirectMusicLoader8* Loader;
DWORD SoundCount,FileCount;

//////////////////////////////////////////////////////////////////////////
// MUSIC
DLL BOOL DM_Create(DWORD SoundCount,DWORD FileCount){
	HRESULT hr=-1;
	Com_Init();
	Performance=new IDirectMusicPerformance8*[SoundCount];
	Segment=new IDirectMusicSegment8*[FileCount];
	memset(Performance,0,4*SoundCount);
	memset(Segment,0,4*FileCount);
	SoundCount=SoundCount;
	FileCount=FileCount;
	return 1;}
DLL void DM_Delete(){
	for(DWORD i=0;i<SoundCount;i++){
		if(Performance[i]){
			Performance[i]->CloseDown();
			SAFE_RELEASE(Performance[i]);}}
	for(i=0;i<FileCount;i++) SAFE_RELEASE(Segment[i]);
	SAFE_RELEASE(Loader);
	Com_Uninit();}
DLL BOOL DM_SetPath(const char* Path){
	WCHAR WTEXT[MAX_PATH];
	SAFE_RELEASE(Loader);
	HRESULT hr=-1;
	if(CoCreateInstance(CLSID_DirectMusicLoader,0,CLSCTX_INPROC,IID_IDirectMusicLoader8,(void**)&Loader)!=0){
		KE_Add("로더 COM 초기화 실패.");
		return 0;}
	hr=Loader->SetSearchDirectory(GUID_DirectMusicAllTypes,WORDTEXT(Path,WTEXT),0);
	if(FAILED(hr)){
		KE_Add("음악 위치 지정 실패.");
		return 0;}
	return 1;}
DLL BOOL DM_Load(DWORD ID,const char* Name){
	WCHAR WTEXT[1024];
	if(ID>=FileCount){
		KE_Add("음악 파일 갯수를 초과했습니다.");
		return 0;}
	if(Segment[ID]){
		KE_Add("이미 지정되어있는 음악 파일 ID 입니다.");
		return 0;}
	HRESULT hr=-1;
	hr=Loader->LoadObjectFromFile(CLSID_DirectMusicSegment,IID_IDirectMusicSegment8,WORDTEXT(Name,WTEXT),(void**)&Segment[ID]);
	if(FAILED(hr)){
		KE_Add("음악 불러오기 실패.");
		return 0;}
	return 1;}
DLL BOOL DM_Play(DWORD ID){
	for(DWORD i=0;i<SoundCount;i++){
		if(!Performance[i]){
			if(CoCreateInstance(CLSID_DirectMusicPerformance,0,CLSCTX_INPROC,IID_IDirectMusicPerformance8,(void**)&Performance[i])!=0){
				KE_Add("퍼포먼스 COM 초기화 실패.");
				return 0;}
			HRESULT hr=Performance[i]->InitAudio(0,0,0,DMUS_APATH_SHARED_STEREOPLUSREVERB,64,DMUS_AUDIOF_ALL,0);
			if(FAILED(hr)){
				Performance[i]->Release();
				Performance[i]=0;
				KE_Add("퍼포먼스 생성 실패.");
				return 0;}
			break;}
		else if(Performance[i]->IsPlaying(0,0)) break;}
	if(FAILED(Segment[ID]->Download(Performance[i]))){
		KE_Add("음악 다운로드 실패.");
		return 0;}
	Performance[i]->PlaySegmentEx(Segment[ID],0,0,0,0,0,0,0);
	return 1;}
DLL void DM_Stop(){
	for(DWORD i=0;i<SoundCount;i++){
		if(Performance[i]) Performance[i]->Stop(0,0,0,0);}}
*/