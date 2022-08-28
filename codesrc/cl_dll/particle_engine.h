/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

Particle Engine
Written by Andrew Lucas
*/

#if !defined ( PARTICLE_ENGINE_H )
#define PARTICLE_ENGINE_H
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
CParticleEngine

====================
*/
class CParticleEngine
{
public:
	void Init( void );
	void VidInit( void );
	void Shutdown( void );

	void CreateCluster( char *szPath, vec3_t origin, vec3_t dir, int iId );
	particle_system_t *CreateSystem( char *szPath, vec3_t origin, vec3_t dir, int iId, particle_system_t *parent = NULL );
	void RemoveSystem( int iId );

	particle_system_t *AllocSystem( void );
	cl_particle_t *AllocParticle( particle_system_t *pSystem );

	void Update( void );
	void DrawParticles( void );
	void CullSystems( void );
	void UpdateSystems( void );

	vec3_t LightForParticle( cl_particle_t *pParticle );
	bool CheckLightBBox( cl_particle_t *pParticle, cl_dlight_t *pLight );

	void EnvironmentCreateFirst( particle_system_t *pSystem );

	void CreateParticle( particle_system_t *pSystem, float *flOrigin = NULL, float *flNormal = NULL );
	bool UpdateParticle( cl_particle_t *pParticle );
	void RenderParticle( cl_particle_t *pParticle, float flUp, float flRight );

	int MsgCreateSystem( const char *pszName, int iSize, void *pbuf );

public:
	particle_system_t	*m_pSystemHeader;

	cvar_t *m_pCvarDrawParticles;
	cvar_t *m_pCvarParticleDebug;
	cvar_t *m_pCvarGravity;

	float m_flLastDraw;
	float m_flFrameTime;

	int m_iNumParticles;

	int	m_iNumFreedParticles;
	int	m_iNumCreatedParticles;

	int m_iNumFreedSystems;
	int	m_iNumCreatedSystems;

	vec3_t m_vForward;
	vec3_t m_vRight;
	vec3_t m_vUp;

	vec3_t m_vRRight;
	vec3_t m_vRUp;
};
extern CParticleEngine gParticleEngine;
#endif