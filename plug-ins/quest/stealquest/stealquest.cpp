/* $Id: stealquest.cpp,v 1.1.2.29.6.11 2010-09-01 21:20:46 rufina Exp $
 *
 * ruffina, 2004
 */

#include "stealquest.h"
#include "objects.h"
#include "mobiles.h"

#include "questexceptions.h"

#include "selfrate.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "mercdb.h"
#include "merc.h"
#include "save.h"
#include "act.h"
#include "handler.h"
#include "def.h"

StealQuest::StealQuest( )
             : item( NULL )
{
}

void StealQuest::create( PCharacter *pch, NPCharacter *questman )
{
    Object *chest, *key;
    NPCharacter *thief, *victim;
    Room *hideaway;
    int time;
    DLString name;
    StealQuestRegistrator *reg = StealQuestRegistrator::getThis( );

    charName = pch->getName( );
    state = QSTAT_INIT;
    
    mode = number_range( -1, 3 );

    if (rated_as_newbie( pch ))
	mode = std::min( mode.getValue( ), 1 );
    else if (rated_as_guru( pch ))
	mode = std::max( mode.getValue( ), 1 );

    try {
	item = getRandomItem( pch );
	victim = item->carried_by->getNPC( );
	thief = getRandomVictim( pch );
    
	if (rated_as_newbie( pch )
	    || (rated_as_expert( pch ) && chance( 50 ))
	    || (rated_as_guru( pch ) && chance( 20 )))
	{
	    key = chest = NULL;
	    hideaway = NULL;
	}
	else {
	    chest = createItem<HiddenChest>( reg->chests.randomVnum( ) );
	    fillChest( pch, chest );
	    key = createItem<LockPick>( chest->value[2] );
	    hideaway = findHideaway( pch, thief );
	}
	
	if (!isMobileVisible( thief, pch ) || !isMobileVisible( victim, pch ))
	    mode++;
	    
	if (!isItemVisible( item, pch ))
	    mode++;
	
	if ((rated_as_guru( pch ) && mode < 2) 
                || (rated_as_newbie( pch ) && mode > 1))
	    throw QuestCannotStartException( );

	MobileQuestModel::assign<Robber>( thief );
	MobileQuestModel::assign<RobbedVictim>( victim ); 
	ItemQuestModel::assign<RobbedItem>( item );

    } catch (const QuestCannotStartException &e) {
	destroy( );
	throw e;
    }

    name = victim->getShortDescr( );
    name.upperFirstCharacter( );
    victimName = name;
    victimRoom = victim->in_room->name;
    victimArea = victim->in_room->area->name;
    
    name = thief->getShortDescr( );
    name.upperFirstCharacter( );
    thiefName = name;
    thiefArea = thief->in_room->area->name;
    thiefRoom = thief->in_room->name;
    thiefSex = thief->getSex( );

    itemName = item->getShortDescr( );
    itemWear.assign( item->wear_loc );
    chestRoom = getRoomHint( hideaway );

    obj_from_char( item );

    if (chest) {
	obj_to_obj_random( item, chest );
	obj_to_room( chest, hideaway );
	obj_to_char( key, thief );
    }
    else 
	obj_to_char( item, thief );
    

    time = number_fuzzy( 10 );
    setTime( pch, time );
    
    tell_raw( pch, questman, "� ���� ���� ��� ���� ������� ���������!" );
    
    switch (number_range( 1, 3 )) {
    case 1:  tell_fmt( "{W%3$#^C4{G ����������, %3$P1 ������ ������ � ������ ��������.", 
                        pch, questman, victim );
	     break;
    case 2:  tell_fmt( "{W%3$#^C1{G ���%3$G��|�|�� ������� ���������� � ������ ������� ���������� ����.", 
                        pch, questman, victim );
	     break;
    case 3:  tell_fmt( "���� �������� {W%3$#C4{G, � ������ %3$P1 ��������� � ����� ������.", 
                        pch, questman, victim );
	     break;
    }

    tell_raw( pch, questman, "������������� ��� � ������ {W%s{G ({W%s{G).", 
                  victim->in_room->name, victim->in_room->area->name );
    tell_fmt("� ���� ���� {Y%3$d{G ����%3$I��|��|�, ����� ��������� ���� � ������ �����������.", 
              pch, questman, time );
    
    wiznet( "", "thief [%s] [%d], obj [%s], victim [%s] [%d], chest [%d], mode %d",
		thief->getNameP( '1' ).c_str(), thief->in_room->vnum,
		item->getShortDescr( '1' ).c_str( ),
		victim->getNameP( '1' ).c_str(), victim->in_room->vnum,
		(hideaway ? hideaway->vnum : 0),
		mode.getValue( ) );
}

void StealQuest::clear( Object *obj )
{
    ItemQuestModel::clear( obj );
    
    if (obj) {
	if (obj->carried_by 
	    && obj->carried_by->is_npc( )
	    && victimName ^ obj->carried_by->getNPC( )->getShortDescr( ))
	{
	    save_mobs( obj->carried_by->in_room );
	}
	else
	    extract_obj( obj );
    }
}

void StealQuest::destroy( ) 
{
    clearMobile<RobbedVictim>( );
    clearMobile<Robber>( );
    clearItem<RobbedItem>( );
    destroyItem<HiddenChest>( );
    destroyItem<LockPick>( );
}

Quest::Reward::Pointer StealQuest::reward( PCharacter *ch, NPCharacter *questman ) 
{
    Reward::Pointer r( NEW );

    switch (mode.getValue( )) {
    case -1: r->points = number_range( 1, 5 ); break;
    case 0:  r->points = number_range( 5, 8 ); break;
    case 1:  r->points = number_range( 8, 12 ); break;
    case 2:  r->points = number_range( 12, 16 ); break;
    case 3:  r->points = number_range( 16, 22 ); break;
    default: r->points = number_range( 22, 26 ); break;
    }
    
    if (!chestRoom.getValue( ).empty( ))
	r->points += number_fuzzy( 10 );
    else    
	r->points += number_fuzzy( 3 );
	
    r->points -= hint * 5;
    r->gold = number_fuzzy( r->points );
    r->wordChance = r->points;
    r->scrollChance = number_range( 5, mode * 4 );

    if (chance( mode ))
	r->prac = number_range( 1, 3 );
    
    if (ch->getClan( )->isDispersed( )) 
	r->points *= 2;
    else
	r->clanpoints = r->points;
    
    r->points = std::max( 5, r->points );
    r->clanpoints = std::max( 0, r->clanpoints );
    r->exp = (r->points + r->clanpoints) * 10;

    return Reward::Pointer( r );
}

void StealQuest::info( std::ostream &buf, PCharacter *ch ) 
{
    switch (state.getValue( )) {
    case QSTAT_INIT:
	buf << "� " << russian_case( victimName, '2' ) << " �����-�� ������������." << endl
	    << "������������ ���� ���� � ������ '" 
	    << victimRoom << "' (" << victimArea << ")." << endl;
	break;

    case QSTAT_HUNT_ROBBER:
	buf << "���� ����� ��������, ��� � " << russian_case( victimName, '2' ) 
	    << " ������ " << russian_case( itemName, '4' ) << ". " << endl
	    << "��� - " << russian_case( thiefName, '1' ) 
	    << ", ������ ����� ���������� � ������ '" << thiefRoom << "' (" << thiefArea << ")." << endl;

	    if (!chestRoom.getValue( ).empty( ))
		buf << "�� ������, ������������ ����� �������� ���-�� ����� '" << chestRoom << "'" 
		    << ", ���� �� ����� ��� � ����." << endl;

	    buf << "������������ ���� ���� ����� '" 
	        << victimRoom << "' (" << victimArea << ")." << endl;

	break;

    case QSTAT_FINISHED:
	buf << "���� ������� ���������!" << endl
	    << "������� �� ��������������� �� ����, ��� ������ �����." << endl;
	break;
    }
}

void StealQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    switch (state.getValue( )) {
    case QSTAT_INIT:
        buf << "������, ��� ��������� � " << russian_case( victimName, '2') << " � "
            << victimRoom << " (" << victimArea << ").";
	break;

    case QSTAT_HUNT_ROBBER:
        buf << "������� " << russian_case( itemName, '4' ) << " " << russian_case( victimName, '3' ) << ". "
            << "���, " << russian_case( thiefName, '1' ) << ", ���������� � " 
            << thiefRoom << " (" << thiefArea << ")";

	    if (!chestRoom.getValue( ).empty( ))
                buf << ", ������������ ������ ����� " << chestRoom << ".";
            else 
                buf << ".";
	break;

    case QSTAT_FINISHED:
	buf << "��������� � �������� �� ��������.";
	break;
    }
}

bool StealQuest::isComplete( )
{
    return (state == QSTAT_FINISHED);
}

void StealQuest::helpMessage( ostringstream &buf )
{
    switch (state.getValue( )) {
    case QSTAT_INIT:
	buf << "�� " << russian_case( victimName, '2' )
	    << " �� ������ ���������, ������ �� ������ ����: ";
	break;

    case QSTAT_HUNT_ROBBER:
	buf << "�� ������ �������� " << russian_case( thiefName, '4' )
	    << ", ������ �� ������ ����: ";
	break;
    }
}

Room * StealQuest::helpLocation( ) 
{
    switch (state.getValue( )) {
    case QSTAT_INIT:
	return findMobileRoom<RobbedVictim>( );
	
    case QSTAT_HUNT_ROBBER:
	return findMobileRoom<Robber>( );
    }

    return NULL;
}

bool StealQuest::checkItem( PCharacter *pch, Object *obj )
{
    NPCharacter *victim;

    if (!obj->carried_by || !obj->carried_by->is_npc( ))
	return false;
    
    victim = obj->carried_by->getNPC( );
    
    if (!checkMobileClient( pch, victim ))
	return false;
    
    if (!victim->can_see( obj ))
	return false;

    return ItemQuestModel::checkItem( pch, obj );
}

bool StealQuest::checkMobileVictim( PCharacter *pch, NPCharacter *mob )
{
    int level_diff, min_diff;

    if (!VictimQuestModel::checkMobileVictim( pch, mob )) 
	return false;

    if (mob->in_room->area == item->carried_by->in_room->area)
	return false;;

    level_diff = mob->getRealLevel( ) - pch->getModifyLevel( );
    min_diff = (mob->getRealLevel( ) < 50 ? -10 : -20);
    
    if (level_diff > 15 || level_diff < min_diff) 
	return false;
    
    if ((mode == -1 && level_diff > -10) || level_diff >= 5 * mode)
	return false;

    if (!isThief( mob ))
	return false;
    
    return true;
}

bool StealQuest::checkMobileClient( PCharacter *pch, NPCharacter *mob )
{
    if (!ClientQuestModel::checkMobileClient( pch, mob )) 
	return false;

    if (isThief( mob ))
	return false;
	
    if (mob->carry_number > mob->canCarryNumber( ))
	return false;

    if (mob->getCarryWeight( ) > mob->canCarryWeight( ))
	return false;

    return true;
}

bool StealQuest::isThief( NPCharacter *mob )
{
    if (IS_SET( mob->act, ACT_THIEF ))
	return true;

    if (mob->spec_fun.name == "spec_thief")
	return true;
  
    if (StealQuestRegistrator::getThis( )->thiefs.hasName( mob ))
	return true;
	
    return false;
}


void StealQuest::fillChest( PCharacter *pch, Object *chest )
{
    Object *obj;
    VnumList objects;
    
    /* ������� */
    for (obj = object_list; obj; obj = obj->next) {
	if (IS_SET( obj->pIndexData->area->area_flag, AREA_NOQUEST ))
	    continue;
	if (obj->pIndexData->reset_num <= 0)
	    continue;
	if (obj->pIndexData->cost >= 10000) 
	    continue;
	if (!IS_SET( obj->wear_flags, ITEM_TAKE ))
	    continue;

	if (isBonus( obj->pIndexData, pch ) 
	    && number_range( 1, obj->pIndexData->count ) == 1)
	{
	    objects.push_back( obj->pIndexData->vnum );
	}
    }
    
    if (!objects.empty( )) {
	int count = number_range( 0, 15 );

	for ( ; count; count--) 
	    if (( obj = objects.randomItem( ) ))
		obj_to_obj( obj, chest );
    }
    
    /* ������� */
    if (chance( 1 )) 
	if (( obj = StealQuestRegistrator::getThis( )->bonuses.randomItem( ) ))
	    obj_to_obj( obj, chest );

    obj_to_obj( create_money( 
		    number_range( 0, pch->getModifyLevel( ) ), 
		    number_range( 0, 1000 ) ), chest );
}

bool StealQuest::isBonus( OBJ_INDEX_DATA *pObjIndex, PCharacter *pch )
{
    int olevel = pObjIndex->level;
    int mlevel = pch->getModifyLevel( );

    if (olevel > mlevel + mlevel / 10 || olevel < mlevel - mlevel / 5) 
	return false;

    if (pObjIndex->limit != -1 || olevel > ANGEL) 
	return false;
    
    return true;
}

Room * StealQuest::findHideaway( PCharacter *pch, NPCharacter *thief )
{
    Room *room;
    std::vector<Room *> places, places1, places2;
    std::vector<Room *>::iterator r;

    for (room = room_list; room; room = room->rnext) {
	if (room->area != thief->in_room->area)
	    continue;
	
	if (!checkRoom( pch, room ))
	    continue;
	
	places.push_back( room );
    }
    
    for (r = places.begin( ); r != places.end( ); r++) {
	EXIT_DATA *e;
	int cnt = 0;

	for (int d = 0; d < DIR_SOMEWHERE; d++) {
	    e = (*r)->exit[d];

	    if (e && e->u1.to_room)
		cnt++;
	}
	
	if (cnt == 1)
	    places1.push_back( *r );
	if (cnt == 2)
	    places2.push_back( *r );
    }
    
    if (!places1.empty( ))
	return places1[number_range( 0, places1.size( ) - 1 )];
    if (!places2.empty( ))
	return places2[number_range( 0, places2.size( ) - 1 )];
    if (!places.empty( ))
	return places[number_range( 0, places.size( ) - 1 )];
    
    throw QuestCannotStartException( );
}

DLString StealQuest::getRoomHint( Room * room, Room *from, int depth )
{
    if (!room)
	return "";

    if (depth >= 2) 
	return room->name;

    for (int d = 0; d < DIR_SOMEWHERE; d++) {
	Room *r;

	if (!room->exit[d])
	    continue;
	
	if (!( r = room->exit[d]->u1.to_room ))
	    continue;
	
	if (r == from)
	    continue;
	
	for (int i = 0; i < DIR_SOMEWHERE; i++)
	    if (r->exit[i] && r->exit[i]->u1.to_room == room)
		return getRoomHint( r, room, depth + 1 );
    }

    return room->name;
}

/*
 * StealQuestRegistrator
 */
StealQuestRegistrator * StealQuestRegistrator::thisClass = NULL;

StealQuestRegistrator::StealQuestRegistrator( )
{
    thisClass = this;
}

StealQuestRegistrator::~StealQuestRegistrator( )
{
    thisClass = NULL;
}

