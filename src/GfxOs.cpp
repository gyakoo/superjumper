#include "SuperJumper.h"
//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
HWND					g_hWnd = 0;
IDirect3D9*				g_pD3D = NULL;
IDirect3DDevice9*		g_pd3dDevice = NULL;
IDirect3DTexture9*		g_pTextures[IDT_MAX] = {0};
ID3DXSprite*			g_pSprite = NULL;
ID3DXFont*				g_pFont = NULL;
D3DPRESENT_PARAMETERS	g_d3dpp = {0};
bool					g_fullScreen = false;
const wchar_t*			g_texNames[IDT_MAX]={L"block.png", L"jumping.png", L"jumpingr.png", 
											 L"boost.png", L"heart.png", L"falling.png", 
											 L"arrow.png" };
bool					g_keysDown[256]={0};
RECT					g_clRect={0};
wchar_t					g_fontName[32]={ DEFAULT_FONTNAME };	// current font name
char					g_fontSize= DEFAULT_FONTSIZE;			// current font size

//-----------------------------------------------------------------------------
// Creates direct3D
//-----------------------------------------------------------------------------
HRESULT SJ_InitD3D( )
{
    // Create the D3D object.
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return E_FAIL;

    // Set up the structure used to create the D3DDevice. 
	g_fullScreen = !g_fullScreen;
    SJ_ToggleFullScreen( );
	
    // Create the D3DDevice
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &g_d3dpp, &g_pd3dDevice ) ) )
        return E_FAIL;

	// Create sprite & font interfaces
	if ( FAILED( D3DXCreateSprite( g_pd3dDevice, &g_pSprite ) ) )
		return E_FAIL;

	// Init basic states
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );

	GetClientRect( g_hWnd, &g_clRect );
    return S_OK;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SJ_DestroyDynResources( )
{
	// textures
	for ( int i = 0; i < IDT_MAX; ++i ) 
		SAFE_RELEASE( g_pTextures[i] );
	
	// sounds
	SJ_DestroySounds();

	// font
	SAFE_RELEASE( g_pFont );

}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
HRESULT SJ_ReloadDynResources( )
{
	SJ_DestroyDynResources();
	// textures
	for ( int i = 0; i < IDT_MAX; ++i )
	{
		swprintf_s( g_txt, L"%s/sprites/%s", g_skin, g_texNames[i] );
		if ( FAILED( D3DXCreateTextureFromFileEx( g_pd3dDevice, g_txt, D3DX_DEFAULT, D3DX_DEFAULT, 
									1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, 
									D3DX_FILTER_NONE, 0, NULL, NULL, &g_pTextures[i] ) ) )
			return E_FAIL;		
	}
	// sounds
	SJ_ReloadSounds();

	// font
	if ( FAILED( D3DXCreateFont( g_pd3dDevice, (int)g_fontSize, 0, FW_DONTCARE, 0, FALSE, 
								 DEFAULT_CHARSET, OUT_TT_PRECIS, DEFAULT_QUALITY, 
								 DEFAULT_PITCH|FF_DONTCARE, g_fontName, &g_pFont ) ) )
		return E_FAIL;
	return S_OK;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SJ_CleanupD3D()
{
	SJ_DestroyDynResources();	
	SAFE_RELEASE( g_pSprite );
    SAFE_RELEASE( g_pd3dDevice );
	SAFE_RELEASE( g_pD3D );
}

//-----------------------------------------------------------------------------
// Changes presentation parameters for windowed or fullscreen
//-----------------------------------------------------------------------------
void SJ_ToggleFullScreen( )
{
	g_fullScreen = ! g_fullScreen;
	ZeroMemory( &g_d3dpp, sizeof( g_d3dpp ) );
    g_d3dpp.Windowed = !g_fullScreen;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	GetClientRect( g_hWnd, &g_clRect );
	if ( g_fullScreen )
	{
		g_d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		g_d3dpp.FullScreen_RefreshRateInHz = 0;
		g_d3dpp.BackBufferWidth = SCREEN_WIDTH;
		g_d3dpp.BackBufferHeight = SCREEN_HEIGHT;
		g_clRect.right = SCREEN_WIDTH;
		g_clRect.bottom = SCREEN_HEIGHT;
		SetCursor( NULL );
	}		
	ShowCursor( BOOL(!g_fullScreen) );
	if ( g_pd3dDevice != NULL )
		g_pd3dDevice->ShowCursor( BOOL(!g_fullScreen) );
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SJ_Render()
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 5, 5, 10 ), 1.0f, 0 );
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
    {
		if ( SUCCEEDED( g_pSprite->Begin(D3DXSPRITE_ALPHABLEND) ) )
		{
			g_states[g_curState].Render();
			g_pSprite->End();
		}
        g_pd3dDevice->EndScene();
    }
	HRESULT hr = g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	if ( hr == D3DERR_DEVICELOST || (g_keysDown[VK_F1] && g_timeToNextKey <= 0.0f) )
	{
		if ( g_keysDown[VK_F1] )
		{
			SJ_ToggleFullScreen();
			g_timeToNextKey = 0.1f;
		}
		g_pSprite->OnLostDevice();
		g_pFont->OnLostDevice();
		g_pd3dDevice->Reset(&g_d3dpp);
		g_pSprite->OnResetDevice();
		g_pFont->OnResetDevice();
		SJ_Pause( PAUSE_FULLSCREEN );
	}
}
