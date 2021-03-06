/* $Id$
 *
 * ruffina, 2004
 */
#include "religion.h"

/*-------------------------------------------------------------------
 * Religion
 *------------------------------------------------------------------*/
Religion::Religion( )
{
}

Religion::Religion( const DLString &n ) : name( n )
{
}

Religion::~Religion( )
{
}

const DLString &Religion::getName( ) const
{
    return name;
}

const DLString &Religion::getRussianName( ) const
{
    return DLString::emptyString;
}

bool Religion::isValid( ) const
{
    return false;
}

bool Religion::isAllowed( Character * ) const
{
    return false;
}

const DLString & Religion::getShortDescr( ) const
{
    return DLString::emptyString;
}
const DLString & Religion::getDescription( ) const
{
    return DLString::emptyString;
}
void Religion::tattooFight( Object *, Character * ) const
{
}

const DLString& Religion::getNameFor( Character * ) const
{
    return getShortDescr( );
}

/*-------------------------------------------------------------------
 * ReligionManager
 *------------------------------------------------------------------*/
ReligionManager* religionManager = 0;

ReligionManager::ReligionManager( ) 
{
    checkDuplicate( religionManager );
    religionManager = this;
}

ReligionManager::~ReligionManager( )
{
    religionManager = 0;
}

GlobalRegistryElement::Pointer ReligionManager::getDumbElement( const DLString &name ) const
{
    return Religion::Pointer( NEW, name );
}

GLOBALREF_IMPL(Religion, '-')
XMLGLOBALREF_IMPL(Religion)
