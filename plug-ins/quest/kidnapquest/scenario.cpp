/* $Id: scenario.cpp,v 1.1.2.9.18.3 2007/09/29 19:34:03 rufina Exp $
 *
 * ruffina, 2004
 */
#include "scenario.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"

#include "object.h"
#include "handler.h"
#include "act.h"
#include "merc.h"
#include "def.h"

RACE(none);

/*
 * KSPrinceData
 */
void KSPrinceData::dress( NPCharacter *mob, NPCharacter *king ) 
{
    QuestMobileAppearence::dress( mob );

    if (mob->getRace( ) == race_none)
	mob->setRace( king->getRace( )->getName( ) );
}


/*
 * KidnapScenario
 */
void KidnapScenario::onQuestStart( PCharacter *hero, NPCharacter *questman, NPCharacter *king )
{
    tell_raw( hero, questman, 
	      "� {W%s{G ��������� ���������. ������ ��������� ���� ������.",
                   king->getNameP( '2' ).c_str() );
    tell_raw( hero, questman, 
	     "��� %s � ��������� ��� ��������� {W%s{G ({W%s{G).",
                   GET_SEX(king, "���", "���", "��"), king->in_room->name, king->in_room->area->name );
}

