/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __RACEHANNEL_H__
#define __RACECHANNEL_H__

#include "worldchannel.h"

class RaceChannel : public WorldChannel {
XML_OBJECT    
public:
    typedef ::Pointer<RaceChannel> Pointer;
    
    RaceChannel( );

protected:
    virtual bool isGlobalListener( Character *, Character * ) const;
    virtual bool canTalkGlobally( Character * ) const;

    virtual void outputVict( Character *, Character *, const DLString &, const DLString & ) const;
    virtual void outputSelf( Character *, const DLString &, const DLString & ) const;
    virtual void postOutput( Character *outputTo, const DLString &message ) const;
};

#endif
