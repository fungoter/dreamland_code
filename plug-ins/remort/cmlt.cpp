/* $Id: cmlt.cpp,v 1.1.2.6.4.7 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * ��� ����� �� ���� ��� 'Dream Land' ����������� Igor {Leo} � Olga {Varda}*
 * ��������� ������ � ��������� ����� ����, � ����� ������ ������ ��������:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    � ��� ���������, ��� ��������� � ����� � ���� MUD                    *
 ***************************************************************************/

#include "cmlt.h"
#include "commonattributes.h"
#include "xmlattributestatistic.h"
#include "victorybonus.h"

#include "class.h"

#include "remortdata.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "pcmemoryinterface.h"
#include "pcrace.h"

#include "merc.h"
#include "comm.h"
#include "loadsave.h"
#include "act.h"
#include "def.h"

COMMAND(CMlt, "mlt")
{
    PCMemoryInterface *pcm;
    DLString arguments = constArguments; 
    DLString arg = arguments.getOneArgument( );
    
    if (ch->is_npc( )) 
	return;
    
    if (ch->isCoder( ) && !arg.empty( )) {
	try {
	    doCount( ch, arg.toInt( ) );
	    return;
	} catch (const ExceptionBadType &e) {
	}
    }
    
    pcm = ch->getPC( );
    
    if (!arg.empty( )) 
	if (!( pcm = PCharacterManager::find( arg ) )) {
	    ch->println( "� DreamLand ��� ������ � ����� ������." );
	    return;
	}

    if (!ch->is_immortal( ) && ch->getPC( ) != pcm ) {
	ch->send_to("��� ���������� ������ �� ���.\n\r");
	return;
    }
    
    if (pcm != ch->getPC( )) 
	doShowOther( ch, pcm );
    else
	doShowSelf( ch->getPC( ) );
}

void CMlt::doShowOther( Character *ch, PCMemoryInterface *pcm )
{
    Remorts &r = pcm->getRemorts( );

    ch->send_to("{B ����         �����        ����� ����   �����{x\n\r");

    for (unsigned int i = 0; i < r.size( ); i++) {
	LifeData &life = r[i];

	ch->printf( " %-12s %-12s     %5ld      %s\r\n", 
		    life.race.getValue( ).c_str( ),
		    life.classCh.getValue( ).c_str( ),
		    life.time.getValue( ),
		    (life.bonus ? "*" : ""));
    }
    
    for (int i = 0; i < stat_table.size; i++)
	ch->printf("%d %s ", r.stats[i].getValue( ), stat_table.name( i ).c_str( ) );

    ch->printf("\n%d lvl, %d hp, %d mana, %d sp %s, %d owners\n",
                r.level.getValue( ),
		r.hp.getValue( ), r.mana.getValue( ), r.skillPoints.getValue( ),
		(r.pretitle ? "pretitle" : ""),
		r.owners.getValue( ));
}

void CMlt::doShowSelf( PCharacter *ch )
{
    std::basic_ostringstream<char> str;

    XMLAttributeStatistic::Pointer attr = ch->getAttributes( ).findAttr<XMLAttributeStatistic>( "questdata" );
    int victories = attr ? attr->getAllVictoriesCount( ) : 0;
    int vasted = attr ? attr->getVasted( ) : 0;
    
    if (victories > 0) {
	str << fmt( ch, 
	            "����� �� �������%1$G��|�|�� {W%2$d{x ����������%2$I��|��|�� ����%2$I�|��|���",
	            ch, victories );
	
	if (vasted)
	    str << ", ������� {W" << vasted << "{x �� ���� ����� �� ������";
	else if (victories >= VictoryPrice::COUNT_PER_LIFE)  
	    str << ", ���� �� ������� �� ���� ������ �� ������";

	str << "." << endl;
    }
    else
	str << fmt( ch, "�� ���� �� �������%1$G��|�|�� �� ������ ������������� ������.", ch )
	    << endl;
    
    Remorts &r = ch->getRemorts( );
    int r_cnt = r.size( ), b_cnt = r.countBonusLifes( );

    if (r_cnt > 0) {
	str << fmt( ch, "�� �����%1$G��|�|�� {W%2$d{x ����%2$I�|�|��, ", ch, r_cnt );

	if (r_cnt == b_cnt) {
	    if (r_cnt == 1)
		str << "������� ���� ����� �� ������";
	    else
		str << "{W���{x �� ������� ���� ����� �� ������";
	}
	else
	    str << "{W" << b_cnt << "{x �� ������� "
		<< (b_cnt > 1 ? "����" : "����")
		<< " ����� �� ������";
	str << ":" << endl; 

	for (int i = 0; i < r_cnt; i++) {
	    LifeData &life = r[i];
	    PCRace *race = raceManager->find( life.race )->getPC( );
	    int age = 17 + life.time / 20;

	    str << fmt( ch,
			"     %N1 %N1, ���������%G���|��|��� � �������� %d %s",
			(ch->getSex( ) == SEX_FEMALE ?
			      race->getFemaleName( ).c_str( )
			    : race->getMaleName( ).c_str( )),
			professionManager->find( life.classCh )->getRusName( ).c_str( ),
			ch,
			age, GET_COUNT(age, "����", "���", "���") )
		<< endl;
	}

	str << endl;
    }
    else
	str << "�� ������ ������ �����." << endl;
    
    if (r_cnt > 0 || vasted > 0) {
	str << endl
	    << fmt( ch, "�� ������%1$G��|�|�� {W%2$d{x owner ����%2$I�|��|��� � �����%1$G��|�|�� � �������� �������:", 
			ch, r.owners.getValue( ) ) 
	    << endl;

	str << (r.hp > 0          ? fmt( ch, "     %d ��������\n", r.hp.getValue( ) ) : "")
	    << (r.mana > 0        ? fmt( ch, "     %d ����\n", r.mana.getValue( ) ) : "")
	    << (r.skillPoints > 0 ? fmt( ch, "     %d skill points\n", r.skillPoints.getValue( ) ) : "")
	    << (r.level > 0       ? fmt( ch, "     %1$d ����%1$I���|��|��� \n", r.level.getValue( ) ) : "")
	    << (r.pretitle        ?          "     ������� ��������\n" : "");

	for (int i = 0; i < stat_table.size; i++)
	    if (r.stats[i] > 0)
		str << fmt( ch, "     +%d � %s\n", 
				r.stats[i].getValue( ), 
				stat_table.message( i, '3' ).c_str( ) );
    }

    ch->send_to( str );
}


void CMlt::doCount( Character* ch, int n )
{
    std::basic_ostringstream<char> buf;
    PCharacterMemoryList::const_iterator pos;
    const PCharacterMemoryList& list = PCharacterManager::getPCM( );
    int cnt = 0;
    
    for (pos = list.begin( ); pos != list.end( ); pos++) {
        PCMemoryInterface *pcm = pos->second;
	
        if (pcm->getRemorts( ).size( ) == (unsigned)n) {
            buf << pcm->getName( );
	    
	    if (pcm->getAttributes( ).isAvailable( "fixremort1" ))
		buf << " *";
	    if (pcm->getAttributes( ).isAvailable( "fixremort2" ))
		buf << " **";
	    
	    buf << endl;
            cnt++;
        }
    }
    
    if (cnt > 80)
        ch->printf( "�� ������� �����. %d ���.\r\n", cnt );
    else if (!buf.str( ).empty( )) {
        buf << "Count: " << cnt << endl;
        ch->send_to( buf );
    }
    else
        ch->send_to( "������ ���.\r\n" );
}

void CMlt::doLimit( Character *ch )
{
    ostringstream buf;
    PCharacterMemoryList::const_iterator pos;
    const PCharacterMemoryList& list = PCharacterManager::getPCM( );
    
    for (pos = list.begin( ); pos != list.end( ); pos++) {
        PCMemoryInterface *pcm = pos->second;
	Remorts &r = pcm->getRemorts( );
	
        if (r.size( ) == 0)
	    continue;
	
	buf << pcm->getName( ) << ": ";

	if (r.level) {
	    buf << "[-" << level << " level]";
	    r.points += 10 * r.level;
	    r.level = 0;
	}
	
	if (r.hp > 200) {
	    buf << " [-" << r.hp - 200 << " hp]";
	    r.points += 5 * max( r.hp - 200, 0 ) / 50;
	    r.hp = min( r.hp.getValue( ), 200 );
	}

	for (int i = 0; i < stat_table.size; i++) 
	    if (r.stats[i] > 2) {
		buf << " [-" << r.stats[i] - 2 << " " << i << "]";
		r.points += 10 * max( r.stats[i] - 2, 0 );
		r.stats[i] = min( r.stats[i].getValue( ), 2 );
	    }
	
	buf << endl;
    }

    page_to_char( buf.str( ).c_str( ), ch );
}
