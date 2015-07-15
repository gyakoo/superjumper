#include "SuperJumper.h"

// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
static void FormatTime( float t )
{
	const int mins = (int)floor( t / 60.0f);
	const int secs = (int)floor( t - (mins*60.0f) );
	const int msecs= (int)floor( (t - (mins*60.0f+secs) )*100.0f );
	if ( mins > 0 )	swprintf_s( g_txt, L"%02d:%02d.%02d", mins, secs, msecs );
	else			swprintf_s( g_txt, L"%02d.%02d", secs, msecs );
}
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
static bool LoadLevel( int levelNumber, bool sound=true )
{
	g_nextLevel = levelNumber;
	if ( ! SJ_NextLevel() )
	{
		SJ_Pause( PAUSE_GAMEEND );
		return false;
	}
	if ( sound )
		SJ_PlaySound( IDS_NEWLEVEL );
	// prepare each player
	sPlayerInfo* pl = NULL;
	sSprite* sp = NULL;
	for ( char p = 0; p < g_nPlayers; ++p )
	{
		pl = g_player+p;
		pl->countBlocksByLevel = 0;
		sp = g_sprites + pl->sprId;
		if ( pl->lives == 0 )
		{
			pl->lives = 1; // one live more if no lives, when level is complete
			pl->countBlocks = 0;
		}else if ( sp->active == 0 && sp->y+g_plDim[1] > SCREEN_HEIGHT )
		{
			sp->alpha = 1.0f;
			if ( p == 0 )
			{
				sp->active = 1;
				sp->x = g_clRect.right/2.0f - g_plDim[0]/2.0f;
				sp->y = 50.0f;
			}else
			{
				*sp = g_sprites[0]; // if we are inactive and out of screen (no spawned)
				sp->x += g_plDim[0];
			}
		}
	}
	return true;
}
static void PlayerDead( sPlayerInfo* pl )
{
	--pl->lives;
	SJ_PlaySound( IDS_DIE );
	pl->countBlocks -= pl->countBlocksByLevel;
	if ( pl->lives == 0 ) 
	{
		// how many players with lives == 0?
		char nAlives = 0;
		for ( char p = 0; p < g_nPlayers; ++p ) 
			if ( g_player[p].lives > 0 )
				++nAlives;
		if ( nAlives == 0 )
		{
			SJ_Pause( PAUSE_DEAD );
			return;
		}
	}

	// how many active players?
	char nActives = 0;
	for ( char p = 0; p < g_nPlayers; ++p ) 
		if ( g_sprites[g_player[p].sprId].active ) 
			++nActives;

	// if no actives, then restart level
	if ( nActives == 0 )
	{
		SJ_Pause( PAUSE_DIE );
	}
}
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
void SJ_GameNormal_Update( float elapsedTime )
{
	if ( g_keysDown[VK_ESCAPE] && g_timeToNextKey <= 0.0f )
	{
		g_timeToNextKey = 0.3f;
		if ( g_paused && (g_pausedState == PAUSE_RETURNTOMENU || g_pausedState == PAUSE_GAMEEND) )
			SJ_ChangeState( ST_MENU );
		else
			SJ_Pause( PAUSE_RETURNTOMENU );
		return;
	}
	if ( g_keysDown[VK_SPACE] && g_timeToNextKey <= 0.0f )
	{
		g_timeToNextKey = 0.25f;
		if ( g_paused ) // unpause
		{	
			switch ( g_pausedState )
			{
				case PAUSE_LEVELSTART: 
					LoadLevel( g_nextLevel, false ); 
					SJ_Unpause();
				break;
				case PAUSE_LEVELFINISHED: 
					if ( LoadLevel( g_nextLevel, true ) )
						SJ_Pause( PAUSE_LEVELSTART );
				break;
				case PAUSE_DEAD: 
					SJ_Pause( PAUSE_GAMEEND );
					SJ_GameNormal_Begin( g_curState );
				break;
				case PAUSE_DIE:
					LoadLevel( g_nextLevel, true );
					SJ_Pause( PAUSE_LEVELSTART );
				break;
				case PAUSE_GAMEEND:
					LoadLevel( g_nextLevel=0, true );
					SJ_Pause( PAUSE_LEVELSTART );
				break;
				default: SJ_Unpause(); break;
			};
		}else
		{
			SJ_Pause( PAUSE_USER );
		}			
	}	
	// update effects
	sSprite* spr = NULL;
	for ( int i = 0; i < MAX_SPRITES; ++i ) // blocks alpha
	{
		spr = &g_sprites[i];
		if ( spr->active==0 && spr->alpha > 0.0f )
			spr->alpha -= elapsedTime*3.0f;
	}
	// processing from now on, when no pause
	if ( g_paused )
	{ 
		g_states[g_curState].acumTime -= elapsedTime; 
		return; 
	}
	g_levelTime -= elapsedTime;

	// updating all players
	sPlayerInfo* pl = NULL;
	for ( char i = 0; i < g_nPlayers; ++i )
	{
		pl = g_player+i;
		if ( !g_sprites[pl->sprId].active ) continue;
		// player moving and forces
		SJ_UpdatePlayer( elapsedTime, *pl );
		
		// detecting dead
		if ( g_sprites[pl->sprId].y+g_plDim[1] > SCREEN_HEIGHT ||
			( g_gameMode == GAMEMODE_TIMEATTACK && g_levelTime <= 0.0f ) ) 
		{
			g_sprites[pl->sprId].active = 0;
			PlayerDead( pl );
		}
	}
	
	// are we finish level?
	if ( g_remainBlocks == 0 )
	{
		SJ_PlaySound( IDS_ENDLEVEL );
		++g_nextLevel;
		SJ_Pause( PAUSE_LEVELFINISHED );
	}
}
//-----------------------------------------------
void RenderStdSprites( )
{
	const sSprite* spr = NULL;
	D3DXVECTOR3 v(0,0,0);
	D3DCOLOR col;
	float scrOffs[2] = { (g_clRect.right - g_blDim[0]*TILES_WIDTH)/2.0f,
						 (g_clRect.bottom- g_blDim[1]*TILES_HEIGHT)/2.0f };
	// draw blocks
	for ( int i = MAX_SPRITES-1; i >= 0; --i )
	{
		spr = &g_sprites[i];
		if ( spr->active>0 || spr->alpha > 0.0f )
		{
			if ( i < g_nPlayers )
			{
				col = D3DCOLOR_ARGB(int(spr->alpha*255.0f), g_plColors[i][0],g_plColors[i][1],g_plColors[i][2]);
			}else
			{
				col = D3DCOLOR_ARGB(int(spr->alpha*255.0f), 255,255,255);
			}
			v.x = scrOffs[0]+spr->x; v.y = scrOffs[1]+spr->y-(6.0f*spr->alpha);
			g_pSprite->Draw( g_pTextures[spr->texId], NULL, NULL, &v, col );
		}
	}	
}
//-----------------------------------------------
void RenderStdGUI()
{
	#pragma warning(disable:4238)
	RECT r={0};	 
	// -- Drawing pause message --
	if ( g_paused )
	{
		const char* sPaused = "[SPACE] to continue";
		switch ( g_pausedState )
		{
			case PAUSE_DEAD			: sPaused = "[SPACE] to restart"; break;
			case PAUSE_RETURNTOMENU	: sPaused = "[SPACE] Continue, [ESC] Main Menu"; break;
			case PAUSE_GAMEEND		: sPaused = "[SPACE] Restart, [ESC] Main Menu"; break;
		}
		g_pFont->DrawTextA( g_pSprite, sPaused, -1, &r, DT_CALCRECT, 0xffffffff);
		r.left = g_clRect.right/2 - r.right/2;
		r.right += r.left;
		r.top = (LONG)(g_clRect.bottom - r.bottom - 4);
		r.bottom += r.top;
		g_pFont->DrawTextA( g_pSprite, sPaused, -1, &r, DT_NOCLIP, 0xffffffff);
	}
	// -- Drawing message on center --
	if ( g_pausedState == PAUSE_LEVELFINISHED || g_pausedState==PAUSE_DEAD || g_pausedState == PAUSE_GAMEEND )
	{
		r.left = r.top = 0;
		const char* sLvlComp = NULL;
		DWORD color = 0;
		switch ( g_pausedState )
		{
			case PAUSE_DEAD		: sLvlComp = "You're dead!"; color=0xffff8080; break;
			case PAUSE_GAMEEND	: sLvlComp = "FINISH!"; color=0xffff8080; break;
			case PAUSE_LEVELFINISHED: sLvlComp = "Level completed!"; color=0xff80ff80; break;
		}
		g_pFont->DrawTextA( g_pSprite, sLvlComp, -1, &r, DT_CALCRECT, 0);
		r.left = g_clRect.right/2 - r.right/2;
		r.right += r.left;
		r.top = (LONG)(g_clRect.bottom - r.bottom*2 - 4);
		r.bottom += r.top;
		g_pFont->DrawTextA( g_pSprite, sLvlComp, -1, &r, DT_NOCLIP, color );
	}	
	// -- Drawing level name --
	r.left = 10; r.top = g_clRect.bottom-20;
	if ( *g_levelName != 0 )
	{
		swprintf_s( g_txt, L"%03d: %s", g_nextLevel, g_levelName );
		g_pFont->DrawTextW( g_pSprite, g_txt, -1, &r, DT_NOCLIP, 0xffff1010 );
		r.top += 20;
	}
	// -- Drawing hearts --
	sPlayerInfo* pl = NULL;
	r.left = 10; r.top = 10;
	for ( char p = 0; p < g_nPlayers; ++p )
	{
		pl = g_player+p;
		if ( pl->lives > 0 )
		{
			for ( int i = 0; i < pl->lives; ++i )
			{
				g_pSprite->Draw( g_pTextures[IDT_HEART], NULL, NULL, &D3DXVECTOR3( (float)r.left, (float)r.top, 0.0f ), 
								 D3DCOLOR_ARGB(255,g_plColors[p][0],g_plColors[p][1],g_plColors[p][2]) );
				r.left += 12;
			}
		}else
		{
			g_pFont->DrawTextW( g_pSprite, L"Dead", -1, &r, DT_NOCLIP, D3DCOLOR_ARGB(255,g_plColors[p][0],g_plColors[p][1],g_plColors[p][2]) );
		}
		r.left = (p+1)*80;
	}
	// -- Drawing boosts --
	r.left = 10; r.top += 14;	
	for ( char p = 0; p < g_nPlayers; ++p )
	{
		pl = g_player+p;
		if ( pl->lives > 0 )
		{
			for ( int i = 0; i < pl->levelBoosts; ++i )
			{
				g_pSprite->Draw( g_pTextures[IDT_ARROW], NULL, NULL, &D3DXVECTOR3( (float)r.left, (float)r.top, 0.0f ), 
								D3DCOLOR_ARGB(255,g_plColors[p][0],g_plColors[p][1],g_plColors[p][2]));
				r.left += 12;
			}
		}
		r.left = (p+1)*80;
	}
	// -- Drawing blocks count --
	r.left = 10; r.top += 14;
	for ( char p = 0; p < g_nPlayers; ++p )
	{
		pl = g_player+p;
		if ( pl->countBlocks > 0 ) 
		{
			swprintf_s( g_txt, L"%d", pl->countBlocks );
			g_pFont->DrawTextW( g_pSprite, g_txt, -1, &r, DT_NOCLIP, D3DCOLOR_ARGB(255,g_plColors[p][0],g_plColors[p][1],g_plColors[p][2]) );
		}
		r.left = (p+1)*80;
	}
	// -- Drawing time if proceed --
	r.left = 10; r.top += 20;
	if ( g_gameMode == GAMEMODE_TIMEATTACK )
	{
		//FormatTime( g_states[g_curState].acumTime );
		//g_pFont->DrawTextW( g_pSprite, g_txt, -1, &r, DT_NOCLIP, 0xffffffff );
		//r.top += 14;
		FormatTime( g_levelTime );
		g_pFont->DrawTextW( g_pSprite, g_txt, -1, &r, DT_NOCLIP, g_levelTime < 1.0f ? 0xffff2020 : 0xffffffff );
	}
	// -- Drawing version --
	r.left = g_clRect.right-30; r.top = g_clRect.bottom-20;
	g_pFont->DrawTextW( g_pSprite, SJ_VERSION, -1, &r, DT_NOCLIP, 0x55888888 );
#pragma warning(default:4238)
}
//-----------------------------------------------
void SJ_GameNormal_Render( )
{
	RenderStdSprites();
	RenderStdGUI();
}
//-----------------------------------------------
void SJ_GameNormal_Begin( char lastState )
{
	for ( char p = 0; p < g_nPlayers; ++p )
	{
		g_player[p].lives = g_livesMax;
		g_player[p].countBlocks = g_player[p].countBlocksByLevel = 0;
		g_sprites[ g_player[p].sprId ].active = 1;		
	}
	LoadLevel( g_nextLevel = 0 );
	SJ_Pause( PAUSE_LEVELSTART );
}