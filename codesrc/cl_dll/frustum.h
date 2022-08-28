//==============================================================================//
//			Copyright (c) 2010 - 2011, Richard Roh·Ë, All rights reserved.		//
//==============================================================================//

#pragma once
#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "windows.h"
#include "gl/gl.h"
#include "pm_defs.h"
#include "cl_entity.h"
#include "com_model.h"
#include "ref_params.h"
#include "dlight.h"
#include "parsemsg.h"
#include "cvardef.h"

#define PITCH				0
#define YAW					1
#define ROLL				2

#define FARCLIP_OFF			0
#define	FARCLIP_DEPTH		1
#define FARCLIP_RADIAL		2

typedef float Q_vec3_t[3];

typedef struct Q_mplane_s
{
    Q_vec3_t	vNormal;
    float		flDist; 
    byte		type;
    byte		signbits;
    byte		pad[2];
} Q_mplane_t;

/*
===============
CFrustum

===============
*/
class FrustumCheck
{
public:
	void SetFrustum( Q_vec3_t vAngles, Q_vec3_t vOrigin, float flFOV, float flFarDist = 0, bool bView = false );
	bool CullBox( Q_vec3_t vMins, Q_vec3_t vMaxs );
	bool RadialCullBox( Q_vec3_t vMins, Q_vec3_t vMaxs );
	bool ExtraCullBox( Q_vec3_t vMins, Q_vec3_t vMaxs );

	void SetExtraCullBox( Q_vec3_t vMins, Q_vec3_t vMaxs );
	void DisableExtraCullBox( void );
	
	float CalcFov( float flFovX, float flWidth, float flHeight );
	void Q_AngleVectors( Q_vec3_t vAngles, Q_vec3_t vForward, Q_vec3_t vRight, Q_vec3_t vUp );
	int  Q_BoxOnPlaneSide( Q_vec3_t emins, Q_vec3_t emaxs, Q_mplane_t *p );
	int	 Q_SignbitsForPlane( Q_mplane_t *pOut );
	void Q_RotatePointAroundVector( Q_vec3_t vDest, const Q_vec3_t vDir, Q_vec3_t vPoint, float flDegrees );
	void Q_CrossProduct( Q_vec3_t v1, Q_vec3_t v2, Q_vec3_t cross );

private:
	Q_mplane_t	m_sFrustum[5];
	int		m_iFarClip;

	Q_vec3_t	m_vCullBoxMins;
	Q_vec3_t	m_vCullBoxMaxs;

	bool		m_bExtraCull;
	Q_vec3_t	m_vExtraCullMins;
	Q_vec3_t	m_vExtraCullMaxs;	
};
#endif