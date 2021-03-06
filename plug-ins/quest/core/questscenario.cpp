/* $Id$
 *
 * ruffina, 2004
 */
#include "questscenario.h"
#include "questexceptions.h"

#include "object.h"
#include "npcharacter.h"
#include "pcharacter.h"

#include "handler.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

RACE(none);

QuestScenario::~QuestScenario( )
{
}

bool QuestScenario::applicable( PCharacter *, NPCharacter * )
{
    return false;
}

const DLString &
QuestScenariosContainer::getRandomScenario( PCharacter *ch )
{
    Scenarios::iterator i, result = scenarios.end( );
    int count = 0;
    
    for (i = scenarios.begin( ); i != scenarios.end( ); i++)
	if (i->second->applicable( ch ))
	    if (number_range( 0, count++ ) == 0) 
		result = i;

    if (result == scenarios.end( ))
	throw QuestCannotStartException( );
    
    return result->first;
}

QuestScenario::Pointer
QuestScenariosContainer::getScenario( const DLString &name )
{
    Scenarios::iterator i = scenarios.find( name );
    
    if (i == scenarios.end( ))
	throw QuestRuntimeException( "wrong scenario name: " + name );
	
    return i->second;
}

QuestItemAppearence::QuestItemAppearence( )
                        : wear( 0, &wear_flags ),
			  extra( 0, &extra_flags )
{
}

void QuestItemAppearence::dress( Object *obj )
{
    if (!name.empty( ))
	obj->setName( (name + " " + obj->pIndexData->name).c_str( ) );
	
    if (!shortDesc.empty( ))
	obj->setShortDescr( shortDesc.c_str( ) );
	
    if (!desc.empty( ))
	obj->setDescription( desc.c_str( ) );

    if (!extraDesc.empty( ))
	obj->addExtraDescr( obj->getName( ), extraDesc );

    SET_BIT( obj->wear_flags, wear.getValue( ) );
    SET_BIT( obj->extra_flags, extra.getValue( ) );
}

QuestMobileAppearence::QuestMobileAppearence( )
                           : sex( SEX_MALE, &sex_table ),
			     align( N_ALIGN_NULL, &align_table )
{
    race.assign( race_none );
}

void QuestMobileAppearence::dress( NPCharacter *mob ) 
{
    mob->setName( name + " " + mob->pIndexData->player_name );
    mob->setShortDescr( shortDesc );
    mob->setLongDescr( longDesc + "\r\n" );
    mob->setDescription( desc + "\r\n" );
    mob->setSex( sex.getValue( ) ); 

    if (race != race_none)
	mob->setRace( race.getName( ) );
    
    switch (align.getValue( )) {
    case N_ALIGN_GOOD: mob->alignment = 1000; break;
    case N_ALIGN_EVIL: mob->alignment = -1000; break;
    case N_ALIGN_NEUTRAL: mob->alignment = 0; break;
    }
}


int VnumList::randomVnum( )
{
    if (size( ) == 0)
	return -1;

    return at( number_range( 0, size( ) - 1 ) );
}

Object * VnumList::randomItem( )
{
    int vnum;
    OBJ_INDEX_DATA *pObjIndex;

    if (( vnum = randomVnum( ) ) > 0)
	if (( pObjIndex = get_obj_index( vnum ) ))
	    return create_object( pObjIndex, 0 );
    
    return NULL;
}

bool NameList::hasName( NPCharacter *mob )
{
    DLString arg;
    DLString names = mob->pIndexData->player_name;

    while (!( arg = names.getOneArgument( ) ).empty( ))
	if (hasElement( arg ))
	    return true;
	
    return false;
}

