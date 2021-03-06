/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTRELIGION_H
#define DEFAULTRELIGION_H

#include "xmlstring.h"
#include "xmlflags.h"
#include "xmltableelement.h"
#include "xmlvariablecontainer.h"

#include "religion.h"

class DefaultReligion : public Religion, public XMLTableElement, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<DefaultReligion> Pointer;
    
    DefaultReligion( );
    
    virtual const DLString & getName( ) const;
    virtual const DLString &getRussianName( ) const;
    virtual void setName( const DLString & );
    virtual bool isValid( ) const;
    virtual void loaded( );
    virtual void unloaded( );
    
    virtual const DLString &getShortDescr( ) const;
    virtual const DLString &getDescription( ) const;
    virtual bool isAllowed( Character * ) const;
    virtual const DLString& getNameFor( Character * ) const;

protected:
    XML_VARIABLE XMLString  shortDescr;
    XML_VARIABLE XMLString  nameRus;
    XML_VARIABLE XMLString  description;
    XML_VARIABLE XMLFlags   align, ethos;
};

#endif
