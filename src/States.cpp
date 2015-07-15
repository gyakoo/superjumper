#include "SuperJumper.h"

//-----------------------------------------------------------------------------
inline void NullF( float ){}
inline void NullC( char ){}
inline void NullV(){}
//-----------------------------------------------------------------------------
sStateInfo	g_states[ST_MAX] = { 
	{.0f, SJ_Splash_Update		, SJ_Splash_Render		, NullC						, NullV				},	// ST_SPLASH
	{.0f, SJ_Menu_Update		, SJ_Menu_Render		, SJ_Menu_Begin				, SJ_Menu_End		},	// ST_MENU
	{.0f, SJ_Credits_Update		, SJ_Credits_Render		, SJ_Credits_Begin			, SJ_Credits_End	},	// ST_CREDITS
	{.0f, SJ_GameIntro_Update	, SJ_GameIntro_Render	, SJ_GameIntro_Begin		, SJ_GameIntro_End	},	// ST_GAME_INTRO
	{.0f, SJ_GameNormal_Update	, SJ_GameNormal_Render	, SJ_GameNormal_Begin		, NullV				},	// ST_GAME_LEVELS
	{.0f, SJ_GameEnd_Update		, SJ_GameEnd_Render		, SJ_GameEnd_Begin			, SJ_GameEnd_End	},	// ST_GAME_END
};
char		g_curState = -1; // We don't star with st_splash because we call to SJ_ChangeState() initially

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SJ_ChangeState( char newState )
{
	// return if out of bounds or same state as current
	if ( g_curState == newState || newState < 0 || newState >= ST_MAX ) 
		return;	
	// we update g_curState now, because begin/end could use it
	const char lastState = g_curState;
	g_curState = newState;
	// end/begin
	if ( lastState >= 0 && lastState < ST_MAX ) 
		g_states[lastState].End();
	g_states[g_curState].Begin( lastState );
	g_states[ g_curState ].acumTime = 0.0f;
}
