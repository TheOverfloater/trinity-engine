/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

Client side entity manager
Written by Andrew Lucas
Transparency code by Neil "Jed" Jedrzejewski
*/

#if !defined( BSPLOADER_H )
#define BSPLOADER_H
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

class CPropManager
{
public:
	void Init( void );
	void VidInit( void );
	void Shutdown( void );
	void ClearEntityData( void );

	void GenerateEntityList( void );
	void SetupVBO( void );

	void Reset( void );
	void RenderProps( void );
	void RenderSkyProps( void );
	void RenderPropsSolid( void );

	// Models
	bool PostLoadModel( char *modelname, studiohdr_t *hdr, cl_entity_t *pEntity );
	bool LoadMDL(char *name, cl_entity_t *pEntity, entity_t *pBSPEntity);
	modeldata_t *GetHeader( const char *name );

	bool SetupCable( cabledata_t *cable, entity_t *entity );
	void DrawCables( void );

	void ParseEntities( void );
	void LoadEntVars( void );
	char *ValueForKey (entity_t *ent, char *key);
public:

	int	m_iEntDataSize;
	char *m_pEntData;

	bool m_bAvailable;

	// Entity list extracted from BSP.
	int	m_iNumBSPEntities;
	entity_t m_pBSPEntities[MAXRENDERENTS];

	// Total amount of renderable entities.
	int m_iNumEntities;
	cl_entity_t m_pEntities[MAXRENDERENTS];

	// Total amount of renderable entities.
	int m_iNumModelLights;
	cl_entity_t m_pModelLights[MAXRENDERENTS];

	// Studio header pointers.
	int m_iNumHeaders;
	modeldata_t	m_pHeaders[MAXRENDERENTS];

	// Decals
	decal_msg_cache m_pDecals[MAXRENDERENTS];
	int m_iNumDecals;

	cabledata_t		m_pCables[MAXRENDERENTS];
	int	m_iNumCables;

	cvar_t *m_pCvarDrawClientEntities;

	// Extra data for all entities.
	entextradata_t *m_pCurrentExtraData;
	entextradata_t m_pExtraData[MAXRENDERENTS];
	entextrainfo_t m_pExtraInfo[MAXRENDERENTS];
	int m_iNumExtraData;

	brushvertex_t *m_pVertexData;
	int	m_iNumTotalVerts;

	GLuint m_uiIndexBuffer;
	unsigned int *m_pIndexBuffer;
};

extern CPropManager gPropManager;
#endif