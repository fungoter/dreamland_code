/* $Id: gangchef.cpp,v 1.1.2.4.6.2 2008/04/14 19:36:15 rufina Exp $
 *
 * ruffina, 2003
 */

#include "gangsters.h"
#include "xmlattributegangsters.h"
#include "gangchef.h"

#include "room.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "fight.h"
#include "act.h"
#include "def.h"

bool GangChef::death( Character *killer ) 
{
    Gangsters *gquest = Gangsters::getThis( );

    if (!killer)
	return false;

    killer = gquest->getActor( killer );
    
    log("GangChef: killed by " << killer->getNameP( ));

    if (!gquest->isLevelOK( killer )) {
	act_p("�� �����! ��� �� ���� ������ �����?", killer, 0, 0, TO_CHAR, POS_RESTING);
	LogStream::sendNotice( ) << "BAD guy in the Lair: " << killer->getNameP( ) 
				 << " lvl " << killer->getModifyLevel( ) << endl;

	gquest->state = Gangsters::ST_BROKEN;		
    } else {
	gquest->state = Gangsters::ST_CHEF_KILLED;		
	gquest->chefKiller = killer->getName( );
    }
    
    gquest->scheduleDestroy( );
    return false;
}
   
void GangChef::greet( Character *mob ) 
{
    Gangsters *gquest = Gangsters::getThis( );

    if (!gquest->isLevelOK( gquest->getActor( mob ) )) {
	act_p("$c1 ����� '{R��� ������!{x'", ch, 0, mob, TO_VICT, POS_RESTING);
	gquest->exorcism( mob );
	return;
    }
	
    if (mob->is_npc( ))
	return;

    mob->getPC( )->getAttributes( ).getAttr<XMLAttributeGangsters>( 
	    gquest->getQuestID( ) )->setJoined( );

    if (is_safe_nomessage( ch, mob ))
	return;
    
    if (!ch->fighting)
	act_p("$c1 ����� ������ � ������ ������ �� ���������� � ����������� ������.",
	       ch, 0, mob, TO_ROOM, POS_RESTING);

    /* force guest to begin the fight */
    multi_hit( mob, ch );
}

void GangChef::fight( Character *victim ) 
{
    Character *mob, *ch_next;
    
    for (mob = ch->in_room->people; mob; mob = ch_next) {
	ch_next = mob->next_in_room;

	if (!mob->is_npc( ))
	    continue;
	
	if (mob == ch)
	    continue;

	if (!mob->getNPC( )->behavior 
	    || !mob->getNPC( )->behavior.getDynamicPointer<GangMob>( )) 
	    continue;	
	
	if (!mob->can_see( victim ))  	    
	    continue;
	
	if (mob->fighting)
	    continue;

	multi_hit( mob, victim );
    }
    
    GangMob::fight( victim );
}

bool GangChef::spec( ) 
{
    return true;
}

