/* $Id$
 *
 * ruffina, 2004
 */
#include "questmodels.h"
#include "questmodels-impl.h"
#include "questscenario.h"

#include "skillreference.h"
#include "skill.h"
#include "mobilebehaviormanager.h"
#include "occupations.h"
#include "pcharacter.h"
#include "save.h"
#include "merc.h"
#include "def.h"
#include "roomtraverse.h"

GSN(detect_hide);
GSN(detect_invis);
GSN(truesight);
GSN(acute_vision);
GSN(improved_detect);

/*--------------------------------------------------------------------
 * RoomQuestModel 
 *--------------------------------------------------------------------*/
bool RoomQuestModel::checkRoom( PCharacter *ch, Room *room ) 
{
    if (IS_SET(room->room_flags, ROOM_SOLITARY|ROOM_PRIVATE|ROOM_NO_QUEST ))
	return false;
    
    if (IS_SET( room->area->area_flag, AREA_NOQUEST ))
	return false;
    
    if (!room->isCommon( ))
	return false;

    return true;
}

bool RoomQuestModel::checkRoomClient( PCharacter *pch, Room *room ) 
{
    if (room->area->low_range > pch->getModifyLevel( ))
	return false;

    if (!checkRoom( pch, room ))
	return false;

    return true;
}

bool RoomQuestModel::checkRoomVictim( PCharacter *pch, Room *room ) 
{
    if (IS_SET( room->room_flags, ROOM_SAFE|ROOM_NO_DAMAGE ))
	return false;

    if (IS_SET( room->area->area_flag, AREA_HOMETOWN ))
	return false;

    if (!checkRoom( pch, room ))
	return false;
    
    return true;
}

Room * RoomQuestModel::getRandomRoomClient( PCharacter *pch )
{
    RoomList rooms;
    
    findClientRooms( pch, rooms );
    return getRandomRoom( rooms );
}

Room * RoomQuestModel::getRandomRoom( RoomList &rooms )
{
    if (rooms.empty( ))
	return NULL;
    else
	return rooms[number_range( 0, rooms.size( ) - 1 )];
}

void RoomQuestModel::findClientRooms( PCharacter *pch, RoomList &rooms )
{
    for (Room * r = room_list; r; r = r->rnext)
	if (checkRoomClient( pch, r ))
	    rooms.push_back( r );

    if (rooms.empty( ))
	throw QuestCannotStartException( );
}

void RoomQuestModel::findClientRooms( PCharacter *pch, RoomList &rooms, VnumList &vnums )
{
    Room *r;
    
    for (VnumList::iterator v = vnums.begin( ); v != vnums.end( ); v++)
	if (( r = get_room_index( *v ) ))
	    if (checkRoomClient( pch, r ))
		rooms.push_back( r );

    if (rooms.empty( ))
	throw QuestCannotStartException( );
}

Room * RoomQuestModel::getDistantRoom( PCharacter *pch, RoomList &rooms, Room *from, int range, int attempts )
{
    int tries = 0;
    
    while (!rooms.empty( ) && tries++ < attempts) {
	int i = number_range( 0, rooms.size( ) - 1 );

	if (room_distance( pch, rooms[i], from, 10000 ) > range)
	    return rooms[i];

	rooms.erase( rooms.begin( ) + i );
    }
    
    throw QuestCannotStartException( );
}

/*--------------------------------------------------------------------
 * ItemQuestModel 
 *--------------------------------------------------------------------*/
bool ItemQuestModel::checkItem( PCharacter *pch, Object *obj )
{
    if (obj->item_type == ITEM_KEY 
	|| obj->item_type == ITEM_MAP
	|| obj->item_type == ITEM_POTION
	|| obj->item_type == ITEM_MONEY)
	return false;

    if (IS_SET( obj->extra_flags, ITEM_ROT_DEATH|ITEM_VIS_DEATH
				  |ITEM_MELT_DROP|ITEM_NOSAVEDROP
				  |ITEM_NOREMOVE|ITEM_NODROP ))
	return false;

    if (obj->pIndexData->limit != -1 || obj->timer > 0)
	return false;
    
    if (obj->mustDisappear( pch ))
	return false;
    
    if (obj->isAntiAligned( pch ) || !IS_SET( obj->wear_flags, ITEM_TAKE ))
	return false;

    return true;
}

bool ItemQuestModel::isItemVisible( Object *obj, PCharacter *pch )
{
    if (IS_OBJ_STAT( obj, ITEM_INVIS ) && !gsn_detect_invis->usable( pch ))
	return false;

    return true;
}

Object * ItemQuestModel::getRandomItem( PCharacter *pch )
{
    Object *obj;
    ItemList objects;

    for (obj = object_list; obj; obj = obj->next) {
	if (!checkItem( pch, obj ))
	    continue;
	
	if (number_range( 1, obj->pIndexData->count ) == 1)
	    objects.push_back( obj );
    }

    if (objects.empty( ))
	throw QuestCannotStartException( );

    return objects[ number_range( 0, objects.size( ) - 1 ) ];
}

void ItemQuestModel::clear( Object *obj )
{
    if (obj) {
	obj->behavior->unsetObj( );
	obj->behavior.clear( );
    }
}

void ItemQuestModel::destroy( Object *obj )
{
    if (obj) {
	obj->behavior.clear( );
	extract_obj( obj );
    }
}

/*--------------------------------------------------------------------
 * MobileQuestModel 
 *--------------------------------------------------------------------*/
bool MobileQuestModel::checkMobile( PCharacter *pch, NPCharacter *mob )
{
    if (IS_AFFECTED( mob, AFF_CHARM ))
	return false;

    if (mob->behavior 
	&& (mob->behavior->hasDestiny( ) 
	    || IS_SET(mob->behavior->getOccupation( ), (1 << OCC_SHOPPER))))
	return false;
    
    if (IS_SET( mob->pIndexData->area->area_flag, AREA_NOQUEST ))
	return false;

    if (!mob->in_room)
	return false;
    
    return true;
}

bool MobileQuestModel::isMobileVisible( NPCharacter *mob, PCharacter *pch )
{
    if (IS_AFFECTED( mob, AFF_CAMOUFLAGE ) && !gsn_acute_vision->usable( pch ))
	return false;

    if (gsn_truesight->usable( pch, false ))
	return true;

    if (IS_AFFECTED( mob, AFF_HIDE ) && !gsn_detect_hide->usable( pch ))
	return false;

    if (IS_AFFECTED( mob, AFF_INVISIBLE ) && !gsn_detect_invis->usable( pch ))
	return false;

    if (IS_AFFECTED( mob, AFF_IMP_INVIS ) && !gsn_improved_detect->usable( pch ))
	return false;

    return true;
}

NPCharacter * MobileQuestModel::getRandomMobile( MobileList &mobiles )
{
    if (mobiles.empty( ))
	return NULL;
    else
	return mobiles[ number_range( 0, mobiles.size( ) - 1 ) ];
}

mob_index_data * MobileQuestModel::getRandomMobIndex( MobIndexMap &map )
{
    int n = number_range( 0, map.size( ) - 1 ), count = 0;

    for (MobIndexMap::iterator m = map.begin( ); m != map.end( ); m++, count++)
	if (count == n) 
	    return m->first;

    return NULL;
}

void MobileQuestModel::clear( NPCharacter *mob )
{
    if (mob) {
	MobileBehaviorManager::assignBasic( mob );
	save_mobs( mob->in_room );
    }
}

void MobileQuestModel::destroy( NPCharacter *mob )
{
    if (mob) {
	mob->behavior.clear( );
	extract_char( mob );
    }
}

/*--------------------------------------------------------------------
 * VictimQuestModel 
 *--------------------------------------------------------------------*/
bool VictimQuestModel::checkMobileVictim( PCharacter *pch, NPCharacter *mob )
{
    if (!checkMobile( pch, mob ))
	return false;

    if (IS_SET( mob->act,  ACT_NOTRACK | ACT_SAGE ))
	return false;

    if (IS_SET( mob->imm_flags, IMM_SPELL|IMM_WEAPON ))
        return false;

    if (mob->behavior && mob->behavior->getOccupation( ) & OCC_PRACTICER)
	return false;

    if (mob->fighting)
	return false;
    
    if ((IS_EVIL(mob) && IS_EVIL(pch)) || (IS_GOOD(mob) && IS_GOOD(pch)))
	return false;

    if (mob->in_room->area != mob->pIndexData->area)
	return false;

    if (mob->getRealLevel( ) != mob->pIndexData->level)
	return false;

    return checkRoomVictim( pch, mob->in_room );
}

void VictimQuestModel::findVictims( PCharacter *pch, MobileList &victims )
{
    NPCharacter *mob;

    for (Character *wch = char_list; wch; wch = wch->next) {
	mob = wch->getNPC( );

	if (mob && checkMobileVictim( pch, mob ))
	    victims.push_back( mob );
    }

    if (victims.empty( ))
	throw QuestCannotStartException( );
}

void VictimQuestModel::findVictims( PCharacter *pch, MobIndexMap &victims )
{
    NPCharacter *mob;

    for (Character *wch = char_list; wch; wch = wch->next) {
	mob = wch->getNPC( );

	if (mob && checkMobileVictim( pch, mob ))
	    victims[mob->pIndexData].push_back( mob );
    }

    if (victims.empty( ))
	throw QuestCannotStartException( );
}

NPCharacter * VictimQuestModel::getRandomVictim( PCharacter *pch )
{
    MobileList victims;

    findVictims( pch, victims );
    return getRandomMobile( victims );
}


/*--------------------------------------------------------------------
 * ClientQuestModel 
 *--------------------------------------------------------------------*/
bool ClientQuestModel::checkMobileClient( PCharacter *pch, NPCharacter *mob )
{
    if (IS_SET( mob->act, ACT_AGGRESSIVE|ACT_VAMPIRE ))
	return false;

    return checkMobileClientAggr( pch, mob );
}

bool ClientQuestModel::checkMobileClientAggr( PCharacter *pch, NPCharacter *mob )
{
    if (!checkMobile( pch, mob ))
	return false;
    
    if (!IS_AWAKE( mob ))
	return false;

    if (IS_AFFECTED( mob, AFF_BLOODTHIRST ))
	return false;

    if ((IS_GOOD(pch) && IS_EVIL(mob)) || (IS_EVIL(pch) && IS_GOOD(mob)))
	return false;

    return checkRoomClient( pch, mob->in_room );
}

void ClientQuestModel::findClients( PCharacter *pch, MobileList &clients )
{
    Character *wch;
    NPCharacter *mob;

    for (wch = char_list; wch; wch = wch->next) {
	mob = wch->getNPC( );

	if (mob && checkMobileClient( pch, mob ))
	    clients.push_back( mob );
    }

    if (clients.empty( ))
	throw QuestCannotStartException( );
}

void ClientQuestModel::findClients( PCharacter *pch, MobIndexMap &clients )
{
    NPCharacter *mob;

    for (Character *wch = char_list; wch; wch = wch->next) {
	mob = wch->getNPC( );

	if (mob && checkMobileClient( pch, mob ))
	    clients[mob->pIndexData].push_back( mob );
    }

    if (clients.empty( ))
	throw QuestCannotStartException( );
}


NPCharacter * ClientQuestModel::getRandomClient( PCharacter *pch )
{
    MobileList clients;

    findClients( pch, clients );
    return getRandomMobile( clients );
}


