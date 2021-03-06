/* $Id: defaultspell.h,v 1.1.2.2.18.8 2008/07/04 12:05:08 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef	__DEFAULTSPELL_H__
#define	__DEFAULTSPELL_H__

#include "spell.h"
#include "skill.h"

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmlflags.h"
#include "xmlenumeration.h"
#include "xmlboolean.h"

class DefaultSpell : public Spell, public XMLVariableContainer
{
XML_OBJECT
public:
    typedef ::Pointer<DefaultSpell> Pointer;
    
    DefaultSpell( );

    virtual void setSkill( SkillPointer );
    virtual void unsetSkill( );
    virtual SkillPointer getSkill( ) const;

    virtual void run( Character *, SpellTargetPointer, int );             
    virtual void run( Character *, Character *, int, int ) { }
    virtual void run( Character *, Object *, int, int ) { }
    virtual void run( Character *, char *, int, int ) { }
    virtual void run( Character *, Room *, int, int ) { }

    virtual int getManaCost( Character * );
    virtual int getMaxRange( Character * ) const;		
    virtual Character * getCharSpell( Character *, const DLString &, int *, int * );
    virtual bool spellbane( Character *, Character * ) const; 
    virtual void utter( Character * );
    virtual int getSpellLevel( Character *, int );

    virtual SpellTargetPointer locateTargets( Character *, const DLString &, std::ostringstream & );
    virtual SpellTargetPointer locateTargetObject( Character *, const DLString &, std::ostringstream & );

    virtual int getBeats( ) const;
    virtual int getMana( ) const;
    virtual int getTarget( ) const;
    virtual int getSpellType( ) const;
    virtual bool isCasted( ) const;
    virtual bool isPrayer( Character * ) const;
    virtual bool checkPosition( Character * ) const;

protected:
    void baneMessage( Character *ch, Character *vch ) const;
    void baneDamage( Character *ch, Character *vch, int dam ) const;
    void baneAround( Character *ch, int failChance, int dam ) const;
    void baneForAssist( Character *ch, Character *vch ) const;
    bool baneAction( Character *ch, Character *bch, int failChance, int dam ) const;

    XML_VARIABLE XMLFlags   target;
    XML_VARIABLE XMLEnumeration   position;
    XML_VARIABLE XMLEnumeration   type;
    XML_VARIABLE XMLBoolean casted;

    SkillPointer skill;
};

#endif
