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

#if !defined ( MIRRORMANAGER_H )
#define MIRRORMANAGER_H
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
CMirrorManager

====================
*/
class CMirrorManager
{
public:
	void Init( void );
	void VidInit( void );

	void DrawMirrors( void );
	void DrawMirrorPasses( ref_params_t *pparams );

	void SetupMirrorPass( void );
	void FinishMirrorPass( void );
	void DrawMirrorPass( void );

	void GetEntityList ( void );
	void AddTempEntity( cl_entity_t *pEntity );

	void AllocNewMirror( cl_entity_t *entity );
	void SetupClipping( void );

public:
	cvar_t *m_pCvarDrawMirrors;
	cvar_t *m_pCvarMirrorPlayer;

	cl_entity_t *m_pTempEntities[512];
	int m_iNumTempEntities;

	ref_params_t m_pMirrorParams;
	ref_params_t *m_pViewParams;

	cl_mirror_t *m_pCurrentMirror;
	cl_mirror_t m_pMirrors[MAX_MIRRORS];
	int m_iNumMirrors;
};

extern CMirrorManager gMirrorManager;
#endif