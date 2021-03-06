/* $Id: scenario_dragon.h,v 1.1.2.5.18.1 2007/09/29 19:34:03 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __KIDNAPDRAGONSCENARIO_H__
#define __KIDNAPDRAGONSCENARIO_H__

#include "scenario.h"

class KidnapDragonScenario : public KidnapScenario {
XML_OBJECT
public:

    virtual bool applicable( PCharacter * );

    virtual void msgRemoteReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero );
    virtual void msgKingDeath( NPCharacter *king, Character *killer, PCharacter *hero );
    virtual void msgKidDeath( NPCharacter *kid, Character *killer, PCharacter *hero );

    virtual void actAttackHero( NPCharacter *bandit, PCharacter *hero );
    virtual void actBeginKidnap( NPCharacter *bandit, NPCharacter *kid );
    virtual void actHuntStep( NPCharacter *bandit );
    virtual void actKidnapStep( NPCharacter *bandit, NPCharacter *kid );
    virtual void actEmptyPath( NPCharacter *bandit, NPCharacter *kid );

    virtual void actLegend( NPCharacter *king, PCharacter *hero, ::Pointer<KidnapQuest> quest );
    virtual void actGiveMark( NPCharacter *king, PCharacter *hero, Object *mark, int time );
    virtual void actMarkLost( NPCharacter *king, PCharacter *hero, Object *mark );
    virtual void actAckWaitComplete( NPCharacter *king, PCharacter *hero );
   
    virtual void actHeroWait( NPCharacter *kid );
    virtual void actNoHero( NPCharacter *kid, PCharacter *hero );
    virtual void actHeroDetach( NPCharacter *kid, PCharacter *hero );
    virtual void actWrongGiver( NPCharacter *kid, Character *victim, Object *obj );
    virtual void actWrongMark( NPCharacter *kid, Character *victim, Object *obj );
    virtual void actGoodMark( NPCharacter *kid, Character *victim, Object *obj );
    virtual void actReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero );
    virtual void actBanditsUnleash( NPCharacter *kid, PCharacter *hero, NPCharacter *bandit );
};

#endif
