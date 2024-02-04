/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

Water Shader
Written by Andrew Lucas
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

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "propmanager.h"
#include "particle_engine.h"
#include "bsprenderer.h"
#include "watershader.h"
#include "mirrormanager.h"

#include "r_efx.h"
#include "r_studioint.h"
#include "studio_util.h"
#include "event_api.h"
#include "event_args.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
extern CGameStudioModelRenderer g_StudioRenderer;

extern inline float sgn(float a);

//===========================================
//	ARB SHADERS
//===========================================
char water_vp_depth [] =
"!!ARBvp1.0"
"TEMP R0;"
"TEMP R1;"
"DP4 R0.x, vertex.position, program.local[0];"
"DP4 R0.y, vertex.position, program.local[1];"
"DP4 R0.z, vertex.position, program.local[2];"
"DP4 R0.w, vertex.position, program.local[3];"

"DP4 R1.x, R0, program.local[4];"
"DP4 R1.y, R0, program.local[5];"
"DP4 R1.z, R0, program.local[6];"
"DP4 R1.w, R0, program.local[7];"

"MUL result.texcoord[0].xy, vertex.texcoord, 0.0078125;"
"MOV result.texcoord[1], R1;"
"MOV result.texcoord[2].xyz, vertex.position;"

"MOV result.position, R1;"
"MOV result.fogcoord.x, R1.z;"
"END";

char water_vp_radial [] =
"!!ARBvp1.0"
"TEMP R0;"
"TEMP R1;"
"DP4 R0.x, vertex.position, program.local[0];"
"DP4 R0.y, vertex.position, program.local[1];"
"DP4 R0.z, vertex.position, program.local[2];"
"DP4 R0.w, vertex.position, program.local[3];"

"DP3 R1.x, R0, R0;"
"RSQ R1.x, R1.x;"
"RCP result.fogcoord.x, R1.x;"

"DP4 R1.x, R0, program.local[4];"
"DP4 R1.y, R0, program.local[5];"
"DP4 R1.z, R0, program.local[6];"
"DP4 R1.w, R0, program.local[7];"

"MUL result.texcoord[0].xy, vertex.texcoord, 0.0078125;"
"MOV result.texcoord[1], R1;"
"MOV result.texcoord[2].xyz, vertex.position;"
"MOV result.position, R1;"
"END";

char water_fp_aw_reg [] =
"!!ARBfp1.0"
"OPTION ARB_precision_hint_fastest;"
"PARAM c[8] = { program.local[0..3],"
"{ 1.3, 0.97000003, 0.5, 0 },"
"{ 0.2, 0.15000001, 0.13, 0.11 },"
"{ 0.17, 0.14, 0.16, 1 },"
"{ 0.23, 0.33333334 } };"
"TEMP R0;"
"TEMP R1;"
"TEMP R2;"
"MOV R1, c[5];"
"MAD R0.zw, R1.xyxy, c[3].x, fragment.texcoord[0].xyxy;"
"MAD R0.y, R1.w, c[3].x, fragment.texcoord[0];"
"MAD R0.x, R1.z, -c[3], fragment.texcoord[0];"
"TEX R1.xyz, R0, texture[0], 2D;"
"TEX R0.xyz, R0.zwzw, texture[0], 2D;"
"ADD R2.xyz, R0, R1;"
"MOV R0.xyz, c[6];"
"MAD R1.xy, R0.yzzw, -c[3].x, fragment.texcoord[0];"
"MAD R0.z, R0.x, c[3].x, fragment.texcoord[0].x;"
"TEX R0.xyz, R0.zwzw, texture[0], 2D;"
"ADD R0.xyz, R2, R0;"
"TEX R1.xyz, R1, texture[0], 2D;"
"ADD R0.xyz, R0, R1;"
"ADD R1.xyz, -fragment.texcoord[2], c[0];"
"DP3 R1.w, R1, R1;"
"RSQ R1.w, R1.w;"
"MUL R0.xyz, R0, c[4].z;"
"ADD R0.xyz, R0, -c[6].w;"
"DP3 R0.z, R0, R0;"
"RSQ R0.z, R0.z;"
"MUL R0.xy, R0.z, R0;"
"MUL R2.xy, R0, c[7].x;"
"RCP R2.z, fragment.texcoord[1].w;"
"MUL R0.xy, R2.z, fragment.texcoord[1];"
"MAD R0.xy, R0, c[4].z, R2;"
"ADD R0.xy, R0, c[4].z;"
"MUL R1.z, R1.w, R1;"
"TEX R0, R0, texture[1], 2D;"
"MOV R1.y, -fragment.texcoord[1];"
"MOV R1.x, fragment.texcoord[1];"
"MUL R1.xy, R2.z, R1;"
"MAD R1.xy, R1, c[4].z, R2;"
"MUL R2.x, R1.z, c[2];"
"ADD R1.xy, R1, c[4].z;"
"TEX R1, R1, texture[2], 2D;"
"MUL R2.x, R2, c[4];"
"ADD R0, R0, -R1;"
"MIN_SAT R2.x, R2, c[4].y;"
"MAD R1, R2.x, R0, R1;"
"ADD R0.x, R1, R1.y;"
"ADD R0.x, R0, R1.z;"
"MUL R0.xyz, R0.x, c[1];"
"MOV R0.w, c[6];"
"MUL R0.xyz, R0, c[7].y;"
"ADD R0, R0, -R1;"
"MAD result.color, R0, c[5].x, R1;"
"END";

char water_fp_aw_fog [] =
"!!ARBfp1.0"
"OPTION ARB_precision_hint_fastest;"
"OPTION ARB_fog_linear;"
"PARAM c[8] = { program.local[0..3],"
"{ 1.3, 0.97000003, 0.5, 0 },"
"{ 0.2, 0.15000001, 0.13, 0.11 },"
"{ 0.17, 0.14, 0.16, 1 },"
"{ 0.23, 0.33333334 } };"
"TEMP R0;"
"TEMP R1;"
"TEMP R2;"
"MOV R1, c[5];"
"MAD R0.zw, R1.xyxy, c[3].x, fragment.texcoord[0].xyxy;"
"MAD R0.y, R1.w, c[3].x, fragment.texcoord[0];"
"MAD R0.x, R1.z, -c[3], fragment.texcoord[0];"
"TEX R1.xyz, R0, texture[0], 2D;"
"TEX R0.xyz, R0.zwzw, texture[0], 2D;"
"ADD R2.xyz, R0, R1;"
"MOV R0.xyz, c[6];"
"MAD R1.xy, R0.yzzw, -c[3].x, fragment.texcoord[0];"
"MAD R0.z, R0.x, c[3].x, fragment.texcoord[0].x;"
"TEX R0.xyz, R0.zwzw, texture[0], 2D;"
"ADD R0.xyz, R2, R0;"
"TEX R1.xyz, R1, texture[0], 2D;"
"ADD R0.xyz, R0, R1;"
"ADD R1.xyz, -fragment.texcoord[2], c[0];"
"DP3 R1.w, R1, R1;"
"RSQ R1.w, R1.w;"
"MUL R0.xyz, R0, c[4].z;"
"ADD R0.xyz, R0, -c[6].w;"
"DP3 R0.z, R0, R0;"
"RSQ R0.z, R0.z;"
"MUL R0.xy, R0.z, R0;"
"MUL R2.xy, R0, c[7].x;"
"RCP R2.z, fragment.texcoord[1].w;"
"MUL R0.xy, R2.z, fragment.texcoord[1];"
"MAD R0.xy, R0, c[4].z, R2;"
"ADD R0.xy, R0, c[4].z;"
"MUL R1.z, R1.w, R1;"
"TEX R0, R0, texture[1], 2D;"
"MOV R1.y, -fragment.texcoord[1];"
"MOV R1.x, fragment.texcoord[1];"
"MUL R1.xy, R2.z, R1;"
"MAD R1.xy, R1, c[4].z, R2;"
"MUL R2.x, R1.z, c[2];"
"ADD R1.xy, R1, c[4].z;"
"TEX R1, R1, texture[2], 2D;"
"MUL R2.x, R2, c[4];"
"ADD R0, R0, -R1;"
"MIN_SAT R2.x, R2, c[4].y;"
"MAD R1, R2.x, R0, R1;"
"ADD R0.x, R1, R1.y;"
"ADD R0.x, R0, R1.z;"
"MUL R0.xyz, R0.x, c[1];"
"MOV R0.w, c[6];"
"MUL R0.xyz, R0, c[7].y;"
"ADD R0, R0, -R1;"
"MAD result.color, R0, c[5].x, R1;"
"END";

char water_fp_uw_reg [] =
"!!ARBfp1.0"
"OPTION ARB_precision_hint_fastest;"
"PARAM c[6] = { program.local[0..1],"
"{ 0.5, 0, 0.2, 0.15000001 },"
"{ 0.13, 0.11, 0.17, 0.14 },"
"{ 0.16, 1, 0.30000001 },"
"{ 0.06666667, 0 } };"
"TEMP R0;"
"TEMP R1;"
"TEMP R2;"
"MOV R1, c[3];"
"MAD R2.xw, R1.yyzz, c[1].x, fragment.texcoord[0].yyzx;"
"MOV R0.zw, c[2];"
"MAD R0.zw, R0, c[1].x, fragment.texcoord[0].xyxy;"
"MAD R0.x, R1, -c[1], fragment.texcoord[0];"
"MOV R0.y, R2.x;"
"TEX R1.xyz, R0, texture[0], 2D;"
"TEX R0.xyz, R0.zwzw, texture[0], 2D;"
"ADD R2.xyz, R0, R1;"
"MOV R0.y, R0.w;"
"MOV R0.w, c[4].x;"
"MAD R1.x, R1.w, -c[1], fragment.texcoord[0];"
"MAD R1.y, R0.w, -c[1].x, fragment.texcoord[0];"
"MOV R0.x, R2.w;"
"TEX R0.xyz, R0, texture[0], 2D;"
"TEX R1.xyz, R1, texture[0], 2D;"
"ADD R0.xyz, R2, R0;"
"ADD R0.xyz, R0, R1;"
"MUL R0.xyz, R0, c[2].x;"
"ADD R0.xyz, R0, -c[4].y;"
"DP3 R0.z, R0, R0;"
"RSQ R0.z, R0.z;"
"MUL R0.zw, R0.z, R0.xyxy;"
"RCP R0.x, fragment.texcoord[1].w;"
"MUL R0.zw, R0, c[4].z;"
"MUL R0.xy, R0.x, fragment.texcoord[1];"
"MAD R0.xy, R0, c[2].x, R0.zwzw;"
"ADD R0.xy, R0, c[2].x;"
"TEX R0, R0, texture[1], 2D;"
"ADD R2.x, R0, R0.y;"
"MOV R1.w, c[4].y;"
"MOV R1.xyz, c[0];"
"ADD R1, -R0, R1;"
"ADD R2.x, R2, R0.z;"
"MUL R1, R2.x, R1;"
"MAD result.color, R1, c[5].x, R0;"
"END";

char water_fp_uw_fog [] =
"!!ARBfp1.0"
"OPTION ARB_precision_hint_fastest;"
"OPTION ARB_fog_linear;"
"PARAM c[6] = { program.local[0..1],"
"{ 0.5, 0, 0.2, 0.15000001 },"
"{ 0.13, 0.11, 0.17, 0.14 },"
"{ 0.16, 1, 0.30000001 },"
"{ 0.06666667, 0 } };"
"TEMP R0;"
"TEMP R1;"
"TEMP R2;"
"MOV R1, c[3];"
"MAD R2.xw, R1.yyzz, c[1].x, fragment.texcoord[0].yyzx;"
"MOV R0.zw, c[2];"
"MAD R0.zw, R0, c[1].x, fragment.texcoord[0].xyxy;"
"MAD R0.x, R1, -c[1], fragment.texcoord[0];"
"MOV R0.y, R2.x;"
"TEX R1.xyz, R0, texture[0], 2D;"
"TEX R0.xyz, R0.zwzw, texture[0], 2D;"
"ADD R2.xyz, R0, R1;"
"MOV R0.y, R0.w;"
"MOV R0.w, c[4].x;"
"MAD R1.x, R1.w, -c[1], fragment.texcoord[0];"
"MAD R1.y, R0.w, -c[1].x, fragment.texcoord[0];"
"MOV R0.x, R2.w;"
"TEX R0.xyz, R0, texture[0], 2D;"
"TEX R1.xyz, R1, texture[0], 2D;"
"ADD R0.xyz, R2, R0;"
"ADD R0.xyz, R0, R1;"
"MUL R0.xyz, R0, c[2].x;"
"ADD R0.xyz, R0, -c[4].y;"
"DP3 R0.z, R0, R0;"
"RSQ R0.z, R0.z;"
"MUL R0.zw, R0.z, R0.xyxy;"
"RCP R0.x, fragment.texcoord[1].w;"
"MUL R0.zw, R0, c[4].z;"
"MUL R0.xy, R0.x, fragment.texcoord[1];"
"MAD R0.xy, R0, c[2].x, R0.zwzw;"
"ADD R0.xy, R0, c[2].x;"
"TEX R0, R0, texture[1], 2D;"
"ADD R2.x, R0, R0.y;"
"MOV R1.w, c[4].y;"
"MOV R1.xyz, c[0];"
"ADD R1, -R0, R1;"
"ADD R2.x, R2, R0.z;"
"MUL R1, R2.x, R1;"
"MAD result.color, R1, c[5].x, R0;"
"END";
//===========================================
//	ARB SHADERS
//===========================================

/*
====================
Init

====================
*/
void CWaterShader::Init( void ) 
{
	// Set up cvar
	m_pCvarWaterShader = gEngfuncs.pfnRegisterVariable( "te_water", "1", FCVAR_ARCHIVE );
	m_pCvarWaterDebug = gEngfuncs.pfnRegisterVariable( "te_water_debug", "0", 0 );

	if(!gBSPRenderer.m_bShaderSupport)
		return;

	GLint iErrorPos, iIsNative;
	glEnable(GL_VERTEX_PROGRAM_ARB);
	gBSPRenderer.glGenProgramsARB(1, &m_uiVertexPrograms[0]); // Depth fog calculations
	gBSPRenderer.glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_uiVertexPrograms[0]);

	gBSPRenderer.glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, sizeof(water_vp_depth)-1, water_vp_depth);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &iErrorPos);
	gBSPRenderer.glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &iIsNative);
	glDisable(GL_VERTEX_PROGRAM_ARB);

	if(iErrorPos != -1 || !iIsNative)
	{
		gBSPRenderer.m_bShaderSupport = false;
		gBSPRenderer.m_bDontPromptShadersError = false;
		return;
	}

	glEnable(GL_VERTEX_PROGRAM_ARB);
	gBSPRenderer.glGenProgramsARB(1, &m_uiVertexPrograms[1]); // Depth fog calculations
	gBSPRenderer.glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_uiVertexPrograms[1]);

	gBSPRenderer.glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, sizeof(water_vp_radial)-1, water_vp_radial);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &iErrorPos);
	gBSPRenderer.glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &iIsNative);
	glDisable(GL_VERTEX_PROGRAM_ARB);

	if(iErrorPos != -1 || !iIsNative)
	{
		gBSPRenderer.m_bShaderSupport = false;
		gBSPRenderer.m_bDontPromptShadersError = false;
		return;
	}

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	gBSPRenderer.glGenProgramsARB(1, &m_uiFragmentPrograms[0]);
	gBSPRenderer.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_uiFragmentPrograms[0]);

	gBSPRenderer.glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, sizeof(water_fp_aw_reg)-1, water_fp_aw_reg);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &iErrorPos);
	gBSPRenderer.glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &iIsNative);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

	if(iErrorPos != -1 || !iIsNative)
	{
		gBSPRenderer.m_bShaderSupport = false;
		gBSPRenderer.m_bDontPromptShadersError = false;
		return;
	}
	
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	gBSPRenderer.glGenProgramsARB(1, &m_uiFragmentPrograms[1]);
	gBSPRenderer.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_uiFragmentPrograms[1]);

	gBSPRenderer.glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, sizeof(water_fp_aw_fog)-1, water_fp_aw_fog);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &iErrorPos);
	gBSPRenderer.glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &iIsNative);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

	if(iErrorPos != -1 || !iIsNative)
	{
		gBSPRenderer.m_bShaderSupport = false;
		gBSPRenderer.m_bDontPromptShadersError = false;
		return;
	}

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	gBSPRenderer.glGenProgramsARB(1, &m_uiFragmentPrograms[2]);
	gBSPRenderer.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_uiFragmentPrograms[2]);

	gBSPRenderer.glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, sizeof(water_fp_uw_reg)-1, water_fp_uw_reg);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &iErrorPos);
	gBSPRenderer.glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &iIsNative);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

	if(iErrorPos != -1 || !iIsNative)
	{
		gBSPRenderer.m_bShaderSupport = false;
		gBSPRenderer.m_bDontPromptShadersError = false;
		return;
	}
	
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	gBSPRenderer.glGenProgramsARB(1, &m_uiFragmentPrograms[3]);
	gBSPRenderer.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_uiFragmentPrograms[3]);

	gBSPRenderer.glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, sizeof(water_fp_uw_fog)-1, water_fp_uw_fog);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &iErrorPos);
	gBSPRenderer.glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &iIsNative);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

	if(iErrorPos != -1 || !iIsNative)
	{
		gBSPRenderer.m_bShaderSupport = false;
		gBSPRenderer.m_bDontPromptShadersError = false;
		return;
	}
}

/*
====================
ClearEntities

====================
*/
void CWaterShader::ClearEntities( void ) 
{
	if(!m_iNumWaterEntities)
		return;

	for(int i = 0; i < m_iNumWaterEntities; i++)
	{
		glDeleteTextures(1, &m_pWaterEntities[i].reflect);
		glDeleteTextures(1, &m_pWaterEntities[i].refract);
		free(m_pWaterEntities[i].surfaces);
	}

	memset(m_pWaterEntities, NULL, sizeof(m_pWaterEntities));
	m_iNumWaterEntities = NULL;
}

/*
====================
Shutdown

====================
*/
void CWaterShader::Shutdown( void ) 
{
	ClearEntities();
}

/*
====================
VidInit

====================
*/
void CWaterShader::VidInit( void ) 
{
	int iCurrentBinding;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &iCurrentBinding);

	// Load texture
	m_pNormalTexture = gTextureLoader.LoadTexture("gfx/textures/watershader.tga");
	glBindTexture(GL_TEXTURE_2D, iCurrentBinding);

	if(!m_pNormalTexture)
	{
		gEngfuncs.pfnClientCmd("escape\n");	
		MessageBox(NULL, "VIDEO ERROR: Could not load 'gfx/textures/watershader.tga'!\n", "ERROR", MB_OK);
		gEngfuncs.pfnClientCmd("quit\n");
	}

	ClearEntities();
}

/*
====================
Restore

====================
*/
void CWaterShader::Restore( void ) 
{
	if(m_pCvarWaterShader->value < 1)
		return;

	if(!gBSPRenderer.m_bShaderSupport)
		return;

	if(!m_iNumWaterEntities)
		return;

	if(!m_bViewInWater)
		return;

	// End of frame, so reset
	gHUD.m_pFogSettings = m_pMainFogSettings;
}

/*
====================
LoadScript

====================
*/
void CWaterShader::LoadScript( void ) 
{
	char szFile[64];
	char szMapName[64];
	const char *levelname = gEngfuncs.pfnGetLevelName();

	FilenameFromPath((char*)levelname, szMapName);
	sprintf(szFile, "scripts/water_%s.txt", szMapName);

	char *pFile = (char *)gEngfuncs.COM_LoadFile(szFile, 5, NULL);

	if(!pFile)
		pFile = (char *)gEngfuncs.COM_LoadFile("scripts/water_default.txt", 5, NULL);

	if(!pFile)
	{
		gEngfuncs.Con_Printf("Could not load default water definition file 'scripts/water_default.txt'!\n");
		
		memset(&m_pWaterFogSettings, 0, sizeof(fog_settings_t));
		m_flFresnelTerm = 1;
		return;
	}

	char *pToken = pFile;
	while(1)
	{
		char szField[32];
		char szValue[32];

		pToken = gEngfuncs.COM_ParseFile(pToken, szField);

		if(!pToken)
			break;

		pToken = gEngfuncs.COM_ParseFile(pToken, szValue);

		if(!pToken)
			break;
		
		if(!strcmp(szField, "fresnel"))
			m_flFresnelTerm = atof(szValue);
		else if(!strcmp(szField, "colr"))
			m_pWaterFogSettings.color[0] = atof(szValue)/255.0;
		else if(!strcmp(szField, "colg"))
			m_pWaterFogSettings.color[1] = atof(szValue)/255.0;
		else if(!strcmp(szField, "colb"))
			m_pWaterFogSettings.color[2] = atof(szValue)/255.0;
		else if(!strcmp(szField, "fogend"))
			m_pWaterFogSettings.end = atof(szValue);
		else if(!strcmp(szField, "fogstart"))
			m_pWaterFogSettings.start = atof(szValue);
		else
			gEngfuncs.Con_Printf("Unknown field: %s\n", szField);
	}
	gEngfuncs.COM_FreeFile(pFile);

	// always true
	m_pWaterFogSettings.affectsky = true;

	if(m_pWaterFogSettings.end < 1 && m_pWaterFogSettings.start < 1)
		m_pWaterFogSettings.active = false;
	else
		m_pWaterFogSettings.active = true;

	if(m_flFresnelTerm <= 0)
		m_flFresnelTerm = 1;
}

/*
====================
ShouldReflect

====================
*/
bool CWaterShader::ShouldReflect( int index ) 
{
	if(GetWaterOrigin().z > m_vViewOrigin.z)
		return false;

	// Optimization: Try and find a water entity on the same z coord
	for(int i = 0; i < index; i++)
	{
		if(m_pWaterEntities[i].draw)
		{
			if(GetWaterOrigin(&m_pWaterEntities[i]).z == GetWaterOrigin().z)
				return false;
		}
	}
	return true;
}

/*
====================
AddEntity

====================
*/
void CWaterShader::AddEntity( cl_entity_t *entity ) 
{
	if(m_iNumWaterEntities == MAX_WATER_ENTITIES)
		return;

	for(int i = 0; i < m_iNumWaterEntities; i++)
	{
		if(m_pWaterEntities[i].entity == entity)
			return;// Already in cache
	}

	cl_water_t *pWater = &m_pWaterEntities[m_iNumWaterEntities];
	pWater->index = m_iNumWaterEntities;
	m_iNumWaterEntities++;

	int isurfacecount = 0;
	msurface_t *psurfaces = entity->model->surfaces + entity->model->firstmodelsurface;
	for(int i = 0; i < entity->model->nummodelsurfaces; i++)
	{
		int j = 0;
		for(; j < psurfaces[i].polys->numverts; j++)
		{
			if(psurfaces[i].polys->verts[0][2] != (entity->curstate.maxs.z-1))
				break;
		}
		
		if(j != psurfaces[i].polys->numverts)
			continue;

		if(psurfaces[i].flags & SURF_PLANEBACK)
			continue;

		if (psurfaces[i].plane->normal[2] != 1)
			continue;

		isurfacecount++;
	}

	// Allocate array of pointers
	pWater->surfaces = (msurface_t **)malloc(sizeof(msurface_t *)*isurfacecount);

	for(int i = 0; i < entity->model->nummodelsurfaces; i++)
	{
		int j = 0;
		for(; j < psurfaces[i].polys->numverts; j++)
		{
			if(psurfaces[i].polys->verts[0][2] != (entity->curstate.maxs.z-1))
				break;
		}
		
		if(j != psurfaces[i].polys->numverts)
			continue;

		if(psurfaces[i].flags & SURF_PLANEBACK)
			continue;

		if (psurfaces[i].plane->normal[2] != 1)
			continue;

		pWater->surfaces[pWater->numsurfaces] = &psurfaces[i];
		pWater->numsurfaces++;
	}

	if(!pWater->numsurfaces)
	{
		memset(&m_pWaterEntities[m_iNumWaterEntities], 0, sizeof(cl_water_t));
		m_iNumWaterEntities--;
		return;
	}

	pWater->mins = Vector(9999, 9999, 9999);
	pWater->maxs = Vector(-9999, -9999, -9999);

	for(int i = 0; i < pWater->numsurfaces; i++)
	{
		for (glpoly_t *bp = pWater->surfaces[i]->polys; bp; bp = bp->next)
		{
			for(int j = 0; j < bp->numverts; j++)
			{
				if(pWater->mins[0] > bp->verts[j][0])
					pWater->mins[0] = bp->verts[j][0];

				if(pWater->mins[1] > bp->verts[j][1])
					pWater->mins[1] = bp->verts[j][1];

				if(pWater->mins[2] > bp->verts[j][2])
					pWater->mins[2] = bp->verts[j][2];

				if(pWater->maxs[0] < bp->verts[j][0])
					pWater->maxs[0] = bp->verts[j][0];

				if(pWater->maxs[1] < bp->verts[j][1])
					pWater->maxs[1] = bp->verts[j][1];

				if(pWater->maxs[2] < bp->verts[j][2])
					pWater->maxs[2] = bp->verts[j][2];
			}
		}
	}

	pWater->entity = entity;
	pWater->entity->efrag = (efrag_s *)pWater;

	pWater->wplane.dist = psurfaces->plane->dist;
	pWater->wplane.type = psurfaces->plane->type;
	pWater->wplane.pad[0] = psurfaces->plane->pad[0];
	pWater->wplane.pad[1] = psurfaces->plane->pad[1];
	pWater->wplane.signbits = psurfaces->plane->signbits;
	pWater->wplane.normal[2] = 1;

	pWater->reflect = current_ext_texture_id; current_ext_texture_id++;
	pWater->refract = current_ext_texture_id; current_ext_texture_id++;

	pWater->origin[0] = (pWater->mins[0] + pWater->maxs[0]) * 0.5f;
	pWater->origin[1] = (pWater->mins[1] + pWater->maxs[1]) * 0.5f;
	pWater->origin[2] = (pWater->mins[2] + pWater->maxs[2]) * 0.5f;

	glBindTexture(GL_TEXTURE_2D, pWater->reflect);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, pWater->refract);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

/*
====================
SetupClipping

====================
*/
void CWaterShader::SetupClipping( ref_params_t *pparams, bool negative ) 
{
	float	dot;
	float	eq1[4];
	float	eq2[4];
	float	projection[16];

	vec3_t	vDist;
	vec3_t	vNorm;

	vec3_t	vForward;
	vec3_t	vRight;
	vec3_t	vUp;

	AngleVectors(pparams->viewangles, vForward, vRight, vUp );
	VectorSubtract(GetWaterOrigin(), pparams->vieworg, vDist);
	
	VectorInverse(vRight); 
	VectorInverse(vUp);

	if(negative)
	{
		DotProductSSE(&eq1[0], vRight, -m_pCurWater->wplane.normal);
		DotProductSSE(&eq1[1], vUp, -m_pCurWater->wplane.normal);
		DotProductSSE(&eq1[2], vForward, -m_pCurWater->wplane.normal);
		DotProductSSE(&eq1[3], vDist, -m_pCurWater->wplane.normal);
	}
	else
	{
		DotProductSSE(&eq1[0], vRight, m_pCurWater->wplane.normal);
		DotProductSSE(&eq1[1], vUp, m_pCurWater->wplane.normal);
		DotProductSSE(&eq1[2], vForward, m_pCurWater->wplane.normal);
		DotProductSSE(&eq1[3], vDist, m_pCurWater->wplane.normal);
	}

	// Change current projection matrix into an oblique projection matrix
	glGetFloatv(GL_PROJECTION_MATRIX, projection);

	eq2[0] = (sgn(eq1[0]) + projection[8]) / projection[0];
	eq2[1] = (sgn(eq1[1]) + projection[9]) / projection[5];
	eq2[2] = -1.0F;
	eq2[3] = (1.0F + projection[10]) / projection[14];

	dot = eq1[0]*eq2[0] + eq1[1]*eq2[1] + eq1[2]*eq2[2] + eq1[3]*eq2[3];

    projection[2] = eq1[0]*(2.0f/dot);
    projection[6] = eq1[1]*(2.0f/dot);
    projection[10] = eq1[2]*(2.0f/dot) + 1.0F;
    projection[14] = eq1[3]*(2.0f/dot);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(projection);

	glMatrixMode(GL_MODELVIEW);
}

/*
====================
ViewInWater

====================
*/
bool CWaterShader::ViewInWater( void )
{
	Vector mins, maxs;
	for(int i = 0; i < 3; i++)
	{
		mins[i] = m_pCurWater->entity->curstate.origin[i] + m_pCurWater->entity->curstate.mins[i];
		maxs[i] = m_pCurWater->entity->curstate.origin[i] + m_pCurWater->entity->curstate.maxs[i];
	}

	if(m_vViewOrigin[0] > mins[0]
	&& m_vViewOrigin[1] > mins[1]
	&& m_vViewOrigin[2] > mins[2]
	&& m_vViewOrigin[0] < maxs[0]
	&& m_vViewOrigin[1] < maxs[1]
	&& m_vViewOrigin[2] < maxs[2])
		return true;

	return false;
}

/*
====================
DrawWaterPasses

====================
*/
void CWaterShader::DrawWaterPasses( ref_params_t *pparams ) 
{
	if(m_pCvarWaterShader->value < 1)
		return;

	if(!gBSPRenderer.m_bShaderSupport)
		return;

	if(!m_iNumWaterEntities)
		return;

	//Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
	
	m_iNumPasses = NULL;
	m_bViewInWater = false;
	m_pViewParams = pparams;
	m_pMainFogSettings = gHUD.m_pFogSettings;
	gBSPRenderer.m_bMirroring = true;

	VectorCopy(pparams->vieworg, m_vViewOrigin);
	memcpy(&m_pWaterParams, m_pViewParams, sizeof(ref_params_t));

	for(int i = 0; i < m_iNumWaterEntities; i++)
	{
		m_pCurWater = &m_pWaterEntities[i];

		if(!m_pCurWater->draw)
			continue;

		gHUD.viewFrustum.SetFrustum(pparams->viewangles, pparams->vieworg, gHUD.m_iFOV, gHUD.m_pFogSettings.end, true);
		if(gHUD.viewFrustum.CullBox(m_pCurWater->mins, m_pCurWater->maxs))
		{
			// YOU MUST DIE
			m_pCurWater->draw = false;
			continue;
		}

		SetupRefract();
		DrawScene(m_pViewParams, true);
		FinishRefract();

		if(ShouldReflect(i))
		{
			SetupReflect();
			DrawScene(&m_pWaterParams, false);
			FinishReflect();
		}
	}

	for(int i = 0; i < m_iNumWaterEntities; i++)
	{
		m_pCurWater = &m_pWaterEntities[i];

		if(ViewInWater())
		{
			gHUD.m_pFogSettings = m_pWaterFogSettings;
			m_bViewInWater = true;
			break;
		}
	}

	if(m_pCvarWaterDebug->value)
		gEngfuncs.Con_Printf("A total of %d passes drawn for water shader.\n", m_iNumPasses);

	gBSPRenderer.m_bMirroring = false;
	glViewport(GL_ZERO, GL_ZERO, ScreenWidth, ScreenHeight);
}

/*
====================
DrawScene

====================
*/
void CWaterShader::DrawScene( ref_params_t *pparams, bool isrefracting ) 
{
	// Set world renderer
	gBSPRenderer.RendererRefDef(pparams);

	// Draw world
	gBSPRenderer.DrawNormalTriangles();

	R_SaveGLStates();

	if((m_pCvarWaterShader->value > 1) || isrefracting)
	{
		for(int i = 0; i < gBSPRenderer.m_iNumRenderEntities; i++)
		{
			if(gBSPRenderer.m_pRenderEntities[i]->model->type != mod_studio || gBSPRenderer.m_pRenderEntities[i]->index == 0)
				continue;

			if(!gBSPRenderer.m_pRenderEntities[i]->player)
			{
				g_StudioRenderer.m_pCurrentEntity = gBSPRenderer.m_pRenderEntities[i];
				g_StudioRenderer.StudioDrawModel(STUDIO_RENDER);
			}
			else if(gBSPRenderer.m_pRenderEntities[i] != gEngfuncs.GetLocalPlayer())
			{
				entity_state_t *pPlayer = IEngineStudio.GetPlayerState((gBSPRenderer.m_pRenderEntities[i]->index-1));
				g_StudioRenderer.m_pCurrentEntity = gBSPRenderer.m_pRenderEntities[i];
				g_StudioRenderer.StudioDrawPlayer(STUDIO_RENDER, pPlayer);
			}
		}
	}

	if((m_pCvarWaterShader->value > 1) || isrefracting)
	{
		for(int i = 0; i < gBSPRenderer.m_iNumRenderEntities; i++)
		{
			if(gBSPRenderer.m_pRenderEntities[i]->model->type == mod_studio && gBSPRenderer.m_pRenderEntities[i]->index == 0)
			{
				g_StudioRenderer.m_pCurrentEntity = gBSPRenderer.m_pRenderEntities[i];
				g_StudioRenderer.StudioDrawModel(STUDIO_RENDER);
			}
		}
	}

	// Render any props
	gPropManager.RenderProps();

	// Render any transparent triangles
	gBSPRenderer.DrawTransparentTriangles();

	if((m_pCvarWaterShader->value > 1) || isrefracting)
		gParticleEngine.DrawParticles();

	if(m_pCvarWaterDebug->value)
	{
		if(isrefracting)
		{
			gEngfuncs.Con_Printf("Water No %d Refract: %d wpolys, %d epolys, %d studio polys drawn\n", 
				m_pCurWater->index, gBSPRenderer.m_iWorldPolyCounter, gBSPRenderer.m_iBrushPolyCounter,
				gBSPRenderer.m_iStudioPolyCounter);
		}
		else
		{
			gEngfuncs.Con_Printf("Water No %d Reflect: %d wpolys, %d epolys, %d studio polys drawn\n", 
				m_pCurWater->index, gBSPRenderer.m_iWorldPolyCounter, gBSPRenderer.m_iBrushPolyCounter,
				gBSPRenderer.m_iStudioPolyCounter);
		}
	}

	R_RestoreGLStates();
	m_iNumPasses++;
}

/*
====================
SetupRefract

====================
*/
void CWaterShader::SetupRefract( void ) 
{
	glCullFace(GL_FRONT);
	glColor4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90,  1, 0, 0);// put X going down
	glRotatef(90,  0, 0, 1); // put Z going up
	glRotatef(-m_pViewParams->viewangles[2], 1, 0, 0);
	glRotatef(-m_pViewParams->viewangles[0], 0, 1, 0);
	glRotatef(-m_pViewParams->viewangles[1], 0, 0, 1);
	glTranslatef(-m_vViewOrigin[0], -m_vViewOrigin[1], -m_vViewOrigin[2]);

	glViewport(GL_ZERO, GL_ZERO, WATER_RESOLUTION, WATER_RESOLUTION);

	if(GetWaterOrigin().z < m_vViewOrigin[2])
	{
		SetupClipping(m_pViewParams, false);

		gHUD.viewFrustum.SetExtraCullBox(m_pCurWater->entity->curstate.mins, m_pCurWater->entity->curstate.maxs);
		gHUD.m_pFogSettings = m_pWaterFogSettings;
	}
	else
	{
		vec3_t vMins, vMaxs;
		VectorCopy(gBSPRenderer.m_pWorld->maxs, vMaxs);
		VectorCopy(gBSPRenderer.m_pWorld->mins, vMins); 
		vMins.z = GetWaterOrigin().z;
		
		gHUD.viewFrustum.SetExtraCullBox(vMins, vMaxs);
		SetupClipping(m_pViewParams, true);
	}

	RenderFog();
}

/*
====================
FinishRefract

====================
*/
void CWaterShader::FinishRefract( void ) 
{
	//Save mirrored image
	glBindTexture(GL_TEXTURE_2D, m_pCurWater->refract);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, WATER_RESOLUTION, WATER_RESOLUTION, 0);

	//Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	//Restore modelview
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	gHUD.m_pFogSettings = m_pMainFogSettings;

	// Disable culling
	gHUD.viewFrustum.DisableExtraCullBox();
}

/*
====================
SetupReflect

====================
*/
void CWaterShader::SetupReflect( void ) 
{
	vec3_t vForward;
	vec3_t vMins, vMaxs;
	AngleVectors(m_pViewParams->viewangles, vForward, NULL, NULL);

	float flDist = abs(GetWaterOrigin().z - m_vViewOrigin[2]);
	VectorMASSE(m_vViewOrigin, -2*flDist, m_pCurWater->wplane.normal, m_pWaterParams.vieworg);

	flDist = DotProduct(vForward, -m_pCurWater->wplane.normal);
	VectorMASSE(vForward, -2*flDist, -m_pCurWater->wplane.normal, vForward);

	m_pWaterParams.viewangles[0] = -asin(vForward[2])/M_PI*180;
	m_pWaterParams.viewangles[1] = atan2(vForward[1], vForward[0])/M_PI*180;
	m_pWaterParams.viewangles[2] = -m_pViewParams->viewangles[2];

	AngleVectors(m_pWaterParams.viewangles, m_pWaterParams.forward, m_pWaterParams.right, m_pWaterParams.up);
	VectorCopy(m_pWaterParams.viewangles, m_pWaterParams.cl_viewangles);

	glCullFace(GL_FRONT);
	glColor4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90,  1, 0, 0);// put X going down
	glRotatef(90,  0, 0, 1); // put Z going up
	glRotatef(-m_pWaterParams.viewangles[2], 1, 0, 0);
	glRotatef(-m_pWaterParams.viewangles[0], 0, 1, 0);
	glRotatef(-m_pWaterParams.viewangles[1], 0, 0, 1);
	glTranslatef(-m_pWaterParams.vieworg[0], -m_pWaterParams.vieworg[1], -m_pWaterParams.vieworg[2]);

	glViewport(GL_ZERO, GL_ZERO, WATER_RESOLUTION, WATER_RESOLUTION);

	// Cull everything below the water plane
	VectorCopy(gBSPRenderer.m_pWorld->maxs, vMaxs);
	VectorCopy(gBSPRenderer.m_pWorld->mins, vMins); 
	vMins.z = GetWaterOrigin().z;
	
	gHUD.viewFrustum.SetExtraCullBox(vMins, vMaxs);
	SetupClipping(&m_pWaterParams, true);
	RenderFog();
}

/*
====================
FinishReflect

====================
*/
void CWaterShader::FinishReflect( void ) 
{
	//Save mirrored image
	glBindTexture(GL_TEXTURE_2D, m_pCurWater->reflect);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, WATER_RESOLUTION, WATER_RESOLUTION, 0);

	//Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	//Restore modelview
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// Turn culling off
	gHUD.viewFrustum.DisableExtraCullBox();
}

/*
====================
DrawWater

====================
*/
void CWaterShader::DrawWater( void ) 
{
	if(m_pCvarWaterShader->value < 1)
		return;

	if(!gBSPRenderer.m_bShaderSupport)
		return;

	if(!m_iNumWaterEntities)
		return;

	float flMatrix[16];
	float flTime = gEngfuncs.GetClientTime();

	gBSPRenderer.EnableVertexArray();
	gBSPRenderer.SetTexPointer(0, TC_TEXTURE);

	glEnable(GL_VERTEX_PROGRAM_ARB);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	if(gBSPRenderer.m_bRadialFogSupport && gBSPRenderer.m_pCvarRadialFog->value > 0)
		gBSPRenderer.glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_uiVertexPrograms[1]);
	else
		gBSPRenderer.glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_uiVertexPrograms[0]);

	glGetFloatv(GL_PROJECTION_MATRIX, flMatrix);
	gBSPRenderer.glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 4, flMatrix[0], flMatrix[4], flMatrix[8], flMatrix[12]);
	gBSPRenderer.glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 5, flMatrix[1], flMatrix[5], flMatrix[9], flMatrix[13]);
	gBSPRenderer.glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 6, flMatrix[2], flMatrix[6], flMatrix[10], flMatrix[14]);
	gBSPRenderer.glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 7, flMatrix[3], flMatrix[7], flMatrix[11], flMatrix[15]);

	for(int i = 0; i < m_iNumWaterEntities; i++)
	{
		m_pCurWater = &m_pWaterEntities[i];
		
		if(!m_pWaterEntities[i].draw)
			continue;

		if(gHUD.viewFrustum.CullBox(m_pCurWater->mins, m_pCurWater->maxs))
			continue;

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(m_pCurWater->entity->curstate.origin[0], m_pCurWater->entity->curstate.origin[1], m_pCurWater->entity->curstate.origin[2]);

		glGetFloatv(GL_MODELVIEW_MATRIX, flMatrix);
		gBSPRenderer.glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, flMatrix[0], flMatrix[4], flMatrix[8], flMatrix[12]);
		gBSPRenderer.glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 1, flMatrix[1], flMatrix[5], flMatrix[9], flMatrix[13]);
		gBSPRenderer.glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 2, flMatrix[2], flMatrix[6], flMatrix[10], flMatrix[14]);
		gBSPRenderer.glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 3, flMatrix[3], flMatrix[7], flMatrix[11], flMatrix[15]);

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		if(m_vViewOrigin[2] > GetWaterOrigin().z)
		{
			glCullFace(GL_FRONT);
			if(gHUD.m_pFogSettings.active)
				gBSPRenderer.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_uiFragmentPrograms[1]);
			else
				gBSPRenderer.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_uiFragmentPrograms[0]);
		
			gBSPRenderer.glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, gBSPRenderer.m_vRenderOrigin[0], gBSPRenderer.m_vRenderOrigin[1], gBSPRenderer.m_vRenderOrigin[2], 0);
			gBSPRenderer.glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, m_pWaterFogSettings.color[0], m_pWaterFogSettings.color[1], m_pWaterFogSettings.color[2], 0);
			gBSPRenderer.glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, m_flFresnelTerm, 0, 0, 0);
			gBSPRenderer.glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 3, flTime, 0, 0, 0);
		}
		else
		{
			glCullFace(GL_BACK);
			if(gHUD.m_pFogSettings.active)
				gBSPRenderer.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_uiFragmentPrograms[3]);
			else
				gBSPRenderer.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_uiFragmentPrograms[2]);

			gBSPRenderer.glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, m_pWaterFogSettings.color[0], m_pWaterFogSettings.color[1], m_pWaterFogSettings.color[2], 0);
			gBSPRenderer.glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, flTime, 0, 0, 0);
		}

		gBSPRenderer.Bind2DTexture(GL_TEXTURE0_ARB, m_pNormalTexture->iIndex);
		gBSPRenderer.Bind2DTexture(GL_TEXTURE1_ARB, m_pCurWater->refract);

		// Optimization: Try and find a water entity on the same z coord
		int j = 0;
		for(; j < i; j++)
		{
			if(m_pWaterEntities[j].draw)
			{
				if(GetWaterOrigin(&m_pWaterEntities[j]).z == GetWaterOrigin().z)
				{
					gBSPRenderer.Bind2DTexture(GL_TEXTURE2_ARB, m_pWaterEntities[j].reflect);
					break;
				}
			}
		}

		if(j == i)
			gBSPRenderer.Bind2DTexture(GL_TEXTURE2_ARB, m_pCurWater->reflect);

		for(int j = 0; j < m_pCurWater->numsurfaces; j++)
			gBSPRenderer.DrawPolyFromArray(gBSPRenderer.m_pWorld->surfaces, m_pCurWater->surfaces[j]);
	}

	glDisable(GL_VERTEX_PROGRAM_ARB);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glCullFace(GL_FRONT);

	gBSPRenderer.DisableVertexArray();
}

/*
====================
GetWaterOrigin

====================
*/
vec3_t CWaterShader::GetWaterOrigin( cl_water_t *pwater ) 
{
	if(pwater)
		return pwater->origin+pwater->entity->curstate.origin;
	else 
		return m_pCurWater->origin+m_pCurWater->entity->curstate.origin;
}