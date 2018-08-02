/* $Id: social.h,v 1.1.2.1.6.3 2008/05/20 22:11:38 rufina Exp $
 *
 * ruffina, 2004
 */
/* 
 *
 * sturm, 2003
 */
#ifndef SOCIAL_H
#define SOCIAL_H

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlstringlist.h"
#include "xmlenumeration.h"
#include "xmltableelement.h"
#include "socialbase.h"
#include "xmlloader.h"

class Social : public SocialBase, public XMLVariableContainer, 
               public XMLTableElement 
{
XML_OBJECT
public:	
    typedef ::Pointer<Social> Pointer;

    Social( );
    virtual ~Social( );
    
    virtual bool matches( const DLString & ) const;
    inline virtual const DLString &getName( ) const;
    inline virtual void setName( const DLString & );
    inline virtual const DLString &getRussianName( ) const;
    inline const DLString &getShortDesc( ) const;

protected:
    virtual void reaction( Character *, Character *, const DLString & );
    inline virtual int getPosition( ) const;
    inline virtual const DLString & getNoargOther( ) const;
    inline virtual const DLString & getNoargMe( ) const;
    inline virtual const DLString & getAutoOther( ) const;
    inline virtual const DLString & getAutoMe( ) const;
    inline virtual const DLString & getArgOther( ) const;
    inline virtual const DLString & getArgMe( ) const;
    inline virtual const DLString & getArgVictim( ) const;
    inline virtual const DLString & getArgOther2( ) const;
    inline virtual const DLString & getArgMe2( ) const;
    inline virtual const DLString & getArgVictim2( ) const;
    inline virtual const DLString & getErrorMsg( ) const;

private:
    bool mprog( Character *, Character * );

    DLString name;
    XML_VARIABLE XMLString  rusName;
    XML_VARIABLE XMLString  shortDesc;
    XML_VARIABLE XMLString  msgCharNoArgument;
    XML_VARIABLE XMLString  msgOthersNoArgument;
    XML_VARIABLE XMLString  msgCharFound;
    XML_VARIABLE XMLString  msgOthersFound;
    XML_VARIABLE XMLString  msgVictimFound;
    XML_VARIABLE XMLString  msgCharNotFound;
    XML_VARIABLE XMLString  msgCharAuto;
    XML_VARIABLE XMLString  msgOthersAuto;
    XML_VARIABLE XMLString  msgCharFound2;
    XML_VARIABLE XMLString  msgOthersFound2;
    XML_VARIABLE XMLString  msgVictimFound2;
    XML_VARIABLE XMLStringList aliases;

    XML_VARIABLE XMLEnumeration position;
};

inline const DLString& Social::getName( ) const 
{
    return name;
}
inline void Social::setName( const DLString &name ) 
{
    this->name = name;
}
inline const DLString& Social::getRussianName( ) const 
{
    return rusName.getValue( );
}
inline const DLString & Social::getShortDesc( ) const
{
    return shortDesc.getValue( );
}
inline int Social::getPosition( ) const 
{
    return position.getValue( );
}
inline const DLString & Social::getNoargOther( ) const
{
    return msgOthersNoArgument.getValue( );
}
inline const DLString & Social::getNoargMe( ) const
{
    return msgCharNoArgument.getValue( );
}
inline const DLString & Social::getArgMe( ) const
{
    return msgCharFound.getValue( );
}
inline const DLString & Social::getArgOther( ) const
{
    return msgOthersFound.getValue( );
}
inline const DLString & Social::getArgVictim( ) const
{
    return msgVictimFound.getValue( );
}
inline const DLString & Social::getAutoMe( ) const
{
    return msgCharAuto.getValue( );
}
inline const DLString & Social::getAutoOther( ) const
{
    return msgOthersAuto.getValue( );
}
inline const DLString & Social::getArgMe2( ) const
{
    return msgCharFound2.getValue( );
}
inline const DLString & Social::getArgOther2( ) const
{
    return msgOthersFound2.getValue( );
}
inline const DLString & Social::getArgVictim2( ) const
{
    return msgVictimFound2.getValue( );
}
inline const DLString & Social::getErrorMsg( ) const
{
    return msgCharNotFound.getValue( );
}

#endif

