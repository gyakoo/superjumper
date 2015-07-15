#include "SuperJumper.h"
#include <io.h>
#pragma warning ( disable: 4996 )
struct sMenuEntry
{
	wchar_t menuLabel[16];
	wchar_t menuValue[16];
};
enum { MENU_PLAYERS = 0, MENU_MODE, MENU_DIFFICULTY, MENU_SKIN, MENU_EXIT };
static const D3DCOLOR	s_cSelected = 0xffffff00;
static const D3DCOLOR	s_cNormal = 0xffffffff;
static const int		s_nEntries = 5;
static const float		s_keyTime = 0.15f;
static int				s_curEntry = 0;
static const wchar_t*	s_difficulties[]={ L"easy", L"medium", L"hard", L"extreme" };
static const wchar_t*	s_gameModes[]={ L"Normal", L"Time Attack" };
static char				s_diffValue = 0;
static char				s_skinValue = 0;
static wchar_t			s_availSkins[MAX_AVAILSKINS][64] = {0};
static char				s_curSkin = 0;
static sMenuEntry		s_menuEntries[s_nEntries] = { 
	{L"Players:"	, L"1"},
	{L"Mode:"		, L"Normal" },
	{L"Difficulty:"	, L"easy" },
	{L"World:"		, L"default" }, // A 'world' is a 'skin' internally
	{L"Exit"		, L"game"} 
};
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
static void Menu_UpdateLabels()
{
	swprintf_s( s_menuEntries[ MENU_PLAYERS ].menuValue, L"%d", g_nPlayers );
	swprintf_s( s_menuEntries[ MENU_MODE ].menuValue, L"%s", s_gameModes[ g_gameMode ] );
	swprintf_s( s_menuEntries[ MENU_DIFFICULTY ].menuValue, L"%s", s_difficulties[ s_diffValue ] );
	swprintf_s( s_menuEntries[ MENU_SKIN ].menuValue, L"%s", s_availSkins[ s_skinValue ] );
}
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
void SJ_Menu_Update( float elapsedTime )
{
	if ( g_timeToNextKey < 0.0f )
	{
		if ( g_keysDown[VK_DOWN] )
		{
			s_curEntry = (s_curEntry+1)%s_nEntries;
			g_timeToNextKey = s_keyTime;
			SJ_PlaySound( IDS_MENUMOVE );
		}else if ( g_keysDown[VK_UP] )
		{
			--s_curEntry; 
			if ( s_curEntry < 0 ) s_curEntry = s_nEntries-1;
			g_timeToNextKey = s_keyTime;
			SJ_PlaySound( IDS_MENUMOVE );
		}else if ( g_keysDown[VK_LEFT] || g_keysDown[VK_RIGHT] )
		{
			SJ_PlaySound( IDS_MENUCHANGE );
			const char delta = g_keysDown[VK_LEFT]?-1:+1;
			switch ( s_curEntry )
			{
				case MENU_PLAYERS:
					g_nPlayers += delta;
					if ( g_nPlayers < 1 ) g_nPlayers = MAX_PLAYERS;
					else if ( g_nPlayers > MAX_PLAYERS ) g_nPlayers = 1;
				break;
				case MENU_MODE:
					g_gameMode += delta;
					if ( g_gameMode < 0 ) g_gameMode = GAMEMODE_MAX-1;
					else if ( g_gameMode >= GAMEMODE_MAX ) g_gameMode = 0;
				break;
				case MENU_DIFFICULTY:
					s_diffValue += delta;				
					if ( s_diffValue < 0 ) s_diffValue = 3;
					else if ( s_diffValue > 3 ) s_diffValue = 0;
				break;
				case MENU_SKIN:
					s_skinValue += delta;
					if ( s_skinValue < 0 ) s_skinValue = s_curSkin-1;
					else if ( s_skinValue >= s_curSkin ) s_skinValue = 0;
				break;
			}
			Menu_UpdateLabels();
			g_timeToNextKey = s_keyTime;
		}else if ( g_keysDown[VK_RETURN] )
		{
			switch ( s_curEntry )
			{
				case MENU_PLAYERS: SJ_ChangeState( ST_GAME_LEVELS ); break;
				case MENU_EXIT   : PostQuitMessage(0); break;
			}
			g_timeToNextKey = s_keyTime;
		}else if ( g_keysDown[VK_ESCAPE] )
		{
			SJ_PlaySound( IDS_MENUMOVE );
			if ( s_curEntry == MENU_EXIT )
				PostQuitMessage(0);
			else
				s_curEntry = MENU_EXIT;
			g_timeToNextKey = s_keyTime;
		}
	}
}
//-----------------------------------------------
void SJ_Menu_Render( )
{
	RECT r={0};
	r.top = 10; r.left = 10;
	g_pFont->DrawTextW( g_pSprite, SJ_GAME_NAME L" " SJ_VERSION, -1, &r, DT_NOCLIP, 0xffffffff );
	r.top = LONG(g_clRect.bottom/2 - (s_nEntries*20.0f)/2.0f );
	const sMenuEntry* me = NULL;
	for ( int i = 0; i < s_nEntries; ++i )
	{
		me = s_menuEntries+i;
		_swprintf( g_txt, s_curEntry==i?L"< %s %s >":L"%s %s", me->menuLabel, me->menuValue );
		g_pFont->DrawTextW( g_pSprite, g_txt, -1, &r, DT_CALCRECT, 0 );
		r.left = LONG( g_clRect.right/2.0f - (r.right-r.left)/2.0f );
		if ( s_curEntry == i )
			r.left += LONG(sinf(g_states[g_curState].acumTime*10.0f)*2.0f);
		g_pFont->DrawTextW( g_pSprite, g_txt, -1, &r, DT_NOCLIP, s_curEntry==i?s_cSelected:s_cNormal );
		r.top += 20;
	}
	r.top = g_clRect.bottom - 20;
	r.left = 10;
	g_pFont->DrawTextW( g_pSprite, SJ_AUTHOR, -1, &r, DT_NOCLIP, 0x44ffffff );
	g_pSprite->Draw( g_pTextures[ IDT_BLOCK ], NULL, NULL, &D3DXVECTOR3( g_clRect.right*0.75f - g_blDim[0]/2.0f,
																		 g_clRect.bottom*0.25f - g_blDim[1]/2.0f, 0.0f ), 0xffffffff );
}
//-----------------------------------------------
void SJ_Menu_Begin( char lastState )
{
	g_timeToNextKey = 0.5f;//s_keyTime*0.5f;
	
	ZeroMemory( s_availSkins, sizeof(s_availSkins) );
	s_curSkin = 0;
	// -- Retrieve all available skins
	wchar_t path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH-1, path);
	_swprintf( g_txt, L"%s\\*", path );
#ifdef _WIN64
	PVOID OldValue = NULL;
	Wow64DisableWow64FsRedirection( &OldValue );
#endif
	WIN32_FIND_DATA ffdata;
	HANDLE hFind = FindFirstFile( g_txt, &ffdata );
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( ffdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				static const wchar_t* keypaths[] = { L"config.txt", L"levels", L"levels\\easy", L"levels\\medium", 
													 L"levels\\hard", L"levels\\extreme", L"sprites" };
				bool validSkin = true;
				for ( int i = 0; i < 7 && validSkin; ++i )
				{
					_swprintf( g_txt, L"%s\\%s\\%s", path, ffdata.cFileName, keypaths[i] );
					validSkin = validSkin && _waccess_s( g_txt, 0 ) == 0;
				}
				if (  validSkin && s_curSkin < MAX_AVAILSKINS )
				{
					wcscpy_s( s_availSkins[s_curSkin], ffdata.cFileName );
					if ( _wcsicmp( ffdata.cFileName, DEFAULT_SKIN ) == 0 ) 
						s_skinValue = s_curSkin;
					++s_curSkin;
				}
			}else
			{
			}
		}while ( FindNextFile(hFind,&ffdata)!=0 );
		FindClose(hFind);
	}
#ifdef _WIN64
	if ( OldValue )
		Wow64RevertWow64FsRedirection( OldValue );
#endif
	Menu_UpdateLabels();
}
//-----------------------------------------------
void SJ_Menu_End()
{
	// here we prepare system to run the game
	SJ_ChangeDifficulty( s_difficulties[s_diffValue] );

	// depending on next state
	switch ( s_curEntry )
	{
		case MENU_PLAYERS:
			g_nextLevel = 0;
			SJ_ChangeSkin( s_availSkins[ s_skinValue ] );
			// first player is white or red depending if single or multiplayer
			if ( g_nPlayers == 1 ){ SETPLAYER_WHITE(g_plColors[0]); }
			else				  { SETPLAYER_RED(g_plColors[0]); }
			SETPLAYER_BLUE(g_plColors[1]);
			SETPLAYER_GREEN(g_plColors[2]);			
			SETPLAYER_MAGENTA(g_plColors[3]);
		break;
	}
}
#pragma warning ( default: 4996 )