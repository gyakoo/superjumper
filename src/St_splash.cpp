#include "SuperJumper.h"

//-----------------------------------------------
static const char	SST_FADEIN = 0;
static const char	SST_FADEOUT= 1;
static const char   SST_WAIT   = 2;
static const float	s_fadeTime = 2.5f;
static const float	s_timeFactor = 1.0f;
static const float  s_waitTime   = 0.5f;
static char			s_subState = SST_FADEIN;

// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
void SJ_Splash_Update( float elapsedTime )
{
	UNREFERENCED_PARAMETER(elapsedTime);
	sStateInfo& st = g_states[ST_SPLASH];
	const float acumTime = st.acumTime * s_timeFactor;
	switch ( s_subState )
	{
		case SST_FADEIN: 
			if ( acumTime > s_fadeTime )
			{
				st.acumTime = 0.0f;
				s_subState = SST_FADEOUT;
			}break;
		case SST_FADEOUT: 
			if ( acumTime > s_fadeTime )
			{
				st.acumTime = 0.0f;
				s_subState = SST_WAIT;
			}break;
		case SST_WAIT:
			if ( acumTime > s_waitTime )
			{
				SJ_ChangeState( ST_MENU );
			}break;
	}	
	if ( g_keysDown[ VK_SPACE ] || g_keysDown[ VK_ESCAPE ] )
		SJ_ChangeState( ST_MENU );
}
//-----------------------------------------------
void SJ_Splash_Render( )
{
	const sStateInfo& st = g_states[ST_SPLASH];
	const float acumTime = st.acumTime * s_timeFactor;
	float textAlpha = 0.0f;
	switch ( s_subState )
	{
		case SST_FADEIN	: textAlpha = acumTime/s_fadeTime; break;
		case SST_FADEOUT: textAlpha = 1.0f - acumTime/s_fadeTime; if ( textAlpha < .0f ) textAlpha = 0.0f; break;
	}	
	RECT r = {0};
	g_pFont->DrawTextW( g_pSprite, SJ_GAME_NAME, -1, &r, DT_CALCRECT, 0 );
	r.left = LONG(g_clRect.right/2.0f - r.right/2.0f);
	r.top  = LONG(g_clRect.bottom/2.0f - r.bottom/2.0f);
	const int textAlphaValue = int(textAlpha * 255.0f) & 0x000000ff;				
	g_pFont->DrawTextW( g_pSprite, SJ_GAME_NAME, -1, &r, DT_NOCLIP, D3DCOLOR_ARGB(textAlphaValue, 255,255,255) );

	
}