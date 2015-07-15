#include "SuperJumper.h"

sSprite			g_sprites[MAX_SPRITES] = {0};
short			g_remainBlocks = 0;
wchar_t			g_levelName[64] = {0};
int				g_nextLevel = 0;
char			g_gameMode = GAMEMODE_NORMAL;
float			g_plDim[2] = {16.0f,16.0f}; // dimensions for player (16x16)
float			g_blDim[2] = {32.0f,32.0f}; // dimensions for blocks (32x32)
char			g_livesMax = 3;				// default lives
char			g_nPlayers = 1;
float			g_levelTime = 0.0f;

sPlayerInfo		g_player[MAX_PLAYERS] = { 
				{0.0f,{0.0f,0.0f},{0.0f,0.0f},true,-1,g_livesMax,0.0f, 0, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT },
				{0.0f,{0.0f,0.0f},{0.0f,0.0f},true,-1,g_livesMax,0.0f, 1, 'W', 'S', 'A', 'D' },
				{0.0f,{0.0f,0.0f},{0.0f,0.0f},true,-1,g_livesMax,0.0f, 2, 'I', 'K', 'J', 'L' },
				{0.0f,{0.0f,0.0f},{0.0f,0.0f},true,-1,g_livesMax,0.0f, 3, 'W', 'S', 'A', 'D' } };
unsigned char	g_plColors[MAX_PLAYERS][3] = {0};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SJ_ResetPlayer( sPlayerInfo& pl )
{
	pl.gravTime = 0.0f;
	pl.curForce[0] = pl.curForce[1] = 0.0f;
	pl.levelBoosts = -1;
	g_sprites[pl.sprId].texId = IDT_JUMPING;
	g_sprites[pl.sprId].alpha = 1.0f;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SJ_UpdatePlayer( float elapsedTime, sPlayerInfo& pl )
{
	pl.timeToNextKey -= elapsedTime;
	// boosting
	if ( g_keysDown[pl.keyUp] && pl.canBoost && pl.levelBoosts != 0)
	{
		if ( g_sprites[pl.sprId].texId == IDT_FALLING ) 
			pl.curForce[1] = -g_boostForce;
		else
			pl.curForce[1] -= g_boostForce;
		pl.gravTime = 0.0f;
		pl.canBoost = false;
		g_sprites[pl.sprId].texId = IDT_BOOST;
		--pl.levelBoosts;
		SJ_PlaySound( IDS_BOOST );
	}
	if ( g_keysDown[pl.keyDown] && pl.timeToNextKey <= 0.0f )
	{
		pl.curForce[1] += g_boostForce*2.0f;
		g_sprites[pl.sprId].texId = IDT_FALLING;
		pl.timeToNextKey = 0.20f;
	}
	// left-right movement
	if ( g_keysDown[pl.keyLeft] || g_keysDown[pl.keyRight] )
	{
		pl.curForce[0] += (g_keysDown[pl.keyLeft]?-g_lateralForce*elapsedTime*100.0f: int(g_keysDown[pl.keyRight])*g_lateralForce*elapsedTime*100.0f); 
	
		// sprite depending on X direction if not boosting
		if ( g_sprites[pl.sprId].texId != IDT_BOOST && g_sprites[pl.sprId].texId != IDT_FALLING && g_sprites[pl.sprId].x != pl.lastPos[0] )
			g_sprites[pl.sprId].texId = ( g_sprites[pl.sprId].x >= pl.lastPos[0] )? IDT_JUMPING : IDT_JUMPINGR;
	}

	// updating times and forces
	pl.gravTime += elapsedTime;
	pl.curForce[0] += -pl.curForce[0]*elapsedTime*g_lateralFriction;
	pl.curForce[1] += -pl.curForce[1]*elapsedTime;

	// update player (apply forces)		
	const float gravity = pl.gravTime*pl.gravTime*g_gravTimeFactor*(g_gravity/(g_plDim[1]/2.0f));
	const float newX = g_sprites[pl.sprId].x + pl.curForce[0]*elapsedTime;
	const float newY = g_sprites[pl.sprId].y + gravity*elapsedTime*100.0f + pl.curForce[1]*elapsedTime;
	
	// if not collision, then move player to target
	if ( !SJ_ComputeCollisions( pl, newX, newY ) )
	{
		pl.lastPos[0] = g_sprites[pl.sprId].x;
		pl.lastPos[1] = g_sprites[pl.sprId].y;
		g_sprites[pl.sprId].x = newX;
		g_sprites[pl.sprId].y = newY;
	}
}