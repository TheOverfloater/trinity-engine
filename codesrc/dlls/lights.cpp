/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== lights.cpp ========================================================

  spawn and think functions for editor-placed lights

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
//RENDERERS START
#include "player.h"

class CLight : public CPointEntity
{
public:
	virtual void	KeyValue( KeyValueData* pkvd ); 
	virtual void	SendInitMessage( CBasePlayer *player );
	void EXPORT	LightStyleThink( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

private:
	int		m_iStyle;
	int		m_iszPattern;
	BOOL	m_bAlreadySent;
};
LINK_ENTITY_TO_CLASS( light, CLight );

TYPEDESCRIPTION	CLight::m_SaveData[] = 
{
	DEFINE_FIELD( CLight, m_iStyle, FIELD_INTEGER ),
	DEFINE_FIELD( CLight, m_iszPattern, FIELD_STRING ),
	DEFINE_FIELD( CLight, m_bAlreadySent, FIELD_BOOLEAN ),
};

IMPLEMENT_SAVERESTORE( CLight, CPointEntity );

//
// Cache user-entity-field values until spawn is called.
//
extern int gmsgLightStyle;
void CLight :: SendInitMessage( CBasePlayer *player )
{
	char szPattern[64];
	memset(szPattern, 0, sizeof(szPattern));

	if (m_iStyle >= 32)
	{
		if (FBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
			strcpy(szPattern,"a");
		else if (m_iszPattern)
			strcpy(szPattern,(char *)STRING( m_iszPattern ));
		else
			strcpy(szPattern,"m");

		if(player)
			MESSAGE_BEGIN(MSG_ONE, gmsgLightStyle, NULL, player->pev);
		else
			MESSAGE_BEGIN(MSG_ALL, gmsgLightStyle, NULL);

			WRITE_BYTE( m_iStyle );
			WRITE_STRING( szPattern );
		MESSAGE_END();
	}

	m_bAlreadySent = TRUE;
}
void CLight :: KeyValue( KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "style"))
	{
		m_iStyle = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pitch"))
	{
		pev->angles.x = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pattern"))
	{
		m_iszPattern = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
	{
		CPointEntity::KeyValue( pkvd );
	}
}
void CLight :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	char szPattern[64];
	memset(szPattern, 0, sizeof(szPattern));

	if (m_iStyle >= 32)
	{
		if ( !ShouldToggle( useType, !FBitSet(pev->spawnflags, SF_LIGHT_START_OFF) ) )
			return;

		if (FBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
		{
			if (m_iszPattern)
				strcpy(szPattern,(char *)STRING( m_iszPattern ));
			else
				strcpy(szPattern,"m");
			ClearBits(pev->spawnflags, SF_LIGHT_START_OFF);
		}
		else
		{
			strcpy(szPattern,"a");
			SetBits(pev->spawnflags, SF_LIGHT_START_OFF);
		}
	}

	MESSAGE_BEGIN(MSG_ALL, gmsgLightStyle, NULL);
			WRITE_BYTE(m_iStyle);
			WRITE_STRING(szPattern);
	MESSAGE_END();
	LIGHT_STYLE(m_iStyle,szPattern); 
}
//RENDERERS END

//
// shut up spawn functions for new spotlights
//
LINK_ENTITY_TO_CLASS( light_spot, CLight );


class CEnvLight : public CLight
{
public:
	void	KeyValue( KeyValueData* pkvd ); 
	void	Spawn( void );
};

LINK_ENTITY_TO_CLASS( light_environment, CEnvLight );

void CEnvLight::KeyValue( KeyValueData* pkvd )
{
	if (FStrEq(pkvd->szKeyName, "_light"))
	{
		int r, g, b, v, j;
		char szColor[64];
		j = sscanf( pkvd->szValue, "%d %d %d %d\n", &r, &g, &b, &v );
		if (j == 1)
		{
			g = b = r;
		}
		else if (j == 4)
		{
			r = r * (v / 255.0);
			g = g * (v / 255.0);
			b = b * (v / 255.0);
		}

		// simulate qrad direct, ambient,and gamma adjustments, as well as engine scaling
		r = pow( r / 114.0, 0.6 ) * 264;
		g = pow( g / 114.0, 0.6 ) * 264;
		b = pow( b / 114.0, 0.6 ) * 264;

		pkvd->fHandled = TRUE;
		sprintf( szColor, "%d", r );
		CVAR_SET_STRING( "sv_skycolor_r", szColor );
		sprintf( szColor, "%d", g );
		CVAR_SET_STRING( "sv_skycolor_g", szColor );
		sprintf( szColor, "%d", b );
		CVAR_SET_STRING( "sv_skycolor_b", szColor );
	}
	else
	{
		CLight::KeyValue( pkvd );
	}
}


void CEnvLight :: Spawn( void )
{
	char szVector[64];
	UTIL_MakeAimVectors( pev->angles );

	sprintf( szVector, "%f", gpGlobals->v_forward.x );
	CVAR_SET_STRING( "sv_skyvec_x", szVector );
	sprintf( szVector, "%f", gpGlobals->v_forward.y );
	CVAR_SET_STRING( "sv_skyvec_y", szVector );
	sprintf( szVector, "%f", gpGlobals->v_forward.z );
	CVAR_SET_STRING( "sv_skyvec_z", szVector );

	CLight::Spawn( );
}
