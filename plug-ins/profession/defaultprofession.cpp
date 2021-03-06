/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultprofession.h"
#include "profflags.h"

#include "grammar_entities_impl.h"
#include "pcharacter.h"
#include "alignment.h"
#include "room.h"
#include "race.h"
#include "merc.h"
#include "def.h"

PROF(universal);

/*-------------------------------------------------------------------
 * ProfessionHelp 
 *------------------------------------------------------------------*/
const DLString ProfessionHelp::TYPE = "ProfessionHelp";

void ProfessionHelp::setProfession( Profession::Pointer prof )
{
    StringSet kwd;

    this->prof = prof;
    
    if (!keyword.empty( ))
	kwd.fromString( keyword );

    kwd.insert( prof->getName( ) );
    kwd.insert( prof->getRusName( ).ruscase( '1' ) );
    kwd.insert( prof->getMltName( ).ruscase( '1' ) );
    fullKeyword = kwd.toString( ).toUpper( );

    helpManager->registrate( Pointer( this ) );
}

void ProfessionHelp::unsetProfession( )
{
    helpManager->unregistrate( Pointer( this ) );
    prof.clear( );
    fullKeyword = "";
}

void ProfessionHelp::getRawText( Character *ch, ostringstream &in ) const
{
    in << "��������� {C" << prof->getRusName( ).ruscase( '1' ) << "{x ��� {C"
       << prof->getName( ) << "{x" << endl << endl;
        
    in << *this << endl;

    in << "{c��������{x  : " << align_name_for_range( prof->getMinAlign( ), prof->getMaxAlign( ) ) << endl;

    if (prof->getEthos( ).equalsToBitNumber( ETHOS_LAWFUL ))
        in << "{c����{x      : " << "���������������" << endl;

    if (prof->getSex( ).equalsToBitNumber( SEX_FEMALE ))
        in << "{c���{x       : " << "�������" << endl;
    else if (prof->getSex( ).equalsToBitNumber( SEX_MALE ))
        in << "{c���{x       : " << "�������" << endl;

    bool found = false;

    in << "{c���������{x : ";
    for (int i = 0; i < stat_table.size - 1; i++) {
        int stat = prof->getStat( i );
        if (stat != 0) {
            if (found) 
                in << ", ";
            in << (stat > 0 ? "+" : "") << stat << " � " << stat_table.message( i, '3' );
            found = true;
        }
    }
    if (!found)
        in << "��� ���������";
    in << endl;

    in << "{c���. ����{x : " << prof->getPoints( ) << endl;
        
    in << endl << "{c����� � ������ �����{x: ";
    if (prof->getIndex( ) == prof_universal) {
        in << " (������� �� ��������� ���������)";
    } else {
        found = false;
        for (int i = 0; i < item_table.size; i++) {
            int m = prof->getWearModifier( i );
            if (m != 0) {
                if (found)
                    in << ", ";
                in << (m > 0 ? "+" : "") << m << " � " << item_table.message( i, '3' );
                found = true;
            }
        }
    }

    in << endl;
    in << endl << "��������� ��� ���� ���������� ����� � %H% [(class stats,��������� ��������������)]" << endl;
}

/*-------------------------------------------------------------------
 * ProfessionTitles
 *------------------------------------------------------------------*/
const DLString ProfessionTitlesByLevel::TYPE = "ProfessionTitlesByLevel";

const DLString & ProfessionTitlesByLevel::build( const PCMemoryInterface *pcm ) const
{
    unsigned int level = pcm->getLevel( );

    if (level >= size( ))
	return DLString::emptyString;
	
    const ProfessionTitlePair &pair = (*this)[level]; 

    return (pcm->getSex( ) == SEX_FEMALE 
		? pair.female.getValue( ) : pair.male.getValue( ));
}

const DLString & ProfessionTitlesByConstant::build( const PCMemoryInterface *pcm ) const
{
    return title.getValue( );
}

/*-------------------------------------------------------------------
 * DefaultProfession
 *------------------------------------------------------------------*/
DefaultProfession::DefaultProfession( )
		: stats( &stat_table ),
		  wearModifiers( &item_table ),
		  flags( 0, &prof_flags ),
		  align( 0, &align_table ),
		  ethos( 0, &ethos_table ),
		  sex( 0, &sex_table )
{
}


void DefaultProfession::loaded( )
{
    professionManager->registrate( Pointer( this ) );

    if (help)
	help->setProfession( Pointer( this ) );
}

void DefaultProfession::unloaded( )
{
    if (help)
	help->unsetProfession( );

    professionManager->unregistrate( Pointer( this ) );
}

const DLString & DefaultProfession::getRusName( ) const
{
    return rusName.getValue( );
}
const DLString & DefaultProfession::getMltName( ) const
{
    return mltName.getValue( );
}
int DefaultProfession::getWeapon( ) const
{
    return weapon.getValue( );
}
int DefaultProfession::getSkillAdept( ) const
{
    return skillAdept.getValue( );
}
int DefaultProfession::getParentAdept( ) const
{
    return parentAdept.getValue( );
}
int DefaultProfession::getThac00( Character * ) const
{
    return thac00.getValue( );
}
int DefaultProfession::getThac32( Character * ) const
{
    return thac32.getValue( );
}
int DefaultProfession::getHpRate( ) const
{
    return hpRate.getValue( );
}
int DefaultProfession::getManaRate( ) const
{
    return manaRate.getValue( );
}
Flags DefaultProfession::getFlags( Character * ) const
{
    return flags;
}
int DefaultProfession::getPoints( ) const
{
    return points.getValue( );
}

int DefaultProfession::getStat( bitnumber_t s, Character * ) const 
{
    return stats[s];
}

const DLString & DefaultProfession::getTitle( const PCMemoryInterface *pcm ) const
{
    return titles->build( pcm );
}

const Flags & DefaultProfession::getSex( ) const
{
    return sex;
}
const Flags & DefaultProfession::getEthos( ) const
{
    return ethos;
}
const Flags & DefaultProfession::getAlign( ) const
{
    return align;
}
int DefaultProfession::getMinAlign( ) const
{
    return minAlign.getValue( );
}
int DefaultProfession::getMaxAlign( ) const
{
    return maxAlign.getValue( );
}
int DefaultProfession::getWearModifier( int type ) const
{
    return wearModifiers[type];
}

bool DefaultProfession::isPlayed( ) const
{
    return true;
}

GlobalBitvector DefaultProfession::toVector( Character * ) const
{
    GlobalBitvector bv( professionManager );

    bv.set( getIndex( ) );
    return bv;
}

DLString DefaultProfession::getNameFor( Character *ch, const Grammar::Case &c ) const
{
    if (ch && ch->getConfig( )->rucommands)
	return getRusName( ).ruscase( c );
    else
	return getName( );
}

DLString DefaultProfession::getWhoNameFor( Character *ch ) const
{
    if (ch && ch->getConfig( )->rucommands)
	return whoNameRus;
    else
	return whoName;
}

