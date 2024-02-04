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

#include "windows.h"
#include "hud.h"
#include "cl_util.h"
#include <gl/glu.h>

#include "const.h"
#include "studio.h"
#include "entity_state.h"
#include "triangleapi.h"
#include "event_api.h"
#include "pm_defs.h"
#include "pm_movevars.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <fstream> 
#include <iostream>

#include "propmanager.h"
#include "bsprenderer.h"
#include "particle_engine.h"
#include "watershader.h"
#include "mirrormanager.h"

#include "r_efx.h"
#include "r_studioint.h"
#include "studio_util.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
extern CGameStudioModelRenderer g_StudioRenderer;

using namespace std;

extern "C"
{
#include "pm_shared.h"
}

float turbsin[] = {
 0, 0.19633, 0.392541, 0.588517, 0.784137, 0.979285, 1.17384, 1.3677,
 1.56072, 1.75281, 1.94384, 2.1337, 2.32228, 2.50945, 2.69512, 2.87916,
 3.06147, 3.24193, 3.42044, 3.59689, 3.77117, 3.94319, 4.11282, 4.27998,
 4.44456, 4.60647, 4.76559, 4.92185, 5.07515, 5.22538, 5.37247, 5.51632,
 5.65685, 5.79398, 5.92761, 6.05767, 6.18408, 6.30677, 6.42566, 6.54068,
 6.65176, 6.75883, 6.86183, 6.9607, 7.05537, 7.14579, 7.23191, 7.31368,
 7.39104, 7.46394, 7.53235, 7.59623, 7.65552, 7.71021, 7.76025, 7.80562,
 7.84628, 7.88222, 7.91341, 7.93984, 7.96148, 7.97832, 7.99036, 7.99759,
 8, 7.99759, 7.99036, 7.97832, 7.96148, 7.93984, 7.91341, 7.88222,
 7.84628, 7.80562, 7.76025, 7.71021, 7.65552, 7.59623, 7.53235, 7.46394,
 7.39104, 7.31368, 7.23191, 7.14579, 7.05537, 6.9607, 6.86183, 6.75883,
 6.65176, 6.54068, 6.42566, 6.30677, 6.18408, 6.05767, 5.92761, 5.79398,
 5.65685, 5.51632, 5.37247, 5.22538, 5.07515, 4.92185, 4.76559, 4.60647,
 4.44456, 4.27998, 4.11282, 3.94319, 3.77117, 3.59689, 3.42044, 3.24193,
 3.06147, 2.87916, 2.69512, 2.50945, 2.32228, 2.1337, 1.94384, 1.75281,
 1.56072, 1.3677, 1.17384, 0.979285, 0.784137, 0.588517, 0.392541, 0.19633,
 9.79717e-16, -0.19633, -0.392541, -0.588517, -0.784137, -0.979285, -1.17384, -1.3677,
 -1.56072, -1.75281, -1.94384, -2.1337, -2.32228, -2.50945, -2.69512, -2.87916,
 -3.06147, -3.24193, -3.42044, -3.59689, -3.77117, -3.94319, -4.11282, -4.27998,
 -4.44456, -4.60647, -4.76559, -4.92185, -5.07515, -5.22538, -5.37247, -5.51632,
 -5.65685, -5.79398, -5.92761, -6.05767, -6.18408, -6.30677, -6.42566, -6.54068,
 -6.65176, -6.75883, -6.86183, -6.9607, -7.05537, -7.14579, -7.23191, -7.31368,
 -7.39104, -7.46394, -7.53235, -7.59623, -7.65552, -7.71021, -7.76025, -7.80562,
 -7.84628, -7.88222, -7.91341, -7.93984, -7.96148, -7.97832, -7.99036, -7.99759,
 -8, -7.99759, -7.99036, -7.97832, -7.96148, -7.93984, -7.91341, -7.88222,
 -7.84628, -7.80562, -7.76025, -7.71021, -7.65552, -7.59623, -7.53235, -7.46394,
 -7.39104, -7.31368, -7.23191, -7.14579, -7.05537, -6.9607, -6.86183, -6.75883,
 -6.65176, -6.54068, -6.42566, -6.30677, -6.18408, -6.05767, -5.92761, -5.79398,
 -5.65685, -5.51632, -5.37247, -5.22538, -5.07515, -4.92185, -4.76559, -4.60647,
 -4.44456, -4.27998, -4.11282, -3.94319, -3.77117, -3.59689, -3.42044, -3.24193,
 -3.06147, -2.87916, -2.69512, -2.50945, -2.32228, -2.1337, -1.94384, -1.75281,
 -1.56072, -1.3677, -1.17384, -0.979285, -0.784137, -0.588517, -0.392541, -0.19633
};

//===========================================
//	ARB SHADER
//===========================================
char fog_fp[] =
"!!ARBfp1.0"
"OPTION ARB_precision_hint_fastest;"
"TEMP R0;"
"ABS R0.x, fragment.fogcoord.x;" 
"SUB R0.y, state.fog.params.z, R0.x;"
"MUL_SAT R0.x, R0.y, state.fog.params.w;"
"MOV result.color, state.fog.color;"
"SUB result.color.a, 1.0, R0.x;"
"END";

char decal_fp[] =
"!!ARBfp1.0"
"TEMP R0; TEMP R1;"
"TEX R1, fragment.texcoord[0], texture[0], 2D;"
"ABS R0.x, fragment.fogcoord.x;" 
"SUB R0.y, state.fog.params.z, R0.x;"
"MUL_SAT R0.w, R0.y, state.fog.params.w;"
"MUL R0.xyz, R0.w, R1;"
"SUB R1.x, 1.0, R0.w;"
"MAD result.color.x, 0.498, R1.x, R0.x;"
"MAD result.color.y, 0.498, R1.x, R0.y;"
"MAD result.color.z, 0.498, R1.x, R0.z;"
"MOV result.color.w, R1.w;"
"END";

char shadow_fp[] =
"!!ARBfp1.0"
"OPTION ARB_fragment_program_shadow;"
"OPTION ARB_precision_hint_fastest;"
"PARAM c[5] = {"
"{0, -0.00390625},"
"{-0.00390625, 0},"
"{0.00390625, 0},"
"{0, 0.00390625},"
"{5, 1}};"
"TEMP R0;"
"TEMP R1;"
"RCP R0.x, fragment.texcoord[2].w;"
"MUL R0.xyz, fragment.texcoord[2], R0.x;"
"TEX R0.w, R0, texture[2], SHADOW2D;"
"ADD R1.xyz, R0, c[0];"
"TEX R1.w, R1, texture[2], SHADOW2D;"
"ADD R0.w, R1.w, R0.w;"
"ADD R1.xyz, R0, c[1];"
"TEX R1.w, R1, texture[2], SHADOW2D;"
"ADD R0.w, R1.w, R0.w;"
"ADD R1.xyz, R0, c[2];"
"TEX R1.w, R1, texture[2], SHADOW2D;"
"ADD R0.w, R1.w, R0.w;"
"ADD R1.xyz, R0, c[3];"
"TEX R1.w, R1, texture[2], SHADOW2D;"
"ADD R0.w, R1.w, R0.w;"
"RCP R1.w, c[4].x;"
"MUL R1.w, R0.w, R1.w;"
"TXP R0, fragment.texcoord[0], texture[0], 2D;"
"MUL R1, R0, R1.w;"
"TXP R0, fragment.texcoord[1], texture[1], 1D;"
"MUL R1, R1, R0;"
"MUL result.color.xyz, fragment.color.primary, R1;"
"MOV result.color.w, c[4].y;"
"END";
//===========================================
//	ARB SHADER
//===========================================

/*
====================
Shutdown

====================
*/
void CBSPRenderer::Shutdown( void )
{	
	FreeBuffer();

	// Clear previous
	if(m_iNumSurfaces)
	{	
		delete [] m_pSurfaces;
		m_pSurfaces = nullptr;
		m_iNumSurfaces = NULL;
	}

	ClearDetailObjects();
	DeleteDecals();
}

/*
====================
Init

====================
*/
void CBSPRenderer::Init( void ) 
{
	//
	// Check extensions
	//
	if (!ExtensionSupported("GL_ARB_multitexture"))
	{
		gEngfuncs.pfnClientCmd("escape\n");	
		MessageBox(NULL, "VIDEO ERROR: Your hardware does not support multitexturing!\n\nPress Ok to quit the game.\n", "ERROR", MB_OK);
		gEngfuncs.pfnClientCmd("quit\n");	
	}

	if (!ExtensionSupported("ARB_vertex_buffer_object"))
	{
		gEngfuncs.pfnClientCmd("escape\n");	
		MessageBox(NULL, "VIDEO ERROR: Your hardware does not support vertex buffer objects!\n\nPress Ok to quit the game.\n", "ERROR", MB_OK);
		gEngfuncs.pfnClientCmd("quit\n");	
	}

	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &m_iTUSupport);
	if (m_iTUSupport < 3)
	{
		gEngfuncs.pfnClientCmd("escape\n");	
		MessageBox(NULL, "VIDEO ERROR: Your hardware does not support enough multitexture units!\n\nPress Ok to quit the game.\n", "ERROR", MB_OK);
		gEngfuncs.pfnClientCmd("quit\n");
	}

	if( ExtensionSupported("GL_NV_fog_distance") )
	{
		// Radial fog is supported
		m_bRadialFogSupport = true;
	}

	if (ExtensionSupported("ARB_fragment_program") && ExtensionSupported("ARB_vertex_program"))
	{
		// Shaders are supported
		m_bShaderSupport = true;
		m_bDontPromptShaders = true;
	}

	if (ExtensionSupported("GL_ARB_fragment_program_shadow")&&!ExtensionSupported("PARANOIA_HACKS_V1"))
	{
		m_bShadowPCFSupport = true;
		m_bDontPromptShadowPCF = true;
	}

	if (ExtensionSupported("GL_ARB_shadow")&&!ExtensionSupported("PARANOIA_HACKS_V1"))
	{
		m_bShadowSupport = true;
		m_bDontPromptShadow = true;
	}

	if(!ExtensionSupported("PARANOIA_HACKS_V1"))
	{
		m_bDontPromptParanoia = true;
	}

	//
	// Load our OGL functions
	//

	glActiveTextureARB				= (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
	glClientActiveTextureARB		= (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");
	glMultiTexCoord2fARB			= (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");

	glBindBufferARB					= (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
	glGenBuffersARB					= (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
	glBufferDataARB					= (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
	glDeleteBuffersARB				= (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");

	glLockArraysEXT					= (PFNGLLOCKARRAYSEXTPROC)wglGetProcAddress("glLockArraysEXT");
	glUnlockArraysEXT				= (PFNGLUNLOCKARRAYSEXTPROC)wglGetProcAddress("glUnlockArraysEXT");

	glTexImage3DEXT					= (PFNGLTEXIMAGE3DEXTPROC)wglGetProcAddress("glTexImage3DEXT");

	glGenProgramsARB				= (PFNGLGENPROGRAMSARBPROC)wglGetProcAddress("glGenProgramsARB");
	glBindProgramARB				= (PFNGLBINDPROGRAMARBPROC)wglGetProcAddress("glBindProgramARB");
	glProgramStringARB				= (PFNGLPROGRAMSTRINGARBPROC)wglGetProcAddress("glProgramStringARB");
	glGetProgramivARB				= (PFNGLGETPROGRAMIVARBPROC)wglGetProcAddress("glGetProgramivARB");

	glProgramLocalParameter4fARB	= (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)wglGetProcAddress("glProgramLocalParameter4fARB");
	glFogCoordPointer				= (PFNGLFOGCOORDPOINTEREXTPROC)wglGetProcAddress("glFogCoordPointer");

#ifdef HL25_UPDATE
	glUseProgram					= (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
#endif

	//
	// Initialize basic stuff
	//

	m_iEngineLightmapIndex = current_ext_texture_id;
	current_ext_texture_id++;

	m_iDetailLightmapIndex = current_ext_texture_id;
	current_ext_texture_id++;

	// 0 normal
	AddLightStyle(0, "m");

	// 1 FLICKER (first variety)
	AddLightStyle(1, "mmnmmommommnonmmonqnmmo");

	// 2 SLOW STRONG PULSE
	AddLightStyle(2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

	// 3 CANDLE (first variety)
	AddLightStyle(3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

	// 4 FAST STROBE
	AddLightStyle(4, "mamamamamama");

	// 5 GENTLE PULSE 1
	AddLightStyle(5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");

	// 6 FLICKER (second variety)
	AddLightStyle(6, "nmonqnmomnmomomno");

	// 7 CANDLE (second variety)
	AddLightStyle(7, "mmmaaaabcdefgmmmmaaaammmaamm");

	// 8 CANDLE (third variety)
	AddLightStyle(8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

	// 9 SLOW STROBE (fourth variety)
	AddLightStyle(9, "aaaaaaaazzzzzzzz");

	// 10 FLUORESCENT FLICKER
	AddLightStyle(10, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	AddLightStyle(11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

	// 12 UNDERWATER LIGHT MUTATION
	AddLightStyle(12, "mmnnmmnnnmmnn");

	m_iFrameCount					= 0;
	m_iVisFrame						= 0;
	m_bDontPromptShadersError		= true;

	gEngfuncs.pfnAddCommand ("te_dump", RenderersDumpInfo);
	gEngfuncs.pfnAddCommand ("te_detail_auto", GenDetail );
	gEngfuncs.pfnAddCommand ("te_exportworld", ExportWorld );

	m_pCvarDrawWorld				= CVAR_CREATE( "te_world", "1", 0 );
	m_pCvarSpeeds					= CVAR_CREATE( "te_speeds", "0", 0 );
	m_pCvarDetailTextures			= CVAR_CREATE( "te_detail", "1", 0 );
	m_pCvarWorldShaders				= CVAR_CREATE( "te_world_shaders", "1", FCVAR_ARCHIVE );
	m_pCvarWireFrame				= CVAR_CREATE( "te_wireframe", "0", 0 );
	m_pCvarDynamic					= CVAR_CREATE( "te_dynlights", "1", 0 );
	m_pCvarRadialFog				= CVAR_CREATE( "te_radialfog", "1", 0 );
	m_pCvarPCFShadows				= CVAR_CREATE( "te_shadows_filter", "1", FCVAR_ARCHIVE );
	m_pCvarShadows					= CVAR_CREATE( "te_shadows", "1", FCVAR_ARCHIVE );

	//
	// Load shaders
	//

	// Don't bother
	if(!m_bShaderSupport)
		return;

	GLint iErrorPos, iIsNative;
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glGenProgramsARB(1, &m_iFogFragmentID);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_iFogFragmentID);

	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, sizeof(fog_fp)-1, fog_fp);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &iErrorPos);
	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &iIsNative);

	if(iErrorPos != -1 || !iIsNative)
	{
		m_bShaderSupport = false;
		m_bDontPromptShadersError = false;
		return;
	}

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glGenProgramsARB(1, &m_iDecalFragmentID);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_iDecalFragmentID);

	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, sizeof(decal_fp)-1, decal_fp);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &iErrorPos);
	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &iIsNative);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

	if(iErrorPos != -1 || !iIsNative)
	{
		m_bShaderSupport = false;
		m_bDontPromptShadersError = false;
		return;
	}

	if(!m_bShadowPCFSupport)
		return;

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glGenProgramsARB(1, &m_iShadowFragmentID);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_iShadowFragmentID);

	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, sizeof(shadow_fp)-1, shadow_fp);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &iErrorPos);
	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &iIsNative);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

	if(iErrorPos != -1 || !iIsNative)
	{
		m_bShaderSupport = false;
		m_bDontPromptShadersError = false;
	}

	// Mark as contents_solid
	m_dummyNode.contents = CONTENTS_SOLID;
};

/*
====================
VidInit

====================
*/
void CBSPRenderer::VidInit( void )
{
	if (!IEngineStudio.IsHardware())
	{
		gEngfuncs.pfnClientCmd("escape\n");	
		MessageBox(NULL, "VIDEO ERROR: This game does not support Software mode!\nTry using the -gl parameter in the command line.\n\nPress Ok to quit the game.\n", "ERROR", MB_OK);
		exit(-1);
	}
	if (IEngineStudio.IsHardware() == 2)
	{
		gEngfuncs.pfnClientCmd("escape\n");	
		MessageBox(NULL, "VIDEO ERROR: This game does not support DirectX!\nTry using the -gl parameter in the command line.\n\nPress Ok to quit the game.\n", "ERROR", MB_OK);
		exit(-1);
	}

	if(!m_bDontPromptShaders)
		gEngfuncs.Con_Printf("ERROR! Your hardware doesn't support assembly shaders! Advanced effects will be disabled.\n");

	if(!m_bDontPromptShadersError)
		gEngfuncs.Con_Printf("ERROR! There was an error with the shaders! Advanced effects will remain disabled.\n");

	if(!m_bDontPromptShadow)
		gEngfuncs.Con_Printf("Error! Your hardware doesn't support shadow mapping! Shadows will remain disabled.\n");

	if(!m_bDontPromptShadowPCF)
		gEngfuncs.Con_Printf("Error! Your hardware doesn't support shadow filtering! Filtering on shadows won't be available.\n");

	if(!m_bDontPromptParanoia)
		gEngfuncs.Con_Printf("Paranoia's shitty opengl32.dll was detected! This conflicts with shadow mapping! Remove this dll in order to have shadow maps.\n");

	// Clear this
	VectorClear(m_vSkyOrigin);
	VectorClear(m_vSkyWorldOrigin);

	if(m_bShadowSupport)
	{
		for(int i = 0; i < MAX_DYNLIGHTS; i++)
		{
			if(m_pDynLights[i].depth)
			{
				glDeleteTextures(1, &m_pDynLights[i].depth);
				m_pDynLights[i].depth = 0;
			}
		}
	}

	// Clear all lightstyles.
	memset(m_iLightStyleValue,		0, sizeof(m_iLightStyleValue));
	memset(m_pDynLights,			0, sizeof(m_pDynLights));
	memset(m_pDetailTextures,		0, sizeof(m_pDetailTextures));
	memset(m_pDecalGroups,			0, sizeof(m_pDecalGroups));
	memset(m_pNormalTextureList,	0, sizeof(m_pNormalTextureList));
	memset(m_pMultiPassTextureList,	0, sizeof(m_pMultiPassTextureList));
	memset(&m_pFlashlightTextures,	0, sizeof(m_pFlashlightTextures));
	memset(&gHUD.m_pSkyFogSettings,	0, sizeof(fog_settings_t));
	memset(&gHUD.m_pFogSettings,	0, sizeof(fog_settings_t));

	// Clear previous
	if(m_iNumSurfaces)
	{	
		delete [] m_pSurfaces;
		m_pSurfaces = nullptr;
		m_iNumSurfaces = NULL;
	}

	if(m_pSurfacePointersArray)
	{
		delete[] m_pSurfacePointersArray;
		m_pSurfacePointersArray = nullptr;
	}

	if(m_pDetailObjectSurfaces)
	{
		delete [] m_pDetailObjectSurfaces;
		m_pDetailObjectSurfaces = nullptr;
	}

	m_numDetailSurfaces = 0;

	if(m_bShadowSupport)
	{
		int iCurrentBinding;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &iCurrentBinding);

		for(int i = 0; i < MAX_DYNLIGHTS; i++)
		{
			m_pDynLights[i].depth = current_ext_texture_id;
			current_ext_texture_id++;

			glBindTexture(GL_TEXTURE_2D, m_pDynLights[i].depth);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, DEPTHMAP_RESOLUTION, DEPTHMAP_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
			glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);
		}

		glBindTexture(GL_TEXTURE_2D, iCurrentBinding);
	}

	// Get pointer to very first dynamic and entity light, key doesn't matter.
	m_pFirstELight			= gEngfuncs.pEfxAPI->CL_AllocElight(0);
	m_pFirstDLight			= gEngfuncs.pEfxAPI->CL_AllocDlight(0);
	m_fSkySpeed				= NULL;
	m_pWorld				= NULL;
	m_iNumDetailTextures	= NULL;
	m_iFrameCount			= NULL;
	m_iVisFrame				= NULL;
	m_iNumDecalGroups		= NULL;
	m_iNumTextures			= NULL;
	m_bMirroring			= false;
	m_bSpecialFog			= false;
	m_iNumFlashlightTextures = NULL;

	ClearDetailObjects();
	DeleteDecals();

	// A call to VidInit means a reload
	m_bReloaded	= true;
}

/*
====================
ExtensionSupported

====================
*/
bool CBSPRenderer::ExtensionSupported( const char *ext )
{
	const char * extensions = (const char *)glGetString( GL_EXTENSIONS );
	const char * start = extensions;
	const char * ptr;

	while ( ( ptr = strstr ( start, ext ) ) != NULL )
	{
		// we've found, ensure name is exactly ext
		const char * end = ptr + strlen ( ext );
		if ( isspace ( *end ) || *end == '\0' )
			return true;

		start = end;
	}
	return false;
}

/*
====================
GetRenderEnts

====================
*/
void CBSPRenderer::GetRenderEnts( void )
{
	// Clear counters
	m_iNumRenderEntities = NULL;
	m_iNumModelLights = NULL;

	cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();
	cl_entity_t *pView = gEngfuncs.GetViewModel();

	int iMsg = pPlayer->curstate.messagenum;

	// Clear water shader class
	for(int i = 0; i < gWaterShader.m_iNumWaterEntities; i++)
		gWaterShader.m_pWaterEntities[i].draw = false;

	// Clear mirror class
	for(int i = 0; i < gMirrorManager.m_iNumMirrors; i++)
		gMirrorManager.m_pMirrors[i].draw = false;

	for ( int i = 1; i < MAXRENDERENTS; i++ )
	{
		cl_entity_t *pEntity = gEngfuncs.GetEntityByIndex(i);

		if (!pEntity)
			break;

		if (pEntity == pView)
			continue;

		if (!pEntity->model)
			continue;

		if (pEntity->curstate.messagenum != iMsg)
			continue;

		if (pEntity->curstate.effects & EF_NODRAW)
			continue;

		if((pEntity->curstate.effects & FL_MIRROR) && gMirrorManager.m_pCvarDrawMirrors->value > 0)
		{
			if(!pEntity->efrag)
				gMirrorManager.AllocNewMirror(pEntity);

			if(pEntity->efrag)
				((cl_mirror_t *)pEntity->efrag)->draw = true;

			continue;
		}

		if((pEntity->curstate.effects & FL_WATERSHADER) && m_bShaderSupport && gWaterShader.m_pCvarWaterShader->value > 0)
		{
			if(!pEntity->efrag)
				gWaterShader.AddEntity(pEntity);

			if(pEntity->efrag)
				((cl_water_t *)pEntity->efrag)->draw = true;

			continue;
		}

		if ( pEntity->curstate.effects & FL_ELIGHT )
		{
			// mlights are static
			mlight_t *mlight = &m_pModelLights[m_iNumModelLights];

			mlight->color.x	= (float)pEntity->curstate.rendercolor.r/255;
			mlight->color.y	= (float)pEntity->curstate.rendercolor.g/255;
			mlight->color.z	= (float)pEntity->curstate.rendercolor.b/255;
			mlight->radius = pEntity->curstate.renderamt*9.5;

			mlight->origin = pEntity->origin;
			mlight->mins.x = mlight->origin.x - mlight->radius;
			mlight->mins.y = mlight->origin.y - mlight->radius;
			mlight->mins.z = mlight->origin.z - mlight->radius;
			mlight->maxs.x = mlight->origin.x + mlight->radius;
			mlight->maxs.y = mlight->origin.y + mlight->radius;
			mlight->maxs.z = mlight->origin.z + mlight->radius;

			mlight->spotcos = 0;
			mlight->flashlight = false;

			m_iNumModelLights++;
			continue;
		}

		if ( pEntity->curstate.effects & FL_DLIGHT )
		{
			cl_dlight_t *dlight	= CL_AllocDLight(pEntity->index);

			dlight->color.x		= (float)pEntity->curstate.rendercolor.r/255;
			dlight->color.y		= (float)pEntity->curstate.rendercolor.g/255;
			dlight->color.z		= (float)pEntity->curstate.rendercolor.b/255;
			dlight->radius		= pEntity->curstate.renderamt;
			dlight->origin		= pEntity->curstate.origin;
			dlight->die			= gEngfuncs.GetClientTime() + 1;
			continue;
		}

		if ( pEntity->curstate.effects & FL_SPOTLIGHT )
		{
			if(!m_bShaderSupport || m_pCvarWorldShaders->value < 1)
				continue;

			if(pEntity->curstate.body >= m_iNumFlashlightTextures)
			{
				gEngfuncs.Con_Printf("Texture index greater than the amount of flashlight textures loaded!\n");
				continue;
			}

			cl_dlight_t *dlight	= CL_AllocDLight(pEntity->index);

			dlight->color.x		= (float)pEntity->curstate.rendercolor.r/255;
			dlight->color.y		= (float)pEntity->curstate.rendercolor.g/255;
			dlight->color.z		= (float)pEntity->curstate.rendercolor.b/255;
			dlight->radius		= pEntity->curstate.renderamt;
			dlight->origin		= pEntity->curstate.origin;
			dlight->cone_size   = pEntity->curstate.scale;
			dlight->angles		= pEntity->angles;
			dlight->die			= gEngfuncs.GetClientTime() + 1;
			dlight->textureindex = m_pFlashlightTextures[pEntity->curstate.body]->iIndex;
			dlight->noshadow	= pEntity->curstate.sequence;

			dlight->frustum.SetFrustum(dlight->angles, dlight->origin, dlight->cone_size*1.2, dlight->radius);
			continue;
		}

		if ( pEntity->curstate.effects & EF_LIGHT )
		{
			cl_dlight_t *dlight	= CL_AllocDLight(pEntity->index);

			dlight->color.x		= 1.0;
			dlight->color.y		= 1.0;
			dlight->color.z		= 1.0;
			dlight->radius		= 500;
			dlight->origin		= pEntity->curstate.origin;
			dlight->die			= gEngfuncs.GetClientTime() + 1;
			dlight->decay		= 500;
			continue;
		}

		if (pEntity->model->type == mod_brush || pEntity->model->type == mod_studio)
		{
			m_pRenderEntities[m_iNumRenderEntities] = pEntity;
			m_iNumRenderEntities++;
		}
	}

	m_bGotAdditional = false;

	dlight_t *el = m_pFirstELight;
	for( int i = 0; i < MAX_GOLDSRC_ELIGHTS; i++, el++ )
	{
		if (el->die < gEngfuncs.GetClientTime() || !el->radius)
			continue;

		mlight_t *mlight = &m_pModelLights[m_iNumModelLights];
		m_iNumModelLights++;

		mlight->color.x = (float)el->color.r/255;
		mlight->color.y = (float)el->color.g/255;
		mlight->color.z = (float)el->color.b/255;
		mlight->radius = el->radius * 2;
		mlight->origin = el->origin;
		mlight->mins.x = mlight->origin.x - mlight->radius;
		mlight->mins.y = mlight->origin.y - mlight->radius;
		mlight->mins.z = mlight->origin.z - mlight->radius;
		mlight->maxs.x = mlight->origin.x + mlight->radius;
		mlight->maxs.y = mlight->origin.y + mlight->radius;
		mlight->maxs.z = mlight->origin.z + mlight->radius;
		mlight->flashlight = false;
		mlight->spotcos = 0;
	}

	cl_dlight_t *dl = m_pDynLights;
	for(int i = 0; i < MAX_DYNLIGHTS; i++, dl++)
	{
		if (dl->die < gEngfuncs.GetClientTime() || !dl->radius)
			continue;

		mlight_t *mlight = &m_pModelLights[m_iNumModelLights];

		mlight->color.x = dl->color.x;
		mlight->color.y = dl->color.y;
		mlight->color.z = dl->color.z;
		mlight->radius = dl->radius;
		mlight->origin = dl->origin;
		mlight->mins.x = dl->origin.x - mlight->radius;
		mlight->mins.y = dl->origin.y - mlight->radius;
		mlight->mins.z = dl->origin.z - mlight->radius;
		mlight->maxs.x = dl->origin.x + mlight->radius;
		mlight->maxs.y = dl->origin.y + mlight->radius;
		mlight->maxs.z = dl->origin.z + mlight->radius;
		mlight->spotcos = 0;

		if(dl->key == -666)
			mlight->flashlight = true;
		else
			mlight->flashlight = false;

		if(dl->cone_size)
		{
			vec3_t vAngles = dl->angles;
			FixVectorForSpotlight(vAngles);
			AngleVectors(vAngles, mlight->forward, NULL, NULL);
			mlight->spotcos = dl->cone_size;
			mlight->frustum = &dl->frustum;
		}

		m_iNumModelLights++;
	}
}

/*
====================
AddEntity

====================
*/
void CBSPRenderer::AddEntity( cl_entity_t *pEntity ) 
{
	if(pEntity->model->type != mod_studio)
		return;

	m_pRenderEntities[m_iNumRenderEntities] = pEntity;
	m_iNumRenderEntities++;
}

/*
====================
GetAdditionalLights

====================
*/
void CBSPRenderer::GetAdditionalLights( void )
{
	if(m_bGotAdditional)
		return;

	// Got them all
	m_bGotAdditional = true;

	cl_entity_t *pLight = gPropManager.m_pModelLights;
	for(int i = 0; i < gPropManager.m_iNumModelLights; i++, pLight++)
	{
		if(m_pPVS[pLight->visframe >> 3] & (1 << (pLight->visframe&7)))
		{
			mlight_t *mlight = &m_pModelLights[m_iNumModelLights];
			m_iNumModelLights++;

			mlight->color.x = (float)pLight->curstate.rendercolor.r/255;
			mlight->color.y = (float)pLight->curstate.rendercolor.g/255;
			mlight->color.z = (float)pLight->curstate.rendercolor.b/255;
			mlight->radius = pLight->curstate.renderamt * 9.5;
			mlight->origin = pLight->curstate.origin;
			mlight->mins.x = mlight->origin.x - mlight->radius;
			mlight->mins.y = mlight->origin.y - mlight->radius;
			mlight->mins.z = mlight->origin.z - mlight->radius;
			mlight->maxs.x = mlight->origin.x + mlight->radius;
			mlight->maxs.y = mlight->origin.y + mlight->radius;
			mlight->maxs.z = mlight->origin.z + mlight->radius;

			mlight->flashlight = false;
			mlight->spotcos = 0;
		}
	}
}

/*
====================
AddLightStyle

====================
*/
void CBSPRenderer::AddLightStyle( int iNum, char *szStyle )
{
	memset(&m_pLightStyles[iNum], 0, sizeof(lightstyle_t));

	m_pLightStyles[iNum].length = strlen(szStyle);
	strcpy(m_pLightStyles[iNum].map, szStyle);
};

/*
====================
FilterEntities

====================
*/
int CBSPRenderer::FilterEntities( int type, struct cl_entity_s *pEntity, const char *modelname )
{
	if ( pEntity->model->type == mod_brush )
		return FALSE;

	if ( pEntity->curstate.effects & FL_DLIGHT )
		return FALSE;

	if ( pEntity->curstate.effects & FL_ELIGHT )
		return FALSE;

	if ( pEntity->curstate.effects & FL_SPOTLIGHT )
		return FALSE;

	return TRUE;
}

/*
====================
SetupPreFrame

====================
*/
void CBSPRenderer::SetupPreFrame ( ref_params_t *pparams )
{
	// Get the leaf we're on
	m_pWorld	= IEngineStudio.GetModelByIndex(1);
	m_pViewLeaf	= Mod_PointInLeaf( pparams->vieworg, m_pWorld );

	//Ensure proper shadow maps
	SetupSpotlightVis();

	//Get current vis data
	m_pPVS = Mod_LeafPVS(m_pViewLeaf, m_pWorld);

	//Get additional lights here
	GetAdditionalLights();

	//Bind VBO at frame start
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_uiBufferIndex);

	// Set pointers up at start of frame
	glVertexPointer(3, GL_FLOAT, sizeof(brushvertex_t), OFFSET(brushvertex_t, pos));
	glNormalPointer(GL_FLOAT, sizeof(brushvertex_t), OFFSET(brushvertex_t, normal));

	if(m_bSpecialFog)
		glFogCoordPointer(GL_FLOAT, sizeof(brushvertex_t), OFFSET(brushvertex_t, fogcoord));
}

/*
====================
SetupSpotlightVis

====================
*/
void CBSPRenderer::SetupSpotlightVis( void )
{
	if(!m_bShaderSupport || m_pCvarWorldShaders->value < 1)
		return;

	if(!m_bShadowSupport || m_pCvarShadows->value < 1)
		return;

	float flTime = gEngfuncs.GetClientTime();
	cl_dlight_t *pLight = m_pDynLights;
	for(int i = 0; i < MAX_DYNLIGHTS; i++, pLight++)
	{
		if(pLight->die <= flTime || !pLight->radius || !pLight->cone_size || pLight->key == -666)
			continue;

		// Make sure the spotlight sees it's polygons
		mleaf_t *pLeaf = Mod_PointInLeaf(pLight->origin, m_pWorld);
		R_MarkLeaves(pLeaf);
	}
}
/*
====================
CheckTextures

====================
*/
void CBSPRenderer::CheckTextures ( void )
{
	char szFile[64];
	char szTexture[32];

	model_t *pWorld = IEngineStudio.GetModelByIndex(1);
	for(int i = 0; i < pWorld->numtextures; i++)
	{
		if(!pWorld->textures[i])
			continue;
		
		// Skip specials
		if(!strcmp(pWorld->textures[i]->name, "origin")
			||!strcmp(pWorld->textures[i]->name, "clip")
			||!strcmp(pWorld->textures[i]->name, "null")
			||!strcmp(pWorld->textures[i]->name, "skip")
			||!strcmp(pWorld->textures[i]->name, "hint"))
			continue;

		strcpy(szTexture, pWorld->textures[i]->name);
		strLower(szTexture);

		if(gTextureLoader.TextureHasFlag("world", szTexture, TEXFLAG_ALTERNATE))
		{
			gEngfuncs.Con_DPrintf("World has '%s' marked as using an alternate texture.\n", pWorld->textures[i]->name);
			sprintf(szFile, "gfx/textures/world/%s.dds", szTexture );
			cl_texture_t *pTexture = gTextureLoader.LoadTexture(szFile, pWorld->textures[i]->gl_texturenum);

			if(pTexture)
			{
				gEngfuncs.Con_DPrintf("Loaded '%s'\n",szFile);
				continue;
			}
		}

		gTextureLoader.LoadWADTexture(szTexture, pWorld->textures[i]->gl_texturenum);
	}
}
/*
====================
SetupRenderer

====================
*/
void CBSPRenderer::SetupRenderer ( void )
{
	if (!m_bReloaded)
		return;

	glPushAttrib(GL_TEXTURE_BIT);

	// Get pointer to world
	m_pWorld = IEngineStudio.GetModelByIndex(1);
	m_bReloaded = false;

	if(!m_pWorld)
	{
		gEngfuncs.pfnClientCmd("escape\n");	
		MessageBox(NULL, "FATAL ERROR: Failed to get world!\n\nPress Ok to quit the game.\n", "ERROR", MB_OK);
		exit(-1);
	}

	// Swap the engine cache for models
	gTextureLoader.LoadTextureScript();
	gPropManager.GenerateEntityList();
	gTextureLoader.LoadWADFiles();

	g_StudioRenderer.StudioSwapEngineCache();
	CheckTextures(); // Do it for world seperately
	gWaterShader.LoadScript();

	LoadDecals();
	CreateTextures();
	LoadDetailFile();

	UploadLightmaps();
	LoadDetailTextures();
	GenerateVertexArray();
	
	InitSky();
	RemoveSky();

	gTextureLoader.FreeWADFiles();
	gPropManager.ClearEntityData();

	glPopAttrib();
}

/*
====================
RendererRefDef

====================
*/
void CBSPRenderer::RendererRefDef ( ref_params_t *pparams )
{
	gHUD.viewFrustum.SetFrustum(pparams->viewangles, pparams->vieworg, gHUD.m_iFOV, gHUD.m_pFogSettings.end, true);
	VectorCopy( pparams->viewangles,	m_vViewAngles	);
	VectorCopy( pparams->vieworg,		m_vRenderOrigin	);

	gParticleEngine.m_iNumParticles			= NULL;
	m_iBrushPolyCounter						= NULL;
	m_iWorldPolyCounter						= NULL;
	m_iStudioPolyCounter					= NULL;
	m_pWorld								= IEngineStudio.GetModelByIndex(1);

	// Advance frame count here
	m_iFrameCount++;

	// Clear HL's dlight array
	dlight_t *dl = m_pFirstDLight;
	for(int i = 0; i < MAX_GOLDSRC_DLIGHTS; i++, dl++)
		memset(dl, 0, sizeof(dlight_t));

	PushDynLights();
	ClearToFogColor();
	DisableWorldDrawing(pparams);

	if(!pparams->onlyClientDraw)
		m_bCanDraw = true;
};

/*
====================
DisableWorldDrawing

====================
*/
void CBSPRenderer::DisableWorldDrawing( ref_params_t *pparams )
{
	if (m_pTrueRootNode)
	{
		gEngfuncs.Con_Printf("%s - Called without having been restored first!\n");
		return;
	}

	m_pTrueRootNode = m_pWorld->nodes;
	m_pWorld->nodes = &m_dummyNode;
};

/*
====================
RestoreWorldDrawing

====================
*/
void CBSPRenderer::RestoreWorldDrawing( void )
{
	// Shouldn't happen
	if (!m_pTrueRootNode)
		return;

	// Restore and clear pointer
	m_pWorld->nodes = m_pTrueRootNode;
	m_pTrueRootNode = nullptr;

	// Mark all leaves with current visframe
	R_MarkLeaves(m_pViewLeaf);
};

/*
====================
RemoveSky

====================
*/
void CBSPRenderer::RemoveSky( void )
{
	bool foundSky = false;
	msurface_t* surfaces = m_pWorld->surfaces;
	for (int i = 0; i < m_pWorld->numsurfaces; i++)
	{
		if (surfaces[i].flags & SURF_DRAWSKY)
		{
			glpoly_t *p = surfaces[i].polys;
			p->numverts = -p->numverts;
			foundSky = true;
		}
	}

	if(!foundSky)
		m_bDrawSky = false;
};

/*
====================
ClearDetailObjects

====================
*/
void CBSPRenderer::ClearDetailObjects( void )
{
	for(int i = 0; i < m_numDetailSurfaces; i++)
	{
		msurface_t* psurf = &m_pDetailObjectSurfaces[i];

		if(psurf->polys)
			free(psurf->polys);

		if(psurf->plane)
			delete psurf->plane;

		if(psurf->texinfo)
			delete psurf->texinfo;
	}

	memset(m_pDetailObjects, 0, sizeof(m_pDetailObjects));
	m_iNumDetailObjects = NULL;
	m_iNumDetailSurfaces = NULL;
}

/*
====================
LoadDetailFile

Comment: BLARGH
====================
*/
void CBSPRenderer::LoadDetailFile( void )
{
	char szPath[64];
	char szFile[64];
	sprintf(szFile, "%s", m_pWorld->name);
	strcpy(&szFile[strlen(szFile)-3], "edd");

	int iOffset = 0;
	byte *pFile = gEngfuncs.COM_LoadFile(szFile, 5, NULL);
	if(!pFile) 
		return;

	bool bNeedsUpload[MAX_LIGHTMAPS];
	memset(bNeedsUpload, 0, sizeof(bNeedsUpload));

	vector<msurface_t> surfacesArray;

	memcpy(&m_iNumDetailObjects, &pFile[iOffset], sizeof(int));iOffset+=4;
	for(int i = 0; i < m_iNumDetailObjects; i++)
	{
		memcpy(&m_pDetailObjects[i].mins, &pFile[iOffset], sizeof(vec3_t));iOffset+=12;
		memcpy(&m_pDetailObjects[i].maxs, &pFile[iOffset], sizeof(vec3_t));iOffset+=12;

		int iNumSurfaces = 0;
		memcpy(&iNumSurfaces, &pFile[iOffset], sizeof(int));iOffset+=4;

		if(!iNumSurfaces)
			continue;

		m_pDetailObjects[i].firstsurface = surfacesArray.size();
		surfacesArray.resize(surfacesArray.size() + iNumSurfaces);
		m_pDetailObjects[i].numsurfaces = iNumSurfaces; m_iNumDetailSurfaces += iNumSurfaces;

		msurface_t *psurfaces = &surfacesArray[m_pDetailObjects[i].firstsurface];
		for(int j = 0; j < iNumSurfaces; j++)
		{
			char szTexName[16];
			memcpy(&szTexName, &pFile[iOffset], sizeof(char)*16); iOffset += 16;
			memset(psurfaces[j].styles, 255, sizeof(byte)*4);

			int iVertnum = 0;
			memcpy(&psurfaces[j].lightmaptexturenum, &pFile[iOffset], sizeof(int)); iOffset += 4;
			memcpy(&iVertnum, &pFile[iOffset], sizeof(int)); iOffset += 4;

			int iPlusVerts = iVertnum - 4;
			if(iPlusVerts < 0) iPlusVerts = 0;
			
			psurfaces[j].polys = (glpoly_t *)malloc(sizeof(glpoly_t)+sizeof(float)*VERTEXSIZE*iPlusVerts);
			
			glpoly_t *pPoly = psurfaces[j].polys;
			pPoly->chain = NULL; pPoly->flags = NULL; pPoly->next = NULL; pPoly->numverts = NULL;
			memset(pPoly->verts, 0, sizeof(float)*VERTEXSIZE*4);
			pPoly->numverts = iVertnum;

			psurfaces[j].plane = new mplane_t;
			memset(psurfaces[j].plane, 0, sizeof(mplane_t));
			memcpy(&psurfaces[j].plane->normal, &pFile[iOffset], sizeof(vec3_t)); iOffset += 12;
			memcpy(&psurfaces[j].plane->dist, &pFile[iOffset], sizeof(float)); iOffset += 4;

			psurfaces[j].texinfo = new mtexinfo_t;
			memset(psurfaces[j].texinfo, 0, sizeof(mtexinfo_t));

			// Mark as transparent
			if(szTexName[0] == '{' && m_pDetailObjects[i].rendermode != kRenderTransAlpha)
				m_pDetailObjects[i].rendermode = kRenderTransAlpha;

			// See if we already have it
			int k;
			for(k = 0; k < m_iNumTextures; k++)
			{
				if(!strcmp(m_pNormalTextureList[k].name, szTexName))
				{
					psurfaces[j].texinfo->texture = &m_pNormalTextureList[k];
					break;
				}
			}

			if(k == m_iNumTextures)
			{
				cl_texture_t *pTexture = NULL;
				psurfaces[j].texinfo->texture = &m_pNormalTextureList[m_iNumTextures];
				texture_t *pMultiPass = &m_pMultiPassTextureList[m_iNumTextures];
				m_iNumTextures++;

				// See if it's alternate flagged
				if(gTextureLoader.TextureHasFlag("world", szTexName, TEXFLAG_ALTERNATE))
				{
					sprintf(szPath, "gfx/textures/world/%s.dds", szTexName );
					pTexture = gTextureLoader.LoadTexture(szPath);
				}
	
				// Just load it from a WAD then
				if(!pTexture)
					pTexture = gTextureLoader.LoadWADTexture(szTexName);

				if(!pTexture)
				{
					gEngfuncs.Con_Printf("Error! .edd file couldn't load texture: %s!\n", szTexName);
					gEngfuncs.COM_FreeFile(pFile); ClearDetailObjects();
					return;
				}

				psurfaces[j].texinfo->texture->width = pTexture->iWidth;
				psurfaces[j].texinfo->texture->height = pTexture->iHeight;
				psurfaces[j].texinfo->texture->gl_texturenum = pTexture->iIndex;
				strcpy(psurfaces[j].texinfo->texture->name, szTexName);
				memcpy(pMultiPass, psurfaces[j].texinfo->texture, sizeof(texture_t));
			}

			memcpy(&psurfaces[j].texinfo->flags, &pFile[iOffset], sizeof(int)); iOffset += 4;
			memcpy(&psurfaces[j].texinfo->mipadjust, &pFile[iOffset], sizeof(float)); iOffset += 4;
			memcpy(&psurfaces[j].texinfo->vecs, &pFile[iOffset], sizeof(float)*8); iOffset += sizeof(float)*8;

			memcpy(&psurfaces[j].texturemins, &pFile[iOffset], sizeof(short)*2); iOffset += 4;
			memcpy(&psurfaces[j].extents, &pFile[iOffset], sizeof(short)*2); iOffset += 4;
			memcpy(&psurfaces[j].light_s, &pFile[iOffset], sizeof(int)); iOffset += 4;
			memcpy(&psurfaces[j].light_t, &pFile[iOffset], sizeof(int)); iOffset += 4;
			memcpy(&psurfaces[j].flags, &pFile[iOffset], sizeof(int)); iOffset += 4;

			int iXSize = (psurfaces[j].extents[0]>>4)+1;
			int iYSize = (psurfaces[j].extents[1]>>4)+1;
			int iSize = iXSize*iYSize;

			// Read lightmap
			psurfaces[j].samples = new color24[iSize];
			memset(psurfaces[j].samples, 0, sizeof(color24)*iSize);
			memcpy(psurfaces[j].samples, &pFile[iOffset], sizeof(color24)*iSize); iOffset += sizeof(color24)*iSize;
			
			// Read vertexes in
			memcpy(psurfaces[j].polys->verts, &pFile[iOffset], VERTEXSIZE*sizeof(float)*psurfaces[j].polys->numverts);
			iOffset += VERTEXSIZE*4*psurfaces[j].polys->numverts;
		}
	}
	gEngfuncs.COM_FreeFile(pFile);

	m_pDetailObjectSurfaces = new msurface_t[surfacesArray.size()];
	memcpy(m_pDetailObjectSurfaces, surfacesArray.data(), sizeof(msurface_t)*surfacesArray.size());

	//
	// Get leaf numbers
	//
	for(int i = 0; i < m_iNumDetailObjects; i++)
	{
		entextradata_t pTemp;
		memset(&pTemp, 0, sizeof(entextradata_t));

		pTemp.absmin = m_pDetailObjects[i].mins;
		pTemp.absmax = m_pDetailObjects[i].maxs;

		SV_FindTouchedLeafs(&pTemp, m_pWorld->nodes);

		memcpy(m_pDetailObjects[i].leafnums, pTemp.leafnums, sizeof(short)*MAX_ENT_LEAFS);
		m_pDetailObjects[i].numleafs = pTemp.num_leafs;
	}
}

/*
====================
CreateTextures

====================
*/
void CBSPRenderer::CreateTextures( void )
{
	//
	// Load flashlight texture
	//
	m_pFlashlightTextures[0] = gTextureLoader.LoadTexture("gfx/textures/flashlight.tga", FALSE, true, false, true);
	m_iNumFlashlightTextures++;

	if(!m_pFlashlightTextures[0])
	{
		gEngfuncs.pfnClientCmd("escape\n");	
		MessageBox(NULL, "ERROR: Failed to load gfx/textures/flashlight.tga.\n\nPress Ok to quit the game.\n", "ERROR", MB_OK);
		exit(-1);
	}

	// ALWAYS Bind
	glBindTexture(GL_TEXTURE_2D, m_pFlashlightTextures[0]->iIndex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	for(int i = 0; i < gTextureLoader.m_iNumTextureEntries; i++)
	{
		texentry_t *pEntry = &gTextureLoader.m_pTextureEntries[i];
		if(!strcmp(pEntry->szModel, "flashlight"))
		{
			char szPath[64];
			sprintf(szPath, "gfx/textures/%s.tga", pEntry->szTexture);
			cl_texture_t *pTexture = gTextureLoader.LoadTexture(szPath, FALSE, true, false, true);
			if(pTexture)
			{
				m_pFlashlightTextures[m_iNumFlashlightTextures] = pTexture;
				m_iNumFlashlightTextures++;

				glBindTexture(GL_TEXTURE_2D, pTexture->iIndex);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
	}

	//
	// Create spotlight attenuation texture.
	//
	color24 buf1[256];
	for (int i = 0; i < 256; i++)
	{
		float dist = (float)i;
		float att = (((dist*dist) / (256*256)) - 1) * -255;

		if (i == 255 || i == 0)
			att = 0;

		buf1[i].r = buf1[i].g = buf1[i].b = (unsigned char)att;
	}

	m_iAttenuation1DTexture = current_ext_texture_id;
	current_ext_texture_id++;

	glBindTexture(GL_TEXTURE_1D, m_iAttenuation1DTexture);
	glTexImage1D (GL_TEXTURE_1D, 0, 3, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, buf1);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	//
	// Create 3d attenuation texture for point lights
	//
	color24 *buf2 = new color24[64*64*64];
	float f = (64/2) * (64/2);

	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 64; y++)
		{
			for (int z = 0; z < 64; z++)
			{
				vec3_t vec;
				vec[0] = (float)x - (64/2);
				vec[1] = (float)y - (64/2);
				vec[2] = (float)z - (64/2);
				float dist = vec.Length();
				if (dist > (64/2))
					dist = (64/2);

				float att;
				if (x==0 || y == 0 || z==0 || x==64-1 || y==64-1 || z==64-1)
					att = 0;
				else
					att = (((dist*dist) / f) -1 ) * -255;

				buf2[x*64*64 + y*64 + z].r = (unsigned char)(att);
				buf2[x*64*64 + y*64 + z].g = (unsigned char)(att);
				buf2[x*64*64 + y*64 + z].b = (unsigned char)(att);
			}
		}
	}

	m_iAtten3DPoint = current_ext_texture_id; current_ext_texture_id++;
	glBindTexture(GL_TEXTURE_3D, m_iAtten3DPoint);
	glTexImage3DEXT(GL_TEXTURE_3D, 0, 3, 64, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, buf2);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	delete[] buf2;

	//
	// Create color dummy texture
	//
	color24 buf3[16*16];

	for (int i = 0; i < 256; i++)
	{
		buf3[i].r = 127;
		buf3[i].g = 127;
		buf3[i].b = 255;
	}

	m_iLightDummy = current_ext_texture_id; current_ext_texture_id++;
	glBindTexture(GL_TEXTURE_2D, m_iLightDummy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D (GL_TEXTURE_2D, 0, 3, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, buf3);

	//
	// Copy over all world textures
	//
	for(int i = 0; i < m_pWorld->numtextures; i++)
	{
		if(!m_pWorld->textures[i])
			continue;

		memcpy(&m_pNormalTextureList[m_iNumTextures], m_pWorld->textures[i], sizeof(texture_t));
		memcpy(&m_pMultiPassTextureList[m_iNumTextures], m_pWorld->textures[i], sizeof(texture_t));
		m_iNumTextures++;
	}

	// Relink animations
	for(int i = 0; i < m_iNumTextures; i++)
	{
		if(m_pNormalTextureList[i].anim_next)
		{
			for(int j = 0; j < m_iNumTextures; j++)
			{
				if(!strcmp(m_pNormalTextureList[j].name, m_pNormalTextureList[i].anim_next->name))
				{
					m_pNormalTextureList[i].anim_next = &m_pNormalTextureList[j];
					break;
				}
			}
		}
	}
}

/*
====================
FreeBuffer

====================
*/
void CBSPRenderer::FreeBuffer( void )
{
	if (m_uiBufferIndex)
	{
		glDeleteBuffersARB(1, &m_uiBufferIndex);
		m_uiBufferIndex = NULL;
	}

	if (m_pBufferData)
	{
		delete[] m_pBufferData;
		m_pBufferData = NULL;
	}

	if (m_pFacesExtraData)
	{
		delete[] m_pFacesExtraData;
		m_pFacesExtraData = NULL;
	}
}


/*
====================
GenerateVertexArray

====================
*/
void CBSPRenderer::GenerateVertexArray( void )
{
	int iNumFaces = 0;
	int iCurFace = 0;

	int iNumVerts = gPropManager.m_iNumTotalVerts;
	int iCurVert = gPropManager.m_iNumTotalVerts;

	// delete existing data
	FreeBuffer();

	msurface_t *surfaces = m_pWorld->surfaces;
	for(int i = 0; i < m_pWorld->numsurfaces; i++)
	{
		if(surfaces[i].flags & SURF_DRAWSKY)
			continue;

		if(!(surfaces[i].flags & SURF_DRAWTURB) && surfaces[i].polys->next)
			continue; // lets be sure

		for (glpoly_t *bp = surfaces[i].polys; bp; bp = bp->next)
			iNumVerts += 3 + (bp->numverts-3)*3;

		iNumFaces++;
	}

	if(m_iNumDetailObjects)
	{
		for(int i = 0; i < m_iNumDetailObjects; i++)
		{
			msurface_t *psurf = m_pDetailObjectSurfaces + m_pDetailObjects[i].firstsurface;
			for(int j = 0; j < m_pDetailObjects[i].numsurfaces; j++, psurf++)
			{
				iNumVerts += 3 + (psurf->polys->numverts-3)*3;
				iNumFaces++;
			}
		}
	}

	int nbPointers = m_pWorld->numsurfaces;
	if(m_iNumDetailObjects)
		nbPointers += m_numDetailSurfaces;

	m_pSurfacePointersArray = new brushface_t*[nbPointers];
	for(int i = 0; i < iNumFaces; i++)
		m_pSurfacePointersArray[i] = nullptr;

	// Set array up
	m_pBufferData = new brushvertex_t[iNumVerts];
	memset(m_pBufferData, 0, sizeof(brushvertex_t)*iNumVerts);
	m_iTotalVertCount = iNumVerts;
	m_iTotalTriCount = iNumVerts/3;

	//Copy over prop manager data
	memcpy(m_pBufferData,gPropManager.m_pVertexData, sizeof(brushvertex_t)*gPropManager.m_iNumTotalVerts);

	m_pFacesExtraData = new brushface_t[iNumFaces];
	memset(m_pFacesExtraData, 0, sizeof(brushface_t)*iNumFaces);
	m_iTotalFaceCount = iNumFaces;

	int pointerIndex = 0;
	for(int i = 0; i < m_pWorld->numsurfaces; i++, pointerIndex++)
	{
		if (surfaces[i].flags & SURF_DRAWSKY)
			continue;

		glpoly_t *poly = surfaces[i].polys;

		if(poly->numverts < 3)
			continue;

		if(!(surfaces[i].flags & SURF_DRAWTURB) && poly->next)
			continue; // lets be sure

		brushface_t* ext = &m_pFacesExtraData[iCurFace];
		VectorCopy(surfaces[i].texinfo->vecs[0], ext->s_tangent);
		VectorCopy(surfaces[i].texinfo->vecs[1], ext->t_tangent);
		VectorNormalize(ext->s_tangent);
		VectorNormalize(ext->t_tangent);
		VectorCopy(surfaces[i].plane->normal, ext->normal);
		ext->index = i;

		// Set ptr
		m_pSurfacePointersArray[pointerIndex] = ext;

		if (surfaces[i].flags & SURF_PLANEBACK)
			VectorInverse(ext->normal);

		// Link up with renderlist textures
		for(int j = 0; j < m_iNumTextures; j++)
		{
			if(!strcmp(m_pNormalTextureList[j].name, surfaces[i].texinfo->texture->name))
			{
				m_pSurfaces[ext->index].regtexture = &m_pNormalTextureList[j];
				m_pSurfaces[ext->index].mptexture = &m_pMultiPassTextureList[j];
				break;
			}
		}

		detailtexentry_t *dtex = NULL;
		if(m_pSurfaces[ext->index].regtexture)
		{
			if(m_pSurfaces[ext->index].regtexture->offsets[3])
				dtex = &m_pDetailTextures[m_pSurfaces[ext->index].regtexture->offsets[2]];
		}

		float column = surfaces[i].lightmaptexturenum%LIGHTMAP_NUMROWS;
		float row = (surfaces[i].lightmaptexturenum/LIGHTMAP_NUMROWS)%LIGHTMAP_NUMCOLUMNS;

		ext->start_vertex = iCurVert;
		for (glpoly_t *bp = surfaces[i].polys; bp; bp = bp->next)
		{
			brushvertex_t pVertexes[3];
			float *v = bp->verts[0];

			for(int j = 0; j < 3; j++, v += VERTEXSIZE)
			{
				pVertexes[j].pos[0] = v[0];
				pVertexes[j].pos[1] = v[1];
				pVertexes[j].pos[2] = v[2];
				pVertexes[j].texcoord[0] = v[3];
				pVertexes[j].texcoord[1] = v[4];
				pVertexes[j].lightmaptexcoord[0] = (v[5]+column)/LIGHTMAP_NUMCOLUMNS;
				pVertexes[j].lightmaptexcoord[1] = (v[6]+row)/LIGHTMAP_NUMROWS;
				pVertexes[j].normal[0] = ext->normal[0];
				pVertexes[j].normal[1] = ext->normal[1];
				pVertexes[j].normal[2] = ext->normal[2];
				
				if(dtex)
				{
					pVertexes[j].detailtexcoord[0] = v[3]*dtex->xscale;
					pVertexes[j].detailtexcoord[1] = v[4]*dtex->yscale;
				}
				
				if(m_bSpecialFog)
				{
					if(pVertexes[j].pos.z < 0)
					{
						pVertexes[j].fogcoord = abs(pVertexes[j].pos.z/(m_pWorld->mins.z+300));

						if(pVertexes[j].fogcoord > 1)
							pVertexes[j].fogcoord = 1;

						if(pVertexes[j].fogcoord < 0)
							pVertexes[j].fogcoord = 0;
					}
					else
					{
						pVertexes[j].fogcoord = abs(pVertexes[j].pos.z/(m_pWorld->maxs.z-300));
						
						if(pVertexes[j].fogcoord > 1)
							pVertexes[j].fogcoord = 1;

						if(pVertexes[j].fogcoord < 0)
							pVertexes[j].fogcoord = 0;
					}

					pVertexes[j].fogcoord *= gHUD.m_pFogSettings.end;
				}
			}

			memcpy(&m_pBufferData[iCurVert], &pVertexes[0], sizeof(brushvertex_t)); iCurVert++;
			memcpy(&m_pBufferData[iCurVert], &pVertexes[1], sizeof(brushvertex_t)); iCurVert++;
			memcpy(&m_pBufferData[iCurVert], &pVertexes[2], sizeof(brushvertex_t)); iCurVert++;

			for(int j = 0; j < (bp->numverts-3); j++, v += VERTEXSIZE)
			{
				memcpy(&pVertexes[1], &pVertexes[2], sizeof(brushvertex_t));
				
				pVertexes[2].pos[0] = v[0];
				pVertexes[2].pos[1] = v[1];
				pVertexes[2].pos[2] = v[2];
				pVertexes[2].texcoord[0] = v[3];
				pVertexes[2].texcoord[1] = v[4];
				pVertexes[2].lightmaptexcoord[0] = (v[5]+column)/LIGHTMAP_NUMCOLUMNS;
				pVertexes[2].lightmaptexcoord[1] = (v[6]+row)/LIGHTMAP_NUMROWS;
				pVertexes[2].normal[0] = ext->normal[0];
				pVertexes[2].normal[1] = ext->normal[1];
				pVertexes[2].normal[2] = ext->normal[2];

				if(dtex)
				{
					pVertexes[2].detailtexcoord[0] = v[3]*dtex->xscale;
					pVertexes[2].detailtexcoord[1] = v[4]*dtex->yscale;
				}

				if(m_bSpecialFog)
				{
					if(pVertexes[2].pos.z < 0)
					{
						pVertexes[2].fogcoord = abs(pVertexes[2].pos.z/(m_pWorld->mins.z+300));
						
						if(pVertexes[2].fogcoord > 1)
							pVertexes[2].fogcoord = 1;

						if(pVertexes[2].fogcoord < 0)
							pVertexes[2].fogcoord = 0;
					}
					else
					{
						pVertexes[2].fogcoord = abs(pVertexes[2].pos.z/(m_pWorld->maxs.z-300));
						
						if(pVertexes[2].fogcoord > 1)
							pVertexes[2].fogcoord = 1;

						if(pVertexes[2].fogcoord < 0)
							pVertexes[2].fogcoord = 0;
					}

					pVertexes[2].fogcoord *= gHUD.m_pFogSettings.end;
				}

				memcpy(&m_pBufferData[iCurVert], &pVertexes[0], sizeof(brushvertex_t)); iCurVert++;
				memcpy(&m_pBufferData[iCurVert], &pVertexes[1], sizeof(brushvertex_t)); iCurVert++;
				memcpy(&m_pBufferData[iCurVert], &pVertexes[2], sizeof(brushvertex_t)); iCurVert++;
			}
		}
		ext->num_vertexes = iCurVert - ext->start_vertex;
		iCurFace++;
	}

	if(m_iNumDetailObjects)
	{
		int iCounter = 0;
		for(int i = 0; i < m_iNumDetailObjects; i++, pointerIndex++)
		{
			detailobject_t *pObject = &m_pDetailObjects[i];
			for(int j = 0; j < pObject->numsurfaces; j++)
			{
				msurface_t *psurf = m_pDetailObjectSurfaces + pObject->firstsurface;
				glpoly_t *poly = psurf->polys;

				if(poly->numverts < 3)
					continue;

				brushface_t* ext = &m_pFacesExtraData[iCurFace];
				VectorCopy(psurf->texinfo->vecs[0], ext->s_tangent);
				VectorCopy(psurf->texinfo->vecs[1], ext->t_tangent);
				VectorNormalize(ext->s_tangent);
				VectorNormalize(ext->t_tangent);
				VectorCopy(psurf->plane->normal, ext->normal);
				ext->index = m_pWorld->numsurfaces+iCounter; iCounter++;

				// Set ptr
				m_pSurfacePointersArray[pointerIndex] = ext;

				if (psurf->flags & SURF_PLANEBACK)
					VectorInverse(ext->normal);

				// Link up with renderlist textures
				for(int k = 0; k < m_iNumTextures; k++)
				{
					if(!strcmp(m_pNormalTextureList[k].name, psurf->texinfo->texture->name))
					{
						m_pSurfaces[ext->index].regtexture = &m_pNormalTextureList[k];
						m_pSurfaces[ext->index].mptexture = &m_pMultiPassTextureList[k];
						break;
					}
				}

				detailtexentry_t *dtex = NULL;
				if(m_pSurfaces[ext->index].regtexture)
				{
					if(m_pSurfaces[ext->index].regtexture->offsets[3])
						dtex = &m_pDetailTextures[m_pSurfaces[ext->index].regtexture->offsets[2]];
				}

				float column = psurf->lightmaptexturenum%LIGHTMAP_NUMROWS;
				float row = (psurf->lightmaptexturenum/LIGHTMAP_NUMROWS)%LIGHTMAP_NUMCOLUMNS;

				ext->start_vertex = iCurVert;
				for (glpoly_t *bp = psurf->polys; bp; bp = bp->next)
				{
					brushvertex_t pVertexes[3];
					float *v = bp->verts[0];

					for(int k = 0; k < 3; k++, v += VERTEXSIZE)
					{
						pVertexes[k].pos[0] = v[0];
						pVertexes[k].pos[1] = v[1];
						pVertexes[k].pos[2] = v[2];
						pVertexes[k].texcoord[0] = v[3];
						pVertexes[k].texcoord[1] = v[4];
						pVertexes[k].lightmaptexcoord[0] = (v[5]+column)/LIGHTMAP_NUMCOLUMNS;
						pVertexes[k].lightmaptexcoord[1] = (v[6]+row)/LIGHTMAP_NUMROWS;
						pVertexes[k].normal[0] = ext->normal[0];
						pVertexes[k].normal[1] = ext->normal[1];
						pVertexes[k].normal[2] = ext->normal[2];

						if(dtex)
						{
							pVertexes[k].detailtexcoord[0] = v[3]*dtex->xscale;
							pVertexes[k].detailtexcoord[1] = v[4]*dtex->yscale;
						}

						if(m_bSpecialFog)
						{
							if(pVertexes[k].pos.z < 0)
							{
								pVertexes[k].fogcoord = abs(pVertexes[k].pos.z/(m_pWorld->mins.z+300));
								
								if(pVertexes[k].fogcoord > 1)
									pVertexes[k].fogcoord = 1;

								if(pVertexes[k].fogcoord < 0)
									pVertexes[k].fogcoord = 0;
							}
							else
							{
								pVertexes[k].fogcoord = abs(pVertexes[k].pos.z/(m_pWorld->maxs.z-300));
								
								if(pVertexes[k].fogcoord > 1)
									pVertexes[k].fogcoord = 1;

								if(pVertexes[k].fogcoord < 0)
									pVertexes[k].fogcoord = 0;
							}

							pVertexes[k].fogcoord *= gHUD.m_pFogSettings.end;
						}
					}

					memcpy(&m_pBufferData[iCurVert], &pVertexes[0], sizeof(brushvertex_t)); iCurVert++;
					memcpy(&m_pBufferData[iCurVert], &pVertexes[1], sizeof(brushvertex_t)); iCurVert++;
					memcpy(&m_pBufferData[iCurVert], &pVertexes[2], sizeof(brushvertex_t)); iCurVert++;

					for(int k = 0; k < (bp->numverts-3); k++, v += VERTEXSIZE)
					{
						memcpy(&pVertexes[1], &pVertexes[2], sizeof(brushvertex_t));
						
						pVertexes[2].pos[0] = v[0];
						pVertexes[2].pos[1] = v[1];
						pVertexes[2].pos[2] = v[2];
						pVertexes[2].texcoord[0] = v[3];
						pVertexes[2].texcoord[1] = v[4];
						pVertexes[2].lightmaptexcoord[0] = (v[5]+column)/LIGHTMAP_NUMCOLUMNS;
						pVertexes[2].lightmaptexcoord[1] = (v[6]+row)/LIGHTMAP_NUMROWS;
						pVertexes[2].normal[0] = ext->normal[0];
						pVertexes[2].normal[1] = ext->normal[1];
						pVertexes[2].normal[2] = ext->normal[2];

						if(dtex)
						{
							pVertexes[2].detailtexcoord[0] = v[3]*dtex->xscale;
							pVertexes[2].detailtexcoord[1] = v[4]*dtex->yscale;
						}

						if(m_bSpecialFog)
						{
							if(pVertexes[2].pos.z < 0)
							{
								pVertexes[2].fogcoord = abs(pVertexes[2].pos.z/(m_pWorld->mins.z+300));
								
								if(pVertexes[2].fogcoord > 1)
									pVertexes[2].fogcoord = 1;

								if(pVertexes[2].fogcoord < 0)
									pVertexes[2].fogcoord = 0;
							}
							else
							{
								pVertexes[2].fogcoord = abs(pVertexes[2].pos.z/(m_pWorld->maxs.z-300));
								
								if(pVertexes[2].fogcoord > 1)
									pVertexes[2].fogcoord = 1;

								if(pVertexes[2].fogcoord < 0)
									pVertexes[2].fogcoord = 0;
							}

							pVertexes[2].fogcoord *= gHUD.m_pFogSettings.end;
						}

						memcpy(&m_pBufferData[iCurVert], &pVertexes[0], sizeof(brushvertex_t)); iCurVert++;
						memcpy(&m_pBufferData[iCurVert], &pVertexes[1], sizeof(brushvertex_t)); iCurVert++;
						memcpy(&m_pBufferData[iCurVert], &pVertexes[2], sizeof(brushvertex_t)); iCurVert++;
					}
				}
				ext->num_vertexes = iCurVert - ext->start_vertex;
				iCurFace++;
			}
		}
	}

	glGenBuffersARB( 1, &m_uiBufferIndex );
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_uiBufferIndex );
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(brushvertex_t)*iNumVerts, m_pBufferData, GL_STATIC_DRAW_ARB );
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, NULL );
};

/*
====================
EnableVertexArray

====================
*/
void CBSPRenderer::EnableVertexArray( void )
{
	glEnableClientState(GL_VERTEX_ARRAY);

	if(m_bSpecialFog)
	{
		glEnableClientState(GL_FOG_COORD_ARRAY);
		glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
	}
};

/*
====================
DisableVertexArray

====================
*/
void CBSPRenderer::DisableVertexArray( void )
{
	glDisableClientState(GL_VERTEX_ARRAY);
	if(m_bSpecialFog)
	{
		glDisableClientState(GL_FOG_COORD_ARRAY);
		glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FRAGMENT_DEPTH_EXT);
	}

	SetTexPointer(0, TC_OFF);
	SetTexPointer(1, TC_OFF);
	SetTexPointer(2, TC_OFF);
	SetTexPointer(3, TC_OFF);

	// 2010-09-30 -- Not setting this caused problems with Steam HL
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
};

/*
====================
SetTexPointer

====================
*/
void CBSPRenderer::SetTexPointer(int unitnum, int tc)
{
	if (unitnum >= m_iTUSupport)
		return;

	if (m_iTexPointer[unitnum] == tc)
		return;

	glClientActiveTextureARB(unitnum + GL_TEXTURE0_ARB);
	
	switch(tc)
	{
	case TC_OFF:
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		break;

	case TC_TEXTURE:
		glTexCoordPointer(2, GL_FLOAT, sizeof(brushvertex_t), OFFSET(brushvertex_t, texcoord));
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		break;

	case TC_DETAIL_TEXTURE:
		glTexCoordPointer(2, GL_FLOAT, sizeof(brushvertex_t), OFFSET(brushvertex_t, detailtexcoord));
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		break;

	case TC_LIGHTMAP:
		glTexCoordPointer(2, GL_FLOAT, sizeof(brushvertex_t), OFFSET(brushvertex_t, lightmaptexcoord));
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		break;

	case TC_VERTEX_POSITION:
		glTexCoordPointer(3, GL_FLOAT, sizeof(brushvertex_t), OFFSET(brushvertex_t, pos));
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		break;
	}

	m_iTexPointer[unitnum] = tc;
}

/*
====================
ResetRenderer

====================
*/
void CBSPRenderer::ResetRenderer( void )
{
	for (int i = 0; i < 3; i++)
	{
		glActiveTextureARB( GL_TEXTURE0_ARB + i );
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		glDisable(GL_TEXTURE_2D);
	}

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	glColor4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
}

/*
====================
ResetCache

====================
*/
void CBSPRenderer::ResetCache( void )
{
	for(int i = 0; i < 3; i++)
		m_uiCurrentBinds[i] = 0;

	for(int i = 0; i < 4; i++)
	{
		m_iEnvStates[i] = ENVSTATE_NOSTATE;
		m_iTexPointer[i] = TC_NOSTATE;
	}
}

/*
====================
HasDynLights

====================
*/
bool CBSPRenderer::HasDynLights( void )
{
	float time	= gEngfuncs.GetClientTime();
	cl_dlight_t *dl = m_pDynLights;

	if(m_pCurrentEntity->curstate.renderfx == 70)
		return false;

	if(IsEntityTransparent(m_pCurrentEntity))
		return false;

	if(!m_bShaderSupport || m_pCvarWorldShaders->value <= 0)
		return false;

	if(m_pCvarDynamic->value < 1)
		return false;

	for (int i = 0; i < MAX_DYNLIGHTS; i++, dl++)
	{
		if (dl->die >= time && dl->radius)
			return true;
	}

	return false;
};

/*
====================
PrepareRenderer

====================
*/
void CBSPRenderer::PrepareRenderer( void )
{
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	m_bSecondPassNeeded = HasDynLights();

	ResetCache();
	SetTexPointer(0, TC_LIGHTMAP);
	SetTexPointer(1, TC_TEXTURE);
	SetTexPointer(2, TC_DETAIL_TEXTURE);

	// clear all chains
	for(int i = 0; i < m_iNumTextures; i++)
	{
		m_pNormalTextureList[i].texturechain = NULL;
		m_pMultiPassTextureList[i].texturechain = NULL;
	}
};

/*
====================
DrawNormalTriangles

====================
*/
void CBSPRenderer::DrawNormalTriangles( void )
{
	if (!m_bCanDraw)
		return;

	 // Disable water fog, it's just black anyway
	if(glIsEnabled(GL_FOG))
		glDisable(GL_FOG);

	// Set up view leaf
	if ( m_pViewLeaf->contents != CONTENTS_SOLID )
		m_iVisFrame = m_pViewLeaf->visframe;
	else
		m_iVisFrame = -2;

	R_SaveGLStates();
	RenderFog();

	DrawSky();
	DrawWorld();
	R_RestoreGLStates();

	// So it's called only once
	m_bCanDraw = false;
};


/*
====================
DrawTransparentTriangles

====================
*/
void CBSPRenderer::DrawTransparentTriangles( void )
{
	if(!m_pCvarDrawWorld->value)
		return;

	m_pCurrentEntity = gEngfuncs.GetEntityByIndex(0);

	ResetRenderer();
	EnableVertexArray();

	if(g_StudioRenderer.m_pCvarDrawEntities->value >= 1)
	{
		for (int i = 0; i < m_iNumRenderEntities; i++)
		{
			if (IsEntityTransparent(m_pRenderEntities[i]) && m_pRenderEntities[i]->curstate.renderfx != 70)
				DrawBrushModel(m_pRenderEntities[i], false);
		}
	}

	ResetRenderer();
	DrawDecals();

	DisableVertexArray();
	ResetRenderer();
};

/*
====================
DrawWorld

====================
*/
void CBSPRenderer::DrawWorld( void )
{
	// Restore mins/maxs
	RestoreWorldDrawing();

	if(!m_pCvarDrawWorld->value)
		return;

	m_pCurrentEntity = gEngfuncs.GetEntityByIndex(0);
	VectorCopy(m_vRenderOrigin, m_vVecToEyes);

	EnableVertexArray();
	PrepareRenderer();
	RecursiveWorldNode(m_pWorld->nodes);

	// Draw all static entities
	for (int i = 0; i < m_iNumRenderEntities; i++)
	{
		if ((m_pRenderEntities[i]->curstate.renderfx != 70) && !IsEntityMoved(m_pRenderEntities[i]) && !IsEntityTransparent(m_pRenderEntities[i]))
			DrawBrushModel(m_pRenderEntities[i], true);
	}

	RenderFirstPass();
	DrawDynamicLightsForWorld();
	RenderFinalPasses();

	// Now render moved entities seperately each
	for (int i = 0; i < m_iNumRenderEntities; i++)
	{
		if (m_pRenderEntities[i]->curstate.renderfx != 70 && IsEntityMoved(m_pRenderEntities[i]) && !IsEntityTransparent(m_pRenderEntities[i]))
			DrawBrushModel(m_pRenderEntities[i], false);
	}

	if(m_iNumDetailObjects > 0)
	{
		// Draw detail objects
		PrepareRenderer();
		DrawDetails();
		RenderFirstPass(true);
		DrawDynamicLightsForDetails();
		RenderFinalPasses(true);
	}

	DisableVertexArray();
	ResetRenderer();

	// clear texture chains
	for(int i = 0; i < m_iNumTextures; i++)
	{
		m_pNormalTextureList[i].texturechain = NULL;
		m_pMultiPassTextureList[i].texturechain = NULL;
	}
};

/*
====================
DrawDetails

====================
*/
void CBSPRenderer::DrawDetails( void )
{
	if(!m_iNumDetailObjects)
		return;

	VectorCopy(m_vRenderOrigin, m_vVecToEyes);
	m_pCurrentEntity = gEngfuncs.GetEntityByIndex(0);

	detailobject_t *pCurObject = m_pDetailObjects;
	for(int i = 0; i < m_iNumDetailObjects; i++, pCurObject++)
	{
		int j = 0;
		for (j = 0; j < pCurObject->numleafs; j++)
			if (m_pPVS[pCurObject->leafnums[j] >> 3] & (1 << (pCurObject->leafnums[j]&7) ))
				break;

		if(j == pCurObject->numleafs)
			continue;

		if(gHUD.viewFrustum.CullBox(pCurObject->mins, pCurObject->maxs))
			continue;

		if(!m_bShaderSupport || m_pCvarWorldShaders->value < 1)
		{
			float time = gEngfuncs.GetClientTime();
			cl_dlight_t *dl = m_pDynLights;

			for(int j = 0; j < MAX_DYNLIGHTS; j++)
			{
				if(dl[j].die < time || !dl[j].radius)
					continue;

				m_pCurrentDynLight = &dl[j];
				m_vCurDLightOrigin = m_pCurrentDynLight->origin;

				if(m_pCurrentDynLight->cone_size)
				{
					if(m_pCurrentDynLight->frustum.CullBox((const vec3_t)pCurObject->mins, (const vec3_t)pCurObject->maxs))
						continue;
				}
				else
				{
					SetDynLightBBox();
					if(CullDynLightBBox(pCurObject->mins, pCurObject->maxs))
						continue;
				}
				
				for(int k = 0; k < pCurObject->numsurfaces; k++)
				{
					msurface_t *psurf = m_pDetailObjectSurfaces + pCurObject->firstsurface + k;
					if (psurf->dlightframe != m_iFrameCount)
					{
						psurf->dlightbits = 0;
						psurf->dlightframe = m_iFrameCount;
					}
					psurf->dlightbits |= 1<<j;
				}
			}
		}
			
		bool bDynLit = DynamicLighted(pCurObject->mins, pCurObject->maxs);
		pCurObject->visframe = m_iFrameCount; // For dynlights
		msurface_t *psurf = m_pDetailObjectSurfaces + pCurObject->firstsurface;
		for(int j = 0; j < pCurObject->numsurfaces; j++, psurf++)
		{
			float dot;
			mplane_t *pplane = psurf->plane;
			SSEDotProductSub(&dot, &m_vVecToEyes, &pplane->normal, &pplane->dist);

			if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
				(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
			{
				SurfaceToChain(m_pDetailObjectSurfaces, psurf, bDynLit);
				psurf->visframe = m_iFrameCount;
			}
		}
	}
}

/*
====================
RenderFirstPass

====================
*/
void CBSPRenderer::RenderFirstPass( bool bSecond )
{
	msurface_t* psurfbase = nullptr;
	if(bSecond)
	{
		Bind2DTexture(GL_TEXTURE0_ARB, m_iDetailLightmapIndex);
		psurfbase = m_pDetailObjectSurfaces;
	}
	else
	{
		Bind2DTexture(GL_TEXTURE0_ARB, m_iEngineLightmapIndex);
		psurfbase = m_pWorld->surfaces;
	}

	//Render normal ones first
	for(int i = 0; i < m_iNumTextures; i++)
	{
		texture_t *pTexture = TextureAnimation(&m_pNormalTextureList[i], m_pCurrentEntity->curstate.frame);
		msurface_t *psurface = m_pNormalTextureList[i].texturechain;

		// Nothing to draw
		if(!psurface)
			continue;

		if( pTexture->offsets[3] && m_pCvarDetailTextures->value >= 1 )
		{
			SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_MUL_X2, ENVSTATE_MUL_X2);
			Bind2DTexture(GL_TEXTURE1_ARB, pTexture->gl_texturenum);
			Bind2DTexture(GL_TEXTURE2_ARB, pTexture->offsets[3]);		
		}
		else
		{
			SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_MUL_X2);
			Bind2DTexture(GL_TEXTURE1_ARB, pTexture->gl_texturenum);
		}

		if(pTexture->name[0] == '{')
		{
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.5);
		}

		if((m_pCurrentEntity->curstate.effects & FL_CONVEYOR) && stristr(pTexture->name, "scroll"))
		{
			while(psurface)
			{
				DrawScrollingPoly(psurfbase, psurface);
				psurface = psurface->texturechain;
				m_iWorldPolyCounter++;
			}
		}
		else
		{
			while(psurface)
			{
				DrawPolyFromArray(psurfbase, psurface);
				psurface = psurface->texturechain;
				m_iWorldPolyCounter++;
			}
		}

		if(pTexture->name[0] == '{')
		{
			glDisable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0);
		}

		if(m_pCvarWireFrame->value >= 1)
		{
			SetTexEnvs(ENVSTATE_OFF, ENVSTATE_OFF, ENVSTATE_OFF,ENVSTATE_OFF);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(1);

			if(m_pCvarWireFrame->value >= 2)
				glDisable(GL_DEPTH_TEST);

			if(gHUD.m_pFogSettings.active)
				glDisable(GL_FOG);

			if(m_pCurrentEntity->index == 0)
				glColor4f(GL_ONE, GL_ZERO, GL_ZERO, GL_ONE);
			else
				glColor4f(GL_ONE, GL_ZERO, GL_ONE, GL_ONE);
			
			psurface = pTexture->texturechain;
			while(psurface)
			{
				DrawPolyFromArray(psurfbase, psurface);
				psurface = psurface->texturechain;
			}

			glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
			if(m_pCvarWireFrame->value >= 2)
				glEnable(GL_DEPTH_TEST);

			if(gHUD.m_pFogSettings.active)
				glEnable(GL_FOG);

			glColor4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
		}
	}

	if(gHUD.m_pFogSettings.active) 
		glDisable(GL_FOG);

	//Render lightmaps only now
	for(int i = 0; i < m_iNumTextures; i++)
	{
		texture_t *pTexture = &m_pMultiPassTextureList[i];
		msurface_t *psurface = m_pMultiPassTextureList[i].texturechain;

		// Nothing to draw
		if(!psurface)
			continue;

		if(pTexture->name[0] == '{')
		{
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.5);

			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_REPLACE);
			Bind2DTexture(GL_TEXTURE1_ARB, pTexture->gl_texturenum);
			while(psurface)
			{
				DrawPolyFromArray(psurfbase, psurface);
				psurface = psurface->texturechain;
			}

			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
			glDepthFunc(GL_EQUAL);
			SetTexEnvs(ENVSTATE_REPLACE);

			psurface = pTexture->texturechain;
			while(psurface)
			{
				DrawPolyFromArray(psurfbase, psurface);
				psurface = psurface->texturechain;
			}
			glDepthFunc(GL_LEQUAL);
			m_iWorldPolyCounter++;

			glDisable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0);
		}
		else
		{
			SetTexEnvs( ENVSTATE_REPLACE );
			while(psurface)
			{
				DrawPolyFromArray(psurfbase, psurface);
				psurface = psurface->texturechain;
				m_iWorldPolyCounter++;
			}
		}
	}

	if(gHUD.m_pFogSettings.active) 
		glEnable(GL_FOG);
}

/*
====================
RenderFinalPasses

====================
*/
void CBSPRenderer::RenderFinalPasses( bool bSecond )
{
	if(!m_bShaderSupport || m_pCvarWorldShaders->value <= 0)
		return;

	if(!m_bSecondPassNeeded)
		return;

	if(m_pCvarDynamic->value == 2)
		return;

	if(m_pCvarDynamic->value < 1)
		return;

	if(gHUD.m_pFogSettings.active)
		glDisable(GL_FOG);

	msurface_t* psurfbase = nullptr;
	if(bSecond)
		psurfbase = m_pDetailObjectSurfaces;
	else
		psurfbase = m_pWorld->surfaces;

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

	SetTexPointer(0, TC_TEXTURE);
	SetTexPointer(1, TC_DETAIL_TEXTURE);

	for(int i = 0; i < m_iNumTextures; i++)
	{
		texture_t *pTexture = TextureAnimation(&m_pNormalTextureList[i], m_pCurrentEntity->curstate.frame);
		msurface_t *psurface = m_pMultiPassTextureList[i].texturechain;

		if(!psurface)
			continue;
			
		if (pTexture->offsets[3] && m_pCvarDetailTextures->value)
		{
			// has detail texture
			SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_MUL_X2);
			Bind2DTexture( GL_TEXTURE0_ARB, pTexture->gl_texturenum );
			Bind2DTexture( GL_TEXTURE1_ARB, pTexture->offsets[3] );
		}
		else
		{
			SetTexEnvs(ENVSTATE_REPLACE);
			Bind2DTexture( GL_TEXTURE0_ARB, pTexture->gl_texturenum );
		}

		if((m_pCurrentEntity->curstate.effects & FL_CONVEYOR) && stristr(pTexture->name, "scroll"))
		{
			while(psurface)
			{
				DrawScrollingPoly(psurfbase, psurface);
				psurface = psurface->texturechain;
			}
		}
		else
		{
			while(psurface)
			{
				DrawPolyFromArray(psurfbase, psurface);
				psurface = psurface->texturechain;
			}
		}
	}

	if(gHUD.m_pFogSettings.active)
	{
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_iFogFragmentID);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (int i = 0; i < m_iNumTextures; i++)
		{
			msurface_t *s = m_pMultiPassTextureList[i].texturechain;
			while(s)
			{
				DrawPolyFromArray(psurfbase, s);
				msurface_t *next = s->texturechain;
				s = next;
			}
		}

		glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}

	SetTexEnvs(ENVSTATE_OFF, ENVSTATE_OFF, ENVSTATE_OFF, ENVSTATE_OFF);

	SetTexPointer(0, TC_OFF);
	SetTexPointer(1, TC_OFF);
	SetTexPointer(2, TC_OFF);
	SetTexPointer(3, TC_OFF);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	if(m_pCvarWireFrame->value >= 1)
	{
		glColor4f(1, 1, 1, 1);
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		glColor4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
		glLineWidth(1);

		if(m_pCvarWireFrame->value >= 2)
			glDisable(GL_DEPTH_TEST);

		for (int i = 0; i < m_iNumTextures; i++)
		{
			msurface_t *s = m_pMultiPassTextureList[i].texturechain;
			while(s)
			{
				DrawPolyFromArray(psurfbase, s);
				msurface_t *next = s->texturechain;
				s = next;
			}
		}
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	}

	if(gHUD.m_pFogSettings.active)
		glEnable(GL_FOG);
}

/*
====================
RecursiveWorldNode

====================
*/
void CBSPRenderer::RecursiveWorldNode(mnode_t *node)
{
	int			c, side;
	mplane_t	*plane;
	msurface_t	*surf, **mark;
	mleaf_t		*pleaf;
	float		dot;

	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	if (node->visframe != m_iVisFrame && m_iVisFrame != -2)
		return;

	if (gHUD.viewFrustum.CullBox(node->minmaxs, node->minmaxs+3))
		return;

// if a leaf node, draw stuff
	if (node->contents < 0)
	{
		pleaf = (mleaf_t *)node;
		mark = pleaf->firstmarksurface;
		c = pleaf->nummarksurfaces;

		if (c)
		{
			do
			{
				(*mark)->visframe = m_iFrameCount;
				mark++;
			} while (--c);
		}
		return;
	}

// node is just a decision point, so go down the apropriate sides
// find which side of the node we are on
	plane = node->plane;
	switch (plane->type)
	{
	case PLANE_X:
		dot = m_vRenderOrigin[0] - plane->dist;	break;
	case PLANE_Y:
		dot = m_vRenderOrigin[1] - plane->dist;	break;
	case PLANE_Z:
		dot = m_vRenderOrigin[2] - plane->dist;	break;
	default:
		SSEDotProductSub(&dot, &m_vRenderOrigin, &plane->normal, &plane->dist);
		break;
	}

	if (dot >= 0) 
		side = 0;
	else 
		side = 1;

// recurse down the children, front side first
	RecursiveWorldNode (node->children[side]);

// draw stuff
	c = node->numsurfaces;
	if (c)
	{
		bool bIsLit = DynamicLighted(node->minmaxs, node->minmaxs+3);
		surf = m_pWorld->surfaces + node->firstsurface;

		if (dot < 0 -BACKFACE_EPSILON)
			side = SURF_PLANEBACK;
		else if (dot > BACKFACE_EPSILON)
			side = 0;

		for(int i = 0; i < node->numsurfaces; i++, surf++)
		{
			if (surf->visframe != m_iFrameCount)
				continue;

			// don't backface underwater surfaces, because they warp
			if ( !(surf->flags & SURF_UNDERWATER) && ( (dot < 0) ^ !!(surf->flags & SURF_PLANEBACK)) )
				continue;// wrong side

			if ( surf->flags & SURF_DRAWSKY )
				continue;

			if ( surf->flags & SURF_DRAWTURB )
				EmitWaterPolys(surf);
			else
				SurfaceToChain(m_pWorld->surfaces, surf, bIsLit);

			m_iWorldPolyCounter++;
		}
	}

// recurse down the back side
	RecursiveWorldNode (node->children[!side]);
};

/*
====================
MarkBrushFaces

====================
*/
void CBSPRenderer::MarkBrushFaces ( vec3_t mins, vec3_t maxs )
{
	if(m_bShaderSupport && m_pCvarWorldShaders->value >= 1)
		return;

	if(m_pCvarDynamic->value < 1)
		return;

	cl_dlight_t *dl = m_pDynLights;
	for(int i = 0; i < MAX_DYNLIGHTS; i++, dl++)
	{
		if(dl->die <= gEngfuncs.GetClientTime() || !dl->radius)
			continue;

		m_pCurrentDynLight = dl;
		VectorCopy(dl->origin, m_vCurDLightOrigin);
		SetDynLightBBox();

		if(CullDynLightBBox(mins, maxs))
			continue;

		model_t *pmodel = m_pCurrentEntity->model;
		msurface_t *psurf = &pmodel->surfaces[pmodel->firstmodelsurface];
		for(int j = 0; j < pmodel->nummodelsurfaces; j++, psurf++)
		{
			if (psurf->dlightframe != m_iFrameCount)
			{
				psurf->dlightbits = 0;
				psurf->dlightframe = m_iFrameCount;
			}
			psurf->dlightbits |= 1<<i;
		}
	}
}


/*
====================
DrawBrushModel

====================
*/
void CBSPRenderer::DrawBrushModel ( cl_entity_t *pEntity, bool bStatic )
{
	vec3_t		mins, maxs;
	int			i;
	msurface_t	*psurf;
	float		dot;
	mplane_t	*pplane;
	vec3_t		trans;
	vec3_t		rorigin;
	bool		bRotated = false;

	m_pCurrentEntity = pEntity;
	model_t *pModel = m_pCurrentEntity->model;

	if( m_pCurrentEntity->model == m_pWorld || m_pCurrentEntity->model->type != mod_brush )
		return;

	if( m_pCurrentEntity->curstate.rendermode != NULL && m_pCurrentEntity->curstate.renderamt == NULL)
		return;

	if (m_pCurrentEntity->angles[0] || m_pCurrentEntity->angles[1] || m_pCurrentEntity->angles[2])
	{
		bRotated = true;
		for (i=0 ; i<3 ; i++)
		{
			mins[i] = m_pCurrentEntity->origin[i] - pModel->radius;
			maxs[i] = m_pCurrentEntity->origin[i] + pModel->radius;
		}
	}
	else
	{
		VectorAdd (m_pCurrentEntity->origin, pModel->mins, mins);
		VectorAdd (m_pCurrentEntity->origin, pModel->maxs, maxs);
	}

	if (m_pCurrentEntity->curstate.renderfx == 70)
	{	
		if(!m_fSkySpeed)
		{
			trans = (m_pCurrentEntity->origin - m_vSkyOrigin)+m_vRenderOrigin;
			VectorSubtract (m_vSkyOrigin, m_pCurrentEntity->origin, m_vVecToEyes);	
		}
		else
		{
			trans = (m_pCurrentEntity->origin - m_vSkyOrigin)+m_vRenderOrigin;
			trans = trans-(m_vRenderOrigin-m_vSkyWorldOrigin)/m_fSkySpeed;
			vec3_t vSkyOrigin = m_vSkyOrigin+(m_vRenderOrigin-m_vSkyWorldOrigin)/m_fSkySpeed;
			VectorSubtract (vSkyOrigin, m_pCurrentEntity->origin, m_vVecToEyes);	
		}
	}
	else
	{
		if (gHUD.viewFrustum.CullBox (mins, maxs))
			return;

		VectorSubtract (m_vRenderOrigin, m_pCurrentEntity->origin, m_vVecToEyes);
	}

	if (bRotated)
	{
		vec3_t	temp;
		vec3_t	forward, right, up;

		VectorCopy (m_vVecToEyes, temp);
		AngleVectors (m_pCurrentEntity->angles, forward, right, up);
		DotProductSSE(&m_vVecToEyes[0], temp, forward);
		DotProductSSE(&m_vVecToEyes[1], temp, right); 
		DotProductSSE(&m_vVecToEyes[2], temp, up);
		m_vVecToEyes[1] = -m_vVecToEyes[1];
	}

	if(m_pCurrentEntity->curstate.renderfx == 70)
	{
		VectorCopy(m_pCurrentEntity->origin, rorigin);
		VectorCopy(trans, m_pCurrentEntity->origin);
	}

	if(!bStatic)
	{
		glPushMatrix();
		m_pCurrentEntity->angles[0] = -m_pCurrentEntity->angles[0];
		R_RotateForEntity (m_pCurrentEntity);
		m_pCurrentEntity->angles[0] = -m_pCurrentEntity->angles[0];

		if(m_pCurrentEntity->curstate.renderfx == 70)
			VectorCopy(rorigin, m_pCurrentEntity->origin);

		//
		// draw polys
		//
		PrepareRenderer();
	}

	// Mark any possible lit surfaces
	MarkBrushFaces(mins, maxs);

	if ( m_pCurrentEntity->curstate.rendermode == kRenderTransAdd )
	{
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		float alpha = 1.0 - (float)m_pCurrentEntity->curstate.renderamt/255.0;
		glColor4f(1, 1, 1, alpha);
	}
	else if (m_pCurrentEntity->curstate.rendermode == kRenderTransTexture )
	{
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		float alpha = (float)m_pCurrentEntity->curstate.renderamt/255.0;
		glColor4f(1, 1, 1, alpha);
	}
	else if (m_pCurrentEntity->curstate.rendermode == kRenderTransColor)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		float alpha = (float)m_pCurrentEntity->curstate.renderamt/255.0;
		float r = (float)m_pCurrentEntity->curstate.rendercolor.r/255.0;
		float g = (float)m_pCurrentEntity->curstate.rendercolor.g/255.0;
		float b = (float)m_pCurrentEntity->curstate.rendercolor.b/255.0;
		glColor4f(r, g, b, alpha);
	}

	bool bIsLit = DynamicLighted(mins, maxs);
	psurf = &m_pWorld->surfaces[pModel->firstmodelsurface];
	for (i = 0; i < pModel->nummodelsurfaces; i++, psurf++)
	{
		pplane = psurf->plane;
		SSEDotProductSub(&dot, &m_vVecToEyes, &pplane->normal, &pplane->dist);
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) 
			|| (!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
		{
			psurf->visframe = m_iFrameCount;

			if ( psurf->flags & SURF_DRAWSKY )
				continue;

			if (psurf->flags & SURF_DRAWTURB) EmitWaterPolys(psurf);
			else SurfaceToChain(m_pWorld->surfaces, psurf, bIsLit);
		}
	}

	//
	// Render every pass
	//
	if(!bStatic)
		RenderFirstPass();

	if (m_pCurrentEntity->curstate.rendermode == kRenderTransTexture 
		|| m_pCurrentEntity->curstate.rendermode == kRenderTransAdd 
		|| m_pCurrentEntity->curstate.rendermode == kRenderTransColor)
	{
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	if(!bStatic)
	{
		DrawDynamicLightsForEntity(m_pCurrentEntity);
		RenderFinalPasses();
		glPopMatrix();
	}

	m_pCurrentEntity->visframe = m_iFrameCount;
}

/*
====================
DrawPolyFromArray

====================
*/
void CBSPRenderer::DrawPolyFromArray( msurface_t* psurfbase, msurface_t* psurf )
{
	int surfaceIndex = psurf - psurfbase;
	brushface_t *pbrushface = m_pSurfacePointersArray[surfaceIndex];

	glDrawArrays(GL_TRIANGLES, pbrushface->start_vertex, pbrushface->num_vertexes );
}

/*
====================
SurfaceToChain

====================
*/
void CBSPRenderer::SurfaceToChain(msurface_t* psurfbase, msurface_t *s, bool dynlit)
{
	int surfaceIndex = s - psurfbase;
	brushface_t *pbrushface = m_pSurfacePointersArray[surfaceIndex];
	clientsurfdata_t *pclsurf = &m_pSurfaces[pbrushface->index];

	for (int i = 1; i < MAXLIGHTMAPS && s->styles[i] != 255 || s->dlightframe; i++)
	{
		
		if (m_iLightStyleValue[s->styles[i]] != pclsurf->cached_light[i] || s->dlightframe)
		{
			BuildLightmap(s, surfaceIndex, m_pEngineLightmaps);

			int smax = (s->extents[0]>>4)+1;
			int tmax = (s->extents[1]>>4)+1;

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			Bind2DTexture(GL_TEXTURE0_ARB, m_iEngineLightmapIndex);
			glTexSubImage2D(GL_TEXTURE_2D, 0, pclsurf->light_s, pclsurf->light_t, smax, tmax, GL_RGB, GL_UNSIGNED_BYTE, m_pBlockLights);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			break;
		}
	}
	
	if(dynlit && pclsurf->mptexture)
	{
		s->texturechain = pclsurf->mptexture->texturechain;
		pclsurf->mptexture->texturechain = s;
	}
	else if(pclsurf->regtexture)
	{
		s->texturechain = pclsurf->regtexture->texturechain;
		pclsurf->regtexture->texturechain = s;
	}
};

/*
====================
DrawScrollingPoly

====================
*/
void CBSPRenderer::DrawScrollingPoly(msurface_t* psurfbase, msurface_t *s)
{
	glpoly_t *p = s->polys;
	float *v = p->verts[0];

	float speed = m_pCurrentEntity->curstate.rendercolor.b;
	speed += (m_pCurrentEntity->curstate.rendercolor.g<<8);
	speed *= (gEngfuncs.GetClientTime()*0.0625*0.007325);

	if(m_pCurrentEntity->curstate.rendercolor.r == 0)
		speed *= -1;

	int surfaceIndex = s - psurfbase;
	brushface_t *pbrushface = m_pSurfacePointersArray[surfaceIndex];
	brushvertex_t *pVert = &m_pBufferData[pbrushface->start_vertex];

	glBegin( GL_TRIANGLES );
	if(m_iTexPointer[0] == TC_LIGHTMAP)
	{
		for(int i = 0; i < pbrushface->num_vertexes; i++, pVert++)
		{
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, pVert->lightmaptexcoord[0], pVert->lightmaptexcoord[1]);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, pVert->texcoord[0]+speed, pVert->texcoord[1]);
			glMultiTexCoord2fARB(GL_TEXTURE2_ARB, pVert->detailtexcoord[0]+speed, pVert->detailtexcoord[1]);
			glVertex3fv(pVert->pos);			
		}
	}
	else
	{
		for(int i = 0; i < pbrushface->num_vertexes; i++, pVert++)
		{
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, pVert->texcoord[0]+speed, pVert->texcoord[1]);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, pVert->detailtexcoord[0]+speed, pVert->detailtexcoord[1]);
			glVertex3fv(pVert->pos);			
		}
	}
	glEnd();
}

/*
====================
EmitWaterPolys

====================
*/
void CBSPRenderer::EmitWaterPolys( msurface_t *fa )
{
	if(m_pCurrentEntity->curstate.skin == CONTENTS_WATER
		||m_pCurrentEntity->curstate.skin == CONTENTS_SLIME
		||m_pCurrentEntity->curstate.skin == CONTENTS_LAVA)
	{
		// only draw if it's on the top
		for (glpoly_t *bp = fa->polys; bp; bp = bp->next)
		{
			for(int i = 0; i < bp->numverts; i++)
			{
				if(bp->verts[i][2] != (m_pCurrentEntity->curstate.maxs.z-1))
					return;
			}
		}
	}


	SetTexEnvs(ENVSTATE_MUL);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	Bind2DTexture(GL_TEXTURE0_ARB, fa->texinfo->texture->gl_texturenum);

	float fltime = gEngfuncs.GetClientTime();
	for (glpoly_t *p= fa->polys; p; p = p->next)
	{
		glBegin (GL_POLYGON);
		for (int i = 0; i < p->numverts; i++)
		{
			float os = p->verts[i][3];
			float ot = p->verts[i][4];

			float s = os + turbsin[(int)((ot*0.125+fltime) * 40) & 255];
			float t = ot + turbsin[(int)((os*0.125+fltime) * 40) & 255];
			float ssin = turbsin[(int)((os*0.125+(fltime*3.2)) * 40) & 255];
			float tsin = turbsin[(int)((ot*0.125+(fltime*3.2)) * 40) & 255];
			float height = (ssin - tsin)*m_pCurrentEntity->curstate.scale;

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, s*(1.0/64), t*(1.0/64));
			glVertex3f(p->verts[i][0], p->verts[i][1], p->verts[i][2] + height);
		}
		glEnd ();
	}

	if(m_pCvarWireFrame->value >= 1)
	{
		SetTexEnvs(ENVSTATE_OFF, ENVSTATE_OFF, ENVSTATE_OFF,ENVSTATE_OFF);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(1);

		if(m_pCvarWireFrame->value >= 2)
			glDisable(GL_DEPTH_TEST);

		if(gHUD.m_pFogSettings.active)
			glDisable(GL_FOG);

		if(m_pCurrentEntity->index == 0)
			glColor4f(1.0, 0.0, 0.0, 1.0);
		else
			glColor4f(1.0, 0.0, 1.0, 1.0);

		for (glpoly_t *p= fa->polys; p; p = p->next)
		{
			glBegin (GL_POLYGON);
			for (int i = 0; i < p->numverts; i++)
			{
				float height = (sin(p->verts[i][3]*0.125+(gEngfuncs.GetClientTime()*5))*10 - sin(p->verts[i][4]*0.125+(gEngfuncs.GetClientTime()*5))*15)*m_pCurrentEntity->curstate.scale;
				glVertex3f(p->verts[i][0], p->verts[i][1], p->verts[i][2] + height);
			}
			glEnd ();
		}

		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		if(m_pCvarWireFrame->value >= 2)
			glEnable(GL_DEPTH_TEST);

		if(gHUD.m_pFogSettings.active)
			glEnable(GL_FOG);
	}
}

/*
====================
MarkLights

====================
*/
void CBSPRenderer::MarkLights( cl_dlight_t *pLight, int iBit, mnode_t *node)
{
	if (node->contents < 0)
		return;

	mplane_t *splitplane = node->plane;
	float dist /*= DotProduct (pLight->origin, splitplane->normal) - splitplane->dist*/;
	SSEDotProductSub(&dist, &pLight->origin, &splitplane->normal, &splitplane->dist);
	
	if (dist > pLight->radius)
	{
		MarkLights (pLight, iBit, node->children[0]);
		return;
	}
	if (dist < -pLight->radius)
	{
		MarkLights (pLight, iBit, node->children[1]);
		return;
	}

	msurface_t *surf = m_pWorld->surfaces + node->firstsurface;
	for(int i = 0; i < node->numsurfaces; i++, surf++)
	{
		if (surf->dlightframe != m_iFrameCount)
		{
			surf->dlightframe = m_iFrameCount;
			surf->dlightbits = 0;
		}
		surf->dlightbits |= iBit;
	}

	MarkLights (pLight, iBit, node->children[0]);
	MarkLights (pLight, iBit, node->children[1]);
}

/*
====================
PushDynLights

====================
*/
void CBSPRenderer::PushDynLights( void )
{
	if(m_bShaderSupport && m_pCvarWorldShaders->value >= 1)
		return;
	
	if(m_pCvarDynamic->value < 1)
		return;

	cl_dlight_t *dl = m_pDynLights;
	for(int i = 0; i < MAX_DYNLIGHTS; i++, dl++)
	{
		if(dl->die <= gEngfuncs.GetClientTime() || !dl->radius)
			continue;

		MarkLights(dl, 1<<i, m_pWorld->nodes);
	}
}

/*
====================
AnimateLight

====================
*/
void CBSPRenderer::AnimateLight( void )
{
	int			i,j,k;
	
	i = (int)(gEngfuncs.GetClientTime()*10);
	for (j = 0; j < MAX_LIGHTSTYLES; j++)
	{
		if (!m_pLightStyles[j].length)
		{
			m_iLightStyleValue[j] = 256;
			continue;
		}
		k = i % m_pLightStyles[j].length;
		k = m_pLightStyles[j].map[k] - 'a';
		k = k*22;
		m_iLightStyleValue[j] = k;
	}	
}

/*
====================
AddDynamicLights

====================
*/
void CBSPRenderer::AddDynamicLights( msurface_t *surf )
{
	vec3_t		impact, local;

	int smax = (surf->extents[0]>>4)+1;
	int tmax = (surf->extents[1]>>4)+1;
	mtexinfo_t *tex = surf->texinfo;

	cl_dlight_t *dl = m_pDynLights;
	for(int i = 0; i < MAX_DYNLIGHTS; i++, dl++)
	{
		if ( !(surf->dlightbits & (1<<i) ) )
			continue;

		vec3_t origin = dl->origin;
		if(IsEntityMoved(m_pCurrentEntity))
		{
			VectorSubtract(origin, m_pCurrentEntity->origin, origin);
			if(m_pCurrentEntity->angles[0] 
			|| m_pCurrentEntity->angles[1] 
			|| m_pCurrentEntity->angles[2])
			{
				vec3_t forward, right, up, temp;
				AngleVectors (m_pCurrentEntity->angles, forward, right, up);

				VectorCopy (origin, temp);
				DotProductSSE(&origin[0], temp, forward);
				DotProductSSE(&origin[1], temp, right);
				DotProductSSE(&origin[2], temp, up);
				origin[1] = -origin[1];
			}
		}

		float dist;
		SSEDotProductSub(&dist, &origin, &surf->plane->normal, &surf->plane->dist);
		float rad = dl->radius - fabs(dist);

		for (int j = 0; j < 3; j++)
			impact[j] = origin[j] - surf->plane->normal[j]*dist;

		local[0] = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3] - surf->texturemins[0];
		local[1] = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3] - surf->texturemins[1];

		int fsacc;
		int ftacc = 0;
		color24 *bl = m_pBlockLights;

		for(int t = 0; t < tmax; t++, ftacc += 16)
		{
			int td = local[1] - ftacc;
			
			if ( td < 0 )
				td = -td;
			
			fsacc = 0;
			for (int s = 0; s < smax; s++, fsacc += 16, bl++)
			{
				int sd = local[0] - fsacc;

				if ( sd < 0 )
					sd = -sd;

				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);

				if ( dist < rad )
				{
					int iR = bl->r + (rad-dist)*dl->color[0];
					int iG = bl->g + (rad-dist)*dl->color[1];
					int iB = bl->b + (rad-dist)*dl->color[2];
					ClampColor(iR,iG,iB,bl);
				}
			}
		}
	}
}


/*
====================
BuildLightmap

====================
*/
void CBSPRenderer::BuildLightmap( msurface_t *surf, int surfindex, color24 *out )
{
	color24 *bl = m_pBlockLights;
	color24 *lightmap = surf->samples;

	int smax = (surf->extents[0]>>4)+1;
	int tmax = (surf->extents[1]>>4)+1;
	int size = smax*tmax;

	if (!lightmap || size > BLOCKLIGHTS_SIZE)
		return;
		
	for (int i = 0; i < size; i++)
	{
		m_pBlockLights[i].r = lightmap[i].r;
		m_pBlockLights[i].g = lightmap[i].g;
		m_pBlockLights[i].b = lightmap[i].b;
	}

	for (int maps = 1; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++)
	{
		lightmap += size;// skip to next lightmap
		m_pSurfaces[surfindex].cached_light[maps] = m_iLightStyleValue[surf->styles[maps]];
		if ( m_pSurfaces[surfindex].cached_light[maps] != 0 )
		{
			float scale = (float)m_iLightStyleValue[surf->styles[maps]]/255;
			for (int i = 0; i < size; i++)
			{
				int iR = (int)m_pBlockLights[i].r + (int)lightmap[i].r*scale;
				int iG = (int)m_pBlockLights[i].g + (int)lightmap[i].g*scale;
				int iB = (int)m_pBlockLights[i].b + (int)lightmap[i].b*scale;
				ClampColor(iR,iG,iB,&m_pBlockLights[i]);
			}
		}
	}

	if(surf->dlightframe == m_iFrameCount && surf->dlightbits)
		AddDynamicLights(surf);
	else if(surf->dlightframe && surf->dlightframe != m_iFrameCount)
		surf->dlightframe = 0;

	for (int i = 0; i < size; i++)
	{
		// Darken pixels with low values, helps make maps darker
		float flIntensity = (m_pBlockLights[i].r + m_pBlockLights[i].g + m_pBlockLights[i].b)/3;
		flIntensity = flIntensity/50;

		if(flIntensity > 1)
			flIntensity = 1;

		m_pBlockLights[i].r = m_pBlockLights[i].r*flIntensity;
		m_pBlockLights[i].g = m_pBlockLights[i].g*flIntensity;
		m_pBlockLights[i].b = m_pBlockLights[i].b*flIntensity;
	}

	int column = surf->lightmaptexturenum%LIGHTMAP_NUMROWS;
	int row = (surf->lightmaptexturenum/LIGHTMAP_NUMROWS)%LIGHTMAP_NUMCOLUMNS;

	m_pSurfaces[surfindex].light_s = surf->light_s+BLOCK_WIDTH*column;
	m_pSurfaces[surfindex].light_t = surf->light_t+BLOCK_HEIGHT*row;

	for (int i = 0; i < tmax; i++)
	{
		color24 *dest = out+BLOCK_WIDTH*BLOCK_HEIGHT*LIGHTMAP_NUMCOLUMNS*row+BLOCK_WIDTH*column;
		dest += (((BLOCK_WIDTH*LIGHTMAP_NUMCOLUMNS*surf->light_t)+surf->light_s)+(BLOCK_WIDTH*LIGHTMAP_NUMCOLUMNS*i));

		for (int j = 0; j < smax; j++)
		{
			dest[j].r = bl->r;
			dest[j].g = bl->g;
			dest[j].b = bl->b;
			bl++;
		}
	}
};

/*
====================
UploadLightmaps

====================
*/
void CBSPRenderer::UploadLightmaps( void )
{
	memset(m_iLightStyleValue, 0, sizeof(m_iLightStyleValue));
	memset(m_pEngineLightmaps, 0, sizeof(m_pEngineLightmaps));
	memset(m_pDetailLightmaps, 0, sizeof(m_pDetailLightmaps));
	m_iNumLightmaps = NULL;

	//
	// Allocate all surface infos
	//
	m_pSurfaces = new clientsurfdata_t[(m_pWorld->numsurfaces+m_iNumDetailSurfaces)];
	memset(m_pSurfaces, NULL, sizeof(clientsurfdata_t)*(m_pWorld->numsurfaces+m_iNumDetailSurfaces));
	m_iNumSurfaces = m_pWorld->numsurfaces+m_iNumDetailSurfaces;

	if(m_pWorld->lightdata)
	{
		//
		// Convert and merge lightmaps into one
		//
		for (int i = 0; i < m_pWorld->numsurfaces; i++)
		{
			msurface_t *pSurf = &m_pWorld->surfaces[i];

			if (pSurf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
				continue;

			BuildLightmap(pSurf, i, m_pEngineLightmaps);

			if(pSurf->lightmaptexturenum > m_iNumLightmaps)
				m_iNumLightmaps = pSurf->lightmaptexturenum;
		}
	}
	else
	{
		//
		// Count lightmaps anyway
		//
		for (int i = 0; i < m_pWorld->numsurfaces; i++)
		{
			msurface_t *pSurf = &m_pWorld->surfaces[i];

			if (pSurf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
				continue;

			if(pSurf->lightmaptexturenum > m_iNumLightmaps)
				m_iNumLightmaps = pSurf->lightmaptexturenum;
		}

		//
		// Fill with half-bright
		//
		memset(m_pEngineLightmaps, 128, sizeof(m_pEngineLightmaps));
	}

	//
	// Upload the large texture
	//
	glBindTexture(GL_TEXTURE_2D, m_iEngineLightmapIndex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D (GL_TEXTURE_2D, 0, 3, LIGHTMAP_RESOLUTION, LIGHTMAP_RESOLUTION, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pEngineLightmaps);

	if(!m_iNumDetailSurfaces)
		return;

	//
	// Load lightmaps of detail surfaces
	//
	int iSurfaceIndex = m_pWorld->numsurfaces;
	for(int i = 0; i < m_iNumDetailObjects; i++)
	{
		msurface_t *psurf = m_pDetailObjectSurfaces + m_pDetailObjects[i].firstsurface;
		for(int j = 0; j < m_pDetailObjects[i].numsurfaces; j++, psurf++)
		{
			// Upload it
			BuildLightmap(psurf, iSurfaceIndex, m_pDetailLightmaps);

			//Append
			iSurfaceIndex++;
		}
	}

	//
	// Upload the large texture
	//
	glBindTexture(GL_TEXTURE_2D, m_iDetailLightmapIndex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D (GL_TEXTURE_2D, 0, 3, LIGHTMAP_RESOLUTION, LIGHTMAP_RESOLUTION, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pDetailLightmaps);
}

/*
====================
Bind2DTexture

====================
*/
void CBSPRenderer::Bind2DTexture(GLenum texture, GLuint id)
{
	int idx = texture - GL_TEXTURE0_ARB;
	if (m_uiCurrentBinds[idx] != id)
	{
		glActiveTextureARB( texture );
		glBindTexture( GL_TEXTURE_2D, id );

		m_uiCurrentBinds[idx] = id;
	}
}

/*
====================
SetTexEnv_Internal

====================
*/
void CBSPRenderer::SetTexEnv_Internal(int env)
{
	switch(env)
	{
	case ENVSTATE_OFF:
		glDisable(GL_TEXTURE_2D);
		break;

	case ENVSTATE_REPLACE:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		break;

	case ENVSTATE_MUL_CONST:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_CONSTANT_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_MUL_PREV_CONST:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_MUL:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		break;

	case ENVSTATE_MUL_X2:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 2);
		break;

	case ENVSTATE_ADD:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_ADD);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_DOT:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGBA_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_DOT_CONST:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGBA_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_CONSTANT_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_PREVCOLOR_CURALPHA:
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
		break;
	}
}
/*
====================
SetTexEnvs

====================
*/
void CBSPRenderer::SetTexEnvs(int env0, int env1, int env2, int env3)
{
	if (m_iEnvStates[0] != env0) 
	{
		glActiveTextureARB( GL_TEXTURE0_ARB );
		SetTexEnv_Internal(env0);
		m_iEnvStates[0] = env0;
	}
	if (m_iEnvStates[1] != env1) 
	{
		glActiveTextureARB( GL_TEXTURE1_ARB );
		SetTexEnv_Internal(env1);
		m_iEnvStates[1] = env1;
	}
	if (m_iEnvStates[2] != env2) 
	{
		glActiveTextureARB( GL_TEXTURE2_ARB );
		SetTexEnv_Internal(env2);
		m_iEnvStates[2] = env2;
	}
	if (m_iEnvStates[3] != env3) 
	{
		glActiveTextureARB( GL_TEXTURE3_ARB );
		SetTexEnv_Internal(env3);
		m_iEnvStates[3] = env3;
	}
};

/*
====================
TextureAnimation

====================
*/
texture_t *CBSPRenderer::TextureAnimation (texture_t *base, int frame)
{
	int	reletive;
    int	count;

	if (frame)
	{
		if(base->alternate_anims)
			base = base->alternate_anims;
	}
	
	if ((base->name[0] != '+') || (!base->anim_total))
		return base;

	reletive = (int)(gEngfuncs.GetClientTime()*20) % base->anim_total;

	count = 0;	
	while (base->anim_min > reletive || base->anim_max <= reletive)
	{
		base = base->anim_next;
		if (!base)
			gEngfuncs.Con_Printf("TextureAnimation: broken cycle");
		if (++count > 100)
			gEngfuncs.Con_Printf("TextureAnimation: infinite cycle");
	}

	return base;
}

/*
====================
LoadDetailTexture

====================
*/
cl_texture_t *CBSPRenderer::LoadDetailTexture( char *texname )
{
	// load the texture file
	char szPath[256];
	sprintf(szPath, "gfx/textures/details/%s.dds", texname);
	cl_texture_t *pTexture = gTextureLoader.LoadTexture(szPath);

	if ( !pTexture )
		return 0;
	
	return pTexture;
};

/*
====================
ParseDetailTextureFile

====================
*/
void CBSPRenderer::ParseDetailTextureFile( void )
{
	char szLevelName[256];
	m_iNumDetailTextures = 0;

	strcpy( szLevelName, gEngfuncs.pfnGetLevelName() );

	if ( strlen(szLevelName) == 0 )
		return;

	szLevelName[strlen(szLevelName)-4] = 0;
	strcat(szLevelName, "_detail.txt");

	char *pfile = (char *)gEngfuncs.COM_LoadFile( szLevelName, 5, NULL);
	if (!pfile)
	{
		gEngfuncs.Con_Printf("BSP Renderer: Failed to load detail texture file for %s\n", szLevelName);
		return;
	}

	char *ptext = pfile;
	while(1)
	{
		char temp[256];
		char texture[256];
		char detailtexture[256];
		char sz_xscale[64];
		char sz_yscale[64];

		if (m_iNumDetailTextures >= MAX_DETAIL_TEXTURES)
		{
			gEngfuncs.Con_Printf("BSP Renderer: Too many entries in detail texture file %s\n", szLevelName);
			break;
		}

		ptext = gEngfuncs.COM_ParseFile(ptext, texture);
		if (!ptext) 
			break;
		
		if ( texture[0] == '{' )
		{
			ptext = gEngfuncs.COM_ParseFile(ptext, temp);
			strcat(texture, temp);
		}

		if (!ptext) 
			break;

		ptext = gEngfuncs.COM_ParseFile(ptext, detailtexture);
		if (!ptext) 
			break;

		ptext = gEngfuncs.COM_ParseFile(ptext, sz_xscale);
		if (!ptext) 
			break;

		ptext = gEngfuncs.COM_ParseFile(ptext, sz_yscale);
		if (!ptext) 
			break;

		float i_xscale = atof(sz_xscale);
		float i_yscale = atof(sz_yscale);

		if (strlen(texture) > 32 || strlen(detailtexture) > 32)
		{
			gEngfuncs.Con_Printf("BSP Renderer: Error in detail texture file %s: token too large\n", szLevelName);
			gEngfuncs.Con_Printf("(entry %d: %s - %s)\n", m_iNumDetailTextures, texture, detailtexture);
			continue;
		}

		cl_texture_t *pTexture = LoadDetailTexture(detailtexture);

		if (!pTexture)
			continue;

		strLower(texture);
		strcpy( m_pDetailTextures[m_iNumDetailTextures].texname, texture );
		strcpy( m_pDetailTextures[m_iNumDetailTextures].detailtexname, detailtexture );
		m_pDetailTextures[m_iNumDetailTextures].xscale = i_xscale;
		m_pDetailTextures[m_iNumDetailTextures].yscale = i_yscale;
		m_pDetailTextures[m_iNumDetailTextures].texindex = pTexture->iIndex;
		m_iNumDetailTextures++;
	}
	gEngfuncs.COM_FreeFile( pfile );
};

/*
====================
LoadDetailTextures

====================
*/
void CBSPRenderer::LoadDetailTextures( void )
{
	ParseDetailTextureFile();

	texture_t* tex = m_pNormalTextureList;
	for (int i = 0; i < m_iNumTextures; i++)
	{
		if (tex[i].name[0] == 0)
			continue;

		char szName[16];
		strcpy(szName, tex[i].name);
		strLower(szName);

		int j = 0;
		for(; j < m_iNumDetailTextures; j++)
		{
			if(!strcmp(m_pDetailTextures[j].texname, szName))
				break;
		}

		// Found detail texture
		if(j != m_iNumDetailTextures)
		{
			tex[i].offsets[2] = j;
			tex[i].offsets[3] = m_pDetailTextures[j].texindex;
		}
	}
};

/*
====================
FindIntersectionPoint

====================
*/
void CBSPRenderer::FindIntersectionPoint( const vec3_t &p1, const vec3_t &p2, const vec3_t &normal, const vec3_t &planepoint, vec3_t &newpoint )
{
	vec3_t planevec;
	vec3_t linevec;
	float planedist, linedist;

	VectorSubtract( planepoint, p1, planevec );
	VectorSubtract( p2, p1, linevec );

	DotProductSSE(&planedist, normal, planevec);
	DotProductSSE(&linedist, normal, linevec);

	if (linedist != 0)
	{
		VectorMASSE( p1, planedist/linedist, linevec, newpoint );
		return;
	}
	VectorClear( newpoint );
};

/*
====================
ClipPolygonByPlane

====================
*/
int CBSPRenderer::ClipPolygonByPlane (const vec3_t *arrIn, int numpoints, vec3_t normal, vec3_t planepoint, vec3_t *arrOut)
{
	int i, cur, prev;
	int first = -1;
	int outCur = 0;
	float dots[64];
	for (i = 0; i < numpoints; i++)
	{
		vec3_t vecDir;
		VectorSubtract( arrIn[i], planepoint, vecDir );
		DotProductSSE(&dots[i], vecDir, normal);
		
		if (dots[i] > 0) 
			first = i;
	}

	if (first == -1) 
		return 0;

	VectorCopy( arrIn[first], arrOut[outCur] );
	outCur++;

	cur = first + 1;
	if (cur == numpoints) 
		cur = 0;

	while (cur != first)
	{
		if (dots[cur] > 0)
		{
			VectorCopy( arrIn[cur], arrOut[outCur] );
			cur++; outCur++;

			if (cur == numpoints) 
				cur = 0;
		}
		else
			break;
	}

	if (cur == first) 
		return outCur;

	if (dots[cur] < 0)
	{
		vec3_t newpoint;
		if (cur > 0) 
			prev = cur-1;
		else 
			prev = numpoints - 1;

		FindIntersectionPoint( arrIn[prev], arrIn[cur], normal, planepoint, newpoint );
		VectorCopy( newpoint, arrOut[outCur] );
	}
	else
	{
		VectorCopy( arrIn[cur], arrOut[outCur] );
	}

	outCur++;
	cur++;

	if (cur == numpoints) 
		cur = 0;

	while (dots[cur] < 0)
	{
		cur++;
		if (cur == numpoints) cur = 0;
	}

	if (cur > 0) 
		prev = cur-1;
	else 
		prev = numpoints - 1;

	if (dots[cur] > 0 && dots[prev] < 0)
	{
		vec3_t newpoint;
		FindIntersectionPoint( arrIn[prev], arrIn[cur], normal, planepoint, newpoint );
		VectorCopy( newpoint, arrOut[outCur] );
		outCur++;
	}

	while (cur != first)
	{
		VectorCopy( arrIn[cur], arrOut[outCur] );
		cur++; outCur++;
		if (cur == numpoints) cur = 0;
	}
	return outCur;
}

/*
====================
GetUpRight

====================
*/
void CBSPRenderer::GetUpRight(vec3_t forward, vec3_t &up, vec3_t &right)
{
	VectorClear(up);

	if (forward.x || forward.y)
		up.z = 1;
	else
		up.x = 1;

	right = CrossProduct(forward, up);
	VectorNormalizeFast(right);

	up = CrossProduct(forward, right);
	VectorNormalizeFast(up);
};

/*
====================
LoadDecalTexture

====================
*/
cl_texture_t *CBSPRenderer::LoadDecalTexture( const char *texname )
{
	char path[256];	
	sprintf(path, "gfx/textures/decals/%s.dds", texname);

	cl_texture_t *pTexture = gTextureLoader.LoadTexture(path);

	if (!pTexture)
	{
		gEngfuncs.Con_Printf("BSP Renderer: Missing decal texture %s!\n", path);
		return NULL;
	}

	// ALWAYS Bind
	glBindTexture(GL_TEXTURE_2D, pTexture->iIndex);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	return pTexture;
};

/*
====================
LoadDecals

====================
*/
void CBSPRenderer::LoadDecals( void )
{
	char *pfile = (char *)gEngfuncs.COM_LoadFile("gfx/textures/decals/decalinfo.txt", 5, NULL);
	if (!pfile)
	{
		gEngfuncs.Con_Printf("BSP Renderer: Cannot open file \"gfx/textures/decals/decalinfo.txt\"\n");
		return;
	}

	int counter = 0;
	char *ptext = pfile;
	while(1)
	{
		// store position where group names recorded
		char *groupnames = ptext;

		// loop until we'll find decal names
		int numgroups = 0;
		char token[256];
		while(1)
		{
			ptext = gEngfuncs.COM_ParseFile(ptext, token);
			if (!ptext) 
				goto getout;

			if (token[0] == '{') 
				break;

			numgroups++;
		}

		decalgroupentry_t tempentries[MAX_GROUPENTRIES];
		int numtemp = 0;
		while(1)
		{
			char sz_xsize[64];
			char sz_ysize[64];
			ptext = gEngfuncs.COM_ParseFile(ptext, token);

			if (!ptext) 
				goto getout;
			if (token[0] == '}') 
				break;


			ptext = gEngfuncs.COM_ParseFile(ptext, sz_xsize);

			if (!ptext) 
				goto getout;

			ptext = gEngfuncs.COM_ParseFile(ptext, sz_ysize);

			if (!ptext) 
				goto getout;

			cl_texture_t *pTexture = LoadDecalTexture(token);

			if (!pTexture)
				continue;

			strcpy(tempentries[numtemp].szName, token);
			tempentries[numtemp].gl_texid = pTexture->iIndex;
			tempentries[numtemp].xsize = atof(sz_xsize) / 2;
			tempentries[numtemp].ysize = atof(sz_ysize) / 2;
			numtemp++;
		}

		// get back to group names
		for (int i = 0; i < numgroups; i++)
		{
			groupnames = gEngfuncs.COM_ParseFile(groupnames, token);
			if (!numtemp)
			{
				gEngfuncs.Con_Printf("BSP Renderer: got empty decal group: %s\n", token);
				continue;
			}

			m_pDecalGroups[m_iNumDecalGroups].iSize = numtemp;
			strcpy(m_pDecalGroups[m_iNumDecalGroups].szName, token);
			memcpy(m_pDecalGroups[m_iNumDecalGroups].entries, tempentries, sizeof(tempentries));

			m_iNumDecalGroups++;
			counter++;
		}
	}

getout:
	gEngfuncs.COM_FreeFile( pfile );
	gEngfuncs.Con_Printf("BSP Renderer: %d decal groups created\n", counter);
};

/*
====================
AllocDecal

====================
*/
customdecal_t *CBSPRenderer::AllocDecal( void )
{
	customdecal_t *ret = &m_pDecals[m_iCurDecal];

	if (m_iNumDecals < MAX_CUSTOMDECALS)
		m_iNumDecals++;

	m_iCurDecal++;
	if (m_iCurDecal == MAX_CUSTOMDECALS)
		m_iCurDecal = 0; // get decals from root again

	if(ret->inumpolys)
	{
		for(int i = 0; i < ret->inumpolys; i++)
			delete [] ret->polys[i].pverts;

		delete [] ret->polys;

	}

	memset(ret, 0, sizeof(customdecal_t));
	return ret;
}

/*
====================
AllocStaticDecal

====================
*/
customdecal_t *CBSPRenderer::AllocStaticDecal( void )
{
	if (m_iNumStaticDecals < MAX_STATICDECALS)
	{
		customdecal_t *ret = &m_pStaticDecals[m_iNumStaticDecals];
		m_iNumStaticDecals++;
		return ret;
	}
	return NULL;
}

/*
====================
FindDecalByName

====================
*/
decalgroupentry_t *CBSPRenderer::FindDecalByName( const char *szName )
{
	for(int i = 0; i < m_iNumDecalGroups; i++)
	{
		if (m_pDecalGroups[i].iSize == 0)
			continue;

		for(int j = 0; j < m_pDecalGroups[i].iSize; j++)
		{
			if(!strcmp(m_pDecalGroups[i].entries[j].szName, szName))
				return &m_pDecalGroups[i].entries[j];
		}
	}
	return NULL;
}

/*
====================
GetRandomDecal

====================
*/
decalgroupentry_t *CBSPRenderer::GetRandomDecal( decalgroup_t *group )
{
	if (group->iSize == 0)
		return NULL;

	if (group->iSize == 1)
		return &group->entries[0];

	int idx = gEngfuncs.pfnRandomLong( 0, group->iSize-1 );

	return &group->entries[idx];
}

/*
====================
FindGroup

====================
*/
decalgroup_t *CBSPRenderer::FindGroup(const char *_name)
{
	for ( int i = 0; i < m_iNumDecalGroups; i++)
	{
		if (!strcmp(m_pDecalGroups[i].szName, _name))
			return &m_pDecalGroups[i];
	}

	return NULL; // nothing found
}

/*
====================
CullDecalBBox

====================
*/
bool CBSPRenderer::CullDecalBBox( vec3_t mins, vec3_t maxs )
{
	if (mins[0] > m_vDecalMaxs[0]) 
		return true;

	if (mins[1] > m_vDecalMaxs[1]) 
		return true;

	if (mins[2] > m_vDecalMaxs[2]) 
		return true;

	if (maxs[0] < m_vDecalMins[0]) 
		return true;

	if (maxs[1] < m_vDecalMins[1]) 
		return true;

	if (maxs[2] < m_vDecalMins[2]) 
		return true;

	return false;
}

/*
====================
CreateDecal

====================
*/
void CBSPRenderer::CreateDecal( vec3_t endpos, vec3_t pnormal, const char *name, int persistent )
{
	vec3_t mins, maxs;
	vec3_t decalpos, decalnormal;
	decalgroupentry_t *pDecalTex;

	m_pWorld = IEngineStudio.GetModelByIndex(1);
	if (!m_pWorld)
	{
		if (m_iCacheDecals >= MAX_DECAL_MSG_CACHE)
			return;

		strcpy(m_pMsgCache[m_iCacheDecals].name, name);
		m_pMsgCache[m_iCacheDecals].normal = pnormal;
		m_pMsgCache[m_iCacheDecals].pos = endpos;
		m_pMsgCache[m_iCacheDecals].persistent = persistent;
		m_iCacheDecals++;
		return;
	}

	if(!persistent)
	{
		decalgroup_t *pGroup = FindGroup(name);
	
		if(!pGroup)
			return;

		pDecalTex = GetRandomDecal(pGroup);
	}
	else
	{
		pDecalTex = FindDecalByName(name);
	}

	if(!pDecalTex)
		return;

	int xsize = pDecalTex->xsize;
	int ysize = pDecalTex->ysize;

	float radius = (xsize > ysize) ? xsize : ysize;

	m_vDecalMins[0] = endpos[0] - radius;
	m_vDecalMins[1] = endpos[1] - radius;
	m_vDecalMins[2] = endpos[2] - radius;
	m_vDecalMaxs[0] = endpos[0] + radius;
	m_vDecalMaxs[1] = endpos[1] + radius;
	m_vDecalMaxs[2] = endpos[2] + radius;

	customdecal_t *pDecal = NULL;
	if (persistent)
	{
		pDecal = AllocStaticDecal();

		if (!pDecal)
			return;
	}
	else
	{
		for(int i = 0; i < m_iNumDecals; i++)
		{
			if(m_pDecals[i].texinfo->group != pDecalTex->group)
				continue;

			xsize = m_pDecals[i].texinfo->xsize;
			ysize = m_pDecals[i].texinfo->ysize;
			radius = (xsize>ysize) ? xsize:ysize;

			mins[0] = m_pDecals[i].position[0] - radius;
			mins[1] = m_pDecals[i].position[1] - radius;
			mins[2] = m_pDecals[i].position[2] - radius;
			maxs[0] = m_pDecals[i].position[0] + radius;
			maxs[1] = m_pDecals[i].position[1] + radius;
			maxs[2] = m_pDecals[i].position[2] + radius;
			
			if(!CullDecalBBox(mins, maxs))
			{
				for(int j = 0; j < m_pDecals[i].inumpolys; j++)
					delete [] m_pDecals[i].polys[j].pverts;

				delete [] m_pDecals[i].polys;
				memset(&m_pDecals[i], 0, sizeof(customdecal_t));
				pDecal = &m_pDecals[i];
				break;
			}
		}

		if (!pDecal)
			pDecal = AllocDecal();
	}

	if(!pDecal)
		return;

	pDecal->texinfo = pDecalTex;
	VectorCopy(endpos, pDecal->position);
	VectorCopy(pnormal, pDecal->normal);

	RecursiveCreateDecal(m_pWorld->nodes, pDecalTex, pDecal, endpos, pnormal);

	for(int i = 1; i < MAXRENDERENTS; i++)
	{
		cl_entity_t *pEntity = gEngfuncs.GetEntityByIndex(i);

		if ( !pEntity )
			break;

		if ( !pEntity->model )
			continue;
		
		if ( pEntity->model->type != mod_brush )
			continue;

		if( pEntity->curstate.solid == SOLID_NOT )
			continue;

		if(pEntity->angles[0] || pEntity->angles[1] || pEntity->angles[2])
		{
			mins[0] = pEntity->origin[0] - pEntity->model->radius;
			mins[1] = pEntity->origin[1] - pEntity->model->radius;
			mins[2] = pEntity->origin[2] - pEntity->model->radius;
			maxs[0] = pEntity->origin[0] + pEntity->model->radius;
			maxs[1] = pEntity->origin[1] + pEntity->model->radius;
			maxs[2] = pEntity->origin[2] + pEntity->model->radius;
		}
		else
		{
			mins = pEntity->origin + pEntity->model->mins;
			maxs = pEntity->origin + pEntity->model->maxs;
		}

		if (CullDecalBBox(mins, maxs))
			continue;

		if(pEntity->origin[0]||pEntity->origin[1]||pEntity->origin[2])
		{
			VectorSubtract(endpos, pEntity->origin, decalpos);
			if(pEntity->angles[0] || pEntity->angles[1] || pEntity->angles[2])
			{
				vec3_t temp, forward, right, up;
				AngleVectors (pEntity->angles, forward, right, up);

				VectorCopy (decalpos, temp);
				decalpos[0] = DotProduct (temp, forward);
				decalpos[1] = -DotProduct (temp, right);
				decalpos[2] = DotProduct (temp, up);

				VectorCopy (pnormal, temp);
				decalnormal[0] = DotProduct (temp, forward);
				decalnormal[1] = -DotProduct (temp, right);
				decalnormal[2] = DotProduct (temp, up);
			}
			else
			{
				VectorSubtract(endpos, pEntity->origin, decalpos);
				VectorCopy(pnormal, decalnormal);
			}
		}
		else
		{
			VectorCopy(pnormal, decalnormal);
			VectorCopy(endpos, decalpos);
		}

		msurface_t *surf = &m_pWorld->surfaces[pEntity->model->firstmodelsurface];
		for(int k = 0; k < pEntity->model->nummodelsurfaces; k++, surf++)
		{
			float dot;
			mplane_t *pplane = surf->plane;
			SSEDotProductSub(&dot, &decalpos, &pplane->normal, &pplane->dist);

			if(dot < 0)
				dot *= -1;

			if(dot < radius)
			{
				vec3_t normal = pplane->normal;

				if(surf->flags & SURF_PLANEBACK)
					VectorInverse(normal);

				if( DotProduct(normal,decalnormal) < 0.01 )
					continue;

				DecalSurface(surf, pDecalTex, pEntity, pDecal, decalpos, decalnormal);
			}
		}
	}
}

/*
====================
RecursiveCreateDecal

====================
*/
void CBSPRenderer::RecursiveCreateDecal( mnode_t *node, decalgroupentry_t *texptr, customdecal_t *pDecal, vec3_t endpos, vec3_t pnormal )
{
	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	if (node->contents < 0)
		return;

	if(CullDecalBBox(node->minmaxs, (node->minmaxs+3)))
		return;

	int xsize = texptr->xsize;
	int ysize = texptr->ysize;

	float radius = (xsize > ysize) ? xsize : ysize;

	int side;
	float dot;
	mplane_t *plane = node->plane;

	switch (plane->type)
	{
	case PLANE_X:
		dot = endpos[0] - plane->dist;	break;
	case PLANE_Y:
		dot = endpos[1] - plane->dist;	break;
	case PLANE_Z:
		dot = endpos[2] - plane->dist;	break;
	default:
		SSEDotProductSub(&dot, &endpos, &plane->normal, &plane->dist); 
		break;
	}

	if (dot >= 0) 
		side = 0;
	else 
		side = 1;

// recurse down the children, front side first
	RecursiveCreateDecal(node->children[side], texptr, pDecal, endpos, pnormal);

// draw stuff
	int c = node->numsurfaces;
	if (c)
	{
		msurface_t *surf = m_pWorld->surfaces + node->firstsurface;

		if (dot < 0 -BACKFACE_EPSILON)
			side = SURF_PLANEBACK;
		else if (dot > BACKFACE_EPSILON)
			side = 0;

		for ( ; c ; c--, surf++)
		{	
			float dot;
			mplane_t *pplane = surf->plane;
			SSEDotProductSub(&dot, &endpos, &pplane->normal, &pplane->dist);

			if(dot < 0)
				dot *= -1;

			if(dot < radius)
			{
				vec3_t normal = pplane->normal;

				if(surf->flags & SURF_PLANEBACK)
					VectorInverse(normal);

				if( DotProduct(normal,pnormal) < 0.01 )
					continue;

				DecalSurface(surf, texptr, NULL, pDecal, endpos, pnormal);
			}
		}
	}

	RecursiveCreateDecal (node->children[!side], texptr, pDecal, endpos, pnormal);
}

/*
====================
DecalSurface

====================
*/
void CBSPRenderer::DecalSurface(msurface_t *surf, decalgroupentry_t *texptr, cl_entity_t *pEntity, customdecal_t *pDecal, vec3_t endpos, vec3_t pnormal)
{
	vec3_t norm;
	vec3_t right, up;

	vec3_t dverts1[64];
	vec3_t dverts2[64];

	if(pEntity && surf->texinfo->texture->name[0] == '{'
		&& pEntity->curstate.rendermode == kRenderTransAlpha)
		return;

	if(stristr(surf->texinfo->texture->name, "scroll")
		&& pEntity->curstate.eflags == EFLAG_CONVEYOR)
		return;

	if(surf->flags & SURF_DRAWTURB || surf->flags & SURF_DRAWSKY)
		return;

	GetUpRight(pnormal, up, right);

	int xsize = texptr->xsize;
	int ysize = texptr->ysize;

	float texc_orig_x = DotProduct(endpos, right);
	float texc_orig_y = DotProduct(endpos, up);

	glpoly_t *p = surf->polys;
	float *v = p->verts[0];

	for (int j = 0; j < p->numverts; j++, v+= VERTEXSIZE)
		VectorCopy( v, dverts1[j] );

	int nv;
	vec3_t planepoint;
	VectorMASSE(endpos, -xsize, right, planepoint);
	nv = ClipPolygonByPlane (dverts1, p->numverts, right, planepoint, dverts2);

	VectorMASSE(endpos, xsize, right, planepoint);
	nv = ClipPolygonByPlane (dverts2, nv, right*-1, planepoint, dverts1);

	VectorMASSE(endpos, -ysize, up, planepoint);
	nv = ClipPolygonByPlane (dverts1, nv, up, planepoint, dverts2);

	VectorMASSE(endpos, ysize, up, planepoint);
	nv = ClipPolygonByPlane (dverts2, nv, up*-1, planepoint, dverts1);

	if (!nv)
		return;

	if(pDecal->polys)
	{
		customdecalpoly_t *ppolys = new customdecalpoly_t[(pDecal->inumpolys+1)];
		memcpy(ppolys, pDecal->polys, sizeof(customdecalpoly_t)*pDecal->inumpolys);
		delete [] pDecal->polys; pDecal->polys = ppolys; pDecal->inumpolys++;
	}
	else
	{
		pDecal->polys = new customdecalpoly_t;
		pDecal->inumpolys++;
	}

	customdecalpoly_t *pPoly = &pDecal->polys[(pDecal->inumpolys-1)];
	pPoly->pverts = new customdecalvert_t[nv];
	pPoly->numverts = nv;

	for(int j = 0; j < nv; j++)
	{
		float texc_x = (DotProduct(dverts1[j], right) - texc_orig_x)/xsize;
		float texc_y = (DotProduct(dverts1[j], up) - texc_orig_y)/ysize;

		pPoly->pverts[j].texcoord[0] = (texc_x + 1)/2;
		pPoly->pverts[j].texcoord[1] = (texc_y + 1)/2;
		pPoly->pverts[j].position = dverts1[j];
	}

	pPoly->surface = surf;
	pPoly->entity = pEntity;
}

/*
====================
CreateCachedDecals

====================
*/
void CBSPRenderer::CreateCachedDecals( void )
{
	for (int i = 0; i < gPropManager.m_iNumDecals; i++)
	{
		pmtrace_t pTrace;
		gEngfuncs.pEventAPI->EV_SetTraceHull(2);

		// Z Axis
		gEngfuncs.pEventAPI->EV_PlayerTrace(gPropManager.m_pDecals[i].pos+Vector(0, 0, 2), gPropManager.m_pDecals[i].pos-Vector(0, 0, 2), PM_WORLD_ONLY, -2, &pTrace);

		if(pTrace.fraction == 1 || pTrace.fraction == 0)
			gEngfuncs.pEventAPI->EV_PlayerTrace(gPropManager.m_pDecals[i].pos-Vector(0, 0, 2), gPropManager.m_pDecals[i].pos+Vector(0, 0, 2), PM_WORLD_ONLY, -2, &pTrace);

		// Y Axis
		if(pTrace.fraction == 1 || pTrace.fraction == 0)
			gEngfuncs.pEventAPI->EV_PlayerTrace(gPropManager.m_pDecals[i].pos+Vector(0, 2, 0), gPropManager.m_pDecals[i].pos-Vector(0, 2, 0), PM_WORLD_ONLY, -2, &pTrace);

		if(pTrace.fraction == 1 || pTrace.fraction == 0)
			gEngfuncs.pEventAPI->EV_PlayerTrace(gPropManager.m_pDecals[i].pos-Vector(0, 2, 0), gPropManager.m_pDecals[i].pos+Vector(0, 2, 0), PM_WORLD_ONLY, -2, &pTrace);

		// X Axis
		if(pTrace.fraction == 1 || pTrace.fraction == 0)
			gEngfuncs.pEventAPI->EV_PlayerTrace(gPropManager.m_pDecals[i].pos+Vector(2, 0, 0), gPropManager.m_pDecals[i].pos-Vector(2, 0, 0), PM_WORLD_ONLY, -2, &pTrace);

		if(pTrace.fraction == 1 || pTrace.fraction == 0)
			gEngfuncs.pEventAPI->EV_PlayerTrace(gPropManager.m_pDecals[i].pos-Vector(2, 0, 0), gPropManager.m_pDecals[i].pos+Vector(2, 0, 0), PM_WORLD_ONLY, -2, &pTrace);

		if(pTrace.fraction == 1 || pTrace.fraction == 0)
			pTrace.plane.normal = Vector(0, 0, 1);

		CreateDecal(gPropManager.m_pDecals[i].pos, pTrace.plane.normal, gPropManager.m_pDecals[i].name, gPropManager.m_pDecals[i].persistent);
	}

	for (int i = 0; i < m_iCacheDecals; i++)
	{
		CreateDecal(m_pMsgCache[i].pos, m_pMsgCache[i].normal, m_pMsgCache[i].name, m_pMsgCache[i].persistent);
	}

	m_iCacheDecals = 0;
	gPropManager.m_iNumDecals = 0;
}

/*
====================
DrawSingleDecal

====================
*/
void CBSPRenderer::DrawSingleDecal( customdecal_t *decal )
{
	Bind2DTexture(GL_TEXTURE0_ARB, decal->texinfo->gl_texid);

	for(int i = 0; i < decal->inumpolys; i++)
	{
		customdecalpoly_t *ppoly = &decal->polys[i];

		if (ppoly->surface->visframe != m_iFrameCount)
			continue;

		if ( ppoly->entity )
		{
			if(IsEntityMoved(ppoly->entity))
			{
				glPushMatrix();
				ppoly->entity->angles[0] = -ppoly->entity->angles[0];	// stupid quake bug
				R_RotateForEntity(ppoly->entity);
				ppoly->entity->angles[0] = -ppoly->entity->angles[0];	// stupid quake bug
			}
		}

		glBegin(GL_POLYGON);
		for (int k = 0; k < ppoly->numverts; k++)
		{
			glTexCoord2f(ppoly->pverts[k].texcoord[0], ppoly->pverts[k].texcoord[1]);
			glVertex3fv(ppoly->pverts[k].position);
		}
		glEnd();

		if(m_pCvarWireFrame->value)
		{
			glDisable(GL_TEXTURE_2D);
			glLineWidth(1);
			glColor4f(0.0, 1.0, 1.0, 1.0);

			SetTexEnvs(ENVSTATE_OFF, ENVSTATE_OFF, ENVSTATE_OFF, ENVSTATE_OFF);

			if(m_pCvarWireFrame->value >= 2)
			{
				if(gHUD.m_pFogSettings.active && !m_bSecondPassNeeded)
					glDisable(GL_FOG);
			}

			for( int j = 2; j < ppoly->numverts; j++ )
			{
				glBegin (GL_LINE_STRIP);
				glVertex3fv (ppoly->pverts[0].position);
				glVertex3fv (ppoly->pverts[j-1].position);
				glVertex3fv (ppoly->pverts[j].position);
				glVertex3fv (ppoly->pverts[0].position);
				glEnd ();
			}

			if(m_pCvarWireFrame->value >= 2)
			{
				if(gHUD.m_pFogSettings.active && !m_bSecondPassNeeded)
					glEnable(GL_FOG);
			}

			glEnable(GL_TEXTURE_2D);
			glColor4f(GL_ONE, GL_ZERO, GL_ONE, GL_ONE);
		}

		if ( ppoly->entity && IsEntityMoved(ppoly->entity))
			glPopMatrix();
	}
}

/*
====================
DrawDecals

====================
*/
void CBSPRenderer::DrawDecals( void )
{
	CreateCachedDecals();

	if (!m_iNumDecals && !m_iNumStaticDecals)
		return;

	ResetCache();

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

	glPolygonOffset ( -1, -1 );
	glEnable(GL_POLYGON_OFFSET_FILL);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1);

	SetTexEnvs(ENVSTATE_REPLACE);
	glActiveTextureARB(GL_TEXTURE0_ARB);

	if(gHUD.m_pFogSettings.active && m_pCvarWorldShaders->value && m_bShaderSupport)
	{
		glDisable(GL_FOG);
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_iDecalFragmentID);
	}
	else if(gHUD.m_pFogSettings.active)
	{
		glDisable(GL_FOG);
	}

	for (int i = 0; i < m_iNumDecals; i++)
		DrawSingleDecal(&m_pDecals[i]);

	for (int i = 0; i < m_iNumStaticDecals; i++)
		DrawSingleDecal(&m_pStaticDecals[i]);

	glDisable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0);
	glDepthMask(GL_TRUE);

	glDisable(GL_BLEND);
	glColor4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	if(gHUD.m_pFogSettings.active && m_pCvarWorldShaders->value && m_bShaderSupport)
	{
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
		glEnable(GL_FOG);
	}
	else if(gHUD.m_pFogSettings.active)
	{
		glEnable(GL_FOG);
	}
}

/*
====================
MsgCustomDecal

====================
*/
int CBSPRenderer::MsgCustomDecal(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	vec3_t pos, normal;
	pos.x = READ_COORD();
	pos.y = READ_COORD();
	pos.z = READ_COORD();
	normal.x = READ_COORD();
	normal.y = READ_COORD();
	normal.z = READ_COORD();

	CreateDecal(pos, normal, READ_STRING(), READ_BYTE());
	return 1;
}

/*
====================
DeleteDecals

====================
*/
void CBSPRenderer::DeleteDecals( void )
{
	m_iCurDecal = 0;

	if(m_iNumDecals)
	{
		for(int i = 0; i < m_iNumDecals; i++)
		{
			for(int j = 0; j < m_pDecals[i].inumpolys; j++)
				delete [] m_pDecals[i].polys[j].pverts;

			delete [] m_pDecals[i].polys;
		}

		// Clear array completely
		memset(m_pDecals, 0,	sizeof(m_pDecals));
		m_iNumDecals = NULL;
	}

	if(m_iNumStaticDecals)
	{
		for(int i = 0; i < m_iNumStaticDecals; i++)
		{
			for(int j = 0; j < m_pStaticDecals[i].inumpolys; j++)
				delete [] m_pStaticDecals[i].polys[j].pverts;

			delete [] m_pStaticDecals[i].polys;
		}

		// Clear array completely
		memset(m_pStaticDecals, 0,	sizeof(m_pStaticDecals));
		m_iNumStaticDecals = NULL;
	}
}

/*
====================
SetDynLightBBox

====================
*/
void CBSPRenderer::SetDynLightBBox( void )
{
	m_vDLightMins[0] = m_vCurDLightOrigin[0] - m_pCurrentDynLight->radius;
	m_vDLightMins[1] = m_vCurDLightOrigin[1] - m_pCurrentDynLight->radius;
	m_vDLightMins[2] = m_vCurDLightOrigin[2] - m_pCurrentDynLight->radius;
	m_vDLightMaxs[0] = m_vCurDLightOrigin[0] + m_pCurrentDynLight->radius;
	m_vDLightMaxs[1] = m_vCurDLightOrigin[1] + m_pCurrentDynLight->radius;
	m_vDLightMaxs[2] = m_vCurDLightOrigin[2] + m_pCurrentDynLight->radius;
}

/*
====================
CullDynLightBBox

====================
*/
int CBSPRenderer::CullDynLightBBox (vec3_t mins, vec3_t maxs)
{
	if (mins[0] > m_vDLightMaxs[0]) 
		return TRUE;

	if (mins[1] > m_vDLightMaxs[1]) 
		return TRUE;

	if (mins[2] > m_vDLightMaxs[2]) 
		return TRUE;

	if (maxs[0] < m_vDLightMins[0]) 
		return TRUE;

	if (maxs[1] < m_vDLightMins[1]) 
		return TRUE;

	if (maxs[2] < m_vDLightMins[2]) 
		return TRUE;

	return FALSE;
}

/*
====================
SetupDynLight

====================
*/
void CBSPRenderer::SetupDynLight( void )
{
	glBlendFunc(GL_ONE, GL_ONE);
	glColor3fv(m_pCurrentDynLight->color);

	SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_MUL_PREV_CONST);

	// 3d attenuation texture
	glActiveTextureARB( GL_TEXTURE0_ARB );
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, m_iAtten3DPoint);

	float r = 1 / (m_pCurrentDynLight->radius * 2);
	GLfloat planeS[] = {r, 0, 0, -m_vCurDLightOrigin[0] * r + 0.5};
	GLfloat planeT[] = {0, r, 0, -m_vCurDLightOrigin[1] * r + 0.5};
	GLfloat planeR[] = {0, 0, r, -m_vCurDLightOrigin[2] * r + 0.5};

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
	glTexGenfv(GL_S, GL_EYE_PLANE, planeS);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
	glTexGenfv(GL_T, GL_EYE_PLANE, planeT);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
	glTexGenfv(GL_R, GL_EYE_PLANE, planeR);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);

	// light color (bind dummy texture)
	glActiveTextureARB( GL_TEXTURE1_ARB );
	Bind2DTexture(GL_TEXTURE1_ARB, m_iLightDummy);
}

/*
====================
FinishDynLight

====================
*/
void CBSPRenderer::FinishDynLight( void )
{
	glActiveTextureARB( GL_TEXTURE0_ARB );
	glDisable(GL_TEXTURE_3D);
	glEnable(GL_TEXTURE_2D);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
}

/*
====================
LightCanShadow

====================
*/
bool CBSPRenderer::LightCanShadow( void )
{
	if(!m_bShadowSupport)
		return false;

	if(m_pCvarShadows->value < 1)
		return false;

	if(m_pCurrentEntity->angles[0])
		return false;

	if(m_pCurrentEntity->angles[2])
		return false;

	if(m_pCurrentDynLight->noshadow)
		return false;

	return true;
}

/*
====================
SetupSpotLight

====================
*/
void CBSPRenderer::SetupSpotLight( void )
{
	glBlendFunc(GL_ONE, GL_ONE);

	GLfloat planeS1D[4];
	planeS1D[0] = m_vCurSpotForward[0] / m_pCurrentDynLight->radius;
	planeS1D[1] = m_vCurSpotForward[1] / m_pCurrentDynLight->radius;
	planeS1D[2] = m_vCurSpotForward[2] / m_pCurrentDynLight->radius;
	planeS1D[3] = - DotProduct(m_vCurSpotForward, m_vCurDLightOrigin) / m_pCurrentDynLight->radius;

	// enable automatic texture coordinates generation
	GLfloat planeS[] = {1.0, 0.0, 0.0, 0.0};
	GLfloat planeT[] = {0.0, 1.0, 0.0, 0.0};
	GLfloat planeR[] = {0.0, 0.0, 1.0, 0.0};
	GLfloat planeQ[] = {0.0, 0.0, 0.0, 1.0};

	m_bLightShadow = LightCanShadow();
	float flSize = tan((M_PI/360) * m_pCurrentDynLight->cone_size);
	float flFrustum[] = { 2/(flSize*2), 0, 0, 0, 0, 2/(flSize*2), 0, 0, 0, 0, -1, -1, 0, 0, -2, 0 };

	int bReversed = IsPitchReversed(m_pCurrentDynLight->angles[PITCH]);
	vec3_t vTarget = m_vCurDLightOrigin + (m_vCurSpotForward * m_pCurrentDynLight->radius);

	// setup texture stages
	if(m_bLightShadow) SetTexEnvs(ENVSTATE_MUL, ENVSTATE_MUL, ENVSTATE_MUL);
	else SetTexEnvs(ENVSTATE_MUL, ENVSTATE_MUL);

	// Set color
	glColor4f(m_pCurrentDynLight->color.x, m_pCurrentDynLight->color.y, m_pCurrentDynLight->color.z, 1.0);

	// spotlight texture
	glActiveTextureARB( GL_TEXTURE0_ARB );
	Bind2DTexture(GL_TEXTURE0_ARB, m_pCurrentDynLight->textureindex);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
	glTexGenfv(GL_S, GL_EYE_PLANE, planeS);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
	glTexGenfv(GL_T, GL_EYE_PLANE, planeT);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
	glTexGenfv(GL_R, GL_EYE_PLANE, planeR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
	glTexGenfv(GL_Q, GL_EYE_PLANE, planeQ);

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);

	// load texture projection matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glTranslatef(0.5, 0.5, 0.5);
	glScalef(0.5, 0.5, 0.5);
	glMultMatrixf(flFrustum);

	MyLookAt(m_vCurDLightOrigin[0], m_vCurDLightOrigin[1], m_vCurDLightOrigin[2], vTarget[0], vTarget[1], vTarget[2], 0, 0, bReversed ? -1 : 1);
	glMatrixMode(GL_MODELVIEW);

	// attenuation
	glActiveTextureARB( GL_TEXTURE1_ARB );
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, m_iAttenuation1DTexture);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
	glTexGenfv(GL_S, GL_EYE_PLANE, planeS1D);
	glEnable(GL_TEXTURE_GEN_S);

	if(m_bLightShadow)
	{
		// depth texture
		glActiveTextureARB( GL_TEXTURE2_ARB );
		Bind2DTexture(GL_TEXTURE2_ARB, m_pCurrentDynLight->depth);

		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
		glTexGenfv(GL_S, GL_EYE_PLANE, planeS);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
		glTexGenfv(GL_T, GL_EYE_PLANE, planeT);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
		glTexGenfv(GL_R, GL_EYE_PLANE, planeR);
		glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
		glTexGenfv(GL_Q, GL_EYE_PLANE, planeQ);

		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_TEXTURE_GEN_Q);

		// load texture projection matrix
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();

		glTranslatef(0.5, 0.5, 0.5);
		glScalef(0.5, 0.5, 0.5);

		glMultMatrixf(flFrustum);
		MyLookAt(m_vCurDLightOrigin[0], m_vCurDLightOrigin[1], m_vCurDLightOrigin[2], vTarget[0], vTarget[1], vTarget[2], 0, 0, bReversed ? -1 : 1);
		glMatrixMode(GL_MODELVIEW);
		
		if(m_pCvarPCFShadows->value >= 1 && m_bShadowPCFSupport)
		{
			glEnable(GL_FRAGMENT_PROGRAM_ARB);
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_iShadowFragmentID);
		}
	}
}

/*
====================
FinishSpotLight

====================
*/
void CBSPRenderer::FinishSpotLight( void )
{
	glActiveTextureARB( GL_TEXTURE0_ARB );
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);

	glActiveTextureARB( GL_TEXTURE1_ARB );
	glDisable(GL_TEXTURE_1D);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);

	if(m_bLightShadow)
	{
		if(m_pCvarPCFShadows->value >= 1 && m_bShadowPCFSupport)
			glDisable(GL_FRAGMENT_PROGRAM_ARB);

		glActiveTextureARB(GL_TEXTURE2_ARB);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		
		glMatrixMode(GL_MODELVIEW);

		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_GEN_Q);
	}
}


/*
====================
DrawDynamicLightsForDetails

====================
*/
void CBSPRenderer::DrawDynamicLightsForDetails( void )
{
	if(!m_bShaderSupport || m_pCvarWorldShaders->value <= 0)
		return;

	if(m_pCvarDynamic->value < 1)
		return;

	if(!m_bSecondPassNeeded)
		return;

	if(gHUD.m_pFogSettings.active)
		glDisable(GL_FOG);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);

	// Set this
	m_pCurrentEntity = gEngfuncs.GetEntityByIndex(0);

	float time = gEngfuncs.GetClientTime();
	cl_dlight_t *dl	= m_pDynLights;

	for (int i = 0; i < MAX_DYNLIGHTS; i++, dl++)
	{
		if (dl->die < time || !dl->radius)
			continue;

		m_pCurrentDynLight = dl;
		m_vCurDLightOrigin = m_pCurrentDynLight->origin;

		if(dl->cone_size)
		{
			vec3_t lightangles = m_pCurrentDynLight->angles;
			FixVectorForSpotlight(lightangles);
			AngleVectors(lightangles, m_vCurSpotForward, NULL, NULL);
			SetupSpotLight();
		}
		else
		{
			SetupDynLight();
			SetDynLightBBox();
		}

		detailobject_t *pObject = m_pDetailObjects;
		for(int k = 0; k < m_iNumDetailObjects; k++, pObject++)
		{
			if(pObject->visframe != m_iFrameCount)
				continue;

			if(dl->cone_size)
			{
				if (dl->frustum.CullBox(pObject->mins, pObject->maxs))
					continue;			
			}
			else
			{
				if (CullDynLightBBox(pObject->mins, pObject->maxs))
					continue;		
			}

			msurface_t *psurf = m_pDetailObjectSurfaces + pObject->firstsurface;
			for (int i = 0; i < pObject->numsurfaces; i++, psurf++)
			{
				if (psurf->visframe == m_iFrameCount) // visible
				{
					float dot;
					mplane_t *pplane = psurf->plane;
					SSEDotProductSub(&dot, &m_vCurDLightOrigin, &pplane->normal, &pplane->dist);
					if ( ((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
						(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)) )
					{
						if(dot < m_pCurrentDynLight->radius)
							DrawPolyFromArray(m_pDetailObjectSurfaces, psurf);
					}
				}
			}
		}

		if(dl->cone_size)
			FinishSpotLight();
		else
			FinishDynLight();
	}

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	if(gHUD.m_pFogSettings.active)
		glEnable(GL_FOG);
}


/*
====================
DrawDynamicLightsForWorld

====================
*/
void CBSPRenderer::DrawDynamicLightsForWorld( void )
{
	if(!m_bShaderSupport || m_pCvarWorldShaders->value <= 0)
		return;

	if(m_pCvarDynamic->value < 1)
		return;

	if(!m_bSecondPassNeeded)
		return;

	if(gHUD.m_pFogSettings.active)
		glDisable(GL_FOG);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);

	// Set this
	m_pCurrentEntity = gEngfuncs.GetEntityByIndex(0);

	float time = gEngfuncs.GetClientTime();
	cl_dlight_t *dl	= m_pDynLights;

	for (int i = 0; i < MAX_DYNLIGHTS; i++, dl++)
	{
		if (dl->die < time || !dl->radius)
			continue;

		m_pCurrentDynLight = dl;
		m_vCurDLightOrigin = m_pCurrentDynLight->origin;

		if(dl->cone_size)
		{
			vec3_t lightangles = m_pCurrentDynLight->angles;
			FixVectorForSpotlight(lightangles);
			AngleVectors(lightangles, m_vCurSpotForward, NULL, NULL);
			SetupSpotLight();
		}
		else
		{
			SetupDynLight();
			SetDynLightBBox();
		}

		RecursiveWorldNodeLight(m_pWorld->nodes);

		for(int i = 0; i < m_iNumRenderEntities; i++)
		{
			if((m_pRenderEntities[i]->curstate.renderfx != 70) && !IsEntityMoved(m_pRenderEntities[i]) 
				&& !IsEntityTransparent(m_pRenderEntities[i]) && m_pRenderEntities[i]->visframe == m_iFrameCount)
			{
				if(dl->cone_size)
				{
					if(dl->frustum.CullBox(m_pRenderEntities[i]->curstate.mins, m_pRenderEntities[i]->curstate.maxs))
						continue;
				}
				else
				{
					if(CullDynLightBBox(m_pRenderEntities[i]->curstate.mins, m_pRenderEntities[i]->curstate.maxs))
						continue;
				}

				DrawEntityFacesForLight(m_pRenderEntities[i]);
			}
		}
			
		if(dl->cone_size)
			FinishSpotLight();
		else
			FinishDynLight();
	}

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	if(gHUD.m_pFogSettings.active)
		glEnable(GL_FOG);
}

/*
====================
RecursiveWorldNodeLight

====================
*/
void CBSPRenderer::RecursiveWorldNodeLight( mnode_t *node )
{
	int side;
	float dot;

	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	if (node->visframe != m_iVisFrame)
		return;

	// buz: visible surfaces already marked
	if (node->contents < 0)
		return;

	if (!m_pCurrentDynLight->cone_size)
	{
		if (CullDynLightBBox (node->minmaxs, node->minmaxs+3)) // cull from point light bbox
			return;
	}
	else
	{
		if(m_pCurrentDynLight->frustum.CullBox(node->minmaxs, node->minmaxs+3))
			return;
	}
// node is just a decision point, so go down the apropriate sides
// find which side of the node we are on

	mplane_t *plane = node->plane;
	switch (plane->type)
	{
	case PLANE_X:
		dot = m_vCurDLightOrigin[0] - plane->dist;	break;
	case PLANE_Y:
		dot = m_vCurDLightOrigin[1] - plane->dist;	break;
	case PLANE_Z:
		dot = m_vCurDLightOrigin[2] - plane->dist;	break;
	default:
		SSEDotProductSub(&dot, &m_vCurDLightOrigin, &plane->normal, &plane->dist); 
		break;
	}

	if (dot >= 0) 
		side = 0;
	else 
		side = 1;

// recurse down the children, front side first
	RecursiveWorldNodeLight (node->children[side]);

// draw stuff
	int c = node->numsurfaces;
	if (c)
	{
		msurface_t *surf = m_pWorld->surfaces + node->firstsurface;

		if (dot < 0 -BACKFACE_EPSILON)
			side = SURF_PLANEBACK;
		else if (dot > BACKFACE_EPSILON)
			side = 0;

		for ( ; c ; c--, surf++)
		{
			if (surf->visframe != m_iFrameCount)
				continue;

			if (surf->flags & SURF_DRAWTURB)
				continue;
					
			if (surf->flags & SURF_DRAWSKY)
				continue;

			// don't backface underwater surfaces, because they warp
			if ( !(surf->flags & SURF_UNDERWATER) && ( (dot < 0) ^ !!(surf->flags & SURF_PLANEBACK)) )
				continue;		// wrong side

			mplane_t *pplane = surf->plane;
			SSEDotProductSub(&dot, &m_vCurDLightOrigin, &surf->plane->normal, &surf->plane->dist);

			if(dot < m_pCurrentDynLight->radius)
				DrawPolyFromArray(m_pWorld->surfaces, surf);
		}
	}

// recurse down the back side
	RecursiveWorldNodeLight (node->children[!side]);
}

/*
====================
DynamicLighted

====================
*/
bool CBSPRenderer::DynamicLighted( const vec3_t &vmins, const vec3_t &vmaxs )
{
	if(!m_bSecondPassNeeded)
		return false;

	if(IsEntityTransparent(m_pCurrentEntity))
		return false;

	float time = gEngfuncs.GetClientTime();
	cl_dlight_t *dl = m_pDynLights;

	for(int i = 0; i < MAX_DYNLIGHTS; i++)
	{
		if(dl[i].die < time || !dl[i].radius)
			continue;

		m_pCurrentDynLight = &dl[i];
		m_vCurDLightOrigin = m_pCurrentDynLight->origin;

		if(m_pCurrentDynLight->cone_size)
		{
			if(m_pCurrentDynLight->frustum.CullBox((const vec3_t)vmins, (const vec3_t)vmaxs))
				continue;
		}
		else
		{
			SetDynLightBBox();
			if(CullDynLightBBox(vmins, vmaxs))
				continue;
		}
		
		return true;
	}

	return false;
}

/*
====================
DrawDynamicLightsForEntity

====================
*/
void CBSPRenderer::DrawDynamicLightsForEntity (cl_entity_t *pEntity)
{
	vec3_t		mins, maxs;
	int			rotated;

	if(!m_bShaderSupport || m_pCvarWorldShaders->value <= 0)
		return;

	if(!m_bSecondPassNeeded)
		return;

	if(m_pCvarDynamic->value < 1)
		return;

	if (pEntity->angles[0] || pEntity->angles[1] || pEntity->angles[2])
	{
		rotated = true;
		for (int i = 0; i < 3; i++)
		{
			mins[i] = - pEntity->model->radius;
			maxs[i] = + pEntity->model->radius;
		}
	}
	else
	{
		rotated = false;
		mins = pEntity->model->mins;
		maxs = pEntity->model->maxs;
	}

	float time = gEngfuncs.GetClientTime();
	cl_dlight_t *dl = m_pDynLights;

	if(gHUD.m_pFogSettings.active)
		glDisable(GL_FOG);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);

	for (int l = 0; l < MAX_DYNLIGHTS; l++, dl++)
	{
		vec3_t temp, forward, right, up;

		if (dl->die < time || !dl->radius)
			continue;

		m_pCurrentDynLight = dl;

		VectorSubtract (m_pCurrentDynLight->origin, pEntity->origin, m_vCurDLightOrigin);
		if (rotated)
		{
			AngleVectors (pEntity->angles, forward, right, up);

			VectorCopy (m_vCurDLightOrigin, temp);
			m_vCurDLightOrigin[0] = DotProduct (temp, forward);
			m_vCurDLightOrigin[1] = -DotProduct (temp, right);
			m_vCurDLightOrigin[2] = DotProduct (temp, up);
		}

		if(m_pCurrentDynLight->cone_size)
		{
			vec3_t tmins, tmaxs;
			VectorAdd(mins, m_pCurrentEntity->origin, tmins);
			VectorAdd(maxs, m_pCurrentEntity->origin, tmaxs);

			if(m_pCurrentDynLight->frustum.CullBox(tmins, tmaxs))
				continue;

			AngleVectors(m_pCurrentDynLight->angles, m_vCurSpotForward, NULL, NULL);
			if (rotated)
			{
				VectorCopy (m_vCurSpotForward, temp);
				m_vCurSpotForward[0] = DotProduct (temp, forward);
				m_vCurSpotForward[1] = -DotProduct (temp, right);
				m_vCurSpotForward[2] = DotProduct (temp, up);
			}

			SetupSpotLight();
		}
		else
		{

			SetDynLightBBox();
			if (CullDynLightBBox (mins, maxs))
				continue;

			SetupDynLight();
		}

		DrawEntityFacesForLight(pEntity);

		if(dl->cone_size)
			FinishSpotLight();
		else
			FinishDynLight();
	}

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	if(gHUD.m_pFogSettings.active)
		glEnable(GL_FOG);
}

/*
====================
DrawEntityFacesForLight

====================
*/
void CBSPRenderer::DrawEntityFacesForLight( cl_entity_t *pEntity )
{
	float dot;
	msurface_t *psurf = &m_pWorld->surfaces[pEntity->model->firstmodelsurface];
	for (int i = 0; i < pEntity->model->nummodelsurfaces; i++, psurf++)
	{
		if (psurf->flags & SURF_DRAWTURB)
			continue;

		if (psurf->flags & SURF_DRAWSKY)
			continue;

		if (psurf->visframe == m_iFrameCount) // visible
		{
			mplane_t *pplane = psurf->plane;
			SSEDotProductSub(&dot, &m_vCurDLightOrigin, &pplane->normal, &pplane->dist);
			if ( ((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
				(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)) )
			{
				if(dot < m_pCurrentDynLight->radius)
					DrawPolyFromArray(m_pWorld->surfaces, psurf);
			}
		}
	}
}

/*
====================
InitSky

====================
*/
void CBSPRenderer::InitSky( void )
{
	m_bDrawSky = true;
	if (m_szSkyName[0] == 0)
	{
		m_bDrawSky = false;
		return;
	}

	static char* szSkySuffixes[] = {"lf", "bk", "rt", "ft", "dn", "up"};

	for (int i = 0; i < 6; i++)
	{
		char szPathS[64];
		char szPathL[64];

		sprintf(szPathL, "gfx/env/%s%s_large.dds", m_szSkyName, szSkySuffixes[i]);
		sprintf(szPathS, "gfx/env/%s%s.tga", m_szSkyName, szSkySuffixes[i]);

		cl_texture_t *pTexture = gTextureLoader.LoadTexture(szPathL, FALSE, false, true);

		if(pTexture)
		{
			m_iSkyTextures[i] = pTexture->iIndex;
		}
		else
		{
			pTexture = gTextureLoader.LoadTexture(szPathS, FALSE, true, true);

			if(!pTexture)
			{
				m_bDrawSky = false;
				return;
			}

			m_iSkyTextures[i] = pTexture->iIndex;
		}

		// ALWAYS Bind
		glBindTexture(GL_TEXTURE_2D, pTexture->iIndex);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	}
};


/*
====================
DrawSky

====================
*/
void CBSPRenderer::DrawSky( void )
{
	fog_settings_t pSaved;
	static float projection[16];

	if ( !m_bDrawSky )
		return;

	if(gHUD.m_pSkyFogSettings.active)
	{
		memcpy(&pSaved, &gHUD.m_pFogSettings, sizeof(fog_settings_t));
		memcpy(&gHUD.m_pFogSettings, &gHUD.m_pSkyFogSettings, sizeof(fog_settings_t));
		gHUD.m_pFogSettings.end = gHUD.m_pFogSettings.end/m_fSkySpeed;
		gHUD.m_pFogSettings.start = gHUD.m_pFogSettings.start/m_fSkySpeed;
		ClearToFogColor();
		RenderFog();
	}

	if ( gHUD.m_pFogSettings.active )
	{
		if(!gHUD.m_pFogSettings.affectsky)
			glDisable(GL_FOG);
	}

	if(m_bMirroring)	
	{
		glMatrixMode(GL_PROJECTION);
		glGetFloatv(GL_PROJECTION_MATRIX, projection);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	if ( !gHUD.m_pFogSettings.affectsky || !gHUD.m_pFogSettings.active )
	{
		glDisable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);

		SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_OFF, ENVSTATE_OFF, ENVSTATE_OFF);

		vec3_t m_vPoints[8];
		m_vPoints[0] = m_vRenderOrigin + Vector(0, -10, 0) - Vector(10, 0, 0) + Vector(0, 0, -10);
		m_vPoints[1] = m_vRenderOrigin + Vector(0, -10, 0) + Vector(10, 0, 0) + Vector(0, 0, -10);
		m_vPoints[2] = m_vRenderOrigin - Vector(0, -10, 0) + Vector(10, 0, 0) + Vector(0, 0, -10);
		m_vPoints[3] = m_vRenderOrigin - Vector(0, -10, 0) - Vector(10, 0, 0) + Vector(0, 0, -10);
		m_vPoints[4] = m_vRenderOrigin + Vector(0, -10, 0) - Vector(10, 0, 0) - Vector(0, 0, -10);
		m_vPoints[5] = m_vRenderOrigin + Vector(0, -10, 0) + Vector(10, 0, 0) - Vector(0, 0, -10);
		m_vPoints[6] = m_vRenderOrigin - Vector(0, -10, 0) + Vector(10, 0, 0) - Vector(0, 0, -10);
		m_vPoints[7] = m_vRenderOrigin - Vector(0, -10, 0) - Vector(10, 0, 0) - Vector(0, 0, -10);

		int m_iIDs[6][4] = {
			{1, 2, 6, 5},{2, 3, 7, 6},{3, 0, 4, 7},
			{0, 1, 5, 4},{2, 1, 0, 3},{7, 4, 5, 6}};

		glDepthMask(GL_FALSE);
		for (int i = 0; i < 6; i++)
		{
			Bind2DTexture(GL_TEXTURE0_ARB, m_iSkyTextures[i]);
			glBegin(GL_POLYGON);
				glTexCoord2i(0, 1);
				glVertex3fv(m_vPoints[m_iIDs[i][0]]);
				glTexCoord2i(1, 1);
				glVertex3fv(m_vPoints[m_iIDs[i][1]]);
				glTexCoord2i(1, 0);
				glVertex3fv(m_vPoints[m_iIDs[i][2]]);
				glTexCoord2i(0, 0);
				glVertex3fv(m_vPoints[m_iIDs[i][3]]);
			glEnd();
		}
		glDepthMask(GL_TRUE);
	}

	if(gHUD.m_pFogSettings.active)
		glEnable(GL_FOG);

	//Render all skybox solid ents
	EnableVertexArray();
	for(int i = 0; i < m_iNumRenderEntities; i++)
	{
		if(m_pRenderEntities[i]->curstate.renderfx == 70)
			DrawBrushModel(m_pRenderEntities[i], false);
	}

	ResetRenderer();
	DisableVertexArray();

	//Render all skybox prop entities
	gPropManager.RenderSkyProps();

	//Clear depth buffer for the final time
	glClear(GL_DEPTH_BUFFER_BIT);

	if(gHUD.m_pSkyFogSettings.active)
	{
		memcpy(&gHUD.m_pFogSettings, &pSaved, sizeof(fog_settings_t));
		RenderFog();
	}

	if(m_bMirroring)	
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixf(projection);

		glMatrixMode(GL_MODELVIEW);
	}
};

/*
====================
CL_AllocDLight

====================
*/
cl_dlight_t *CBSPRenderer::CL_AllocDLight( int key )
{
	int		i;
	cl_dlight_t	*dl;
	float time = gEngfuncs.GetClientTime();

// first look for an exact key match
	if (key)
	{
		dl = m_pDynLights;
		for (i = 0; i < MAX_DYNLIGHTS; i++, dl++)
		{
			if (dl->key == key)
			{
				GLuint idepth = dl->depth;
				memset (dl, 0, sizeof(*dl));
				dl->key = key; dl->depth = idepth;
				return dl;
			}
		}
	}

// then look for anything else
	dl = m_pDynLights;
	for (i = 0; i < MAX_DYNLIGHTS; i++, dl++)
	{
		if (dl->die < time)
		{
			GLuint idepth = dl->depth;
			memset (dl, 0, sizeof(*dl));
			dl->key = key; dl->depth = idepth;
			return dl;
		}
	}

	dl = &m_pDynLights[0];
	GLuint idepth = dl->depth;
	memset (dl, 0, sizeof(*dl));
	dl->key = key; dl->depth = idepth;
	return dl;
}

/*
====================
DecayLights

====================
*/
void CBSPRenderer::DecayLights( void )
{
	static float lasttime = 0;

	float time = gEngfuncs.GetClientTime();
	float frametime = time - lasttime;

	if (frametime > 1) 
		frametime = 1;

	if (frametime < 0) 
		frametime = 0;

	lasttime = time;

	cl_dlight_t *dl = m_pDynLights;
	for (int i = 0; i < MAX_DYNLIGHTS; i++, dl++)
	{
		if (dl->die < time || !dl->radius)
			continue;
		
		dl->radius -= frametime*dl->decay;

		if (dl->radius < 0)
			dl->radius = 0;
	}
}

/*
====================
MsgSkyMarker_Sky

====================
*/
int CBSPRenderer::MsgSkyMarker_Sky(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	m_vSkyOrigin.x = READ_COORD();
	m_vSkyOrigin.y = READ_COORD();
	m_vSkyOrigin.z = READ_COORD();

	gHUD.m_pSkyFogSettings.affectsky = true;
	gHUD.m_pSkyFogSettings.end = READ_SHORT();
	gHUD.m_pSkyFogSettings.start = READ_SHORT();
	gHUD.m_pSkyFogSettings.color.x = (float)READ_BYTE()/255;
	gHUD.m_pSkyFogSettings.color.y = (float)READ_BYTE()/255;
	gHUD.m_pSkyFogSettings.color.z = (float)READ_BYTE()/255;
	gHUD.m_pSkyFogSettings.affectsky = (READ_SHORT() == 1) ? false : true;

	if(gHUD.m_pSkyFogSettings.end < 1 && gHUD.m_pSkyFogSettings.start < 1)
		gHUD.m_pSkyFogSettings.active = false;
	else
		gHUD.m_pSkyFogSettings.active = true;

	return 1;
}

/*
====================
MsgSkyMarker_World

====================
*/
int CBSPRenderer::MsgSkyMarker_World(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	m_vSkyWorldOrigin.x = READ_COORD();
	m_vSkyWorldOrigin.y = READ_COORD();
	m_vSkyWorldOrigin.z = READ_COORD();
	m_fSkySpeed	= READ_COORD();
	return 1;
}

/*
====================
MsgDynLight

====================
*/
int CBSPRenderer::MsgDynLight(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	cl_dlight_t *dl = CL_AllocDLight(0);

	dl->origin.x = READ_COORD();
	dl->origin.y = READ_COORD();
	dl->origin.z = READ_COORD();
	dl->radius = READ_BYTE()*10;
	dl->color.x = (float)READ_BYTE()/255;
	dl->color.y = (float)READ_BYTE()/255;
	dl->color.z = (float)READ_BYTE()/255;
	dl->die = READ_BYTE() + gEngfuncs.GetClientTime();
	dl->decay = READ_BYTE()*10;
	return 1;
}

/*
====================
DrawShadowPasses

====================
*/
void CBSPRenderer::DrawShadowPasses( void )
{
	if(!m_bShaderSupport || m_pCvarWorldShaders->value <= 0)
		return;

	if(m_pCvarDynamic->value < 1)
		return;

	if(!m_bShadowSupport || m_pCvarShadows->value < 1)
		return;

	float time = gEngfuncs.GetClientTime();
	cl_dlight_t *dl	= m_pDynLights;

	R_SaveGLStates();
	RenderFog();

	for (int i = 0; i < MAX_DYNLIGHTS; i++, dl++)
	{
		if (dl->die < time || !dl->radius || !dl->cone_size || dl->noshadow)
			continue;

		m_pCurrentDynLight = dl;
		CreateShadowMap();
	}
	R_RestoreGLStates();
}

/*
====================
CreateShadowMap

====================
*/
void CBSPRenderer::CreateShadowMap( void )
{
	float flProj[16];
	float flModel[16];

	// Doing this otherwise fucks shit up
	glGetFloatv(GL_PROJECTION_MATRIX, flProj);
	glGetFloatv(GL_MODELVIEW_MATRIX, flModel);

	// Ahh I love polygonoffset
	glPolygonOffset ( 5, 0 );
	glEnable(GL_POLYGON_OFFSET_FILL);

	//Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
	glColorMask(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);

	glCullFace(GL_FRONT);
	glDisable(GL_BLEND);
	glDisable(GL_FOG);
	glShadeModel(GL_FLAT);

	glViewport(GL_ZERO, GL_ZERO, DEPTHMAP_RESOLUTION, DEPTHMAP_RESOLUTION);

	//Disable texturing
	SetTexEnvs(ENVSTATE_OFF, ENVSTATE_OFF, ENVSTATE_OFF, ENVSTATE_OFF);
	glActiveTextureARB(GL_TEXTURE0_ARB);

	float flSize = tan((M_PI/360) * m_pCurrentDynLight->cone_size);
	float flFrustum[] = {2/(flSize*2), 0, 0, 0, 0, 2/(flSize*2), 0, 0, 0, 0, -1, -1, 0, 0, -2, 0 };

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(flFrustum);

	// Asscawks
	vec3_t vAngles = m_pCurrentDynLight->angles;
	FixVectorForSpotlight(vAngles);
	AngleVectors(vAngles, m_vCurSpotForward, NULL, NULL);

	int bReversed = IsPitchReversed(m_pCurrentDynLight->angles[PITCH]);
	vec3_t vTarget = m_pCurrentDynLight->origin + (m_vCurSpotForward * m_pCurrentDynLight->radius);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	MyLookAt(m_pCurrentDynLight->origin[0], m_pCurrentDynLight->origin[1], m_pCurrentDynLight->origin[2], vTarget[0], vTarget[1], vTarget[2], 0, 0, bReversed ? -1 : 1);

	DrawWorldSolid();

	for(int i = 0; i < m_iNumRenderEntities; i++)
	{
		if(m_pRenderEntities[i]->model->type != mod_studio)
			continue;

		if(!m_pRenderEntities[i]->player)
		{
			g_StudioRenderer.m_pCurrentEntity = gBSPRenderer.m_pRenderEntities[i];
			g_StudioRenderer.StudioDrawModelSolid();
		}
	}

	gPropManager.RenderPropsSolid();

	// Save Depth Buffer
	glBindTexture(GL_TEXTURE_2D, m_pCurrentDynLight->depth);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 0, 0, DEPTHMAP_RESOLUTION, DEPTHMAP_RESOLUTION, 0);

	glViewport(GL_ZERO, GL_ZERO, ScreenWidth, ScreenHeight);
	glColorMask(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	glDisable(GL_POLYGON_OFFSET_FILL);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(flProj);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(flModel);
}

/*
====================
DrawWorldSolid

====================
*/
void CBSPRenderer::DrawWorldSolid( void )
{
	gHUD.viewFrustum.SetFrustum(m_pCurrentDynLight->angles, m_pCurrentDynLight->origin, m_pCurrentDynLight->cone_size, m_pCurrentDynLight->radius);
	VectorCopy(m_pCurrentDynLight->angles, m_vViewAngles);
	VectorCopy(m_pCurrentDynLight->origin, m_vRenderOrigin );
	m_pWorld = IEngineStudio.GetModelByIndex(1);

	// Advance frame count here
	m_iFrameCount++;

	// Render everything
	if ( m_pViewLeaf->contents != CONTENTS_SOLID )
		m_iVisFrame = m_pViewLeaf->visframe;
	else
		m_iVisFrame = -2;

	m_pCurrentEntity = gEngfuncs.GetEntityByIndex(0);
	VectorCopy(m_vRenderOrigin, m_vVecToEyes);

	EnableVertexArray();
	RecursiveWorldNodeSolid(m_pWorld->nodes);
	DrawDetailsSolid();

	if(g_StudioRenderer.m_pCvarDrawEntities->value)
	{
		for (int i = 0; i < m_iNumRenderEntities; i++)
		{
			if (!IsEntityTransparent(m_pRenderEntities[i]))
				DrawBrushModelSolid(m_pRenderEntities[i]);
		}
	}
	DisableVertexArray();
}

/*
====================
RecursiveWorldNodeSolid

====================
*/
void CBSPRenderer::RecursiveWorldNodeSolid(mnode_t *node)
{
	int			c, side;
	mplane_t	*plane;
	msurface_t	*surf, **mark;
	mleaf_t		*pleaf;
	float		dot;

	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	if (node->visframe != m_iVisFrame && m_iVisFrame != -2)
		return;

	if (gHUD.viewFrustum.CullBox(node->minmaxs, node->minmaxs+3))
		return;

// if a leaf node, draw stuff
	if (node->contents < 0)
	{
		pleaf = (mleaf_t *)node;
		mark = pleaf->firstmarksurface;
		c = pleaf->nummarksurfaces;

		if (c)
		{
			do
			{
				(*mark)->visframe = m_iFrameCount;
				mark++;
			} while (--c);
		}
		return;
	}

// node is just a decision point, so go down the apropriate sides
// find which side of the node we are on
	plane = node->plane;
	switch (plane->type)
	{
	case PLANE_X:
		dot = m_vRenderOrigin[0] - plane->dist;	break;
	case PLANE_Y:
		dot = m_vRenderOrigin[1] - plane->dist;	break;
	case PLANE_Z:
		dot = m_vRenderOrigin[2] - plane->dist;	break;
	default:
		SSEDotProductSub(&dot, &m_vRenderOrigin, &plane->normal, &plane->dist);
		break;
	}

	if (dot >= 0) 
		side = 0;
	else 
		side = 1;

// recurse down the children, front side first
	RecursiveWorldNodeSolid (node->children[side]);

// draw stuff
	c = node->numsurfaces;
	if (c)
	{
		surf = m_pWorld->surfaces + node->firstsurface;

		if (dot < 0 -BACKFACE_EPSILON)
			side = SURF_PLANEBACK;
		else if (dot > BACKFACE_EPSILON)
			side = 0;

		for(int i = 0; i < node->numsurfaces; i++, surf++)
		{
			if (surf->visframe != m_iFrameCount)
				continue;

			// don't backface underwater surfaces, because they warp
			if ( !(surf->flags & SURF_UNDERWATER) && ( (dot < 0) ^ !!(surf->flags & SURF_PLANEBACK)) )
				continue;// wrong side

			if ( surf->flags & SURF_DRAWSKY )
				continue;

			if( surf->flags & SURF_DRAWTURB )
				continue;

			DrawPolyFromArray(m_pWorld->surfaces, surf);
		}
	}

// recurse down the back side
	RecursiveWorldNodeSolid(node->children[!side]);
};


/*
====================
DrawDetailsSolid

====================
*/
void CBSPRenderer::DrawDetailsSolid( void )
{
	if(!m_iNumDetailObjects)
		return;

	detailobject_t *pCurObject = m_pDetailObjects;
	for(int i = 0; i < m_iNumDetailObjects; i++, pCurObject++)
	{
		int j = 0;
		for (j = 0; j < pCurObject->numleafs; j++)
			if (m_pPVS[pCurObject->leafnums[j] >> 3] & (1 << (pCurObject->leafnums[j]&7) ))
				break;

		if(j == pCurObject->numleafs)
			continue;

		if(gHUD.viewFrustum.CullBox(pCurObject->mins, pCurObject->maxs))
			continue;

		if(pCurObject->rendermode == kRenderTransAlpha)
		{
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.5);

			SetTexEnvs(ENVSTATE_REPLACE);
			SetTexPointer(0, TC_TEXTURE);
		}

		pCurObject->visframe = m_iFrameCount; // For dynlights
		msurface_t *psurf = m_pDetailObjectSurfaces + pCurObject->firstsurface;
		for(int j = 0; j < pCurObject->numsurfaces; j++, psurf++)
		{
			float dot;
			mplane_t *pplane = psurf->plane;
			SSEDotProductSub(&dot, &m_vVecToEyes, &pplane->normal, &pplane->dist);

			if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
				(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
			{
				if(pCurObject->rendermode == kRenderTransAlpha)
					Bind2DTexture(GL_TEXTURE0_ARB, psurf->texinfo->texture->gl_texturenum);

				DrawPolyFromArray(m_pDetailObjectSurfaces, psurf);
				psurf->visframe = m_iFrameCount;
			}
		}

		if(pCurObject->rendermode == kRenderTransAlpha)
		{
			glDisable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0);

			SetTexEnvs(ENVSTATE_OFF);
			SetTexPointer(0, TC_OFF);
		}
	}
}

/*
====================
DrawBrushModelSolid

====================
*/
void CBSPRenderer::DrawBrushModelSolid ( cl_entity_t *pEntity )
{
	vec3_t		mins, maxs;
	int			i;
	msurface_t	*psurf;
	float		dot;
	mplane_t	*pplane;
	bool		bRotated = false;

	m_pCurrentEntity = pEntity;
	model_t *pModel = m_pCurrentEntity->model;

	if( m_pCurrentEntity->model == m_pWorld || m_pCurrentEntity->model->type != mod_brush || m_pCurrentEntity->curstate.renderfx == 70)
		return;

	if( m_pCurrentEntity->curstate.rendermode != NULL && m_pCurrentEntity->curstate.renderamt == NULL)
		return;

	if (m_pCurrentEntity->angles[0] || m_pCurrentEntity->angles[1] || m_pCurrentEntity->angles[2])
	{
		bRotated = true;
		for (i=0 ; i<3 ; i++)
		{
			mins[i] = m_pCurrentEntity->origin[i] - pModel->radius;
			maxs[i] = m_pCurrentEntity->origin[i] + pModel->radius;
		}
	}
	else
	{
		VectorAdd (m_pCurrentEntity->origin, pModel->mins, mins);
		VectorAdd (m_pCurrentEntity->origin, pModel->maxs, maxs);
	}

	if (gHUD.viewFrustum.CullBox (mins, maxs))
		return;

	VectorSubtract (m_vRenderOrigin, m_pCurrentEntity->origin, m_vVecToEyes);

	if (bRotated)
	{
		vec3_t	temp;
		vec3_t	forward, right, up;

		VectorCopy (m_vVecToEyes, temp);
		AngleVectors (m_pCurrentEntity->angles, forward, right, up);
		DotProductSSE(&m_vVecToEyes[0], temp, forward);
		DotProductSSE(&m_vVecToEyes[1], temp, right); 
		DotProductSSE(&m_vVecToEyes[2], temp, up);
		m_vVecToEyes[1] = -m_vVecToEyes[1];
	}

	m_pCurrentEntity->visframe = m_iFrameCount;

    glPushMatrix();
m_pCurrentEntity->angles[0] = -m_pCurrentEntity->angles[0];
	R_RotateForEntity (m_pCurrentEntity);
m_pCurrentEntity->angles[0] = -m_pCurrentEntity->angles[0];

	if (m_pCurrentEntity->curstate.rendermode == kRenderTransAlpha)
	{
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.25);

		SetTexEnvs(ENVSTATE_REPLACE);
		SetTexPointer(0, TC_TEXTURE);
	}

	psurf = &m_pWorld->surfaces[pModel->firstmodelsurface];
	for (i = 0; i < pModel->nummodelsurfaces; i++, psurf++)
	{
		pplane = psurf->plane;
		SSEDotProductSub(&dot, &m_vVecToEyes, &pplane->normal, &pplane->dist);
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) 
			|| (!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
		{
			psurf->visframe = m_iFrameCount;

			if ( psurf->flags & SURF_DRAWSKY )
				continue;

			if( psurf->flags & SURF_DRAWTURB )
				continue;

			Bind2DTexture(GL_TEXTURE0_ARB, psurf->texinfo->texture->gl_texturenum);
			DrawPolyFromArray(m_pWorld->surfaces, psurf);
		}
	}

	if (m_pCurrentEntity->curstate.rendermode == kRenderTransAlpha)
	{
		SetTexEnvs(ENVSTATE_OFF);
		SetTexPointer(0, TC_OFF);
		glDisable(GL_ALPHA_TEST);
	}

	glPopMatrix();
}