/* $Id: ownercoupon.cpp,v 1.1.2.2 2008/02/24 17:25:40 rufina Exp $
 *
 * ruffina, 2007
 */

#include "logstream.h"
#include "ownercoupon.h"
#include "personalquestreward.h"

#include "pcharacter.h"
#include "object.h"

#include "loadsave.h"
#include "wiznet.h"
#include "merc.h"
#include "def.h"

bool OwnerCoupon::use( Character *ch, const char *arg ) 
{ 
    Object *item;
    
    if (ch->is_npc( ) || IS_AFFECTED(ch, AFF_CHARM))
	return false;
    
    if (!arg[0]) {
	ch->println( "����� ������ ���� �� ������ ������� ����� ��������������?" );
	return true;
    }

    if (!( item = get_obj_carry( ch, arg ) )) {
	ch->println( "� ���� ��� �����." );
	return true;
    }

    if (item->pIndexData->limit >= 0) {
	ch->println( "������ ������ ���������������!" );
	return true;
    }

    if (item->timer > 0) {
	ch->pecho( "%^O1 �������� ����� ��������� �����, �� ����� ����� �������?", item );
	return true;
    }

    if (item->pIndexData == obj->pIndexData) {
	ch->println( "��� ���������." );
	return true;
    }

    if (item->behavior) {
	if (item->behavior.getDynamicPointer<PersonalQuestReward>( )) {
	    ch->pecho( "� %O2 ��� ���� ���������� ��������.", item );
	    return true;
	}

	item->behavior->unsetObj( );
    }
        
    item->setOwner( ch->getNameP( ) );
    SET_BIT(item->extra_flags, ITEM_NOPURGE|ITEM_NOSAC|ITEM_BURN_PROOF);
    item->setMaterial( "platinum" );
    item->behavior.setPointer( new PersonalQuestReward );
    item->behavior->setObj( item );
	
    LogStream::sendNotice( ) 
	<< ch->getName( ) << " personalizes " << item->getShortDescr( '1' ) 
	<< " [vnum " << item->pIndexData->vnum << " ID " << item->getID( ) << "] "
	<< " using " << (obj->getOwner( ) ? obj->getOwner( ) : "nobody") 
	<< "'s coupon [ID " << obj->getID( ) << "]" << endl;

    ch->recho( POS_RESTING, "%^C1 ����������� ����������� � %O5 � %O5.", ch, item, obj );
    ch->pecho( "�� ����������� %O4 � ���� ������ ����.", item );
    ch->pecho( "(�� ��������� ������������� ����������� ������ help owner!)" );
    extract_obj( obj );
    return true; 
}

bool OwnerCoupon::hasTrigger( const DLString &t )
{
    return (t == "use");
}

