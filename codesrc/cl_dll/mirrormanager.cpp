/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

Mirror Manager code
Code by Andrew Lucas
Additional code taken from Id Software
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
#include "mirrormanager.h"
#include "watershader.h"

#include "r_efx.h"
#include "r_studioint.h"
#include "studio_util.h"
#include "event_api.h"
#include "event_args.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
extern CGameStudioModelRenderer g_StudioRenderer;

inline float sgn(float a)
{
    if (a > 0.0F) return (1.0F);
    if (a < 0.0F) return (-1.0F);
    return (0.0F);
}

/*
====================
Init

====================
*/
void CMirrorManager::Init( void ) 
{
	m_pCvarDrawMirrors = gEngfuncs.pfnRegisterVariable( "te_mirrors", "1", 0 );
	m_pCvarMirrorPlayer = gEngfuncs.pfnRegisterVariable( "te_mirror_player", "0", FCVAR_ARCHIVE );
}

/*
====================
VidInit

====================
*/
void CMirrorManager::VidInit( void ) 
{
	for(int i = 0; i < m_iNumMirrors; i++)
		glDeleteTextures(1, &m_pMirrors[i].texture);

	memset(m_pMirrors, 0, sizeof(m_pMirrors));
	m_iNumMirrors = 0;
}

/*
====================
AllocNewMirror

====================
*/
void CMirrorManager::AllocNewMirror( cl_entity_t *entity )
{
	if(m_iNumMirrors == MAX_MIRRORS)
		return;

	if(entity->model->nummodelsurfaces > 1)
	{
		gEngfuncs.Con_Printf("ERROR: Mirror bmodel has more than 1 polygon!\n");
		return;
	}

	cl_mirror_t *pMirror = &m_pMirrors[m_iNumMirrors];
	m_iNumMirrors++;

	msurface_t *psurf = &entity->model->surfaces[entity->model->firstmodelsurface];

	pMirror->mins = Vector(9999, 9999, 9999);
	pMirror->maxs = Vector(-9999, -9999, -9999);

	for(int i = 0; i < psurf->polys->numverts; i++)
	{
		// mins
		if(pMirror->mins.x > psurf->polys->verts[i][0])
			pMirror->mins.x = psurf->polys->verts[i][0];

		if(pMirror->mins.y > psurf->polys->verts[i][1])
			pMirror->mins.y = psurf->polys->verts[i][1];

		if(pMirror->mins.z > psurf->polys->verts[i][2])
			pMirror->mins.z = psurf->polys->verts[i][2];

		// maxs
		if(pMirror->maxs.x < psurf->polys->verts[i][0])
			pMirror->maxs.x = psurf->polys->verts[i][0];

		if(pMirror->maxs.y < psurf->polys->verts[i][1])
			pMirror->maxs.y = psurf->polys->verts[i][1];

		if(pMirror->maxs.z < psurf->polys->verts[i][2])
			pMirror->maxs.z = psurf->polys->verts[i][2];
	}

	pMirror->entity = entity;
	pMirror->entity->efrag = (efrag_s *)pMirror;

	pMirror->texture = current_ext_texture_id;	current_ext_texture_id++;
	pMirror->origin[0] = (pMirror->mins[0] + pMirror->maxs[0]) * 0.5f;
	pMirror->origin[1] = (pMirror->mins[1] + pMirror->maxs[1]) * 0.5f;
	pMirror->origin[2] = (pMirror->mins[2] + pMirror->maxs[2]) * 0.5f;
	pMirror->surface = psurf;

	glBindTexture(GL_TEXTURE_2D, pMirror->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

/*
====================
DrawMirrorPasses

====================
*/
void CMirrorManager::DrawMirrorPasses( ref_params_t *pparams ) 
{
	if(m_pCvarDrawMirrors->value < 1)
		return;

	if(!m_iNumMirrors)
		return;

	//Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

	// Make sure depthranged stuff is not clipped
	gBSPRenderer.m_bMirroring = true;

	memcpy(&m_pMirrorParams, pparams, sizeof(ref_params_t));
	m_pViewParams = pparams;

	for(int i = 0; i < m_iNumMirrors; i++)
	{
		m_pCurrentMirror = &m_pMirrors[i];
		
		if(!m_pCurrentMirror->draw)
			continue;

		gHUD.viewFrustum.SetFrustum(pparams->viewangles, pparams->vieworg, gHUD.m_iFOV, gHUD.m_pFogSettings.end, true);

		if(gHUD.viewFrustum.CullBox(m_pCurrentMirror->mins, m_pCurrentMirror->maxs))
		{
			// YOU MUST DIE
			m_pCurrentMirror->draw = false;
			continue;
		}

		SetupMirrorPass();
		DrawMirrorPass();
		FinishMirrorPass();
	}

	gBSPRenderer.m_bMirroring = false;

	glViewport(GL_ZERO, GL_ZERO, ScreenWidth, ScreenHeight);

}

/*
====================
SetupClipping

====================
*/
void CMirrorManager::SetupClipping( void )
{
	float	dot;
	float	eq1[4];
	float	eq2[4];
	float	projection[16];

	vec3_t	vDist;
	vec3_t	vForward;
	vec3_t	vRight;
	vec3_t	vUp;

	AngleVectors(m_pMirrorParams.viewangles, vForward, vRight, vUp);
	VectorSubtract(m_pCurrentMirror->origin, m_pMirrorParams.vieworg, vDist);
	
	VectorInverse(vRight); 
	VectorInverse(vUp);

	if(m_pCurrentMirror->surface->flags & SURF_PLANEBACK)
	{
		eq1[0] = DotProduct(vRight, m_pCurrentMirror->surface->plane->normal);
		eq1[1] = DotProduct(vUp, m_pCurrentMirror->surface->plane->normal);
		eq1[2] = DotProduct(vForward, m_pCurrentMirror->surface->plane->normal);
		eq1[3] = DotProduct(vDist, m_pCurrentMirror->surface->plane->normal);
	}
	else
	{
		eq1[0] = DotProduct(vRight, (-m_pCurrentMirror->surface->plane->normal));
		eq1[1] = DotProduct(vUp, (-m_pCurrentMirror->surface->plane->normal));
		eq1[2] = DotProduct(vForward, (-m_pCurrentMirror->surface->plane->normal));
		eq1[3] = DotProduct(vDist, (-m_pCurrentMirror->surface->plane->normal));
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
DrawMirrorPass

====================
*/
void CMirrorManager::DrawMirrorPass( void ) 
{
	// Set world renderer
	gBSPRenderer.RendererRefDef(&m_pMirrorParams);

	// Draw world
	gBSPRenderer.DrawNormalTriangles();

	gBSPRenderer.SaveMultiTexture();

	for(int i = 0; i < gBSPRenderer.m_iNumRenderEntities; i++)
	{
		if(gBSPRenderer.m_pRenderEntities[i]->model->type != mod_studio)
			continue;

		if(!gBSPRenderer.m_pRenderEntities[i]->player)
		{
			g_StudioRenderer.m_pCurrentEntity = gBSPRenderer.m_pRenderEntities[i];
			g_StudioRenderer.StudioDrawModel(STUDIO_RENDER);
		}
		else if(m_pCvarMirrorPlayer->value > 0)
		{
			entity_state_t *pPlayer = IEngineStudio.GetPlayerState((gBSPRenderer.m_pRenderEntities[i]->index-1));
			g_StudioRenderer.m_pCurrentEntity = gBSPRenderer.m_pRenderEntities[i];
			g_StudioRenderer.StudioDrawPlayer(STUDIO_RENDER, pPlayer);
		}
	}
	// Draw props
	gPropManager.RenderProps();

	// Draw Transparent world polys
	gBSPRenderer.DrawTransparentTriangles();

	// Draw particles
	gParticleEngine.DrawParticles();

	gBSPRenderer.RestoreMultiTexture();
}

/*
====================
SetupMirrorPass

====================
*/
void CMirrorManager::SetupMirrorPass( void ) 
{
	vec3_t forward;
	AngleVectors(m_pViewParams->viewangles, forward, NULL, NULL);

	float flDist = DotProduct(m_pViewParams->vieworg, m_pCurrentMirror->surface->plane->normal) -  m_pCurrentMirror->surface->plane->dist;
	VectorMASSE(m_pViewParams->vieworg, -2*flDist, m_pCurrentMirror->surface->plane->normal, m_pMirrorParams.vieworg);

	if (m_pCurrentMirror->surface->flags & SURF_PLANEBACK)
	{
		flDist = DotProduct(forward, m_pCurrentMirror->surface->plane->normal);
		VectorMASSE(forward, -2*flDist, m_pCurrentMirror->surface->plane->normal, forward);
	}
	else
	{
		flDist = DotProduct(forward, -m_pCurrentMirror->surface->plane->normal);
		VectorMASSE(forward, -2*flDist, -m_pCurrentMirror->surface->plane->normal, forward);
	}

	m_pMirrorParams.viewangles[0] = -asin(forward[2])/M_PI*180;
	m_pMirrorParams.viewangles[1] = atan2(forward[1], forward[0])/M_PI*180;
	m_pMirrorParams.viewangles[2] = -m_pViewParams->viewangles[2];

	AngleVectors(m_pMirrorParams.viewangles, m_pMirrorParams.forward, m_pMirrorParams.right, m_pMirrorParams.up);
	VectorCopy(m_pMirrorParams.viewangles, m_pMirrorParams.cl_viewangles);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90,  1, 0, 0);// put X going down
	glRotatef(90,  0, 0, 1); // put Z going up
	glRotatef(-m_pMirrorParams.viewangles[2], 1, 0, 0);
	glRotatef(-m_pMirrorParams.viewangles[0], 0, 1, 0);
	glRotatef(-m_pMirrorParams.viewangles[1], 0, 0, 1);
	glTranslatef(-m_pMirrorParams.vieworg[0], -m_pMirrorParams.vieworg[1], -m_pMirrorParams.vieworg[2]);

	glViewport(GL_ZERO, GL_ZERO, MIRROR_RESOLUTION, MIRROR_RESOLUTION);

	glCullFace(GL_FRONT);
	glColor4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	// Set up clipping
	SetupClipping();
}

/*
====================
FinishMirrorPass

====================
*/
void CMirrorManager::FinishMirrorPass( void ) 
{
	//Save mirrored image
	glBindTexture(GL_TEXTURE_2D, m_pCurrentMirror->texture);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, WATER_RESOLUTION, WATER_RESOLUTION, 0);

	//Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

	// Restore projection
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	//Restore modelview
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

/*
====================
DrawMirrors

====================
*/
void CMirrorManager::DrawMirrors( void ) 
{
	if(m_pCvarDrawMirrors->value < 1)
		return;

	if(!m_iNumMirrors)
		return;

	float flProjection[16];
	float flModelView[16];

	GLfloat Splane[] = {1.0f, 0.0f, 0.0f, 0.0f};
	GLfloat Tplane[] = {0.0f, 1.0f, 0.0f, 0.0f};
	GLfloat Rplane[] = {0.0f, 0.0f, 1.0f, 0.0f};
	GLfloat Qplane[] = {0.0f, 0.0f, 0.0f, 1.0f};

	gBSPRenderer.EnableVertexArray();
	gBSPRenderer.glActiveTextureARB(GL_TEXTURE0_ARB);
	gBSPRenderer.SetTexEnvs(ENVSTATE_REPLACE);

	glGetFloatv(GL_PROJECTION_MATRIX, flProjection);
	glGetFloatv(GL_MODELVIEW_MATRIX, flModelView);

	gBSPRenderer.glActiveTextureARB(GL_TEXTURE0_ARB);
	glMatrixMode(GL_TEXTURE);

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_S, GL_EYE_PLANE, Splane);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_T, GL_EYE_PLANE, Tplane);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_R, GL_EYE_PLANE, Rplane);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_Q, GL_EYE_PLANE, Qplane);

	for(int i = 0; i < m_iNumMirrors; i++)
	{
		if(!m_pMirrors[i].draw)
			continue;

		if(gHUD.viewFrustum.CullBox(m_pMirrors[i].mins, m_pMirrors[i].maxs))
			continue;

		glPushMatrix();
		glLoadIdentity();

		glTranslatef(0.5f, 0.5f, 0.0f);
		glScalef(0.5f, 0.5f, 1.0f);

		if(m_pMirrors[i].surface->plane->normal[2] == 1) 
			glScalef(1, -1, 1);
		else 	
			glScalef(-1, 1, 1);

		glMultMatrixf(flProjection);
		glMultMatrixf(flModelView);

		glBindTexture(GL_TEXTURE_2D, m_pMirrors[i].texture);

		model_t *model = m_pMirrors[i].entity->model;
		msurface_t *psurf = &model->surfaces[model->firstmodelsurface];

		gBSPRenderer.DrawPolyFromArray(psurf->polys);
		psurf->visframe = gBSPRenderer.m_iFrameCount;// For decals
		
		glPopMatrix();
	}

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
	glMatrixMode(GL_MODELVIEW);

	gBSPRenderer.DisableVertexArray();
}