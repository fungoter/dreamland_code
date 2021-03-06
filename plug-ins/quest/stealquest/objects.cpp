/* $Id: objects.cpp,v 1.1.2.9.6.3 2010/01/01 15:48:15 rufina Exp $
 *
 * ruffina, 2004
 */

#include "objects.h"
#include "stealquest.h"
#include "mobiles.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "act.h"

/* 
 * HiddenChest 
 */
bool HiddenChest::canLock( Character *ch ) 
{ 
    StealQuest::Pointer quest;

    if (!ourHero( ch ))
	return false;
    
    if (!( quest = getMyQuest<StealQuest>( ch->getPC( ) ) ))
	return false;

    quest->wiznet( "", "%s tries to unlock the chest", ch->getNameP( '1' ).c_str( ) );
    return quest->getItemList<LockPick>( ch->carrying );
}



/* 
 * LockPick 
 */
void LockPick::getByHero( PCharacter *ch ) 
{
    getQuest( ch )->wiznet( "", "%s gets key", ch->getNameP( '1' ).c_str( ) );
    act( "$o1 ������ ������������.", ch, obj, 0, TO_CHAR );
}

void LockPick::getByOther( Character *ch ) 
{ 
    act( "�� ������� $o4.", ch, obj, 0, TO_CHAR );
    act( "$c1 ������ $o4.", ch, obj, 0, TO_ROOM );
}

bool LockPick::ourMobile( NPCharacter *mob ) 
{
    return ObjQuestBehavior::ourMobile( mob )
           && mob->behavior.getDynamicPointer<Robber>( );
}


/* 
 * RobbedItem 
 */	
void RobbedItem::getByHero( PCharacter *ch ) 
{
    getQuest( ch )->wiznet( "", "%s gets item", ch->getNameP( '1' ).c_str( ) );
    ch->pecho( "%1$^O1 ����%1$n��|�� ��������� � �������.", obj );
}

void RobbedItem::getByOther( Character *ch ) 
{
    ch->pecho( "%1$^O1 ������%1$n��|�� � ���� �� ���.", obj );
    act( "$c1 ������ $o4.", ch, obj, 0, TO_ROOM );
}

