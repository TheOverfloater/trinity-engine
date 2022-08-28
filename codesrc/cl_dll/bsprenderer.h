/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

BSP Renderer
Original code by Buzer and Id Software
Extended and/or recoded by Andrew Lucas
*/

#if !defined ( BSPRENDERER_H )
#define BSPRENDERER_H
#if defined( _WIN32 )
#pragma once
#endif

#include "windows.h"
#include "gl/gl.h"
#include "pm_defs.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "dlight.h"
#include "parsemsg.h"
#include "cvardef.h"
#include "textureloader.h"
#include "rendererdefs.h"

/*
====================
CBSPRenderer

====================
*/
class CBSPRenderer
{
public:
	void Init( void );
	void VidInit( void );
	void Shutdown( void );
	void SetupRenderer( void );
	void SetupPreFrame( ref_params_t *vieworg );
	void CheckTextures( void );

	void DrawShadowPasses( void );
	void CreateShadowMap( void );
	void DrawWorldSolid( void );
	void SetupSpotlightVis( void );

	void DrawDetailsSolid ( void );
	void RecursiveWorldNodeSolid( mnode_t *node );
	void DrawBrushModelSolid( cl_entity_t *pEntity );

	void ClearDetailObjects( void );
	void LoadDetailFile( void );
	void DrawDetails( void );

	void RendererRefDef ( ref_params_t *pparams );
	void DrawNormalTriangles( void );
	void DrawTransparentTriangles( void );
	void RenderFirstPass( bool bSecond = false );
	void RenderFinalPasses( void );
	void DrawWorld( void );

	void GetRenderEnts ( void );
	void AddEntity( cl_entity_t *pEntity );
	int FilterEntities ( int type, struct cl_entity_s *pEntity,const char *modelname );//

	void DecayLights( void );
	bool HasDynLights( void );
	void GetAdditionalLights( void );
	cl_dlight_t *CL_AllocDLight( int key );
	int MsgDynLight( const char *pszName, int iSize, void *pbuf );

	void SetupDynLight( void );
	void FinishDynLight( void );
	void SetDynLightBBox( void );
	void SetupSpotLight( void );
	void FinishSpotLight( void );
	bool LightCanShadow( void );
	int	CullDynLightBBox (vec3_t mins, vec3_t maxs);

	void PushDynLights ( void );
	void MarkLights( cl_dlight_t *pLight, int iBit, mnode_t *node);
	void MarkBrushFaces( vec3_t mins, vec3_t maxs );
	void AddDynamicLights( msurface_t *surf );

	void DisableWorldDrawing( ref_params_t *pparam );
	void RestoreWorldDrawing( void );

	void CreateTextures( void );
	void SaveMultiTexture( void );
	void RestoreMultiTexture( void );

	void FreeBuffer( void );
	void GenerateVertexArray( void );
	void EnableVertexArray( void );
	void DisableVertexArray( void );

	void DrawBrushModel ( cl_entity_t *pEntity, bool bStatic = false );
	void RecursiveWorldNode ( mnode_t *node );

	void SurfaceToChain( msurface_t *s, bool dynlit );
	void DrawScrollingPoly( msurface_t *s );
	void EmitWaterPolys( msurface_t *fa );
	void DrawPolyFromArray( glpoly_t *p );

	bool DynamicLighted( const vec3_t &vmins, const vec3_t &vmaxs );
	void DrawDynamicLightsForWorld( void );
	void DrawDynamicLightsForDetails( void );
	void RecursiveWorldNodeLight (mnode_t *node);
	void DrawDynamicLightsForEntity( cl_entity_t *pEntity );
	void DrawEntityFacesForLight( cl_entity_t *pEntity );

	void InitSky ( void );
	void DrawSky( void );
	void RemoveSky ( void );
	int MsgSkyMarker_Sky( const char *pszName, int iSize, void *pbuf );
	int MsgSkyMarker_World( const char *pszName, int iSize, void *pbuf );

	void PrepareRenderer ( void );
	void ResetRenderer( void );
	void ResetCache( void );

	bool ExtensionSupported( const char *ext );
	cl_texture_t *LoadDetailTexture( char *texname );
	void ParseDetailTextureFile( void );
	void LoadDetailTextures( void );

	void AnimateLight( void );
	void UploadLightmaps( void );
	void BuildLightmap( msurface_t *surf, int surfindex, color24 *out );
	void AddLightStyle( int iNum, char *szStyle );

	void SetTexEnvs( int env0 = 0, int env1 = 0, int env2 = 0, int env3 = 0 );
	void SetTexEnv_Internal( int env );
	void SetTexPointer( int unitnum, int tc );
	void Bind2DTexture( GLenum texture, GLuint id );

	texture_t *TextureAnimation( texture_t *base, int frame );

public:
	void DrawDecals( void );
	void LoadDecals( void );
	void DeleteDecals( void );

	decalgroup_t *FindGroup(const char *_name);
	cl_texture_t *LoadDecalTexture(const char *texname);
	decalgroupentry_t *GetRandomDecal( decalgroup_t *group );
	decalgroupentry_t *FindDecalByName( const char *szName );

	bool CullDecalBBox( vec3_t mins, vec3_t maxs );
	void CreateDecal(vec3_t endpos, vec3_t pnormal, const char *name, int persistent = 0);
	void RecursiveCreateDecal( mnode_t *node, decalgroupentry_t *texptr, customdecal_t *pDecal, vec3_t endpos, vec3_t pnormal);
	void DecalSurface(msurface_t *surf, decalgroupentry_t *texptr, cl_entity_t *pEntity, customdecal_t *pDecal, vec3_t endpos, vec3_t pnormal);
	
	int MsgCustomDecal(const char *pszName, int iSize, void *pbuf);

	void CreateCachedDecals( void );
	void DrawSingleDecal(customdecal_t *decal);

	customdecal_t *AllocDecal( void );
	customdecal_t *AllocStaticDecal( void );

	void GetUpRight(vec3_t forward, vec3_t &up, vec3_t &right);
	int ClipPolygonByPlane (const vec3_t *arrIn, int numpoints, vec3_t normal, vec3_t planepoint, vec3_t *arrOut);
	void FindIntersectionPoint( const vec3_t &p1, const vec3_t &p2, const vec3_t &normal, const vec3_t &planepoint, vec3_t &newpoint );

public:
	GLuint				m_uiBufferIndex;
	brushvertex_t		*m_pBufferData;
	brushface_t			*m_pFacesExtraData;

	GLuint				m_uiCurrentBinds[16];

	cl_entity_t			*m_pCurrentEntity;
	cl_dlight_t			*m_pCurrentDynLight;
	model_t				*m_pWorld;
	byte				*m_pPVS;
	mleaf_t				*m_pViewLeaf;

	dlight_t			*m_pFirstDLight;
	dlight_t			*m_pFirstELight;

	int					m_iTotalVertCount;
	int					m_iTotalFaceCount;
	int					m_iTotalTriCount;

	int					m_iTexPointer[4];
	int					m_iEnvStates[4];
	int					m_iTUSupport;

	int					m_iVisFrame;
	int					m_iFrameCount;

	cl_texture_t		*m_pFlashlightTextures[MAX_SPOTLIGHT_TEXTURES];
	int					m_iNumFlashlightTextures;

	int					m_iAttenuation1DTexture;

	bool				m_bCanDraw;
	bool				m_bDrawSky;
	bool				m_bSecondPassNeeded;
	bool				m_bMirroring;
	bool				m_bLightShadow;

	bool				m_bReloaded;
	bool				m_bRadialFogSupport;
	bool				m_bShaderSupport;
	bool				m_bShadowSupport;
	bool				m_bShadowPCFSupport;
	bool				m_bSpecialFog;
	bool				m_bGotAdditional;

	bool				m_bDontPromptShaders;
	bool				m_bDontPromptShadersError;
	bool				m_bDontPromptShadow;
	bool				m_bDontPromptShadowPCF;
	bool				m_bDontPromptParanoia;

	vec3_t				m_vRenderOrigin;
	vec3_t				m_vViewAngles;
	vec3_t				m_vVecToEyes;

	vec3_t				m_vSkyOrigin;
	vec3_t				m_vSkyWorldOrigin;

	vec3_t				m_vCurDLightOrigin;
	vec3_t				m_vCurSpotForward;

	cvar_t				*m_pCvarSpeeds;
	cvar_t				*m_pCvarDetailTextures;
	cvar_t				*m_pCvarDynamic;
	cvar_t				*m_pCvarDrawWorld;
	cvar_t				*m_pCvarWireFrame;
	cvar_t				*m_pCvarWorldShaders;
	cvar_t				*m_pCvarRadialFog;
	cvar_t				*m_pCvarPCFShadows;
	cvar_t				*m_pCvarShadows;

	vec3_t				m_vDLightMins;
	vec3_t				m_vDLightMaxs;

	float				m_fSavedMinsMaxs[6];
	float				m_fSkySpeed;

	cl_entity_t			*m_pRenderEntities[MAXRENDERENTS];
	int					m_iNumRenderEntities;

	cl_dlight_t			m_pDynLights[MAX_DYNLIGHTS];

	mlight_t			m_pModelLights[MAXRENDERENTS];
	int					m_iNumModelLights;

	lightstyle_t		m_pLightStyles[MAX_LIGHTSTYLES];
	int					m_iLightStyleValue[MAX_LIGHTSTYLES];

	color24				m_pBlockLights[BLOCKLIGHTS_SIZE];
	int					m_iNumLightmaps;

	color24				m_pEngineLightmaps[MAX_LIGHTMAPS*BLOCK_WIDTH*BLOCK_HEIGHT];
	int					m_iEngineLightmapIndex;

	color24				m_pDetailLightmaps[MAX_LIGHTMAPS*BLOCK_WIDTH*BLOCK_HEIGHT];
	int					m_iDetailLightmapIndex;

	clientsurfdata_t	*m_pSurfaces;
	int					m_iNumSurfaces;

	detailobject_t		m_pDetailObjects[MAX_MAP_DETAILOBJECTS];
	int					m_iNumDetailObjects;
	int					m_iNumDetailSurfaces;

	int					m_iAtten3DPoint;
	int					m_iLightDummy;
	
	int					m_iSkyTextures[6];

	int					m_iWorldPolyCounter; // wpoly counter
	int					m_iBrushPolyCounter; // bmodel poly counter
	int					m_iStudioPolyCounter; // studiomodel poly counter

	char				m_szSkyName[64];
	char				m_szMapName[64];

	detailtexentry_t	m_pDetailTextures[MAX_DETAIL_TEXTURES];
	int					m_iNumDetailTextures;

	texture_t			m_pNormalTextureList[MAX_MAP_TEXTURES];
	texture_t			m_pMultiPassTextureList[MAX_MAP_TEXTURES];
	int					m_iNumTextures;

	GLuint				m_iFogFragmentID;
	GLuint				m_iShadowFragmentID;
	GLuint				m_iDecalFragmentID;

	PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTextureARB;
	PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;
	PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;

	PFNGLBINDBUFFERARBPROC			glBindBufferARB;
	PFNGLGENBUFFERSARBPROC			glGenBuffersARB;
	PFNGLBUFFERDATAARBPROC			glBufferDataARB;
	PFNGLDELETEBUFFERSARBPROC		glDeleteBuffersARB;

	PFNGLLOCKARRAYSEXTPROC			glLockArraysEXT;
	PFNGLUNLOCKARRAYSEXTPROC		glUnlockArraysEXT;

	PFNGLTEXIMAGE3DEXTPROC			glTexImage3DEXT;

	PFNGLGENPROGRAMSARBPROC			glGenProgramsARB;
	PFNGLBINDPROGRAMARBPROC			glBindProgramARB;
	PFNGLPROGRAMSTRINGARBPROC		glProgramStringARB;
	PFNGLGETPROGRAMIVARBPROC		glGetProgramivARB;

	PFNGLPROGRAMLOCALPARAMETER4FARBPROC		glProgramLocalParameter4fARB;

	PFNGLFOGCOORDPOINTEREXTPROC				glFogCoordPointer;

public:
	customdecal_t		m_pDecals[MAX_CUSTOMDECALS];
	customdecal_t		m_pStaticDecals[MAX_STATICDECALS];
	decal_msg_cache		m_pMsgCache[MAX_DECAL_MSG_CACHE];
	decalgroup_t		m_pDecalGroups[MAX_DECAL_GROUPS];

	int					m_iNumDecals;
	int					m_iNumStaticDecals;
	int					m_iCurDecal;
	int					m_iCacheDecals;
	int					m_iNumDecalGroups;

	vec3_t				m_vDecalMins;
	vec3_t				m_vDecalMaxs;

private:
	GLint				m_iBlendActive;
	GLint				m_iTU1bind;
	GLint				m_iTU2bind;
	GLint				m_iTU3bind;
	GLint				m_iTU4bind;
	GLint				m_iTU1tex2dEnable;
	GLint				m_iTU2tex2dEnable;
	GLint				m_iTU3tex2dEnable;
	GLint				m_iTU4tex2dEnable;
	GLint				m_iActiveTU;
	GLint				m_iCLActiveTU;
	GLint				m_iTU1BlendMode;

	GLint				m_iAlphaFunc;
	GLclampf			m_AlphaVal;
	GLint				m_iAlphaTestEnabled;

};
extern CBSPRenderer gBSPRenderer;
#endif