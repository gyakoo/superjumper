#include "SuperJumper.h"

//-----------------------------------------------------------------------------
// returns: COLL_* numbers (none,top,right,bottom,left)
//-----------------------------------------------------------------------------
int SJ_CollideRects( float x0, float y0, float x1, float y1, const sSprite* spr )
{
	const float x08 = x0+g_plDim[0]/2.0f;
	const float x18 = x1+g_plDim[0]/2.0f;
	const float x40 = spr->x+g_blDim[0]+g_plDim[0]/2.0f;
	const float y40 = spr->y+g_blDim[1]+g_plDim[1]/2.0f;
	const bool lrbounds = ( (x08 >= spr->x-g_plDim[0]/2.0f && x08 <= x40) || (x18 >= spr->x-g_plDim[0]/2.0f && x18 <= x40) );
	if ( (y0+g_plDim[1] <= spr->y && y1+g_plDim[1] >= spr->y) && lrbounds ) return COLL_TOP;	// top
	const float x32 = spr->x+g_blDim[0], y32 = spr->y+g_blDim[1];
	const float y08 = y0+g_plDim[1]/2.0f;
	const float y18 = y1+g_plDim[1]/2.0f;
	if ( (y0 >= y32 && y1 <= y32) && lrbounds ) return COLL_BOTTOM; // bottom
	const bool tbbounds = ( (y08 <= y40 && y08 >= spr->y-g_plDim[1]/2.0f) || (y18 <= y40 && y18 >= spr->y-g_plDim[1]/2.0f) );
	if ( (x0 >= x32 && x1 <= x32) && tbbounds ) return COLL_RIGHT; // right
	if ( (x0+g_plDim[0] <= spr->x && x1+g_plDim[0] >= spr->x) && tbbounds ) return COLL_LEFT; // left
	return 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline void SJ_CollideResponse( sPlayerInfo& pl, int collSide, int sprId )
{
	sSprite* spr = g_sprites+sprId;
	switch ( collSide )
	{
		case COLL_TOP: 
		{
			pl.curForce[1] = -g_topBounce;
			int soundId = IDS_JUMP;
			if ( g_sprites[pl.sprId].texId == IDT_FALLING )
			{
				pl.curForce[1]*=1.5f;
				soundId = IDS_ASS;
			}
			pl.gravTime = 0.0f; 
			pl.canBoost = true;
			// return to jumping texture
			g_sprites[pl.sprId].texId = ( g_sprites[pl.sprId].x >= pl.lastPos[0] )
											? IDT_JUMPING : IDT_JUMPINGR;
			// collision with other player? if not, deactive the other
			if ( sprId >= g_nPlayers )
			{
				--spr->active;
				if ( spr->active == 0 )
				{
					--g_remainBlocks;
					++pl.countBlocks;
					++pl.countBlocksByLevel;
				}
			}
			SJ_PlaySound( soundId );
		}break;
		case COLL_RIGHT: 
		{
			pl.curForce[0] = g_lateralBounce;
			SJ_PlaySound( IDS_COLLIDE );
		}break;
		case COLL_BOTTOM:
		{
			pl.curForce[1] = g_bottomBounce; 
			if ( sprId < g_nPlayers ) 
				pl.curForce[1]*=1.5f;
			pl.gravTime = 0.0f; 
			SJ_PlaySound( IDS_COLLIDE );
		}break;
		case COLL_LEFT:
		{
			pl.curForce[0] = -g_lateralBounce;
			SJ_PlaySound( IDS_COLLIDE );
		}break;
	}			
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool SJ_ComputeCollisions( sPlayerInfo& pl, float newX, float newY )
{
	const sSprite& player = g_sprites[ pl.sprId];
	sSprite* spr=NULL;
	for ( int i = 0; i < MAX_SPRITES; ++i ) // collision with all blocks
	{
		spr = &g_sprites[i];
		if ( spr->active == 0 || i == pl.sprId ) 
			continue; // only for non-players and actives		
		// which side of block?
		const int collSide = SJ_CollideRects( player.x, player.y, newX, newY, spr );
		if ( collSide != COLL_NONE ) 
		{
			SJ_CollideResponse( pl, collSide, i );
			return true;
		}
	}
	// collision with sides of screen
	if ( newX < 0 || (newX+g_plDim[0]) > g_clRect.right-g_plDim[0] )
	{
		pl.curForce[0] = - g_lateralBounce*(newX/fabs(newX));
		SJ_PlaySound( IDS_COLLIDE );
	}
	if ( newY < 0 )
	{
		pl.curForce[1] = g_bottomBounce*0.5f;
		SJ_PlaySound( IDS_COLLIDE );
	}
	return false;
}