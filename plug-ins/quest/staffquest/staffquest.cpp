/* $Id: staffquest.cpp,v 1.1.2.24.6.4 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2003
 */

#include "staffquest.h"
#include "questexceptions.h"
#include "staffbehavior.h"

#include "selfrate.h"

#include "pcharacter.h"
#include "object.h"
#include "room.h"

#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

/*
 * StaffQuest
 */
void StaffQuest::create( PCharacter *pch, NPCharacter *questman ) 
{
    Object *eyed;
    int time;

    charName = pch->getName( );

    try {
	scenName = StaffQuestRegistrator::getThis( )->getRandomScenario( pch );
	eyed = createStaff( getRandomRoomClient( pch ) );
    } 
    catch (const QuestCannotStartException &e) {
	destroy( );
	throw e;
    }

    areaName = eyed->in_room->area->name;
    roomName = eyed->in_room->name;
    objName  = eyed->getShortDescr( );
    
    time = number_range( 15, 25 ); 
    setTime( pch, time );
    
    getScenario( ).onQuestStart( pch, questman );
    tell_raw( pch, questman, "���������� ���������� ����������, ��� �������� ���������� ���������." );
    tell_raw( pch, questman, "���� ���������� ��������� ��� ���!" );	
    tell_raw( pch, questman, "�����, ��� ��� ��������, ���������� {W%s{G", roomName.c_str( ) );
    tell_raw( pch, questman, "� ��������� ��� ����� � ������ ��� ��������� - {W%s{G", areaName.c_str( ) );
    tell_raw( pch, questman, "� ���� ���� {Y%d{G �����%s �� ���������� �������.",
		  time, GET_COUNT(time,"�","�","") ); 
    
    wiznet( scenName.c_str( ), "in room \"%s\" area \"%s\"", 
                               roomName.c_str( ), areaName.c_str( ) );
}

bool StaffQuest::isComplete( ) 
{
    PCharacter *ch = getHeroWorld( );
    
    if (!ch)
	return false;

    return getItemList<StaffBehavior>( ch->carrying ) != NULL;
}

Room * StaffQuest::helpLocation( )
{
    Object *obj = getItemWorld<StaffBehavior>( );
    
    return (obj ? obj->in_room : NULL);
}

void StaffQuest::info( std::ostream &buf, PCharacter *ch ) 
{
    if (isComplete( ))
	buf << "���� ������� {Y���������{x!" << endl
	    << "������� �� ���������������, �� ���� ��� ������ �����!" << endl;
    else 
	buf << "� ���� ������� - ������� " << russian_case( objName.getValue( ), '4' ) << "." << endl
	    << "�����, ��� �� �������, ���������� " << roomName << "." << endl
	    << "� ��������� ��� ����� � ������ ��� ��������� " << areaName << endl;
}

void StaffQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    if (isComplete( ))
	buf << "��������� � �������� �� ��������.";
    else 
        buf << "��������� �������� " << russian_case( objName.getValue( ), '4' ) << " �� "
	    << roomName << " (" << areaName << ").";
}

Quest::Reward::Pointer StaffQuest::reward( PCharacter *ch, NPCharacter *questman ) 
{
    Reward::Pointer r( NEW );

    if (hint.getValue( )) {
	r->gold = number_range( 1, 2 );
	r->points = number_range( 1, 4 );
    }
    else {
	r->gold = number_range( 5, 10 );
	r->points = number_range( 5, 10 );
	r->wordChance = 2 * r->points;
	r->scrollChance = number_range( 3, 7 );

	if (chance( 10 ))
	    r->prac = 1;
    }
    
    r->exp = (r->points + r->clanpoints) * 10;

    act_p("�� ��������� $n4 $C3.", ch, objName.getValue( ).c_str( ), questman, TO_CHAR, POS_RESTING);
    act_p("$c1 �������� $n4 $C3.", ch, objName.getValue( ).c_str( ), questman, TO_ROOM, POS_RESTING);

    return Reward::Pointer( r );
}

void StaffQuest::destroy( ) 
{
    destroyItem<StaffBehavior>( );
}

bool StaffQuest::checkRoomClient( PCharacter *pch, Room *room )
{
    if (room->area->high_range + 20 < pch->getModifyLevel( ))
	return false;

    if (IS_WATER(room) || room->sector_type == SECT_AIR)
	return false;

    if (!ItemQuestModel::checkRoomClient( pch, room ))
	return false;

    return true;
}

StaffScenario & StaffQuest::getScenario( )
{
    return *(StaffQuestRegistrator::getThis( )->getScenario( scenName ).getStaticPointer<StaffScenario>( ));
}

Object * StaffQuest::createStaff( Room *room )
{
    Object *eyed;
    
    eyed = createItem<StaffBehavior>( StaffQuestRegistrator::getThis( )->objVnum );
    getScenario( ).dress( eyed );
    obj_to_room( eyed, room );
    return eyed;
}

/*
 * StaffScenario
 */
bool StaffScenario::applicable( PCharacter *pch ) 
{
    return true;
}

void StaffScenario::onQuestStart( PCharacter *pch, NPCharacter *questman )
{
    if (msg.empty( ))
	tell_raw( pch, questman, "�� ����������� ������������ �������� {W%s{G!", 
	          shortDesc.ruscase( '4' ).c_str( ) );
    else
	tell_raw( pch, questman, msg.c_str( ) );
}

/*
 * StaffQuestRegistrator
 */
StaffQuestRegistrator * StaffQuestRegistrator::thisClass = NULL;

StaffQuestRegistrator::StaffQuestRegistrator( )
{
    thisClass = this;
}

StaffQuestRegistrator::~StaffQuestRegistrator( )
{
    thisClass = NULL;
}

bool StaffQuestRegistrator::applicable( PCharacter *pch ) 
{
    return !rated_as_guru( pch );
}

