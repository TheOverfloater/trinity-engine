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

#include "windows.h"
#include <gl/gl.h>
#include "gl/glext.h"

#include "STDIO.H"
#include "STDLIB.H"
#include "MATH.H"

#include "hud.h"
#include "cl_util.h"
#include "com_model.h"
#include <string.h>
#include "triangleapi.h"
#include "event_api.h"

#include "rendererdefs.h"
#include "bsprenderer.h"
#include "particle_engine.h"
#include "propmanager.h"
#include "watershader.h"
#include "mirrormanager.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
extern CGameStudioModelRenderer g_StudioRenderer;

#ifndef BOX_ON_PLANE_SIDE
#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))
#endif

#define BASE_EXT_TEXTURE_ID		(1<<25) // dont use zero
int current_ext_texture_id = BASE_EXT_TEXTURE_ID;

vec3_t g_vecFull(1.0f, 1.0f, 1.0f); // color of 3d attenuation texture
vec3_t g_vecZero(0.0f, 0.0f, 0.0f); // color of 3d attenuation texture

glstate_t g_savedGLState;

double sqrt(double x);

//==========================
//	stristr
//
//==========================
char *stristr( const char *string, const char *string2 )
{
	int c, len;
	c = tolower( *string2 );
	len = strlen( string2 );

	while (string) {
		for (; *string && tolower( *string ) != c; string++);
		if (*string) {
			if (strnicmp( string, string2, len ) == 0) {
				break;
			}
			string++;
		}
		else {
			return NULL;
		}
	}
	return (char *)string;
}

//==========================
//	strLower
//
//==========================
char *strLower( char *str )
{
	char *temp;

	for ( temp = str; *temp; temp++ ) 
		*temp = tolower( *temp );

	return str;
}

//==========================
// FilenameFromPath
//
//==========================
void FilenameFromPath( char *szin, char *szout )
{
	int lastdot = 0;
	int lastbar = 0;
	int pathlength = 0;

	for(int i = 0; i < (int)strlen(szin); i++)
	{
		if(szin[i] == '/' || szin[i] == '\\')
			lastbar = i+1;

		if( szin[i] == '.' )
			lastdot = i;
	}

	for(int i = lastbar; i < (int)strlen(szin); i++)
	{
		if(i == lastdot)
			break;

		szout[pathlength] = szin[i];
		pathlength++;
	}
	szout[pathlength] = 0;
}

#define shuffle(a, b, c) (((a)<<4) | ((b)<<2) | ((c)))
 
//==========================
// SSE DotProduct Plane EQ
//
//==========================
inline void SSEDotProductSub(float *result, vec3_t *v0, vec3_t *v1, float *subval )
{
	_asm{
		mov             esi,    v0
		mov             edi,    v1
		mov             eax,    result;
		mov             edx,    subval;

		movups  xmm0,   [esi];
		movups  xmm1,   [edi];

		mulps   xmm0,   xmm1;

		movups  xmm2,   xmm0;
		shufps  xmm2,   xmm0, shuffle(0x01, 0x00, 0x02);
		addps   xmm2,   xmm0;
		shufps  xmm0,   xmm2, shuffle(0x02, 0x00, 0x01);
		addps   xmm0,   xmm2;

		subss   xmm0,   [edx];
		movss   [eax],  xmm0;
	}
}
 
//==========================
// SSE DotProduct world coord on Studio Models
//
//==========================
inline void SSEDotProductWorld( float* result, const float* v0, const float* v1 )
{
	_asm{
		mov             esi,    v0
		mov             edi,    v1
		mov             eax,    result;

		movups  xmm0,   [esi];
		movups  xmm1,   [edi];

		mulps   xmm0,   xmm1;

		movups  xmm2,   xmm0;
		shufps  xmm2,   xmm0, shuffle(0x01, 0x00, 0x02);
		addps   xmm2,   xmm0;
		shufps  xmm0,   xmm2, shuffle(0x02, 0x00, 0x01);
		addps   xmm0,   xmm2;

		addss   xmm0,   [edi+12];
		movss   [eax],  xmm0;
	}
}
 
//==========================
// SSE DotProduct
//
//==========================
inline void DotProductSSE( float* result, const float* v0, const float* v1 )
{
	_asm{
		mov             esi,    v0
		mov             edi,    v1
		mov             eax,    result;

		movups  xmm0,   [esi];
		movups  xmm1,   [edi];

		mulps   xmm0,   xmm1;

		movups  xmm2,   xmm0;
		shufps  xmm2,   xmm0, shuffle(0x01, 0x00, 0x02);
		addps   xmm2,   xmm0;
		shufps  xmm0,   xmm2, shuffle(0x02, 0x00, 0x01);
		addps   xmm0,   xmm2;

		movss   [eax],  xmm0;   
	}
}

/*
====================
VectorAddSSE

====================
*/
inline void VectorAddSSE( const float* v0, const float* v1, const float* result )
{
	_asm {
		mov	esi,	v0
		mov	edi,	v1
		mov	eax,	result

		movss	xmm0,	[esi]
		movss	xmm1,	[esi+4]
		movss   xmm2,	[esi+8]

		addss	xmm0,   [edi]
		addss	xmm1,   [edi+4]
		addss	xmm2,   [edi+8]

		movss	[eax],		xmm0
		movss	[eax+4],	xmm1
		movss	[eax+8],	xmm2
	}
}

/*
====================
VectorSubtract

====================
*/
inline void VectorMASSE (const float *veca, float scale, const float *vecb, float *vecc)
{
	_asm {
		mov		eax,  veca;
		mov		ebx,  vecb;
		mov		ecx,  vecc;
		movss	xmm7, scale;

		;scale*vecb
		movss	xmm0, [ebx];
		movss	xmm1, [ebx+4];
		movss	xmm2, [ebx+8];
		mulss	xmm0, xmm7;
		mulss	xmm1, xmm7;
		mulss	xmm2, xmm7;

		;(scale*vecb) + veca
		movss	xmm3, [eax];
		movss	xmm4, [eax+4];
		movss	xmm5, [eax+8];
		addss	xmm0, xmm3;
		addss	xmm1, xmm4;
		addss	xmm2, xmm5;

		;return_it
		movss	[ecx],   xmm0;
		movss	[ecx+4], xmm1;
		movss	[ecx+8], xmm2;
	}
}

/*
====================
VectorRotateSSE

====================
*/
inline void VectorRotateSSE (const float *in1, float in2[3][4], float *out)
{
	DotProductSSE(&out[0], in1, in2[0]);
	DotProductSSE(&out[1], in1, in2[1]);
	DotProductSSE(&out[2], in1, in2[2]);
}

/*
====================
VectorTransformSSE

====================
*/
inline void VectorTransformSSE(const float *in1, float in2[3][4], float *out)
{
	SSEDotProductWorld(&out[0], in1, in2[0]);
	SSEDotProductWorld(&out[1], in1, in2[1]);
	SSEDotProductWorld(&out[2], in1, in2[2]);
}


/*
====================
VectorIRotate

====================
*/
void VectorIRotate (const vec3_t &in1, const float in2[3][4], vec3_t &out)
{
	out[0] = in1[0]*in2[0][0] + in1[1]*in2[1][0] + in1[2]*in2[2][0];
	out[1] = in1[0]*in2[0][1] + in1[1]*in2[1][1] + in1[2]*in2[2][1];
	out[2] = in1[0]*in2[0][2] + in1[1]*in2[1][2] + in1[2]*in2[2][2];
}

//==========================
//	ClampColor
//
//==========================
void ClampColor( int r, int g, int b, color24 *out )
{
	if( r < 0 ) r = 0;
	if( g < 0 ) g = 0;
	if( b < 0 ) b = 0;

	if( r > 255 ) r = 255;
	if( g > 255 ) g = 255;
	if( b > 255 ) b = 255;

	out->r = (unsigned char)r;
	out->g = (unsigned char)g;
	out->b = (unsigned char)b;
}

//==========================
//	R_RotateForEntity
//
//==========================
void R_RotateForEntity (cl_entity_t *e)
{
    glTranslatef (e->origin[0],  e->origin[1],  e->origin[2]);

    glRotatef (e->angles[1],  0, 0, 1);
    glRotatef (-e->angles[0],  0, 1, 0);
    glRotatef (e->angles[2],  1, 0, 0);
}

//==========================
//	IsEntityMoved
//
//==========================
int IsEntityMoved(cl_entity_t *e)
{
	if (e->angles[0] || e->angles[1] || e->angles[2] ||
		e->origin[0] || e->origin[1] || e->origin[2] ||
		e->curstate.renderfx == 70 ||
		e->curstate.effects & FL_CONVEYOR ||
		e->curstate.frame > 0) // skybox models reques separate pass
		return TRUE;
	else
		return FALSE;
}

//==========================
//	IsEntityTransparent
//
//==========================
int IsEntityTransparent(cl_entity_t *e)
{
	if(!e)
		return FALSE;

	if (e->curstate.rendermode == kRenderNormal ||
		e->curstate.rendermode == kRenderTransAlpha)
		return FALSE;
	else
		return TRUE;
}

//==========================
//	IsPitchReversed
//
//==========================
int IsPitchReversed(float pitch)
{
	int quadrant = int(pitch / 90) % 4;
	if ((quadrant == 1) || (quadrant == 2)) 
		return -1;
	
	return 1;
}

//==========================
//	BlackFog
//
//==========================
void BlackFog ( void )
{
	if (gHUD.m_pFogSettings.active)
		glFogfv (GL_FOG_COLOR,  g_vecZero);
	else
		glDisable(GL_FOG);
}

//==========================
//	RenderFog
//
//==========================
void RenderFog ( void ) 
{
	if ( gHUD.m_pFogSettings.active )
	{
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_DENSITY, 1.0f);
		glHint(GL_FOG_HINT, GL_NICEST);

		glFogf(GL_FOG_START, gHUD.m_pFogSettings.start);
		glFogf(GL_FOG_END, gHUD.m_pFogSettings.end);
		glFogfv(GL_FOG_COLOR,  gHUD.m_pFogSettings.color);

		if(gBSPRenderer.m_bRadialFogSupport && gBSPRenderer.m_pCvarRadialFog->value >= 1)
			glFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);
		else
			glFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_PLANE_ABSOLUTE_NV);
	}
	else
	{
		glDisable(GL_FOG);
	}
}

//==========================
//	ClearToFogColor
//
//	Thanks to Laurie
//==========================
void ClearToFogColor( void )
{
	if(!gHUD.m_pFogSettings.active)
		return;

	glClearColor( gHUD.m_pFogSettings.color[0], gHUD.m_pFogSettings.color[1], gHUD.m_pFogSettings.color[2], 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );
}

//==========================
//	DisableFog
//
//==========================
void DisableFog ( void )
{
	glDisable(GL_FOG);
}

void MyLookAt( GLdouble eyex, GLdouble eyey, GLdouble eyez,
			GLdouble centerx, GLdouble centery, GLdouble centerz,
			GLdouble upx, GLdouble upy, GLdouble upz )
{
   GLdouble m[16];
   GLdouble x[3], y[3], z[3];
   GLdouble mag;

   /* Make rotation matrix */

   /* Z vector */
   z[0] = eyex - centerx;
   z[1] = eyey - centery;
   z[2] = eyez - centerz;
   mag = sqrt( z[0]*z[0] + z[1]*z[1] + z[2]*z[2] );
   if (mag) {  /* mpichler, 19950515 */
      z[0] /= mag;
      z[1] /= mag;
      z[2] /= mag;
   }

   /* Y vector */
   y[0] = upx;
   y[1] = upy;
   y[2] = upz;

   /* X vector = Y cross Z */
   x[0] =  y[1]*z[2] - y[2]*z[1];
   x[1] = -y[0]*z[2] + y[2]*z[0];
   x[2] =  y[0]*z[1] - y[1]*z[0];

   /* Recompute Y = Z cross X */
   y[0] =  z[1]*x[2] - z[2]*x[1];
   y[1] = -z[0]*x[2] + z[2]*x[0];
   y[2] =  z[0]*x[1] - z[1]*x[0];

   /* mpichler, 19950515 */
   /* cross product gives area of parallelogram, which is < 1.0 for
    * non-perpendicular unit-length vectors; so normalize x, y here
    */

   mag = sqrt( x[0]*x[0] + x[1]*x[1] + x[2]*x[2] );
   if (mag) {
      x[0] /= mag;
      x[1] /= mag;
      x[2] /= mag;
   }

   mag = sqrt( y[0]*y[0] + y[1]*y[1] + y[2]*y[2] );
   if (mag) {
      y[0] /= mag;
      y[1] /= mag;
      y[2] /= mag;
   }

#define M(row,col)  m[col*4+row]
   M(0,0) = x[0];  M(0,1) = x[1];  M(0,2) = x[2];  M(0,3) = 0.0;
   M(1,0) = y[0];  M(1,1) = y[1];  M(1,2) = y[2];  M(1,3) = 0.0;
   M(2,0) = z[0];  M(2,1) = z[1];  M(2,2) = z[2];  M(2,3) = 0.0;
   M(3,0) = 0.0;   M(3,1) = 0.0;   M(3,2) = 0.0;   M(3,3) = 1.0;
#undef M
   glMultMatrixd( m );
   glTranslated( -eyex, -eyey, -eyez );
}

//==========================
//	MOD_PointInLeaf
//
//==========================
mleaf_t *Mod_PointInLeaf (vec3_t p, model_t *model)
{
	mnode_t *node;
	float d;
	mplane_t *plane;

	node = model->nodes;
	while (1)
	{
		if (node->contents < 0)
			return (mleaf_t *)node;
		plane = node->plane;
		//d = DotProduct (p,plane->normal) - plane->dist;
		SSEDotProductSub(&d, &p, &plane->normal, &plane->dist);
		if (d > 0)
			node = node->children[0];
		else
			node = node->children[1];
	}

	return NULL;	// never reached
}
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, mplane_t *p)
{
	float	dist1, dist2;
	int		sides;

// general case
	switch (p->signbits)
	{
	case 0:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 1:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 2:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 3:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 4:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 5:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 6:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	case 7:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		gEngfuncs.Con_Printf("BoxOnPlaneSide error\n");
		break;
	}

	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;
	return sides;
}
/*
===============
SV_FindTouchedLeafs

===============
*/
void SV_FindTouchedLeafs (entextradata_t *ent, mnode_t *node)
{
	mplane_t	*splitplane;
	mleaf_t		*leaf;
	int			sides;
	int			leafnum;

	if (node->contents == CONTENTS_SOLID)
		return;
	
// add an efrag if the node is a leaf
	model_t *world = IEngineStudio.GetModelByIndex(1);
	if ( node->contents < 0)
	{
		if (ent->num_leafs == MAX_ENT_LEAFS)
			return;

		leaf = (mleaf_t *)node;
		leafnum = leaf - world->leafs - 1;

		ent->leafnums[ent->num_leafs] = leafnum;
		ent->num_leafs++;			
		return;
	}
	
// NODE_MIXED

	splitplane = node->plane;
	sides = BoxOnPlaneSide(ent->absmin, ent->absmax, splitplane);
	
// recurse down the contacted sides
	if (sides & 1)
		SV_FindTouchedLeafs (ent, node->children[0]);
		
	if (sides & 2)
		SV_FindTouchedLeafs (ent, node->children[1]);
}

/*
===================
Mod_DecompressVis
===================
*/

byte *Mod_DecompressVis (byte *in, model_t *model)
{
	static byte	decompressed[MAX_MAP_LEAFS/8];
	int		c;
	byte	*out;
	int		row;

	row = (model->numleafs+7)>>3;	
	out = decompressed;

	if (!in)
	{	// no vis info, so make all visible
		while (row)
		{
			*out++ = 0xff;
			row--;
		}
		return decompressed;		
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}
	
		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out - decompressed < row);
	
	return decompressed;
}

byte *Mod_LeafPVS (mleaf_t *leaf, model_t *model)
{
	if (leaf == model->leafs)
		return Mod_DecompressVis(NULL, model);

	return Mod_DecompressVis (leaf->compressed_vis, model);
}

/*
===============
R_MarkLeaves
===============
*/
void R_MarkLeaves ( mleaf_t *pLeaf )
{
	model_t *pWorld = IEngineStudio.GetModelByIndex(1);
	byte *vis = Mod_LeafPVS (pLeaf, pWorld);
		
	for(int i = 0; i < pWorld->numleafs; i++)
	{
		if (vis[i >> 3] & (1 << (i & 7)))
		{
			mnode_t *node = (mnode_t *)&pWorld->leafs[i+1];
			do
			{
				if (node->visframe == gBSPRenderer.m_pViewLeaf->visframe)
					break;

				node->visframe = gBSPRenderer.m_pViewLeaf->visframe;
				node = node->parent;
			} while (node);
		}
	}
}

byte *ResizeArray( byte *pOriginal, int iSize, int iCount )
{
	byte *pArray = new byte[iSize*(iCount+1)];
	memset(pArray, 0, sizeof(byte)*iSize*(iCount+1));

	if(pOriginal && iCount)
	{
		memmove(pArray, pOriginal, iSize*iCount);
		delete [] pOriginal;
	}

	return pArray;
}

/*
=================
HUD_PrintSpeeds

=================
*/
void HUD_PrintSpeeds ( void )
{
	if( gBSPRenderer.m_pCvarSpeeds->value <= 0 )
		return;

	static float flLastTime;
	float flCurTime = gEngfuncs.GetClientTime();
	float flFrameTime = flCurTime - flLastTime;
	flLastTime = flCurTime;

	// prevent divide by zero
	if(flFrameTime <= 0)
		flFrameTime = 1;

	if(flFrameTime > 1)
		flFrameTime = 1;

	int iFPS = 1/flFrameTime;
	gEngfuncs.Con_Printf("%i wpolys, %i epolys, %i studio polys\n, %i particles, %i fps\n", 
		gBSPRenderer.m_iWorldPolyCounter, gBSPRenderer.m_iBrushPolyCounter, gBSPRenderer.m_iStudioPolyCounter, 
		gParticleEngine.m_iNumParticles, iFPS);
}

/*
=================
R_CalcRefDef

=================
*/
void R_CalcRefDef( ref_params_t *pparams )
{
	// Set this at start
	RenderFog();

	// Set up pre-frame stuff
	gBSPRenderer.SetupPreFrame(pparams);

	// Render shadow maps into depth images
	gBSPRenderer.DrawShadowPasses();

	// Render water shader perspectives
	gWaterShader.DrawWaterPasses(pparams);

	// Render mirror perspectives
	gMirrorManager.DrawMirrorPasses(pparams);

	// Set up basic rendering
	gBSPRenderer.RendererRefDef(pparams);
}

/*
=================
R_DrawNormalTriangles

=================
*/
void R_DrawNormalTriangles( void )
{
	// Set this at start
	RenderFog();

	// Render world
	gBSPRenderer.DrawNormalTriangles();

	// Save everything at this point too
	R_SaveGLStates();

	// Apply fog
	RenderFog();

	// Render props on the list
	gPropManager.RenderProps();

	// Render any cables
	gPropManager.DrawCables();

	// Render water shader
	gWaterShader.DrawWater();

	//Draw mirrors
	gMirrorManager.DrawMirrors();

	// Render transparent entities
	gBSPRenderer.DrawTransparentTriangles();

	// Render particles
	gParticleEngine.DrawParticles();

	//Restore
	R_RestoreGLStates();
}

/*
=================
R_DrawTransparentTriangles

=================
*/
void R_DrawTransparentTriangles( void )
{
	// Set basic fog, and then turn it black for sprites
	RenderFog();
	BlackFog();

	// Reset for HUD display
	if(gBSPRenderer.m_bRadialFogSupport && gBSPRenderer.m_pCvarRadialFog->value >= 1)
		glFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_PLANE_ABSOLUTE_NV);

	// Restore fog params
	gWaterShader.Restore();

	// Disable VBO here
	gBSPRenderer.glBindBufferARB(GL_ARRAY_BUFFER_ARB, NULL);
	gBSPRenderer.SetTexEnvs(ENVSTATE_REPLACE);

	// 2012-05-23 -- Not setting this caused problems with Steam HL
	gBSPRenderer.ResetRenderer();
	gBSPRenderer.glClientActiveTextureARB(GL_TEXTURE0_ARB);
}

void RenderersDumpInfo( void )
{
	gEngfuncs.Con_Printf("Engine Data Dump:\n");
	gEngfuncs.Con_Printf("Number of models in client cache: %i of 4096 max.\n", g_StudioRenderer.m_iNumStudioModels);
	gEngfuncs.Con_Printf("Number of models in engine cache: %i of 512 max.\n", g_StudioRenderer.m_iNumEngineCacheModels);
	gEngfuncs.Con_Printf("Number of tga textures cached: %i on client.\n", gTextureLoader.m_iNumTextures);
	gEngfuncs.Con_Printf("Number of lightmaps: %i of 64 max.\n", gBSPRenderer.m_iNumLightmaps);
	gEngfuncs.Con_Printf("Number of surfaces: %i.\n", gBSPRenderer.m_iTotalFaceCount);
	gEngfuncs.Con_Printf("Number of triangles: %i.\n", gBSPRenderer.m_iTotalTriCount);
	gEngfuncs.Con_Printf("Number of vertexes: %i.\n", gBSPRenderer.m_iTotalVertCount);
	gEngfuncs.Con_Printf("Number of client side entities: %i.\n", gPropManager.m_iNumEntities);
	gEngfuncs.Con_Printf("Number of detail textures: %i.\n", gBSPRenderer.m_iNumDetailTextures);
	gEngfuncs.Con_Printf("Current free texture ID: %i.\n", current_ext_texture_id);
	if(gBSPRenderer.m_bShaderSupport) gEngfuncs.Con_Printf("ARB shaders supported.\n");
	else gEngfuncs.Con_Printf("ARB shaders not supported.\n");
	if(gBSPRenderer.m_bRadialFogSupport) gEngfuncs.Con_Printf("Radial fog supported.\n");
	else gEngfuncs.Con_Printf("Radial fog not supported.\n");
}

void GenDetail( void )
{
	char szLevelName[64];
	char szPathOut[128];

	bool bFound[512];
	memset(bFound, 0, sizeof(bFound));

	strcpy( szLevelName, gEngfuncs.pfnGetLevelName() );
	model_t *pWorld = IEngineStudio.GetModelByIndex(1);

	if ( strlen(szLevelName) == 0 )
		return;

	szLevelName[strlen(szLevelName)-4] = 0;
	strcpy(szPathOut, gEngfuncs.pfnGetGameDirectory());
	strcat(szPathOut, "/");
	strcat(szPathOut, szLevelName);
	strcat(szPathOut, "_detail.txt");

	FILE *fout = fopen(szPathOut, "w");

	int iSize = 0;
	char *pFile = (char *)gEngfuncs.COM_LoadFile( "maps/detailtextures.txt", 5, &iSize );
	if (!pFile)
	{
		gEngfuncs.Con_Printf("Failed to load temp detail texture file for %s\n", szLevelName);
		return;
	}

	int i = NULL;
	while(1)
	{
		// Reached EOF
		if(i >= iSize)
			break;

		char szTexture[32];
		// Skip to next token
		while(1)
		{
			if(i >= iSize)
				break;

			if(pFile[i] != ' ' && pFile[i] != '\n' && pFile[i] != '\r')
				break;

			i++;
		}

		if(i >= iSize)
			break;

		// Read token in
		int j = NULL;
		while(1)
		{
			if(i >= iSize)
				break;

			if(pFile[i] == ' ' || pFile[i] == '\n' || pFile[i] == '\r')
				break;

			szTexture[j] = pFile[i];
			j++; i++;
		}

		//Terminator
		szTexture[j] = 0;

		if(i >= iSize)
			break;

		// Skip to next token
		char szDetail[32];
		while(1)
		{
			if(i >= iSize)
				break;

			if(pFile[i] != ' ' && pFile[i] != '\n' && pFile[i] != '\r')
				break;

			i++;
		}

		if(i >= iSize)
			break;

		// Read token in
		j = NULL;
		while(1)
		{
			if(i >= iSize)
				break;

			if(pFile[i] == ' ' || pFile[i] == '\n' || pFile[i] == '\r')
				break;

			szDetail[j] = pFile[i];
			j++; i++;
		}

		//Terminator
		szDetail[j] = 0;

		if(i >= iSize)
			break;

		cl_texture_t *pDetail = gBSPRenderer.LoadDetailTexture(szDetail);

		if(!pDetail)
		{
			gEngfuncs.Con_Printf("Could not load detail texture: %s\n", szDetail);
			continue;
		}

		texture_t *pTexture = NULL;
		
		char maptexture[16];
		strLower(szTexture);
		strLower(szDetail);

		int i = 0;
		for(; i < pWorld->numtextures; i++)
		{
			if (!pWorld->textures[i] || pWorld->textures[i]->name[0] == 0)
				continue;

			strcpy(maptexture, pWorld->textures[i]->name);
			strLower(maptexture);

			if(!strcmp(maptexture, szTexture))
			{
				pTexture = pWorld->textures[i];
				break;
			}
		}

		if(!pTexture)
			continue;

		bFound[i] = true;
		float detailx = (((float)pTexture->height)/256.0)*(128.0/((float)pDetail->iHeight))*12.0;
		float detaily = (((float)pTexture->width)/256.0)*(128.0/((float)pDetail->iWidth))*12.0;
	
		fprintf(fout, "%s %s %f %f\n", szTexture, szDetail, detaily, detailx);
	}
	gEngfuncs.COM_FreeFile(pFile);
	fclose(fout);

	char *pPrevious = NULL;
	char *pPrevFile = (char *)gEngfuncs.COM_LoadFile( "detail_failures.txt", 5, NULL );

	if(pPrevFile)
	{
		pPrevious = new char[strlen(pPrevFile)];
		memcpy(pPrevious, pPrevFile, sizeof(char)*strlen(pPrevFile));
		gEngfuncs.COM_FreeFile(pPrevFile);
	}

	strcpy(szPathOut, gEngfuncs.pfnGetGameDirectory());
	strcat(szPathOut, "/detail_failures.txt");
	FILE *fList = fopen(szPathOut, "w");

	if(pPrevious)
	{
		fprintf(fList, "%s", pPrevious);
		delete [] pPrevious;
	}

	// Print out world name
	fprintf(fList,"--------------%s\n", pWorld->name);

	for(int i = 0; i < pWorld->numtextures; i++)
	{
		if(bFound[i])
			continue;
		
		fprintf(fList, "%s\n", pWorld->textures[i]->name);
	}

	gEngfuncs.Con_Printf("List of missing textures written.");
	fclose(fList);
}
//===============================
// buz: flashlight managenemt
//===============================
void SetupFlashlight(vec3_t origin, vec3_t angles, float time, float frametime)
{
	pmtrace_t tr;
	vec3_t fwd, right, up;

	static float add = 0;
	float addideal = 0;
	
	AngleVectors(angles, fwd, right, up);

	if(gBSPRenderer.m_bShaderSupport && gBSPRenderer.m_pCvarWorldShaders->value >= 1)
		fwd = origin + (fwd*150);
	else
		fwd = origin + (fwd*1550);
	
	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( origin, fwd, PM_NORMAL, -1, &tr );
	
	if(gBSPRenderer.m_bShaderSupport && gBSPRenderer.m_pCvarWorldShaders->value >= 1)
	{
		if (tr.fraction < 1.0)
			addideal = (1 - tr.fraction)*30;

		float speed = (add - addideal)*10;

		if (speed < 0) 
			speed *= -1;

		if (add < addideal)
		{
			add += frametime*speed;

			if (add > addideal) 
				add = addideal;
		}
		else if (add > addideal)
		{
			add -= frametime*speed;

			if (add < addideal) 
				add = addideal;
		}

		cl_dlight_t *flashlight = gBSPRenderer.CL_AllocDLight(-666);
		flashlight->origin = origin + (up*8) + (right*10);
		flashlight->radius = 700;
		flashlight->die = time + 0.01;
		flashlight->cone_size = 50+add;
		flashlight->color.x = 1.0;
		flashlight->color.y = 1.0;
		flashlight->color.z = 1.0;
		flashlight->textureindex = gBSPRenderer.m_pFlashlightTextures[0]->iIndex;
		flashlight->frustum.SetFrustum(angles, flashlight->origin, flashlight->cone_size*1.2, 700);
		VectorCopy(angles, flashlight->angles);

		mlight_t *pLight = &gBSPRenderer.m_pModelLights[gBSPRenderer.m_iNumModelLights];
		gBSPRenderer.m_iNumModelLights++;
		
		pLight->origin = flashlight->origin;
		pLight->flashlight = true;
		pLight->frustum = &flashlight->frustum;
		pLight->radius = flashlight->radius;
		pLight->spotcos = cos((flashlight->cone_size*2)*0.3*(M_PI*2/360));
		pLight->color = flashlight->color;

		// Shitpickle
		FixVectorForSpotlight(angles);
		AngleVectors(angles, pLight->forward, NULL, NULL);
	}
	else
	{
		cl_dlight_t *flashlight = gBSPRenderer.CL_AllocDLight(-666);
		flashlight->origin = tr.endpos;
		flashlight->radius = 300;
		flashlight->color = Vector(0.6, 0.6, 0.6);
		flashlight->die = time + 0.1;
	}
}
float Q_rsqrt( float number )
{
        long i;
        float x2, y;
        const float threehalfs = 1.5F;
 
        x2 = number * 0.5F;
        y  = number;
        i  = * ( long * ) &y;                       // evil floating point bit level hacking
        i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
        y  = * ( float * ) &i;
        y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration

        return y;
}

float VectorNormalizeFast (float *v)
{
	float ilength;
	DotProductSSE(&ilength, v, v);
	float sqroot = Q_rsqrt(ilength);
	VectorScale(v, sqroot, v);

	return ilength;
}

// not finished
void ExportWorld( void )
{
	model_t *pWorld = IEngineStudio.GetModelByIndex(1);

	if(!pWorld)
		return;

	FILE *pFile = fopen("export.smd", "w");
	if(!pFile)
		return;

	fprintf(pFile, "version 1\nnodes\n");
	fprintf(pFile, "  0 Bone01 -1\nend\n");
	fprintf(pFile, "skeleton\ntime 0\n");
	fprintf(pFile, "0 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000\n");
	fprintf(pFile, "end\ntriangles\n");

	msurface_t *psurf = pWorld->surfaces+pWorld->nummodelsurfaces;
	for(int i = pWorld->nummodelsurfaces; i < pWorld->numsurfaces; i++, psurf++)
	{
		if(psurf->flags & (SURF_DRAWTURB|SURF_DRAWSKY))
			continue;

		brushface_t *pFace = &gBSPRenderer.m_pFacesExtraData[psurf->polys->flags];
		brushvertex_t *pVert = gBSPRenderer.m_pBufferData+pFace->start_vertex;

		for(int j = 0; j < pFace->num_vertexes; j+=3)
		{
			fprintf(pFile, "%s.bmp\n", psurf->texinfo->texture->name);
			fprintf(pFile, "0 %f %f %f %f %f %f %f %f\n", pVert[j].pos.x, pVert[j].pos.y, pVert[j].pos.z, pVert[j].normal.x, pVert[j].normal.y, pVert[j].normal.z, pVert[j].texcoord[0], pVert[j].texcoord[1]);
			fprintf(pFile, "0 %f %f %f %f %f %f %f %f\n", pVert[j+1].pos.x, pVert[j+1].pos.y, pVert[j+1].pos.z, pVert[j+1].normal.x, pVert[j+1].normal.y, pVert[j+1].normal.z, pVert[j+1].texcoord[0], pVert[j+1].texcoord[1]);
			fprintf(pFile, "0 %f %f %f %f %f %f %f %f\n", pVert[j+2].pos.x, pVert[j+2].pos.y, pVert[j+2].pos.z, pVert[j+2].normal.x, pVert[j+2].normal.y, pVert[j+2].normal.z, pVert[j+2].texcoord[0], pVert[j+2].texcoord[1]);
		}
	}

	fprintf(pFile, "end\n");
	fclose(pFile);
}

// Detect litte/big endian
byte bLittleEndian[2] = {1, 0};
unsigned short ByteToUShort( byte *byte )
{
	if(*(short*)bLittleEndian == 1)
		return (byte[0]+(byte[1]<<8));
	else
		return (byte[1]+(byte[0]<<8));
}

unsigned int ByteToUInt( byte *byte )
{
	unsigned int iValue = byte[0];
	iValue += (byte[1]<<8);
	iValue += (byte[2]<<16);
	iValue += (byte[3]<<24);

	return iValue;
}

int ByteToInt( byte *byte )
{
	int iValue = byte[0];
	iValue += (byte[1]<<8);
	iValue += (byte[2]<<16);
	iValue += (byte[3]<<24);

	return iValue;
}

void FixVectorForSpotlight( vec3_t &vec )
{
	if (vec[PITCH] == 0) vec[PITCH] = 1;
	if (vec[PITCH] == 90) vec[PITCH] = 89;
	if (vec[PITCH] == 180) vec[PITCH] = 179;
	if (vec[PITCH] == 270) vec[PITCH] = 269;
	if (vec[PITCH] == -90) vec[PITCH] = -89;
	if (vec[PITCH] == -180) vec[PITCH] = -179;
	if (vec[PITCH] == -270) vec[PITCH] = -269;

	if (vec[YAW] == 0) vec[YAW] = 1;
	if (vec[YAW] == 90) vec[YAW] = 89;
	if (vec[YAW] == 180) vec[YAW] = 179;
	if (vec[YAW] == 270) vec[YAW] = 269;
	if (vec[YAW] == -90) vec[YAW] = -89;
	if (vec[YAW] == -180) vec[YAW] = -179;
	if (vec[YAW] == -270) vec[YAW] = -269;

	if (vec[ROLL] == 0) vec[ROLL] = 1;
	if (vec[ROLL] == 90) vec[ROLL] = 89;
	if (vec[ROLL] == 180) vec[ROLL] = 179;
	if (vec[ROLL] == 270) vec[ROLL] = 269;
	if (vec[ROLL] == -90) vec[ROLL] = -89;
	if (vec[ROLL] == -180) vec[ROLL] = -179;
	if (vec[ROLL] == -270) vec[ROLL] = -269;
}

void R_DisableSteamMSAA( void )
{
	// make sure we start with FBO / AA disabled
	gEngfuncs.pfnClientCmd("_set_vid_level 1");
	gEngfuncs.pfnClientCmd("_sethdmodels 0");
	gEngfuncs.pfnClientCmd("gl_texturemode GL_LINEAR_MIPMAP_LINEAR");
	gEngfuncs.pfnClientCmd("gl_round_down 0");
	gEngfuncs.pfnClientCmd("_restart");
}
#ifdef HL25_UPDATE
bool g_areVertexShadersEnabled = false;
bool g_areFragmentShadersEnabled = false;
GLint g_shaderProgramBinding = 0;
#endif

bool g_glStateSaved = false;

void R_SaveGLStates( void )
{
	assert(g_glStateSaved == false);
	g_glStateSaved = true;

#ifdef HL25_UPDATE
	g_areVertexShadersEnabled = (glIsEnabled(GL_VERTEX_SHADER) == GL_TRUE) ? true : false;
	g_areFragmentShadersEnabled = (glIsEnabled(GL_FRAGMENT_SHADER) == GL_TRUE) ? true : false;

	glGetIntegerv(GL_PROGRAM_BINDING_ARB, &g_shaderProgramBinding);
	gBSPRenderer.glUseProgram(0);
#endif

	glPushAttrib(GL_TEXTURE_BIT|GL_CLIENT_VERTEX_ARRAY_BIT);

	glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &g_savedGLState.active_texunit);
	glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE_ARB, &g_savedGLState.active_clienttexunit);

	g_savedGLState.blending_enabled = glIsEnabled(GL_BLEND) ? true : false;

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	g_savedGLState.alphatest_enabled = glIsEnabled(GL_ALPHA_TEST) ? true : false;
	glGetIntegerv(GL_ALPHA_TEST_FUNC, &g_savedGLState.alphatest_func);
	glGetFloatv(GL_ALPHA_TEST_REF, &g_savedGLState.alphatest_value);

	// Disable these to avoid slowdown bug
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	gBSPRenderer.glClientActiveTextureARB(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	gBSPRenderer.glClientActiveTextureARB(GL_TEXTURE2);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	gBSPRenderer.glClientActiveTextureARB(GL_TEXTURE3);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	gBSPRenderer.glClientActiveTextureARB(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void R_RestoreGLStates( void )
{
	if(!g_glStateSaved)
		return;

	glPopAttrib();

	// load saved matrix for steam version
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	gBSPRenderer.glActiveTextureARB( g_savedGLState.active_texunit );
	gBSPRenderer.glClientActiveTextureARB( g_savedGLState.active_clienttexunit );

	if (g_savedGLState.blending_enabled) 
		glEnable(GL_BLEND);
	else 
		glDisable(GL_BLEND);

	if (g_savedGLState.alphatest_enabled) 
		glEnable(GL_ALPHA_TEST);
	else 
		glDisable(GL_ALPHA_TEST);

	glAlphaFunc(g_savedGLState.alphatest_func, g_savedGLState.alphatest_value);

#ifdef HL25_UPDATE
	if(g_areVertexShadersEnabled)
		glEnable(GL_VERTEX_SHADER);

	if(g_areVertexShadersEnabled)
		glEnable(GL_FRAGMENT_SHADER);

	gBSPRenderer.glUseProgram(g_shaderProgramBinding);
#endif

	g_glStateSaved = false;
}

/*
=================
R_Init

=================
*/
void R_Init( void )
{
	glPushAttrib(GL_TEXTURE_BIT);

	gPropManager.Init();
	gTextureLoader.Init();
	gBSPRenderer.Init();
	gParticleEngine.Init();
	gWaterShader.Init();
	gMirrorManager.Init();

	glPopAttrib();
}

/*
=================
R_VidInit

=================
*/
void R_VidInit( void )
{
	glPushAttrib(GL_TEXTURE_BIT);

	gTextureLoader.VidInit();
	gWaterShader.VidInit();
	gBSPRenderer.VidInit();
	gParticleEngine.VidInit();
	gMirrorManager.VidInit();
	g_StudioRenderer.VidInit();
	gPropManager.VidInit();

	glPopAttrib();
}

/*
=================
R_Shutdown

=================
*/
void R_Shutdown( void )
{
	glPushAttrib(GL_TEXTURE_BIT);

	gTextureLoader.Shutdown();
	gBSPRenderer.Shutdown();
	gPropManager.Shutdown();
	gWaterShader.Shutdown();
	gParticleEngine.Shutdown();

	glPopAttrib();
}