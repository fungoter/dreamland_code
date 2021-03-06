/* $Id$
 *
 * ruffina, 2004
 */
#include <iomanip>

#include "questtrader.h"
#include "xmlattributequestreward.h"
#include "xmlattributequestdata.h"
#include "occupations.h"

#include "affect.h"
#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"

#include "merc.h"
#include "mercdb.h"
#include "wiznet.h"
#include "interp.h"
#include "handler.h"
#include "act.h"
#include "def.h"

/*------------------------------------------------------------------------
 * QuestTrader 
 *-----------------------------------------------------------------------*/
int QuestTrader::getOccupation( )
{
    return BasicMobileDestiny::getOccupation( ) | (1 << OCC_QUEST_TRADER);
}

void QuestTrader::doTrouble( PCharacter *client, const DLString &constArguments )
{
    Article::Pointer article;
    PersonalQuestArticle::Pointer personal;
    DLString arguments, arg;
    
    if (!canServeClient( client ))
	return;
    
    arguments = constArguments;
    arg = arguments.getOneArgument( );
    if (arg.empty( )) {
        tell_act( client, getKeeper( ), "����� ������ ���� �� ������ �������?" );
        return;
    }

    article = findArticle( client, arg );

    if (!article) {
	msgArticleNotFound( client );
	return;
    }
    
    personal = article.getDynamicPointer<PersonalQuestArticle>( );
    
    if (!personal)
	tell_act( client, getKeeper( ), "������, $c1, � �� ���� ������� ���� ��� ����." );
    else
	personal->trouble( client, getKeeper( ) );
}

bool QuestTrader::canServeClient( Character *client )
{
    if (client->is_npc( ))
	return false;

    if (IS_GHOST( client )) {
	say_act( client, getKeeper( ), "����������� ������ ���������� ���������." );
	return false;
    }

    if (IS_AFFECTED( client, AFF_CHARM )) {
	say_act( client, getKeeper( ), "�� �� ������ ������� �����, ���� �� �� �������� �����!" );
	return false;
    }
   
    if (getKeeper( )->fighting) {
	say_act( client, getKeeper( ), "������� �������, $c1, ��� ������ �� �� ����." );
	return false;
    }

    if (!getKeeper( )->can_see( client )) {
	say_act( client, getKeeper( ), "� �� ������� � �����������." );
	return false;
    }
    
    return true;
}

void QuestTrader::msgListEmpty( Character *client )
{
    say_act( client, getKeeper( ), "������, $c1, ��� ������ ���� ����������." );
}

void QuestTrader::msgListRequest( Character *client ) 
{
    act( "$c1 ������ $C4 �������� ������ �����.", client, 0, getKeeper( ), TO_ROOM );
    act( "�� ������� $C4 �������� ������ �����.", client, 0, getKeeper( ), TO_CHAR );
}

void QuestTrader::msgListBefore( Character *client ) 
{
    client->send_to( "�������� ��������� ����� ��� �������:\n\r" );
}

void QuestTrader::msgListAfter( Character *client )
{
    client->send_to( "��� ������� ����-���� ����������� QUEST BUY <����>.\n\r" );
}

void QuestTrader::msgArticleNotFound( Character *client ) 
{
    say_act( client, getKeeper( ), "� ���� ��� �����, $c1." );
}

void QuestTrader::msgArticleTooFew( Character *client, Article::Pointer )
{
    say_act( client, getKeeper( ), "�� ��������." );
}

void QuestTrader::msgBuyRequest( Character *client )
{
    act( "$c1 � ���-�� ������ $C4.", client, 0, getKeeper( ), TO_ROOM );
}

/*----------------------------------------------------------------------------
 * QuestTradeArticle 
 *---------------------------------------------------------------------------*/
void QuestTradeArticle::toStream( Character *client, ostringstream &buf ) const
{
    buf << "    " << setiosflags( ios::right ) << setw( 7 );
    
    price->toStream( client, buf );

    buf << resetiosflags( ios::left )
        << ".........." << descr << " ({c" << name << "{x)" << endl;
}

bool QuestTradeArticle::visible( Character * ) const
{
    return true;
}

bool QuestTradeArticle::available( Character *, NPCharacter * ) const
{
    return true;
}

bool QuestTradeArticle::matches( const DLString &argument ) const
{
    return !argument.empty( )
           && is_name( argument.c_str( ), name.getValue( ).c_str( ) );
}

int QuestTradeArticle::getQuantity( ) const
{
    return 1;
}

void QuestTradeArticle::purchase( Character *client, NPCharacter *questman, const DLString &, int )
{
    if (!price->canAfford( client ))
	say_act( client, questman, "������, $c1, �� � ���� ������������ $n2 ��� �����.",
	         price->toCurrency( ).c_str( ) );
    else if (!client->is_npc( )) {
	price->deduct( client );
	buy( client->getPC( ), questman );
    }
}

/*----------------------------------------------------------------------------
 * ObjectQuestArticle 
 *---------------------------------------------------------------------------*/
void ObjectQuestArticle::buy( PCharacter *client, NPCharacter *questman ) 
{
    Object *obj;

    obj = create_object( get_obj_index( vnum ), client->getRealLevel( ) );
    obj->setOwner( client->getNameP( ) );
    
    buyObject( obj, client, questman );

    act( "$C1 ���� $o4 $c3.", client, obj, questman, TO_ROOM );
    act( "$C1 ���� ���� $o4.", client, obj, questman, TO_CHAR );
    obj_to_char( obj, client );
}

void ObjectQuestArticle::buyObject( Object *obj, PCharacter *client, NPCharacter *questman ) 
{
}

/*----------------------------------------------------------------------------
 * PersonalQuestArticle 
 *---------------------------------------------------------------------------*/
PersonalQuestArticle::PersonalQuestArticle( ) 
                          : gender( 0, &sex_table )
{
}

void PersonalQuestArticle::buyObject( Object *obj, PCharacter *client, NPCharacter *questman ) 
{
    switch (gender.getValue( )) {
    default:
    case SEX_NEUTRAL:
	obj->fmtShortDescr( obj->getShortDescr( ),
	    IS_GOOD(client)    ? "�������|��|���|���|��|��|��" :
	    IS_NEUTRAL(client) ? "�������|��|���|���|��|��|��" :
	     		         "���������|��|���|���|��|��|��", 
	    client->getNameP( '2' ).c_str());
	break;
    case SEX_MALE:
	obj->fmtShortDescr( obj->getShortDescr( ),
	    IS_GOOD(client)    ? "�������|��|���|���|��|��|��" :
	    IS_NEUTRAL(client) ? "�������|��|���|���|��|��|��" :
			         "���������|��|���|���|��|��|��", 
	    client->getNameP( '2' ).c_str());
	break;
    case SEX_FEMALE:
	obj->fmtShortDescr( obj->getShortDescr( ),
	    IS_GOOD(client)    ? "�������|��|��|��|��|��|��" :
	    IS_NEUTRAL(client) ? "�������|��|��|��|��|��|��" :
			         "���������|��|��|��|��|��|��", 
	    client->getNameP( '2' ).c_str());
	break;
    }

    if (troubled) {
	XMLAttributeQuestReward::Pointer attr;

	attr = client->getAttributes( ).getAttr<XMLAttributeQuestReward>( "questreward" );

	if (attr->getCount( vnum ) == 0)
	    attr->setCount( vnum, 1 );
    }
}

void PersonalQuestArticle::trouble( PCharacter *client, NPCharacter *questman ) 
{
    Object *obj;
    int count = 0;
    XMLAttributeQuestReward::Pointer attr;
    
    if (troubled) {
	attr = client->getAttributes( ).findAttr<XMLAttributeQuestReward>( "questreward" );
	
	if (attr)
	    count = attr->getCount( vnum );
    }

    if (count == 0 || count > 3) {
	tell_act( client, questman, "������, $c1, � �� ���� ������� ���� ��� ����." );
	return;
    }
    
    obj = get_obj_world_unique( vnum, client );

    if (obj) {
	tell_act( client, questman, "������, �� � ���� ��� ���� $o1.", obj );
	// extract_obj( obj ); � ��� ��� ���� 
	return;
    }

    buy( client, questman );
    tell_act( client, questman, "� ��������� ���� ��� ���� $t-� ���.", DLString( count ).c_str( ) );
    
    if (count == 3) 
	tell_act( client, questman, "���� ������������! � ��������� ��� � �� ����� ������ ����." );
    
    attr->setCount( vnum, count + 1 );
}

/*---------------------------------------------------------------------------
 * GoldQI 
 *---------------------------------------------------------------------------*/
void GoldQuestArticle::buy( PCharacter *client, NPCharacter *questman ) 
{
    client->gold += amount.getValue( );
    
    act( "$C1 ���� $t ������� ����� $c3.", client, DLString(amount.getValue( )).c_str( ), questman, TO_ROOM );
    act( "$C1 ���� ���� $t ������� �����.", client, DLString(amount.getValue( )).c_str( ), questman, TO_CHAR );
}

/*---------------------------------------------------------------------------
 * ConQI 
 *---------------------------------------------------------------------------*/
void ConQuestArticle::buy( PCharacter *client, NPCharacter *questman ) 
{
    client->perm_stat[STAT_CON]++;

    act( "$C1 �������� �������� $c2.", client, 0, questman, TO_ROOM );
    act( "$C1 �������� ���� ��������.", client, 0, questman, TO_CHAR );
}
    
bool ConQuestArticle::available( Character *client, NPCharacter *questman ) const 
{
    if (client->is_npc( ))
	return false;

    if (client->perm_stat[STAT_CON] < client->getPC( )->getMaxTrain( STAT_CON ))
	return true;
    
    say_act( client, questman, "������, $c1, �� ���� �������� �� ���������." );
    return false;
}

/*---------------------------------------------------------------------------
 * PocketsQI 
 *---------------------------------------------------------------------------*/
#define OBJ_VNUM_QUESTBAG      103

Object * PocketsQuestArticle::findBag( PCharacter *client ) const
{
    Object *obj;

    for (obj = client->carrying; obj; obj = obj->next_content) 
	if (obj->pIndexData->vnum == OBJ_VNUM_QUESTBAG 
	    && !IS_SET(obj->value[1], CONT_WITH_POCKETS)) 
	    break;
    
    return obj;
}

void PocketsQuestArticle::buy( PCharacter *client, NPCharacter *questman ) 
{
    Object *obj = findBag( client );
    
    if (obj) {
	SET_BIT(obj->value[1], CONT_WITH_POCKETS);
	act("$C1 ��������� ������� �� $o4.", client, obj, questman, TO_CHAR);
	act("$C1 ��������� $c5 ������� �� $o4.", client, obj, questman, TO_ROOM);
    }
}

bool PocketsQuestArticle::available( Character *client, NPCharacter *questman ) const 
{
    if (client->is_npc( ))
	return false;

    if (findBag( client->getPC( ) )) 
	return true;

    say_act( client, questman, "������, $c1, �� � �� ���� � ���� ����� ��� ��������." );
    return false;
}

/*---------------------------------------------------------------------------
 * KeyringQI 
 *---------------------------------------------------------------------------*/
#define OBJ_VNUM_QUESTGIRTH    94
#define OBJ_VNUM_QUESTKEYRING  119

void KeyringQuestArticle::buy( PCharacter *client, NPCharacter *questman ) 
{
    Object *girth, *keyring;
    Wearlocation *waist;
    
    if (( girth = get_obj_carry_vnum( client, OBJ_VNUM_QUESTGIRTH ) ) == NULL)
	return;
    
    keyring = create_object( get_obj_index( OBJ_VNUM_QUESTKEYRING ), 0 );
    obj_to_char( keyring, client );
    
    keyring->setOwner( girth->getOwner( ) );
    keyring->setShortDescr( girth->getShortDescr( ) );
    
    if (girth->getRealDescription( ))
	keyring->setDescription( girth->getRealDescription( ) );
	
    if (girth->getRealName( ))
	keyring->setName( girth->getRealName( ) );
	
    if (girth->getRealMaterial( ))
	keyring->setMaterial( girth->getRealMaterial( ) );

    keyring->extra_flags = girth->extra_flags;
    keyring->condition = girth->condition;
    keyring->level = girth->level;

    for (Affect *paf = girth->affected; paf != 0; paf = paf->next)
	affect_to_obj( keyring, paf );

    for (EXTRA_DESCR_DATA *ed = girth->extra_descr; ed != 0; ed = ed->next)
	keyring->addExtraDescr( ed->keyword, ed->description );

    waist = &*girth->wear_loc;
    waist->unequip( girth );
    waist->equip( keyring );
    extract_obj( girth );

    act("$C1 ����������� �������� ������ � $o3.", client, keyring, questman, TO_CHAR);
    act("$C1 ����������� �������� ������ � $o3 $c2.", client, keyring, questman, TO_ROOM);
}

bool KeyringQuestArticle::available( Character *client, NPCharacter *questman ) const 
{
    if (client->is_npc( ))
	return false;

    if (get_obj_carry_vnum( client, OBJ_VNUM_QUESTGIRTH ))
	return true;

    say_act( client, questman, "������, $c1, �� � �� ���� � ���� ����� ��� �������." );
    return false;
}

/*----------------------------------------------------------------------
 * OwnerPrice 
 *---------------------------------------------------------------------*/
const DLString OwnerPrice::LIFE_NAME = "�����������|�|�|��|�|���|��";
const DLString OwnerPrice::VICTORY_NAME = "�����|�||��|�|���|�� � �������";

DLString OwnerPrice::toCurrency( ) const
{
    return LIFE_NAME + " ��� " + VICTORY_NAME;
}

DLString OwnerPrice::toString( Character * ) const
{
    ostringstream buf;

    buf << lifes << " " << LIFE_NAME << " ��� " << victories << VICTORY_NAME;
    return buf.str( );
}

bool OwnerPrice::canAfford( Character *ch ) const
{
    if (ch->is_npc( ))
	return false;
    
    return getValue( ch->getPC( ) ) - ch->getPC( )->getRemorts( ).owners > 0;
}

int OwnerPrice::getValue( PCharacter *ch ) const
{
    int my_victories, my_lifes, total;
    
    my_victories = ch->getPC( )->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" )->getAllVictoriesCount( );
    my_lifes = ch->getPC( )->getRemorts( ).size( );
    total = my_victories / victories + my_lifes / lifes;
    
    return total / 2;
}

void OwnerPrice::induct( Character *ch ) const
{
}

void OwnerPrice::deduct( Character *ch ) const
{
    if (!ch->is_npc( )) {
	ch->getPC( )->getRemorts( ).owners++;
	::wiznet( WIZ_QUEST, 0, 0, "%1$^C1 ����������� owner coupon.", ch );
    }
}

void OwnerPrice::toStream( Character *ch, ostringstream &buf ) const
{
}

/*---------------------------------------------------------------------------
 * OwnerCouponQI
 *---------------------------------------------------------------------------*/
bool OwnerQuestArticle::available( Character *client, NPCharacter *questman ) const 
{
    if (client->is_npc( ))
	return false;

    if (lifePrice.getValue( client->getPC( ) ) <= 0) {
	say_act( client, questman, "������, $c1, �� � ���� �� ������� $n2, ����� ������� ���� �����.", lifePrice.toCurrency( ).c_str( ) );
	return false;
    }

    if (!lifePrice.canAfford( client )) {
	say_act( client, questman, "������, $c1, �� �� ��� �������$g��|�|�� ���������� ���� ���������� ���� �������." );
	return false;
    }

    return true;
}

bool OwnerQuestArticle::visible( Character *client ) const 
{
    return lifePrice.canAfford( client );
}

void OwnerQuestArticle::buyObject( Object *obj, PCharacter *client, NPCharacter *questman ) 
{
    lifePrice.deduct( client );
}

/*----------------------------------------------------------------------------
 * PiercingQuestArticle 
 *---------------------------------------------------------------------------*/
void PiercingQuestArticle::buy( PCharacter *client, NPCharacter *tattoer ) 
{
    client->wearloc.set( wear_ears );
    
    act("$C1 ������ ����� � ������ $c2.",client,0,tattoer,TO_ROOM);
    act("$C1 ������ ���� ����� � ������.",client,0,tattoer,TO_CHAR);
}

bool PiercingQuestArticle::available( Character *client, NPCharacter *tattoer ) const 
{
    if (!visible( client )) {
	say_act( client, tattoer, "� ���� ��� ��������� ���, $c1." );
	say_act( client, tattoer, "�����, ���� ��� ���-������ ���������?" );
	interpret_raw( tattoer, "smirk" );
	return false;
    }

    if (get_eq_char( client, wear_head )) {
	interpret_raw( tattoer, "bonk", client->getNameP( ) );
	say_act( client, tattoer, "����� �����!" );
	return false;
    }

    return true;
}

bool PiercingQuestArticle::visible( Character *client ) const 
{
    return !client->is_npc( ) && !client->getWearloc( ).isSet( wear_ears );
}

/*----------------------------------------------------------------------------
 * TattooQuestArticle 
 *---------------------------------------------------------------------------*/
RELIG(none);

#define OBJ_VNUM_TATTOO 50

void TattooQuestArticle::buy( PCharacter *client, NPCharacter *tattoer ) 
{
    Object *obj;
    const char *leader = client->getReligion( )->getShortDescr( ).c_str( );
    
    obj = create_object( get_obj_index( OBJ_VNUM_TATTOO ), 0 );
    obj->fmtName( obj->getName( ), leader );
    obj->fmtShortDescr( obj->getShortDescr( ), leader );

    obj_to_char( obj, client );
    equip_char( client, obj, wear_tattoo );
    
    act( "$C1 ������� ���� $o4!", client, obj, tattoer, TO_CHAR );
    act( "$C1 ������� $c3 $o4!", client, obj, tattoer, TO_ROOM );
}

bool TattooQuestArticle::available( Character *client, NPCharacter *tattoer ) const 
{
    if (client->is_npc( ))
	return false;

    if (client->getReligion( ) == god_none) {
	say_act( client, tattoer, "$c1, �� �� ������ � ���� � �� ������ �������� ����������." );
	return false;
    }

    if (wear_tattoo->find( client )) {
	say_act( client, tattoer, "�� � ���� ��� ���� ����������, $c1!" );
	return false;
    }

    return true;
}


