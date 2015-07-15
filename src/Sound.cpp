#include "SuperJumper.h"

// ////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////
struct sSoundInfo
{
	BYTE*			soundBuffer;
	IXACT3Wave*		pWave;
	WAVEBANKENTRY	entry;
	int				sampleRate;
};
// ////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////
struct sSoundBank
{
	const wchar_t*	name;
	const float		volume;
};
// ////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////
static IXACT3Engine*		s_pXACTEngine = NULL;
static sSoundInfo			s_sounds[IDS_MAX] = {0};
static sSoundBank			s_soundBank[IDS_MAX]={{L"jump.wav"		, 1.0f},
												 {L"boost.wav"		, 1.0f}, 
												 {L"die.wav"		, 1.0f}, 
												 {L"collide.wav"	, 1.0f},
												 {L"ass.wav"		, 1.0f}, 
												 {L"menumove.wav"	, 0.25f}, 
												 {L"menuchange.wav"	, 0.25f},
												 {L"newlevel.wav"	, 1.0f}, 
												 {L"endlevel.wav"	, 0.25f} };

// ////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////
#pragma pack( push, 1 )
struct sPCMHeader
{
	char	chunkId[4];
	int		chunkSize;
	char	format[4];
	char	subchunk1Id[4];
	int		subchunk1Size;
	short	audioFormat;
	short	numChannels;
	int		sampleRate;
	int		byteRate;
	short	blockAlign;
	short	bitsPerSample;
	char	subchunk2Id[4];
	int		subchunk2Size;
	bool	IsValidFormat( )
	{
		return strncmp( chunkId, "RIFF", 4 ) == 0 && strncmp( format, "WAVE", 4 ) == 0 
				&& audioFormat == 1;
	}
	float	Seconds( )
	{ 
		return ( float(subchunk2Size) / (numChannels*bitsPerSample/8.0f) ) / sampleRate;
	}
};
#pragma pack( pop )
// ////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////
HRESULT SJ_InitSound( )
{
	HRESULT hr;
	if ( FAILED( hr = CoInitializeEx( NULL, COINIT_MULTITHREADED ) ) )
		return hr;
	if ( FAILED( hr = XACT3CreateEngine( 0, &s_pXACTEngine ) ) || ! s_pXACTEngine )
		return hr;
	XACT_RENDERER_DETAILS rendererDetails = {0};
	if ( FAILED( hr = s_pXACTEngine->GetRendererDetails( 0, &rendererDetails ) ) )
		return hr;
	XACT_RUNTIME_PARAMETERS xrParams = {0};
	xrParams.fnNotificationCallback = NULL;
	xrParams.lookAheadTime = XACT_ENGINE_LOOKAHEAD_DEFAULT;
	xrParams.pRendererID = rendererDetails.rendererID;
	if ( FAILED( hr = s_pXACTEngine->Initialize( &xrParams ) ) )
		return hr;
	return S_OK;
}
// ////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////
void SJ_CleanupSound( )
{
	SJ_DestroySounds();
	if ( s_pXACTEngine )
	{
		s_pXACTEngine->ShutDown();
		s_pXACTEngine->Release();
		s_pXACTEngine = NULL;
	}
	CoUninitialize();
}
// ////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////
void PrepareSound( int soundId )
{
	sSoundInfo* snd = s_sounds+soundId;
	snd->pWave->Destroy();
	//snd->entry.Format.nSamplesPerSec = snd->sampleRate + srVariation;
	s_pXACTEngine->PrepareInMemoryWave( XACT_FLAG_UNITS_MS, snd->entry, NULL, 
							snd->soundBuffer, 0, 0, &snd->pWave );
}
// ////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////
void SJ_SoundUpdate( )
{
	if ( s_pXACTEngine )
	{
		s_pXACTEngine->DoWork();
	}
}
// ////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////
void ReleaseSound( int i )
{
	if ( s_sounds[i].soundBuffer )
	{
		free( s_sounds[i].soundBuffer );
		s_sounds[i].soundBuffer = NULL;
	}
	if ( s_sounds[i].pWave )
	{
		s_sounds[i].pWave->Stop( XACT_FLAG_STOP_IMMEDIATE );
		s_sounds[i].pWave->Destroy();
		s_sounds[i].pWave = NULL;
	}
}
// ////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////
void SJ_ReloadSounds( )
{
	SJ_DestroySounds();

	FILE* fSnd = NULL;
	sPCMHeader pcmHeader;
	int bytesRead = 0;
	long dataSize = 0;
	for ( int i = 0; i < IDS_MAX; ++i )
	{
		swprintf_s( g_txt, L"%s/sounds/%s", g_skin, s_soundBank[i].name );
		fSnd = _wfopen( g_txt, L"rb" );
		if ( ! fSnd ) 
		{
			ReleaseSound(i);
			continue;
		}
		// computes data size, because subchunk2size is not always correct
		fseek( fSnd, 0, SEEK_END );
		dataSize = ftell(fSnd) - sizeof(sPCMHeader);
		fseek( fSnd, 0, SEEK_SET );
		if ( fread( &pcmHeader, 1, sizeof(sPCMHeader), fSnd ) == sizeof(sPCMHeader) )
		{
			if ( ! pcmHeader.IsValidFormat() ) continue;
			WAVEBANKENTRY& entry = s_sounds[i].entry;
			entry.Format.wFormatTag = WAVEBANKMINIFORMAT_TAG_PCM;
			entry.Format.wBitsPerSample = pcmHeader.bitsPerSample == 16 ? WAVEBANKMINIFORMAT_BITDEPTH_16 : WAVEBANKMINIFORMAT_BITDEPTH_8;
			entry.Format.nChannels = pcmHeader.numChannels;
			entry.Format.wBlockAlign = pcmHeader.blockAlign;
			entry.Format.nSamplesPerSec = s_sounds[i].sampleRate = pcmHeader.sampleRate;
			entry.Duration = DWORD(pcmHeader.Seconds()*pcmHeader.sampleRate);
			entry.PlayRegion.dwLength = dataSize;
			s_sounds[i].soundBuffer = (BYTE*)malloc( entry.PlayRegion.dwLength );// can we reuse last allocated chunk when is lesser this one?
			bytesRead = fread( s_sounds[i].soundBuffer, 1, entry.PlayRegion.dwLength, fSnd );
			if ( bytesRead != entry.PlayRegion.dwLength ) 
			{ 
				ReleaseSound(i);
				continue; 
			}
			s_pXACTEngine->PrepareInMemoryWave( XACT_FLAG_UNITS_MS, s_sounds[i].entry, NULL, s_sounds[i].soundBuffer, 0, 0, &s_sounds[i].pWave );
			fclose(fSnd);
		}
	}

}
// ////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////
void SJ_DestroySounds( )
{
	for ( int i = 0; i < IDS_MAX; ++i )
		ReleaseSound(i);
}

// ////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////
void SJ_PlaySound( int soundId )
{
	sSoundInfo* snd = s_sounds+soundId;
	if ( snd->pWave )
	{
		PrepareSound( soundId );
		if ( soundId == IDS_JUMP )
			snd->pWave->SetPitch( (XACTPITCH_MIN+(rand()%XACTPITCH_MAX_TOTAL+1)) ); 
		snd->pWave->SetVolume( s_soundBank[soundId].volume );
		snd->pWave->Play();
	}
}