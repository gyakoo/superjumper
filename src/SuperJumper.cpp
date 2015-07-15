#pragma warning( disable: 4996 4238 )
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "SuperJumper.h"
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SJ_Update( float elapsedTime )
{
	g_timeToNextKey -= elapsedTime;	

	SJ_SoundUpdate( );		
	g_states[ g_curState ].Update( elapsedTime );
	g_states[ g_curState ].acumTime += elapsedTime;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
LRESULT WINAPI SJ_MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY: PostQuitMessage( 0 ); return 0;
		case WM_KEYDOWN: g_keysDown[wParam&0xff] = true;  return 0;
		case WM_KEYUP  : g_keysDown[wParam&0xff] = false; return 0;
		case WM_ACTIVATE: 
			ZeroMemory( g_keysDown, sizeof(g_keysDown) ); 
			if ( wParam == WA_INACTIVE ) SJ_Pause( PAUSE_WMINACTIVE );
		return 0;
		case WM_SETCURSOR:  if ( g_fullScreen ) return true; break;
		case WM_SYSKEYDOWN: if ( lParam & 0x20000000 ) return 0; break;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR args, INT)
{
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
    UNREFERENCED_PARAMETER( hInst );

    // Register the window class
    WNDCLASSEX wc = { sizeof( WNDCLASSEX ), CS_CLASSDC, SJ_MsgProc, 0L, 0L,
        GetModuleHandle( NULL ), NULL, LoadCursor(NULL, IDC_ARROW), 
		NULL, NULL, SJ_GAME_NAME, NULL };
    RegisterClassEx( &wc );

    // Create the application's window
    g_hWnd = CreateWindow( SJ_GAME_NAME, SJ_GAME_NAME, WS_OVERLAPPEDWINDOW, 
							100, 100, SCREEN_WIDTH, SCREEN_HEIGHT,
                            NULL, NULL, wc.hInstance, NULL );
	LARGE_INTEGER qfreq={0};
	LARGE_INTEGER qpc0={0}, qpc1={0};	
	QueryPerformanceFrequency(&qfreq);
	float elapsed = 1.0f/50.0f;
    // Initialize Direct3D
    if( SUCCEEDED( SJ_InitD3D( ) ) && SUCCEEDED( SJ_InitSound() ) )
    {
		SJ_ChangeSkin( DEFAULT_SKIN );
		SJ_ChangeState( ST_SPLASH );

		// Selects level from arguments or from pool?
		if ( *args != 0 )	{ SJ_NextLevel( args ); SJ_ChangeState( ST_GAME_LEVELS ); }

        // Show the window
        ShowWindow( g_hWnd, SW_SHOWDEFAULT );
        UpdateWindow( g_hWnd );

        // Enter the message loop
        MSG msg;
        ZeroMemory( &msg, sizeof( msg ) );
        while( msg.message != WM_QUIT )
        {
            if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }else
			{
				QueryPerformanceCounter(&qpc0);
				SJ_Update(elapsed*g_etFactor);
                SJ_Render();
				QueryPerformanceCounter(&qpc1);
				const float newElapsed = float( double(qpc1.QuadPart - qpc0.QuadPart) / qfreq.QuadPart );
				elapsed = (newElapsed+elapsed)*0.5f;
				//if ( elapsed > 1.0f/50.0f ) elapsed = 1.0f/50.0f;
				/*else*/ if ( elapsed < 0.0f )	elapsed = 0.0001f;
			}
        }
    }else
	{
		::MessageBox( g_hWnd, L"Error initializing resources", L"Super Jumper: Error", MB_OK|MB_ICONERROR);
	}
	SJ_CleanupSound();
    SJ_CleanupD3D();
    UnregisterClass( SJ_GAME_NAME, wc.hInstance );
    return 0;
}
#pragma warning(default: 4996 4238)