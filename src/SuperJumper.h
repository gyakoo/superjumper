#ifndef _SUPERJUMPER_H_
#define _SUPERJUMPER_H_
#include <d3dx9.h>
#include <xact3.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
// Useful macros
//-----------------------------------------------------------------------------
#define SAFE_RELEASE(p)		if (p) { (p)->Release(); (p)=NULL; }
#define SAFE_FREE(p)		if (p) { free(p); (p)=NULL; }
#define SKIP_BLANKS(ptr)	while ( *(ptr) == L' ' && *(ptr) ) ++(ptr); \
							if ( *(ptr)==0 ) continue
#define TRIM(ptr)			{ wchar_t* t = ptr; while ( *(t)!=L'\n' && *(t)!=COMMENT_TOKEN && *(t) ) ++(t);\
							  if ( *(t) ) *(t)=0;\
							  --(t); while ( (t)!=(ptr) && iswspace(*(t)) ){ *(t)=0;--(t);} }
#define SETPLAYER_RED(plc)		{(plc)[0]=255; (plc)[1]=100; (plc)[2]=100;}
#define SETPLAYER_GREEN(plc)	{(plc)[0]=100; (plc)[1]=255; (plc)[2]=100;}
#define SETPLAYER_BLUE(plc)		{(plc)[0]=100; (plc)[1]=100; (plc)[2]=255;}
#define SETPLAYER_MAGENTA(plc)	{(plc)[0]=255; (plc)[1]=100; (plc)[2]=255;}
#define SETPLAYER_WHITE(plc)	{(plc)[0]=255; (plc)[1]=255; (plc)[2]=255;}
//-----------------------------------------------------------------------------
// Values
//-----------------------------------------------------------------------------
#define SJ_VERSION			L"0.9"
#define SJ_AUTHOR			L"gyakoo/juampalf"
#define SJ_GAME_NAME		L"Super Jumper"
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		480
#define TILES_WIDTH			19
#define TILES_HEIGHT		14
#define MAX_SPRITES			96	// max. of simultaneous sprites in screen
#define MAX_PLAYERS			4	// max. of simultaneous players
#define MAX_AVAILSKINS		16

#define IDS_JUMP			0	// sound: when jump over a block
#define IDS_BOOST			1	// sound: when apply a boost
#define IDS_DIE				2	// sound: when player dies
#define IDS_COLLIDE			3	// sound: when player collides with something
#define IDS_ASS				4	// sound: when player performs a stomp
#define IDS_MENUMOVE		5	// sound: menu up/down
#define IDS_MENUCHANGE		6	// sound: menu left/right
#define IDS_NEWLEVEL		7	// sound: a level starts
#define IDS_ENDLEVEL		8	// sound: a level is completed
#define IDS_MAX				9	

#define IDT_BLOCK			0	// block texture
#define IDT_JUMPING			1	// jumping texture
#define IDT_JUMPINGR		2	// jumping reverse texture
#define IDT_BOOST			3	// boost texture
#define IDT_HEART			4	// heart for life
#define IDT_FALLING			5	// falling
#define IDT_ARROW			6	// boost icon
#define IDT_MAX				7	// maximum no. of textures

#define COLL_NONE			0	// no collision
#define COLL_TOP			1	// collision with top side of block
#define COLL_RIGHT			2	// collision with right side of block
#define COLL_BOTTOM			3	// collision with bottom side of block
#define COLL_LEFT			4	// collision with left side of block

#define COMMENT_TOKEN		L'#'	// comment token for files
#define LEVEL_NEXT			L"#nxt" // pick next level
#define TILE_BLOCK			L'x'	// block tile
#define TILE_SPAWN0			L'0'	// spawn point 0
#define TILE_SPAWN1			L'1'	// spawn point 1
#define TILE_SPAWN2			L'2'	// spawn point 2
#define TILE_SPAWN3			L'3'	// spawn point 3
#define DEFAULT_SKIN		L"default"		// default skin folder name
#define DEFAULT_DIFFICULTY  L"easy"			// default difficulty level
#define CONFIG_FILE			L"config.txt"	// configuration file name
#define DEFAULT_FONTNAME	L"Verdana"
#define DEFAULT_FONTSIZE	14

#define ST_SPLASH			0		// state: when splash screen
#define ST_MENU				1		// state: when menu screen
#define ST_CREDITS			2		// state: when credits screen
#define ST_GAME_INTRO		3		// state: when playing. intro
#define ST_GAME_LEVELS		4		// state: when playing levels
#define ST_GAME_END			5		// state: end of game
#define ST_GAME_OVER		6		// state: game over
#define ST_MAX				7		// maximum number of states

#define PAUSE_NONE		   -1		// no pause
#define PAUSE_USER			0		// paused by user
#define PAUSE_LEVELFINISHED 1		// level has finished
#define PAUSE_LEVELSTART	2		// before to start level
#define PAUSE_DEAD			3		// when player is dead
#define PAUSE_GAMEEND		4		// when no more levels
#define PAUSE_FULLSCREEN	5		// fullscreen
#define PAUSE_DIE			6
#define PAUSE_WMINACTIVE	7		// window without focus
#define PAUSE_RETURNTOMENU  8

#define GAMEMODE_NORMAL		0		// normal (SP + MP)
#define GAMEMODE_TIMEATTACK	1		// time attack (SP + MP)
#define GAMEMODE_MAX		2

//-----------------------------------------------------------------------------
// Sprite data
//-----------------------------------------------------------------------------
struct sSprite
{
	float x,y;				// x,y 2d position on screen
	float alpha;			// alpha value [0,1]
	char  active;			// 0 is disabled, >0 active
	char  texId;			// texture id for pool
};
//-----------------------------------------------------------------------------
// Player information
//-----------------------------------------------------------------------------
struct sPlayerInfo
{
	float gravTime;			// acumulated time for gravity
	float curForce[2];		// force to apply
	float lastPos[2];		// last known position
	bool  canBoost;			// true when player can impulse up
	char  levelBoosts;		// number of remaining impulses 
	char  lives;			// number of remaining lives
	float timeToNextKey;	// time to wait until next key is available
	char  sprId;			// sprite identificator
	int	  keyUp, keyDown;	// key for up/down
	int	  keyLeft, keyRight;// key for left/right
	int	  countBlocks;		// number of pointed blocks
	int	  countBlocksByLevel;// number of pointed blocks by level
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef void (*FUNC_UPDATEGAME)( float elspasedTime );
typedef void (*FUNC_RENDER)( );
typedef void (*FUNC_BEGINSTATE)( char lastState );
typedef void (*FUNC_ENDSTATE)( );
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct sStateInfo
{
	float				acumTime;
	FUNC_UPDATEGAME		Update;
	FUNC_RENDER			Render;
	FUNC_BEGINSTATE		Begin;
	FUNC_ENDSTATE		End;
};
//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
// -- Graphics related --
extern HWND						g_hWnd;					// window handle
extern IDirect3D9*				g_pD3D;					// d3d object
extern IDirect3DDevice9*		g_pd3dDevice;			// d3d device
extern IDirect3DTexture9*		g_pTextures[IDT_MAX];	// textures pool
extern const wchar_t*			g_texNames[IDT_MAX];	// texture names pool
extern ID3DXSprite*				g_pSprite;				// sprite interface
extern ID3DXFont*				g_pFont;				// font interface
extern D3DPRESENT_PARAMETERS	g_d3dpp;				// present parameters
extern bool						g_fullScreen;			// true when FS
extern float					g_plDim[2];				// player dimensions w/h
extern float					g_blDim[2];				// block dimensions w/h
extern RECT						g_clRect;				// client rect (important when windowed)
extern wchar_t					g_fontName[32];			// current font name
extern char						g_fontSize;				// current font size
// -- Input related --
extern bool						g_keysDown[256];		// true when a key is down
extern float					g_timeToNextKey;		// time until next key can be pressed
// -- Game states and level related --
extern short					g_remainBlocks;			// level remaining blocks
extern wchar_t					g_levelName[64];		// level name
extern int						g_nextLevel;			// level index
extern wchar_t					g_skin[32];				// current skin folder name
extern sStateInfo				g_states[ST_MAX];		// states pool
extern char						g_curState;				// current state
extern char						g_gameMode;				// current game mode
extern float					g_levelTime;			// time available for level (time attack only)
// -- Configuration related --
extern char						g_livesMax;				// config. number of lives
extern float					g_topBounce;			// config. top bounce force
extern float					g_lateralBounce;		// config. lateral bounce force
extern float					g_bottomBounce;			// config. bottom bounce force
extern float					g_boostForce;			// config. impulse force
extern float					g_lateralForce;			// config. moving left/right force
extern float					g_lateralFriction;		// config. lateral friction
extern float					g_gravity;				// config. gravity force
extern float					g_gravTimeFactor;		// config. gravity time multiplier
// -- Players related --
extern sSprite					g_sprites[MAX_SPRITES];	// all sprites
extern sPlayerInfo				g_player[MAX_PLAYERS];	// all supported players
extern char						g_nPlayers;				// number of simultaneous players
extern unsigned char			g_plColors[MAX_PLAYERS][3];	// colors table for players
// -- Misc --
extern wchar_t					g_txt[256];				// shared buffer
extern bool						g_paused;				// true when paused
extern char						g_pausedState;			// state when paused
extern float					g_etFactor;				// elapsed time factor (i.e.bullet time)

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
// -- Graphics and OS related --
HRESULT		SJ_InitD3D				( );				// Creating rendering device
void		SJ_CleanupD3D			( );				// Releases graphics resources
void		SJ_ToggleFullScreen		( );				// Change to fullscreen/windowed
void		SJ_Render				( );				// Performs game rendering
HRESULT		SJ_ReloadDynResources	( );				// Reload all dynamic resources
void		SJ_DestroyDynResources	( );				// Releases all dynamic resources

// -- Misc --
void		SJ_ChangeSkin			( const wchar_t* newSkin );					// Changes current skin
void		SJ_ChangeDifficulty		( const wchar_t* newDiff );					// Changes difficulty level
void		SJ_ReloadConfiguration	( );										// Read configuration values from file
bool		SJ_NextLevel			( const wchar_t* filename = LEVEL_NEXT );	// Loads next level
void		SJ_Pause				( char type );
void		SJ_Unpause				( );

// -- Sound --
HRESULT		SJ_InitSound			( );										// Init sound system
void		SJ_CleanupSound			( );										// Destroy sound system
void		SJ_SoundUpdate			( );
void		SJ_PlaySound			( int soundId );							// Play a sound
void		SJ_DestroySounds		( );										// Destroy all sound buffers
void		SJ_ReloadSounds			( );										// Reload all sounds buffers

// -- Players related --
void		SJ_ResetPlayer			( sPlayerInfo& pl );						// Reset player values
void		SJ_UpdatePlayer			( float elapsedTime, sPlayerInfo& pl );		// Updates a player

// -- Collision related --
bool		SJ_ComputeCollisions	( sPlayerInfo& pl, float newX, float newY );// Players' collisions

// -- Different gameplay states --
void		SJ_ChangeState			( char newState );

void		SJ_Splash_Update		( float elapsedTime );	// Splash: update
void		SJ_Splash_Render		( );					// Splash: render

void		SJ_Menu_Update			( float elapsedTime );	// Menu: update
void		SJ_Menu_Render			( );					// Menu: render
void		SJ_Menu_Begin			( char lastState );		// Menu: begin
void		SJ_Menu_End				( );					// Menu: end

void		SJ_Credits_Update		( float elapsedTime );	// Credits: update
void		SJ_Credits_Render		( );					// Credits: render
void		SJ_Credits_Begin		( char lastState );		// Credits: begin
void		SJ_Credits_End			( );					// Credits: end

void		SJ_GameIntro_Update		( float elapsedTime );	// Game Intro: update
void		SJ_GameIntro_Render		( );					// Game Intro: render
void		SJ_GameIntro_Begin		( char lastState );		// Game Intro: begin
void		SJ_GameIntro_End		( );					// Game Intro: end

void		SJ_GameEnd_Update		( float elapsedTime );	// Game End: update
void		SJ_GameEnd_Render		( );					// Game End: render
void		SJ_GameEnd_Begin		( char lastState );		// Game End: begin
void		SJ_GameEnd_End			( );					// Game End: end

// specific gameplays
void		SJ_GameNormal_Update	( float elapsedTime );
void		SJ_GameNormal_Render	( );
void		SJ_GameNormal_Begin		( char lastState );
#endif