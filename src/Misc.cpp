#include "SuperJumper.h"
#pragma warning ( disable: 4996 )
// Misc variables
wchar_t			g_txt[256]={0};				// shared temporal buffer
float			g_topBounce = 100.0f;		// restitution for top of blocks
float			g_lateralBounce = 300.0f;	// restitution for lateral of blocks
float			g_bottomBounce = 200.0f;	// restitution for bottom of blocks
float			g_boostForce = 150.0f;		// force when boosting
float			g_lateralForce = 25.0f;		// force when moving left/right
float			g_lateralFriction = 8.0f;	// friction for lateral forces
float			g_gravity = 10.0f;			// gravity factor
float			g_gravTimeFactor = 2.0f;	// gravity time factor
float			g_timeToNextKey = 0.0f;		// time to wait for next key 
wchar_t			g_skin[32] = {0};			// current skin folder name
wchar_t			g_difficulty[8] = {0};		// current difficulty
float			g_etFactor = 1.0f;			// elapsed time factor (i.e.bullet time)
bool			g_paused = true;
char			g_pausedState = PAUSE_USER;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SJ_ReloadConfiguration( )
{
	swprintf_s( g_txt, L"%s/%s", g_skin, CONFIG_FILE );
	FILE* f = _wfopen( g_txt, L"rt" );
	if ( !f ) return;
	wchar_t* ptr;
	int skip = 0;
	float* tgtKey;
	while ( !feof(f) )
	{
		fgetws( g_txt, sizeof(g_txt)/sizeof(wchar_t), f );
		ptr = g_txt;
		SKIP_BLANKS(ptr);						// skip line starting blanks
		if ( *ptr == COMMENT_TOKEN ) continue;	// comments
		TRIM(ptr);								// right trim
		tgtKey = NULL;
		if		( _wcsnicmp( ptr, L"topBounce", 9 ) == 0 )		{ tgtKey=&g_topBounce; skip=9; }
		else if ( _wcsnicmp( ptr, L"lateralBounce", 13 ) == 0 )	{ tgtKey=&g_lateralBounce; skip=13; }
		else if ( _wcsnicmp( ptr, L"bottomBounce", 12 ) == 0 )	{ tgtKey=&g_bottomBounce; skip=12; }
		else if ( _wcsnicmp( ptr, L"boostForce", 10 ) == 0 )	{ tgtKey=&g_boostForce; skip=10; }
		else if ( _wcsnicmp( ptr, L"lateralForce", 12 ) == 0 )	{ tgtKey=&g_lateralForce; skip=12; }
		else if ( _wcsnicmp( ptr, L"lateralFriction", 15 ) == 0){ tgtKey=&g_lateralFriction; skip=15; }
		else if ( _wcsnicmp( ptr, L"gravityFactor", 13 ) == 0 )	{ tgtKey=&g_gravTimeFactor; skip=13; }
		else if ( _wcsnicmp( ptr, L"gravity", 7 ) == 0 )		{ tgtKey=&g_gravity; skip=7; }
		else if ( _wcsnicmp( ptr, L"lives", 5 ) == 0 )			{ ptr+=5; SKIP_BLANKS(ptr); g_livesMax = (char)::_wtol(ptr); }
	//	else if ( _wcsnicmp( ptr, L"players", 7 ) == 0 )		{ ptr+=7; SKIP_BLANKS(ptr); g_nPlayers = (char)::_wtol(ptr); }
		else if ( _wcsnicmp( ptr, L"fontName", 8) == 0 )		{ ptr+=8; SKIP_BLANKS(ptr); wcscpy_s( g_fontName, ptr ); }
		else if ( _wcsnicmp( ptr, L"fontSize", 8) == 0 )		{ ptr+=8; SKIP_BLANKS(ptr); g_fontSize = (char)::_wtol(ptr); }
		if ( tgtKey )
		{
			ptr+=skip; // jump to value
			SKIP_BLANKS(ptr);
			*tgtKey = (float)::_wtof(ptr);
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool SJ_NextLevel( const wchar_t* filename /*= LEVEL_NEXT*/ )
{
	// reset player variables
	for ( char i = 0; i < g_nPlayers; ++i )
		SJ_ResetPlayer( g_player[i] );

	// reset level variables
	g_remainBlocks=0;
	g_levelName[0] = 0;
	memset( g_sprites, 0, sizeof(sSprite)*MAX_SPRITES );
	
	if ( wcscmp( filename, LEVEL_NEXT ) == 0 ) // get next from folder
	{
		swprintf_s( g_txt, L"%s/levels/%s/Level_%03d.txt", g_skin, g_difficulty, g_nextLevel );
	}else // a real filename provided
	{
		wcscpy_s( g_txt, filename );
	}
	// performs open
	FILE* f = _wfopen( g_txt, L"rt" );
	if ( !f ) 
	{
		g_nextLevel = 0;
		return false;
	}
	// read from file
	int bcount = g_nPlayers;
	int line = 0;
	while ( ! feof(f) && line < TILES_HEIGHT && bcount < MAX_SPRITES )
	{
		fgetws( g_txt, sizeof(g_txt)/sizeof(wchar_t), f );
		for ( int c = 0; c < TILES_WIDTH && g_txt[c] != L'\0'; ++c )
		{
			int spr = -1; char texId = IDT_JUMPING;
			switch ( g_txt[c] )
			{
				case TILE_BLOCK: spr = bcount; texId = IDT_BLOCK; ++bcount; break;
				case TILE_SPAWN0: 
				case TILE_SPAWN1: 
				case TILE_SPAWN2: 
				case TILE_SPAWN3:
				{
					const char ispawn = char(g_txt[c] - TILE_SPAWN0);
					spr = ispawn < g_nPlayers ? ispawn : -1; 
				}break;
			}
			if ( spr != -1 )
			{
				g_sprites[spr].x = (c*g_blDim[0]); 
				g_sprites[spr].y = (line*g_blDim[1]); 
				g_sprites[spr].active = 1;
				g_sprites[spr].texId = texId;
				g_sprites[spr].alpha = 1.0f;
			}
		}
		++line;
	}
	// level params		
	wchar_t* ptr = NULL;
	while ( ! feof(f) )
	{
		fgetws( g_txt, sizeof(g_txt)/sizeof(wchar_t), f );
		ptr = g_txt;
		SKIP_BLANKS(ptr);
		if ( *ptr == COMMENT_TOKEN ) continue; // comments
		TRIM(ptr);
		if		( _wcsnicmp( ptr, L"levelName", 9 ) == 0 ) 
		{ 
			ptr+=9; 
			SKIP_BLANKS(ptr); 
			wcsncpy_s( g_levelName, ptr, 64 ); 
			g_levelName[63]=0; 
		}else if ( _wcsnicmp( ptr, L"boosts", 6 ) == 0 )    
		{ 
			ptr+=6; 
			SKIP_BLANKS(ptr); 
			const char b = (char)::_wtol(ptr);
			for ( char i = 0; i < g_nPlayers; ++i )
				g_player[i].levelBoosts = b;
		}else if ( _wcsnicmp( ptr, L"time", 4 ) == 0 )
		{
			ptr+=4;
			SKIP_BLANKS(ptr);
			g_levelTime = ::_wtof(ptr);
		}
	}
	fclose(f);

	// counting remaining blocks
	for ( int i = 0; i < MAX_SPRITES; ++i )
	{
		if ( i < g_nPlayers || g_sprites[i].active == 0 ) continue;
		++g_remainBlocks;
	}
	// correcting init for players
	for ( char i = 0; i < g_nPlayers; ++i )
	{
		sSprite& plSpr = g_sprites[ g_player[i].sprId ];
		// center in tile
		plSpr.x += g_blDim[0]/2.0f-g_plDim[0]/2.0f;
		plSpr.y += g_blDim[1]/2.0f-g_plDim[1]/2.0f;	
	}
	return true;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SJ_ChangeSkin( const wchar_t* newSkin )
{
	if ( wcsicmp( g_skin, newSkin ) == 0 ) return; // same skin
	wcscpy_s( g_skin, newSkin );
	g_skin[ sizeof(g_skin)/sizeof(wchar_t)-1 ] = 0; // avoid overflow
	SJ_ReloadConfiguration();
	SJ_ReloadDynResources();
	// some other resets
	g_nextLevel = 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SJ_ChangeDifficulty( const wchar_t* newDiff )
{
	if ( wcsicmp( g_difficulty, newDiff ) == 0 ) return; // same as current
	wcscpy_s( g_difficulty, newDiff );
	g_difficulty[ sizeof(g_difficulty)/sizeof(wchar_t)-1 ] = 0;
	g_nextLevel = 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SJ_Pause( char type )
{
	g_pausedState = type;
	g_paused = true;
}
//-----------------------------------------------------------------------------
void SJ_Unpause( )
{
	g_pausedState = PAUSE_NONE;
	g_paused = false;
}
#pragma warning ( default: 4996 )
