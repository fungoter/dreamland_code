/* $Id: homerecall.cpp,v 1.1.2.15.6.4 2008/07/26 19:08:53 rufina Exp $
 *
 * ruffina, 2004
 */

#include "homerecall.h"

#include "class.h"

#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "pcharactermanager.h"

#include "dreamland.h"
#include "recallmovement.h"
#include "act.h"
#include "merc.h"
#include "arg_utils.h"
#include "mercdb.h"
#include "def.h"

/*----------------------------------------------------------------------------
 * 'homerecall' command 
 *---------------------------------------------------------------------------*/
COMMAND(HomeRecall, "homerecall")
{
    DLString cmd;
    DLString arguments = constArguments;
    PCharacter *pch = ch->getPC( );

    if (ch->is_npc( )) {
	ch->println( "� ���!" );
	return;
    }
    
    if (arguments.empty( )) {
	doRecall( pch, DLString::emptyString );
	return;
    }
    
    cmd = arguments.getOneArgument( );

    if (!pch->is_immortal( )) {
	if (arg_is_list( cmd )) 
	    doListMortal( pch );
	else if (arg_is_help( cmd )) 
	    doUsage( pch ); 
        else
	    doRecall( pch, cmd );
	return;
    }
   
    if (arg_is_list( cmd ))
	doList( pch );
    else if (cmd.strPrefix( "set" ))
	doSet( pch, arguments );
    else if (arg_is_show( cmd ))
	doShow( pch, arguments );
    else if (cmd.strPrefix( "remove" ))
	doRemove( pch, arguments );
    else if (arg_is_help( cmd )) 
	doUsage( pch ); 
    else 
	doRecall( pch, cmd );
}

class HomeRecallMovement : public RecallMovement {
public:
    HomeRecallMovement( Character *ch, const DLString &label )
               : RecallMovement( ch )
    {
	this->label = label;
    }
    HomeRecallMovement( Character *ch, Character *actor, Room *to_room )
               : RecallMovement( ch )
    {
	this->actor = actor;
	this->to_room = to_room;
    }
    
protected:
    DLString label;

    virtual bool findTargetRoom( )
    {
	XMLAttributeHomeRecall::Pointer attr;
	
	if (to_room)
	    return true;

	if (ch->is_npc( )) {
	    msgSelf( ch, "� ���!" );
	    return false;
	}

	attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" );
	if (!attr) {
	    msgSelf( ch, "� ���� ��� ������ ����." );
	    return false;
	}

	int vnum = attr->getLabeledPoint( label );
	if (vnum <= 0 && !label.empty( )) {
	    msgSelf( ch, "� ���� ���� ����, ����������� ����� ������." );
            return false;
	}

	if (!( to_room = get_room_index( vnum ) )) {
	    msgSelf( ch, "�� ��������%1G���|��|���." );
	    return false;
	}

	return true;
    }
    virtual bool canMove( Character *wch )
    {
	if (ch != actor)
	    return true;
	else
	    return checkMount( )
		   && checkShadow( )
		   && checkBloody( wch )
		   && checkPumped( )
		   && checkSameRoom( )
		   && checkForsaken( wch );
    }
    virtual bool tryMove( Character *wch )
    {
	if (ch != actor)
	    return applyInvis( wch );
	else
	    return applyInvis( wch )
		   && applyMovepoints( )
		   && applyWaitstate( );
    }
    virtual void msgOnMove( Character *wch, bool fLeaving )
    {
	if (fLeaving)
	    msgRoomNoParty( wch, 
		            "%1$^C1 ���������%1$G���|��|��� � �������.",
		            "%1$^C1 � %2$C1 ������������ � �������." );
	else
	    msgRoomNoParty( wch, "%1$^C1 ���������� ����� � �����." );
    }
    virtual void msgOnStart( )
    {
	msgRoom( ch, "%1$^C1 ������ ����� ��������� %1$P2 � ������ ���." );
	msgSelf( ch, "�� ������� ����� ��������� ���� � ������ ���." );
    }
    virtual void movePet( NPCharacter *pet )
    {
	HomeRecallMovement( pet, actor, to_room ).moveRecursive( );
    }
};

void HomeRecall::doRecall( PCharacter * ch, const DLString& label )
{
    HomeRecallMovement( ch, label ).move( );
}

void HomeRecall::doSet( PCharacter * ch, DLString &arg )
{
    int vnum;
    PCMemoryInterface *pci;
    DLString name = arg.getOneArgument( );
    DLString vnumArg = arg.getOneArgument( );
    DLString label = arg; 

    try {
	vnum = vnumArg.toInt( );
    } catch (const ExceptionBadType& e) {
	ch->println( "<room vnum> ������ ���� ������." );
	return;
    }
    
    pci = PCharacterManager::find( name );
    if (!pci) {
	ch->println( "������ �� �������." );
	return;
    }
   
    Room *target =  get_room_index( vnum );
    if (!target) {
	ch->println( "������� � ����� ������� �� ����������." );
	return;
    }

    pci->getAttributes( ).getAttr<XMLAttributeHomeRecall>( "homerecall" )->setPoint( vnum, label );
    PCharacterManager::saveMemory( pci );

    if (label.empty( ))
	ch->printf( "��������� %s ���������� �������� ��� � ������� [%d] %s.\r\n", 
		pci->getName( ).c_str( ), vnum, target->name  );
    else
	ch->printf( "��������� %s ���������� ��� � ������ %s � ������� [%d] %s.\r\n", 
		pci->getName( ).c_str( ), label.c_str( ), vnum, target->name  );
}

static void print_room( int vnum, ostringstream &buf )
{
    Room *room = get_room_index( vnum );
    if (!room) {
	buf << "[" << vnum << "] �� ����������!" << endl;
        return;
    }

    buf << "[" << vnum << "] " << room->name << " (" << room->area->name << ")" << endl;
}

void HomeRecall::doShow( PCharacter * ch, DLString &arg )
{
    XMLAttributeHomeRecall::Pointer attr;
    PCMemoryInterface *pci;
    DLString name = arg.getOneArgument( );
    
    pci = PCharacterManager::find( name );
    if (!pci) {
	ch->println( "������ �� �������." );
	return;
    }
    
    attr = pci->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" ); 
    if (!attr) {
	ch->printf( "%s ���������..\r\n", pci->getName( ).c_str( ) );
	return;
    }
    
    ostringstream buf;
    buf << "�������� ���: ";
    print_room( attr->getPoint( ), buf );

    for (XMLAttributeHomeRecall::LabeledPoints::const_iterator l = attr->getLabeled( ).begin( ); l != attr->getLabeled( ).end( ); l++) {
	buf << "��� � ������ '" << l->first << "': ";
        print_room( l->second, buf );
    }
    ch->send_to( buf );
}

void HomeRecall::doRemove( PCharacter * ch, DLString &arg )
{
    XMLAttributeHomeRecall::Pointer attr;
    PCMemoryInterface *pci;
    DLString name = arg.getOneArgument( );
    
    pci = PCharacterManager::find( name );
    if (!pci) {
	ch->println( "������ �� �������." );
	return;
    }
    
    attr = pci->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" ); 
    if (!attr) {
	ch->printf( "%s ���������..\r\n", pci->getName( ).c_str( ) );
	return;
    }

    pci->getAttributes( ).eraseAttribute( "homerecall" );
    PCharacterManager::saveMemory( pci );

    ch->println( "Done." );
}

void HomeRecall::doList( PCharacter *ch ) 
{
    char buf[MAX_STRING_LENGTH];
    int point;
    Room * room;
    PCharacterMemoryList::const_iterator i;
    XMLAttributeHomeRecall::Pointer attr;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
   
    ch->println( "������ ���� ����������, ������� homerecalls: \r\n");
     
    for (i = pcm.begin( ); i != pcm.end( ); i++) {
	attr = i->second->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" ); 

	if (!attr)
	    continue;
	
	point = attr->getPoint( );
	room = get_room_index( point );
	
	sprintf( buf, "%-15s [%-5d] %-25.25s (%s)\r\n", 
		 i->second->getName( ).c_str( ), point, 
		 (room ? room->name : "{Rnull!{x"),
		 (room ? room->area->name : "") );

	ch->send_to( buf );
    }
}

static void print_room_mortal( int vnum, ostringstream &buf )
{
    Room *room = get_room_index( vnum );
    if (!room) {
	buf << "�� ����������!" << endl;
        return;
    }

    buf << room->name << " (" << room->area->name << ")" << endl;
}

void HomeRecall::doListMortal( PCharacter * ch )
{
    XMLAttributeHomeRecall::Pointer attr = ch->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" ); 
    if (!attr) {
	ch->println( "� ���� ��� ������ ����." );
	return;
    }
    
    ostringstream buf;
    buf << "�������� ���: ";
    print_room_mortal( attr->getPoint( ), buf );

    for (XMLAttributeHomeRecall::LabeledPoints::const_iterator l = attr->getLabeled( ).begin( ); l != attr->getLabeled( ).end( ); l++) {
	buf << "��� � ������ '" << l->first << "': ";
        print_room_mortal( l->second, buf );
    }
    ch->send_to( buf );
}

void HomeRecall::doUsage( PCharacter * ch )
{
    std::basic_ostringstream<char> buf;

    buf << "���������: " << endl
	<< "{W{lEhomerecall{lR�����     {x             - ��������� � ���" << endl
	<< "{W{lEhomerecall �����{lR����� �����     {x       - ��������� � ��� � ��������� ������" << endl;
    if (ch->is_immortal( ))
        buf << "{Whomerecall set{x <name> <room vnum>   - ���������� ������ ������� ��� homerecall" << endl
	    << "� ������ ��� ������� ������� ����, �� �������� �� ����� ������ ����" << endl
	    << "{Whomerecall set{x <name> <room vnum> <label>  " << endl
	    << "                                    - ���������� homerecall � ��������� ������" << endl
	    << "{Whomerecall show{x <name>              - ���������� ���-�� homerecall" << endl
	    << "{Whomerecall remove{x <name>            - �������� ����������� ����������� �����" << endl
	    << "{Whomerecall list{x                     - ������ ���� �������, ������� homerecall" << endl;
    else 
	buf << "{W{lEhomerecall list{lR����� ������   {x        - �������� ������ ����� ����� � �����" << endl;

    
    ch->send_to( buf );
}

/*----------------------------------------------------------------------------
 * XMLAttributeHomeRecall
 *---------------------------------------------------------------------------*/
XMLAttributeHomeRecall::XMLAttributeHomeRecall( ) 
{
}

XMLAttributeHomeRecall::~XMLAttributeHomeRecall( ) 
{
}

int XMLAttributeHomeRecall::getPoint( ) const
{
    return point;
}

int XMLAttributeHomeRecall::getLabeledPoint( const DLString &label ) const
{
    if (label.empty( ))
	return point.getValue( );
    else {
	LabeledPoints::const_iterator l = labeled.find( label );
	if (l == labeled.end( ))
            return 0;
        else
            return l->second;
    }
	
}

void XMLAttributeHomeRecall::setPoint( int point, const DLString &label ) 
{
    if (label.empty( ))
	this->point = point;
    else
        labeled[label] = point;
}
    
const XMLAttributeHomeRecall::LabeledPoints & XMLAttributeHomeRecall::getLabeled( ) const
{
    return labeled;
}

