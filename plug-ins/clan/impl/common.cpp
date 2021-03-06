/* $Id$
 *
 * ruffina, 2004
 */
#include "recallmovement.h"

#include "skillreference.h"
#include "skillcommandtemplate.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "affect.h"

#include "act.h"
#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(clanrecall);

/*-----------------------------------------------------------------------
 * ClanRecallMovement
 *-----------------------------------------------------------------------*/
class ClanRecallMovement : public RecallMovement {
public:
    ClanRecallMovement( Character *ch )
                   : RecallMovement( ch )
    {
    }

    ClanRecallMovement( Character *ch, Character *actor, Room *to_room )
                   : RecallMovement( ch, actor, to_room )
    {
    }

protected:
    virtual bool findTargetRoom( )
    {
	int point;
	
	if (to_room)
	    return true;

	if (( point = ch->getClan( )->getRecallVnum( ) ) <= 0) {
	    msgSelf( ch, "�� � ���� ��� ��������� ����." );
	    return false;
	}
	
	if (!gsn_clanrecall->available( ch )) {
	    msgSelf( ch, "����� ��� ���� �� ����� ������." );
	    return false;
	}
	
	if (!( to_room = get_room_index( point ) )) {
	    msgSelf( ch, "�� ��������%1G���|��|���." );
	    return false;
	}

	return true;
    }
    virtual bool canMove( Character *wch )
    {
	if (ch != actor)
	    return true;
	else
	    return checkMount( )
		   && checkShadow( )
		   && checkBloody( wch )
		   && checkPostAffect( )
		   && checkSameRoom( )
		   && checkForsaken( wch );
    }
    virtual bool tryMove( Character *wch )
    {
	if (ch != actor)
	    return applyInvis( wch );
	else
	    return applyInvis( wch )
		   && applyFightingSkill( wch, gsn_clanrecall )
		   && applyMovepoints( )
		   && applyPostAffect( );
    }
    virtual void msgOnMove( Character *wch, bool fLeaving )
    {
	if (fLeaving)
	    msgRoomNoParty( wch,
		            "%1$^C1 ��������.",
			    "%1$^C1 � %2$C1 ��������." );
	else
	    msgRoomNoParty( wch, 
	                    "%1$^C1 ���������� � �������.",
			    "%1$^C1 � %2$C1 ���������� � �������." );
    }
    virtual void movePet( NPCharacter *pet )
    {
	ClanRecallMovement( pet, ch, to_room ).moveRecursive( );
    }
    bool checkPostAffect( )
    {
	if (ch->isAffected( gsn_clanrecall )) {
	    msgSelf( ch, "������� ���� ������� ������ � ��������� �������." );
	    return false;
	}

	return true;
    }
    bool applyPostAffect( )
    {
	postaffect_to_char( ch, gsn_clanrecall, ch->getModifyLevel( ) / 6 + 1 );
	return true;
    }
};

/*
 * 'crecall' skill command
 */

SKILL_RUNP( clanrecall )
{
    ClanRecallMovement( ch ).move( );
}

