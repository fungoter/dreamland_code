/* $Id: corder.h,v 1.1.2.4.6.3 2008/04/05 01:58:48 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef CORDER_H
#define CORDER_H

#include "commandplugin.h"
#include "defaultcommand.h"

class InterpretArguments;

class COrder : public CommandPlugin, public DefaultCommand {
XML_OBJECT
public:
	typedef ::Pointer<COrder> Pointer;
    
	COrder( );

	virtual void run( Character *, const DLString & );
	
private:
	bool canOrder( Character *, Character * );
	Character *findVictim( Character *, DLString & );
	void interpretOrder( Character *, InterpretArguments &, const DLString & );

	static const DLString COMMAND_NAME;
};

#endif

