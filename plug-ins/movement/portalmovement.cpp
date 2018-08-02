/* $Id$
 *
 * ruffina, 2004
 */
#include "portalmovement.h"
#include "move_utils.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skillreference.h"
#include "clanreference.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"

#include "fight_exception.h"
#include "damage_impl.h"
#include "damageflags.h"
#include "interp.h"
#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

CLAN(battlerager);
GSN(spellbane);

PortalMovement::PortalMovement( Character *ch, Object *portal )
                  : Walkment( ch )
{
    this->portal = portal;
}

static DLString oprog_portal_location( Object *portal, Character *ch )
{
    FENIA_STR_CALL( portal, "PortalLocation", "C", ch );
    FENIA_NDX_STR_CALL( portal, "PortalLocation", "OC", portal, ch );
    return "";
}

bool PortalMovement::findTargetRoom( )
{
    DLString rc = oprog_portal_location( portal, ch );
    int targetVnum = (rc.empty( ) || !rc.isNumber( ) ? 0 : rc.toInt( ));

    if (targetVnum != 0) {
	to_room = get_room_index( targetVnum );
    }
    else if (IS_SET(portal->value[2],GATE_RANDOM) || portal->value[3] == -1) {
	to_room = get_random_room( ch );
	portal->value[3] = to_room->vnum; /* keeps record */
    }
    else if (IS_SET(portal->value[2], GATE_BUGGY) && (number_percent( ) < 5)) {
	to_room = get_random_room( ch );
    }
    else {
	to_room = get_room_index( portal->value[3] );
    }

    if (to_room == 0) {
	 msgSelfParty( ch,
		       "%4$^O1 �� ����� ���������� ���.", 
		       "%4$^O1 �� ����� ���������� ���." );
	 return false;
    }

    return true;
}

bool PortalMovement::isNormalExit( )
{
    return IS_SET(portal->value[2], GATE_NORMAL_EXIT);
}

bool PortalMovement::canLeaveMaster( Character *wch )
{
    return !isNormalExit( ) || Walkment::canLeaveMaster( wch );
}

bool PortalMovement::canMove( Character *wch )
{
    return checkCharges( )
	    && Walkment::canMove( wch )
	    && checkCurse( wch );
}

bool PortalMovement::tryMove( Character *wch )
{
    return Walkment::tryMove( wch )
	    && applySpellbane( wch );
}

bool PortalMovement::checkCharges( )
{
    return portal->value[0] >= 0;
}

int PortalMovement::move( )
{
    if (!moveRecursive( )) 
	return RC_MOVE_FAIL;

    if (IS_SET(portal->value[2], GATE_GOWITH)) { 
	obj_from_room( portal );
	obj_to_room( portal, to_room );
    }

    if (portal->value[0] == -1) {
	portal->getRoom( )->echo( POS_RESTING, 
		                  "%^O1 �������� �������� � �����.", 
				  portal );
	extract_obj( portal );
    }

    return RC_MOVE_OK;
}

void PortalMovement::moveOneFollower( Character *wch, Character *fch )
{
    act( "�� �������� �� $C5.", fch, 0, wch, TO_CHAR );
    PortalMovement( fch, portal ).moveRecursive( );
}

bool PortalMovement::moveAtomic( )
{
    if (!Walkment::moveAtomic( ))
	return false;

    if (portal->value[0] > 0) 
	if (--portal->value[0] == 0)
	    portal->value[0] = -1;

    return true;
}

bool PortalMovement::checkClosedDoor( Character *wch )
{
    if (!IS_SET(portal->value[1], EX_CLOSED))
	return true;
	
    if (wch->get_trust( ) >= ANGEL)
	return true;

    if (IS_GHOST( wch ))
	return true;
    
    rc = RC_MOVE_CLOSED;
    msgSelfParty( wch, 
	          "%4$^O1: ��� �������.",
	          "%4$^O1: ��� �������." );
    return false;
}

bool PortalMovement::checkSafe( Character *wch )
{
    return !isNormalExit( ) || Walkment::checkSafe( wch );
}

bool PortalMovement::checkAir( Character *wch )
{
    return !isNormalExit( ) || Walkment::checkAir( wch );
}

bool PortalMovement::checkWater( Character *wch )
{
    return !isNormalExit( ) || Walkment::checkWater( wch );
}

bool PortalMovement::checkCurse( Character *wch )
{
    if (wch->get_trust( ) >= ANGEL)
	return true;
    
    if (IS_SET(portal->value[2], GATE_NOCURSE))
	return true;
    
    if (IS_AFFECTED(wch, AFF_CURSE)) {
	msgSelfParty( wch, 
		      "���� ��������� ������ ���� ����� � %4$O4.",
		      "��������� %2$C2 ������ %2$P3 ����� � %4$O4." );
	return false;
    }
    
    if (IS_SET(from_room->room_flags, ROOM_NO_RECALL)
	 || IS_RAFFECTED(from_room, AFF_ROOM_CURSE))
    {
	msgSelfParty( wch,
		      "��������� ����� ����� ������ ���� ��� ��������.",
		      "��������� ����� ����� ������ %2$C3 ��� ��������." );
	return false;
    }
    
    return true;
}

bool PortalMovement::applyWeb( Character *wch )
{
    return !isNormalExit( ) || Walkment::applyWeb( wch );
}

bool PortalMovement::applySpellbane( Character *wch )
{
    if (wch->is_npc( ))
	return true;
    
    if (!IS_SET(portal->extra_flags, ITEM_MAGIC))
	return true;
		
    if (wch->getClan( ) != clan_battlerager)
	return true;
    
    if (!wch->isAffected( gsn_spellbane ))
	return true;
    
    try {
	act_p("����� $o2 ������������ � ����� spellbane!", wch,portal,0,TO_CHAR,POS_RESTING);
	act_p("����� $o2 ������������ �� spellbane $c2!", wch,portal,0,TO_ROOM,POS_RESTING);
	SkillDamage( wch, wch, gsn_spellbane, DAM_NEGATIVE, wch->max_hit / 3, DAMF_SPELL ).hit( true );
	interpret_raw( wch, "cb", "���� ������� ���������� ��������!" );
    }
    catch (const VictimDeathException &) {
	interpret_raw( wch, "cb", "���� ����� ���������� ��������!" );
	return false;
    }
    
    return true;
}

bool PortalMovement::applyMovepoints( Character *wch )
{
    return !isNormalExit( ) || Walkment::applyMovepoints( wch );
}

int PortalMovement::getMoveCost( Character *wch )
{
    return 1;
}

void PortalMovement::msgOnMove( Character *wch, bool fLeaving )
{
    DLString msg;
    
    if (MOUNTED(wch))
	return;

    if (wch->is_mirror( ))
	return;

    if (ch->invis_level >= LEVEL_HERO || wch->invis_level >= LEVEL_HERO)
	return;
    
    if (fLeaving) {
	if (RIDDEN(wch))
	    msgRoomNoParty( wch,
			    "%1^C1 ������ � %4$O4 ������ �� %2$C6." );
	else	
	    msgRoomNoParty( wch,
			    "%1^C1 ������ � %4$O4." );
	
	if (isNormalExit( )) 
	    msg = "�� ������� � %4$O4.";
	else
	    msg = "�� ������� � %4$O4 � ������������ � ������ �����...";
	
	msgSelf( wch, msg.c_str( ) );
	if (RIDDEN(wch))
	    msgSelf( RIDDEN(wch), msg.c_str( ) );
    }
    else {
	if (isNormalExit( )) 
	    msg = "%1$^C1 ����������";
	else
	    msg = "%1$^C1 ���������� �� %4$O2";

	if (RIDDEN(wch))
	    msg << " ������ �� %2$C6";

	msg << ".";
	msgRoomNoParty( wch, msg.c_str( ) );
    }
}

void PortalMovement::msgEcho( Character *victim, Character *wch, const char *msg )
{
    if (canHear( victim, wch ))
	victim->pecho( msg, 
	               (RIDDEN(wch) ? wch->mount : wch),
	               (MOUNTED(wch) ? wch->mount : wch),
		       wch, 
		       portal );
}

