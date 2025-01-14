//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "extinguisherjet.h"
#include "engine/IEngineSound.h"
#include "fire.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar fire_extinguisher_strength;

//-----------------------------------------------------------------------------
// Networking
//-----------------------------------------------------------------------------
IMPLEMENT_SERVERCLASS_ST( CExtinguisherJet, DT_ExtinguisherJet )
	SendPropInt( SENDINFO( m_bEmit ), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_bUseMuzzlePoint ), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nLength ), 32, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nSize ), 32, SPROP_UNSIGNED ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Save/load 
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CExtinguisherJet )

	//Regular fields
	DEFINE_FIELD( m_bEmit,		FIELD_BOOLEAN ),
	
	DEFINE_KEYFIELD( m_bEnabled,	FIELD_BOOLEAN, "enabled" ),
	DEFINE_KEYFIELD( m_nLength,	FIELD_INTEGER, "length" ),
	DEFINE_KEYFIELD( m_nSize,		FIELD_INTEGER, "size" ),
	DEFINE_KEYFIELD( m_nRadius,	FIELD_INTEGER, "radius" ),
	DEFINE_KEYFIELD( m_flStrength,FIELD_FLOAT,   "strength" ),

	DEFINE_FIELD( m_bAutoExtinguish,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bUseMuzzlePoint,	FIELD_BOOLEAN ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable",	InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable",	InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle",	InputToggle ),

	DEFINE_FUNCTION( ExtinguishThink ),

END_DATADESC()


LINK_ENTITY_TO_CLASS( env_extinguisherjet, CExtinguisherJet );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CExtinguisherJet::CExtinguisherJet( void )
{
	m_bEmit				= false;
	m_bEnabled			= false;
	m_bAutoExtinguish	= true;

	m_nLength			= 128;
	m_nSize				= 8;
	m_flStrength		= fire_extinguisher_strength.GetFloat();
	m_nRadius			= 32;

	// Send to the client even though we don't have a model
	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExtinguisherJet::Spawn( void )
{
	Precache();

	if ( m_bEnabled )
	{
		TurnOn( NULL );
	}
}

void CExtinguisherJet::Precache()
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExtinguisherJet::TurnOn( CBaseCombatWeapon *weapon )
{
	//Turn on sound
	if ( m_bEmit == false )
	{
		if ( weapon != NULL )
		{
			weapon->WeaponSound( SINGLE );
		}
		m_bEnabled = m_bEmit = true;
	}
	
	SetThink( &CExtinguisherJet::ExtinguishThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExtinguisherJet::TurnOff( CBaseCombatWeapon *weapon )
{
	//Turn off sound
	if ( m_bEmit )
	{
		if ( weapon != NULL )
		{
			weapon->WeaponSound(SPECIAL1);
		}
		m_bEnabled = m_bEmit = false;
	}
	
	SetThink( NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CExtinguisherJet::InputEnable( inputdata_t &inputdata )
{
	TurnOn( NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CExtinguisherJet::InputDisable( inputdata_t &inputdata )
{
	TurnOff( NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CExtinguisherJet::InputToggle( inputdata_t &inputdata )
{
	if ( m_bEnabled )
	{
		TurnOff( NULL );
	}
	else
	{
		TurnOn( NULL );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExtinguisherJet::Think( void )
{
	CBaseEntity::Think();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExtinguisherJet::ExtinguishThink( void )
{
	SetNextThink( gpGlobals->curtime + 0.1f );

	if ( m_bEnabled == false )
		return;

	if ( m_bAutoExtinguish == false )
		return;

	Vector	vTestPos;
	Vector	vForward, vRight, vUp;

	AngleVectors( GetAbsAngles(), &vForward );
	
	vTestPos = GetAbsOrigin() + ( vForward * m_nLength );

	trace_t	tr;
	UTIL_TraceLine( GetAbsOrigin(), vTestPos, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	//Extinguish the fire where we hit
	FireSystem_ExtinguishInRadius( tr.endpos, m_nRadius, m_flStrength );
}
