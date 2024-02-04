/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

Renderer base definitions and functions
Written by Andrew Lucas, Richard Rohac, BUzer, Laurie, Botman and Id Software
*/

#ifndef RENDERERDEFS_H
#define RENDERERDEFS_H

#include "windows.h"
#include "gl/gl.h"
#include "gl/glext.h"
#include "dlight.h"
#include "com_model.h"
#include "cl_entity.h"
#include <assert.h>
#include "r_studioint.h"
#include "frustum.h"
#include "studio.h"
#include "pm_defs.h"

#include <vector>
#include <map>
#include <string>

//==============================
//		SHARED DEFS
//
//==============================
#define	MAXRENDERENTS		4096

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

//==============================
//		TEXTURE LOADER DEFS
//
//==============================
#define MAX_TGA_LOADER_TEXTURES 8192

//==============================
//		TEXTURE LOADER STRUCTS
//
//==============================
struct cl_texture_t
{
	char szName[64];

	GLuint iIndex;

	int iBpp;
	unsigned int iWidth;
	unsigned int iHeight;
};


//==============================
//		PARTICLE ENGINE DEFS
//
//==============================
#define SYSTEM_SHAPE_POINT				0
#define SYSTEM_SHAPE_BOX				1
#define SYSTEM_SHAPE_PLANE_ABOVE_PLAYER 2

#define SYSTEM_DISPLAY_NORMAL			0
#define SYSTEM_DISPLAY_PARALELL			1
#define SYSTEM_DISPLAY_PLANAR			2
#define SYSTEM_DISPLAY_TRACER			3

#define SYSTEM_RENDERMODE_ADDITIVE		0
#define SYSTEM_RENDERMODE_ALPHABLEND	1
#define SYSTEM_RENDERMODE_INTENSITY		2

#define PARTICLE_COLLISION_NONE			0
#define PARTICLE_COLLISION_DIE			1
#define PARTICLE_COLLISION_BOUNCE		2
#define PARTICLE_COLLISION_DECAL		3
#define PARTICLE_COLLISION_STUCK		4
#define PARTICLE_COLLISION_NEW_SYSTEM	5

#define PARTICLE_WIND_NONE				0
#define PARTICLE_WIND_LINEAR			1
#define PARTICLE_WIND_SINE				2

#define PARTICLE_LIGHTCHECK_NONE		0
#define PARTICLE_LIGHTCHECK_NORMAL		1
#define PARTICLE_LIGHTCHECK_SCOLOR		2
#define PARTICLE_LIGHTCHECK_MIXP		3

//========================================
//			PARTICLE ENGINE STRUCTS
//
//========================================
struct particle_system_t
{
	int id;
	int shapetype;
	int randomdir;

	vec3_t origin;
	vec3_t dir;

	float minvel;
	float maxvel;
	float maxofs;

	float skyheight;

	float spawntime;
	float fadeintime;
	float fadeoutdelay;
	float velocitydamp;
	float stuckdie;
	float tracerdist;

	float maxheight;

	float windx;
	float windy;
	float windvar;
	float windmult;
	float windmultvar;
	int windtype;

	float maxlife;
	float maxlifevar;
	float systemsize;

	vec3_t primarycolor;
	vec3_t secondarycolor;
	float transitiondelay;
	float transitiontime;
	float transitionvar;
	
	float rotationvar;
	float rotationvel;
	float rotationdamp;
	float rotationdampdelay;

	float rotxvar;
	float rotxvel;
	float rotxdamp;
	float rotxdampdelay;

	float rotyvar;
	float rotyvel;
	float rotydamp;
	float rotydampdelay;

	float scale;
	float scalevar;
	float scaledampdelay;
	float scaledampfactor;
	float veldampdelay;
	float gravity;
	float particlefreq;
	float impactdamp;
	float mainalpha;

	int startparticles;
	int maxparticles;
	int	maxparticlevar;

	int overbright;
	int lightcheck;
	int collision;
	int colwater;
	int displaytype;
	int rendermode;
	int numspawns;

	int fadedistfar;
	int fadedistnear;

	int numframes;
	int framesizex;
	int framesizey;
	int framerate;

	char create[64];
	char deathcreate[64];
	char watercreate[64];

	particle_system_t *createsystem;
	particle_system_t *watersystem;
	particle_system_t *parentsystem;

	cl_texture_t *texture;
	mleaf_t *leaf;

	particle_system_t	*next;
	particle_system_t	*prev;

	struct cl_particle_t *particleheader;

	byte pad[14];
};

struct cl_particle_t
{
	vec3_t velocity;
	vec3_t origin;
	vec3_t color;
	vec3_t scolor;
	vec3_t lastspawn;
	vec3_t normal;

	float spawntime;
	float life;
	float scale;
	float alpha;

	float fadeoutdelay;

	float scaledampdelay;
	float secondarydelay;
	float secondarytime;

	float rotationvel;
	float rotation;

	float rotx;
	float rotxvel;

	float roty;
	float rotyvel;

	float windxvel;
	float windyvel;
	float windmult;

	float texcoords[4][2];

	int frame;

	particle_system_t *pSystem;

	cl_particle_t	*next;
	cl_particle_t	*prev;

	byte pad[4];
};

//==============================
//		BSP RENDERER DEFS
//
//==============================
#define MAX_DECALTEXTURES		128
#define MAX_CUSTOMDECALS		4096
#define MAX_STATICDECALS		1024
#define MAX_GROUPENTRIES		64
#define MAX_DECAL_MSG_CACHE		256
#define MAX_DECAL_GROUPS		256
#define	MAX_LIGHTMAPS			64
#define	MAX_LIGHTSTYLES			64
#define	MAX_STYLESTRING			64
#define MAX_DYNLIGHTS			64
#define MAX_MAP_DETAILOBJECTS	512
#define MAX_DETAIL_TEXTURES		1024
#define MAX_MAP_LEAFS			65534
#define DEPTHMAP_RESOLUTION		256
#define MAX_MAP_TEXTURES		512
#define LIGHTMAP_RESOLUTION		1024
#define LIGHTMAP_NUMCOLUMNS		8
#define LIGHTMAP_NUMROWS		8
#define MAX_SPOTLIGHT_TEXTURES	16

#define MAX_GOLDSRC_DLIGHTS	32
#define MAX_GOLDSRC_ELIGHTS	64

#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWSPRITE		8
#define SURF_DRAWTURB		0x10
#define SURF_DRAWTILED		0x20
#define SURF_DRAWBACKGROUND	0x40
#define SURF_UNDERWATER		0x80
#define SURF_DONTWARP		0x100

#define	BLOCK_WIDTH			128
#define	BLOCK_HEIGHT		128
#define BLOCKLIGHTS_SIZE	(18*18)
#define BACKFACE_EPSILON	0.01

#define	PLANE_X				0
#define	PLANE_Y				1
#define	PLANE_Z				2

#define OFFSET(type, variable) ((const void*)&(((type*)NULL)->variable))

// Texture pointer settings
enum {
	TC_OFF,
	TC_TEXTURE,
	TC_LIGHTMAP,
	TC_VERTEX_POSITION, // for specular and dynamic lighting
	TC_DETAIL_TEXTURE, // for detail texturing
	TC_NOSTATE // uninitialized
};

// Envstate settings
enum {
	ENVSTATE_OFF,
	ENVSTATE_REPLACE,
	ENVSTATE_MUL_CONST,
	ENVSTATE_MUL_PREV_CONST, // ignores texture
	ENVSTATE_MUL,
	ENVSTATE_MUL_X2,
	ENVSTATE_ADD,
	ENVSTATE_DOT,
	ENVSTATE_DOT_CONST,
	ENVSTATE_PREVCOLOR_CURALPHA,
	ENVSTATE_NOSTATE // uninitialized
};

//========================================
//			BSP RENDERER STRUCTS
//
//========================================
struct brushvertex_t
{
	vec3_t	pos;
	vec3_t	normal;

	float	fogcoord;
	float	texcoord[2];
	float	detailtexcoord[2];
	float	lightmaptexcoord[2];

	byte	pad[12];
};

struct brushface_t
{
	int index;
	int start_vertex;
	int num_vertexes;

	vec3_t	normal;
	vec3_t	s_tangent;
	vec3_t	t_tangent;
};

typedef struct detailtexentry_s
{
	char	texname[32];
	char	detailtexname[32];
	int		texindex;
	float	xscale;
	float	yscale;
} detailtexentry_t;

struct decalgroupentry_t
{
	char szName[64];
	int gl_texid;
	int xsize, ysize;
	struct decalgroup_t *group;
};
struct decalgroup_t
{
	char szName[64];
	int	iSize;
	decalgroupentry_t entries[MAX_GROUPENTRIES];
};

typedef struct customdecalvert_s {
	vec3_t position;
	float texcoord[2];
} customdecalvert_t;

typedef struct customdecalpoly_s {
	customdecalvert_t *pverts;
	int numverts;

	msurface_t *surface;
	cl_entity_t *entity;
} customdecalpoly_t;

typedef struct customdecal_s {
	customdecalpoly_t *polys;
	int inumpolys;
	
	const decalgroupentry_t *texinfo;

	vec3_t normal;
	vec3_t position;
	float life;
} customdecal_t;

struct decal_msg_cache
{
	vec3_t	pos;
	vec3_t	normal;
	char	name[16];
	int		persistent;
};

struct clientsurfdata_t
{
	float cached_light[MAXLIGHTMAPS];

	texture_t	*regtexture;
	texture_t	*mptexture;

	int	light_s;
	int light_t;
};

typedef struct
{
	int		length;
	char	map[MAX_STYLESTRING];
} lightstyle_t;

struct detailobject_t
{
	vec3_t mins;
	vec3_t maxs;

	int firstsurface;
	int numsurfaces;

	short leafnums[MAX_ENT_LEAFS*2];
	int numleafs;

	int visframe;
	int rendermode;
};

struct cl_dlight_t
{
	vec3_t	origin;
	vec3_t	color;
	vec3_t	angles;

	float	radius;
	float	die;
	float	decay;
	int		key;
	int		noshadow;

	GLuint	depth;

	// spotlight specific:
	float	cone_size;
	FrustumCheck frustum;	
	int textureindex;
};

//==================================================
//				WATER SHADER DEFS
//
//==================================================
#define MAX_WATER_ENTITIES			64
#define	MAX_WATER_VERTEX_SHADERS	2
#define MAX_WATER_FRAGMENT_SHADERS	4
#define	WATER_RESOLUTION			512

//==================================================
//				WATER SHADER STRUCTS
//
//==================================================
struct cl_water_t
{
	int index;
	cl_entity_t *entity;

	mplane_t wplane;

	vec3_t mins;
	vec3_t maxs;
	vec3_t origin;
	bool draw;

	GLuint refract;
	GLuint reflect;
	GLuint dbuffer;

	msurface_t **surfaces;
	int numsurfaces;
};

//==================================================
//				MIRROR MANAGER DEFS
//
//==================================================
#define MAX_MIRRORS			32
#define	MIRROR_RESOLUTION	512

//==================================================
//				MIRROR MANAGER STRUCTS
//
//==================================================
struct cl_mirror_t
{
	cl_entity_t *entity;

	vec3_t mins;
	vec3_t maxs;

	vec3_t origin;
	msurface_t *surface;

	bool draw;

	GLuint texture;
};
//==============================
//		STUDIO RENDERER DEFS
//
//==============================
#define MAX_MODEL_LIGHTS	6
#define	MAX_MODEL_DECALS	16
#define MAX_CACHE_MODELS	2048
#define MAX_MODEL_SHADERS	14

#define	TEXFLAG_NONE		1 
#define	TEXFLAG_FULLBRIGHT	1 
#define	TEXFLAG_ALTERNATE	2 
#define	TEXFLAG_NOMIPMAP	4
#define	TEXFLAG_ERASE		8

//========================================
//				STUDIO RENDERER STRUCTS
//
//========================================
struct decalvert_t
{
	int vertindex;
	float texcoord[2];
};

struct decalvertinfo_t
{
	vec3_t position;
	byte boneindex;
};

struct decalpoly_t
{
	decalvert_t *verts;
	int numverts;
};

struct studiodecal_t
{
	int entindex;

	decalpoly_t *polys;
	int numpolys;

	decalvertinfo_t *verts;
	int numverts;

	const decalgroupentry_t *texture;

	int totaldecals;
	studiodecal_t *next; // linked list on this entity
};

struct studiovert_t
{
	int vertindex;
	int normindex;
	int texcoord[2];
	byte boneindex;
};

struct studiotri_t
{
	studiovert_t verts[3];
};

struct mlight_t
{
	vec3_t	origin;
	float	radius;
	vec3_t	color;

	bool flashlight;
	vec3_t	forward;	
	float	spotcos;

	FrustumCheck *frustum;

	vec3_t mins;
	vec3_t maxs;
};

struct texentry_t
{
	char szModel[64];
	char szTexture[32];

	int iFlags;
};

struct lighting_ext
{
	vec3_t	ambientlight;
	vec3_t	diffuselight;
	vec3_t	lightdir;
};

//========================================
//			PROP MANAGER DEFINITIONS
//
//========================================
#define MAX_POINTS 64

//========================================
//			PROP MANAGER STRUCTS
//
//========================================
typedef struct epair_s
{
   struct epair_s *next;
   char  *key;
   char  *value;
} epair_t;

typedef struct
{
   vec3_t      origin;
   int         firstbrush;
   int         numbrushes;
   epair_t     *epairs;
} entity_t;

struct vbomesh_t
{
	int start_vertex;
	int num_vertexes;
};

struct vbosubmodel_t
{
	vbomesh_t *meshes;
	int nummeshes;
};

struct vboheader_t
{
	brushvertex_t *pBufferData;
	int numverts;

	unsigned int *indexes;
	int numindexes;

	vbosubmodel_t *submodels;
	int numsubmodels;
};

struct modeldata_t
{
	char name[256];

	studiohdr_t	*pHdr;
	studiohdr_t	*pTexHdr;
	vboheader_t pVBOHeader;	
};

struct entextradata_t
{
	vec3_t absmax;
	vec3_t absmin;
	vec3_t lightorigin;

	int num_leafs;
	short leafnums[MAX_ENT_LEAFS];
	float pbones[MAXSTUDIOBONES][3][4];

	modeldata_t *pModelData;
};

struct entextrainfo_t
{
	int surfindex;
	int lightstyles[4];
	vec3_t prevpos;

	lighting_ext pLighting;
	cl_entity_t *pEntity;
	entextradata_t *pExtraData; // only used by CL ents
};

struct cabledata_t
{
	int iwidth;
	int isegments;

	vec3_t vmins;
	vec3_t vmaxs;

	vec3_t vpoints[MAX_POINTS];
	int inumpoints;

	int num_leafs;
	short leafnums[MAX_ENT_LEAFS];
};

struct glstate_t
{
	glstate_t():
		blending_enabled(false),
		alphatest_enabled(false),
		alphatest_func(0),
		alphatest_value(0),
		active_texunit(0),
		active_clienttexunit(0)
		{}

	bool blending_enabled;

	bool alphatest_enabled;
	GLint alphatest_func;
	GLfloat alphatest_value;

	GLint active_texunit;
	GLint active_clienttexunit;
};

//========================================
//				GLOBAL FUNCTION CALLS
//
//========================================
extern engine_studio_api_t IEngineStudio;

extern void		ClampColor( int r, int g, int b, color24 *out );
extern void		FilenameFromPath( char *szin, char *szout );

extern void		MyLookAt( GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx, GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy, GLdouble upz );
extern mleaf_t	*Mod_PointInLeaf (vec3_t p, model_t *model);
extern byte		*Mod_LeafPVS(mleaf_t *leaf, model_t *model);
extern void		R_MarkLeaves ( mleaf_t *pLeaf );

extern void		HUD_PrintSpeeds( void );
extern void		RenderersDumpInfo( void );
extern void		GenDetail( void );
extern void		SetupFlashlight( vec3_t origin, vec3_t angles, float time, float frametime );
extern void		ExportWorld( void );

extern unsigned short	ByteToUShort( byte *byte );
extern unsigned int		ByteToUInt( byte *byte );
extern int		ByteToInt( byte *byte );

extern void		R_CalcRefDef( ref_params_t *pparams );
extern void		R_DrawNormalTriangles( void );
extern void		R_DrawTransparentTriangles( void );

extern void		RenderFog( void );
extern void		BlackFog( void );
extern void		DisableFog( void );
extern void		ClearToFogColor( void );

extern void		R_RotateForEntity (cl_entity_t *e);
extern int		IsEntityMoved(cl_entity_t *e);
extern int		IsEntityTransparent(cl_entity_t *e);
extern int		IsPitchReversed(float pitch);
extern int		BoxOnPlaneSide ( vec3_t emins, vec3_t emaxs, mplane_t *p );

extern char		*strLower( char *str );
extern char		*stristr( const char *string, const char *string2 );

extern inline void		DotProductSSE( float* result, const float* v0, const float* v1 );
extern inline void		SSEDotProductWorld( float* result, const float* v0, const float* v1 );
extern inline void		SSEDotProductWorldInt( int* result, const float* v0, const float* v1 );
extern inline void		SSEDotProductSub( float* result, vec3_t *v0, vec3_t *v1, float *subval );

extern inline void		VectorAddSSE( const float* v0, const float* v1, const float* result );
extern inline void		VectorMASSE (const float *veca, float scale, const float *vecb, float *vecc);
extern inline void		VectorTransformSSE(const float *in1, float in2[3][4], float *out);
extern inline void		VectorRotateSSE(const float *in1, float in2[3][4], float *out);
extern inline float		VectorNormalizeFast (float *v);

extern void		VectorRotate (const float *in1, const float in2[3][4], float *out);
extern void		VectorIRotate(const vec3_t &in1, const float in2[3][4], vec3_t &out);
extern void		FixVectorForSpotlight( vec3_t &vec );
extern void		SV_FindTouchedLeafs( entextradata_t *ent, mnode_t *node );

extern byte		*ResizeArray( byte *pOriginal, int iSize, int iCount );

extern void		R_DisableSteamMSAA( void );
extern void		R_SaveGLStates( void );
extern void		R_RestoreGLStates( void );

extern void		R_Init( void );
extern void		R_VidInit( void );
extern void		R_Shutdown( void );

extern vec3_t	g_vecFull;
extern vec3_t	g_vecZero;
extern int		current_ext_texture_id;
#endif