//==============================================================================//
//			Copyright (c) 2010 - 2011, Richard Roh·Ë, All rights reserved.		//
//==============================================================================//

//==============================================================================//
//							Optimized view frustum culling						//
//								Based on code from Quake						//
//					  Written for Half-Life: Amnesia modification				//
//==============================================================================//

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "frustum.h"

#include "ref_params.h"
#include "bsprenderer.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#define DEG2RAD( a ) ( a * M_PI ) / 180.0F

/*
====================
CalcFov

====================
*/
float FrustumCheck::CalcFov( float flFovX, float flWidth, float flHeight )
{
    float a;
    float x;

    if(flFovX < 1 || flFovX > 179)
	{
		gEngfuncs.Con_Printf("CalcFov(): bad field of view value!\n");
		flFovX = 90;
	}

    x = flWidth/tan(flFovX/360*M_PI);

    a = atan(flHeight/x);

    a = a*360/M_PI;

    return a;
}

/*
=====================
SetupFrustum

=====================
*/
void FrustumCheck::SetFrustum( Q_vec3_t vAngles, Q_vec3_t vOrigin, float flFOV_x, float flFarDist, bool bView )
{
	Q_vec3_t vVpn, vUp, vRight;
	Q_AngleVectors(vAngles, vVpn, vRight, vUp);

	if(flFOV_x == 90) 
	{
		VectorAdd(vVpn, vRight, m_sFrustum[0].vNormal);
		VectorSubtract(vVpn, vRight, m_sFrustum[1].vNormal);

		VectorAdd(vVpn, vUp, m_sFrustum[2].vNormal);
		VectorSubtract(vVpn, vUp, m_sFrustum[3].vNormal);
	}
	else
	{
		if(bView)
		{
			float flFOV_y = CalcFov(flFOV_x, ScreenWidth, ScreenHeight);

			Q_RotatePointAroundVector(m_sFrustum[0].vNormal, vUp, vVpn, -(90-flFOV_x / 2));
			Q_RotatePointAroundVector(m_sFrustum[1].vNormal, vUp, vVpn, 90-flFOV_x / 2);
			Q_RotatePointAroundVector(m_sFrustum[2].vNormal, vRight, vVpn, 90-flFOV_y / 2);
			Q_RotatePointAroundVector(m_sFrustum[3].vNormal, vRight, vVpn, -(90 - flFOV_y / 2));
		}
		else
		{
			Q_RotatePointAroundVector(m_sFrustum[0].vNormal, vUp, vVpn, -(90-flFOV_x / 2));
			Q_RotatePointAroundVector(m_sFrustum[1].vNormal, vUp, vVpn, 90-flFOV_x / 2);
			Q_RotatePointAroundVector(m_sFrustum[2].vNormal, vRight, vVpn, 90-flFOV_x / 2);
			Q_RotatePointAroundVector(m_sFrustum[3].vNormal, vRight, vVpn, -(90 - flFOV_x / 2));
		}
	}

	for(int i = 0; i < 4; i++)
	{
		m_sFrustum[i].type = PLANE_ANYZ;
		m_sFrustum[i].flDist = DotProduct(vOrigin, m_sFrustum[i].vNormal);
		m_sFrustum[i].signbits = Q_SignbitsForPlane(&m_sFrustum[i]);
	}

	// Reset this value
	m_iFarClip = FARCLIP_OFF;

	if(bView && !gHUD.m_pFogSettings.affectsky)
		return;

	if(flFarDist)
	{
		if(gBSPRenderer.m_pCvarRadialFog->value > 0 
			&& gBSPRenderer.m_bRadialFogSupport && bView)
		{
			m_vCullBoxMins[0] = vOrigin[0] - flFarDist;
			m_vCullBoxMins[1] = vOrigin[1] - flFarDist;
			m_vCullBoxMins[2] = vOrigin[2] - flFarDist;

			m_vCullBoxMaxs[0] = vOrigin[0] + flFarDist;
			m_vCullBoxMaxs[1] = vOrigin[1] + flFarDist;
			m_vCullBoxMaxs[2] = vOrigin[2] + flFarDist;
			m_iFarClip = FARCLIP_RADIAL;
		}
		else
		{
			Q_vec3_t vFarPoint;
			VectorCopy(vVpn, vFarPoint);

			vFarPoint[0] *= flFarDist;
			vFarPoint[1] *= flFarDist;
			vFarPoint[2] *= flFarDist;

			VectorAdd(vOrigin, vFarPoint, vFarPoint);

			m_sFrustum[4].vNormal[0] = vVpn[0] * (-1);
			m_sFrustum[4].vNormal[1] = vVpn[1] * (-1);
			m_sFrustum[4].vNormal[2] = vVpn[2] * (-1);

			m_sFrustum[4].type = PLANE_ANYZ;
			m_sFrustum[4].flDist = DotProduct(vFarPoint, m_sFrustum[4].vNormal);
			m_sFrustum[4].signbits = Q_SignbitsForPlane(&m_sFrustum[4]);
			m_iFarClip = FARCLIP_DEPTH;
		}
	}
}



/*
=====================
SetExtraCullBox

=====================
*/
void FrustumCheck::SetExtraCullBox( Q_vec3_t vMins, Q_vec3_t vMaxs )
{
	VectorCopy(vMins, m_vExtraCullMins);
	VectorCopy(vMaxs, m_vExtraCullMaxs);
	m_bExtraCull = true;
}

/*
=====================
DisableExtraCullBox

=====================
*/
void FrustumCheck::DisableExtraCullBox( void )
{
	m_bExtraCull = false;
}

/*
=====================
RadialCullBox

=====================
*/
bool FrustumCheck::RadialCullBox( Q_vec3_t vMins, Q_vec3_t vMaxs )
{
	if (m_vCullBoxMins[0] > vMaxs[0]) return true;
	if (m_vCullBoxMins[1] > vMaxs[1]) return true;
	if (m_vCullBoxMins[2] > vMaxs[2]) return true;

	if (m_vCullBoxMaxs[0] < vMins[0]) return true;
	if (m_vCullBoxMaxs[1] < vMins[1]) return true;
	if (m_vCullBoxMaxs[2] < vMins[2]) return true;

	return false;
}

/*
=====================
ExtraCullBox

=====================
*/
bool FrustumCheck::ExtraCullBox( Q_vec3_t vMins, Q_vec3_t vMaxs )
{
	if (m_vExtraCullMins[0] > vMaxs[0]) return true;
	if (m_vExtraCullMins[1] > vMaxs[1]) return true;
	if (m_vExtraCullMins[2] > vMaxs[2]) return true;

	if (m_vExtraCullMaxs[0] < vMins[0]) return true;
	if (m_vExtraCullMaxs[1] < vMins[1]) return true;
	if (m_vExtraCullMaxs[2] < vMins[2]) return true;

	return false;
}

/*
=====================
CullBox

=====================
*/
bool FrustumCheck::CullBox( Q_vec3_t vMins, Q_vec3_t vMaxs )
{	
	if(m_bExtraCull)
	{
		if(ExtraCullBox(vMins, vMaxs))
			return true;
	}

	if(m_iFarClip == FARCLIP_DEPTH)
	{
		if(Q_BoxOnPlaneSide(vMins, vMaxs, &m_sFrustum[4]) == 2)
			return true;
	}
	else if(m_iFarClip == FARCLIP_RADIAL)
	{
		if(RadialCullBox(vMins, vMaxs))
			return true;
	}

	for(int i = 0; i < 4; i++)
	{
		if(Q_BoxOnPlaneSide(vMins, vMaxs, &m_sFrustum[i]) == 2)
			return true;
	}

	return false;
}

/*
=====================
Q_AngleVectors

=====================
*/
void FrustumCheck::Q_AngleVectors( Q_vec3_t vAngles, Q_vec3_t vForward, Q_vec3_t vRight, Q_vec3_t vUp )
{
	float angle;
	float sr, sp, sy, cr, cp, cy;
	
	angle = vAngles[YAW] * (M_PI*2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = vAngles[PITCH] * (M_PI*2 / 360);
	sp = sin(angle);
	cp = cos(angle);
	angle = vAngles[ROLL] * (M_PI*2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	vForward[0] = cp*cy;
	vForward[1] = cp*sy;
	vForward[2] = -sp;
	vRight[0] = (-1*sr*sp*cy+-1*cr*-sy);
	vRight[1] = (-1*sr*sp*sy+-1*cr*cy);
	vRight[2] = -1*sr*cp;
	vUp[0] = (cr*sp*cy+-sr*-sy);
	vUp[1] = (cr*sp*sy+-sr*cy);
	vUp[2] = cr*cp;
}

/*
==================
Q_BoxOnPlaneSide

==================
*/
int FrustumCheck::Q_BoxOnPlaneSide( Q_vec3_t emins, Q_vec3_t emaxs, Q_mplane_t *p )
{
	float	dist1, dist2;
	int		sides;

	switch(p->signbits)
	{
		case 0:
			dist1 = p->vNormal[0]*emaxs[0] + p->vNormal[1]*emaxs[1] + p->vNormal[2]*emaxs[2];
			dist2 = p->vNormal[0]*emins[0] + p->vNormal[1]*emins[1] + p->vNormal[2]*emins[2];
			break;
		case 1:
			dist1 = p->vNormal[0]*emins[0] + p->vNormal[1]*emaxs[1] + p->vNormal[2]*emaxs[2];
			dist2 = p->vNormal[0]*emaxs[0] + p->vNormal[1]*emins[1] + p->vNormal[2]*emins[2];
			break;
		case 2:
			dist1 = p->vNormal[0]*emaxs[0] + p->vNormal[1]*emins[1] + p->vNormal[2]*emaxs[2];
			dist2 = p->vNormal[0]*emins[0] + p->vNormal[1]*emaxs[1] + p->vNormal[2]*emins[2];
			break;
		case 3:
			dist1 = p->vNormal[0]*emins[0] + p->vNormal[1]*emins[1] + p->vNormal[2]*emaxs[2];
			dist2 = p->vNormal[0]*emaxs[0] + p->vNormal[1]*emaxs[1] + p->vNormal[2]*emins[2];
			break;
		case 4:
			dist1 = p->vNormal[0]*emaxs[0] + p->vNormal[1]*emaxs[1] + p->vNormal[2]*emins[2];
			dist2 = p->vNormal[0]*emins[0] + p->vNormal[1]*emins[1] + p->vNormal[2]*emaxs[2];
			break;
		case 5:
			dist1 = p->vNormal[0]*emins[0] + p->vNormal[1]*emaxs[1] + p->vNormal[2]*emins[2];
			dist2 = p->vNormal[0]*emaxs[0] + p->vNormal[1]*emins[1] + p->vNormal[2]*emaxs[2];
			break;
		case 6:
			dist1 = p->vNormal[0]*emaxs[0] + p->vNormal[1]*emins[1] + p->vNormal[2]*emins[2];
			dist2 = p->vNormal[0]*emins[0] + p->vNormal[1]*emaxs[1] + p->vNormal[2]*emaxs[2];
			break;
		case 7:
			dist1 = p->vNormal[0]*emins[0] + p->vNormal[1]*emins[1] + p->vNormal[2]*emins[2];
			dist2 = p->vNormal[0]*emaxs[0] + p->vNormal[1]*emaxs[1] + p->vNormal[2]*emaxs[2];
			break;
		default:
			dist1 = dist2 = 0;
			gEngfuncs.Con_Printf("Q_BoxOnPlaneSide error\n");
			break;
	}

	sides = 0;
	if(dist1 >= p->flDist)
		sides = 1;
	if(dist2 < p->flDist)
		sides |= 2;

	return sides;
}

/*
==================
Q_SignbitsForPlane

==================
*/
int FrustumCheck::Q_SignbitsForPlane( Q_mplane_t *pOut )
{
	int	nBits = 0;

	for(int j = 0; j < 3; j++)
	{
		if(pOut->vNormal[j] < 0)
			nBits |= 1<<j;
	}

	return nBits;
}

/*
==========================
Q_RotatePointAroundVector

==========================
*/
void FrustumCheck::Q_RotatePointAroundVector( Q_vec3_t vDest, const Q_vec3_t vDir, Q_vec3_t vPoint, float flDegrees )
{
	float q[3];
	float q3;
	float t[3];
	float t3;

	{
		float hrad;
		float s;

		hrad = DEG2RAD(flDegrees) / 2;
		s = sin(hrad);
		VectorScale(vDir, s, q);
		q3 = cos(hrad);
	}

	Q_CrossProduct(q, vPoint, t);
	VectorMA(t, q3, vPoint, t);
	t3 = DotProduct(q, vPoint);

	Q_CrossProduct(q, t, vDest);
	VectorMA(vDest, t3, q, vDest);
	VectorMA(vDest, q3, t, vDest);
}

/*
=================
Q_CrossProduct

=================
*/
void FrustumCheck::Q_CrossProduct( Q_vec3_t v1, Q_vec3_t v2, Q_vec3_t cross )
{
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}