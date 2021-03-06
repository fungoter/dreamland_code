/* $Id$
 *
 * ruffina, 2004
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include <fstream>
#include <list>
#include <set>

#include <config.h>

#include "grammar_entities_impl.h"
#include "dlfilestream.h"
#include "regexp.h"
#include <xmldocument.h>

#include <skill.h>
#include <spell.h>
#include <skillmanager.h>
#include "feniamanager.h"

#include <affect.h>
#include <object.h>
#include <pcharacter.h>
#include <npcharacter.h>
#include <commandmanager.h>
#include "race.h"

#include "room.h"

#include "olc.h"
#include "olcflags.h"
#include "olcstate.h"
#include "security.h"

#include "interp.h"
#include "merc.h"
#include "handler.h"
#include "act.h"
#include "save.h"
#include "act_move.h"
#include "vnum.h"
#include "mercdb.h"

#include "comm.h"
#include "def.h"


GSN(enchant_weapon);
GSN(enchant_armor);
GSN(none);

using namespace std;


enum {
    NDX_ROOM,
    NDX_OBJ,
    NDX_MOB,
};

static int next_index_data( Character *ch, Room *r, int ndx_type )
{
    AREA_DATA *pArea;
    
    if (!r)
	return -1;

    pArea = r->area;
    if (!pArea)
	return -1;

    for (int i = pArea->min_vnum; i <= pArea->max_vnum; i++) {
	if (!OLCState::can_edit( ch, i ))
	    continue;

	switch (ndx_type) {
	case NDX_ROOM:
	    if (!get_room_index( i ))
		return i;
	    break;
	case NDX_OBJ:
	    if (!get_obj_index( i ))
		return i;
	    break;
	case NDX_MOB:
	    if (!get_mob_index( i ))
		return i;
	    break;
	}
    }

    return -1;
}
    
int next_room( Character *ch, Room *r )
{
    return next_index_data( ch, r, NDX_ROOM );
}

int next_obj_index( Character *ch, Room *r )
{
    return next_index_data( ch, r, NDX_OBJ );
}

int next_mob_index( Character *ch, Room *r )
{
    return next_index_data( ch, r, NDX_MOB );
}

const char *
get_skill_name( int sn, bool verbose )
{
    Skill *skill = SkillManager::getThis( )->find( sn );

    if (skill)
	return skill->getName( ).c_str( );
    else if (verbose)
	return "none";
    else
	return "";
}

void
ptc(Character *ch, const char *fmt, ...)
{
    va_list av;

    va_start(av, fmt);

    DLString rc = vfmt(ch, fmt, av);
    stc(rc.c_str( ), ch);

    va_end(av);
}

AREA_DATA *get_area_data(int vnum)
{
    AREA_DATA *pArea;

    for (pArea = area_first; pArea; pArea = pArea->next) {
	if (pArea->vnum == vnum)
	    return pArea;
    }
    return 0;
}

void display_resets(Character * ch)
{
    Room *pRoom;
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMob = NULL;
    char buf[MAX_STRING_LENGTH];
    char final[MAX_STRING_LENGTH];
    int iReset = 0;

    pRoom = ch->in_room;

    final[0] = '\0';

    stc(
	   " No.  Loads       Description          Location   Vnum   Mx Mn Description"
	   "\n\r"
	   "==== ======== ======================== ======== ======== ===== ==========="
	   "\n\r", ch);

    for (pReset = pRoom->reset_first; pReset; pReset = pReset->next) {
	OBJ_INDEX_DATA *pObj;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_INDEX_DATA *pObjToIndex;
	Room *pRoomIndex;

	final[0] = '\0';
	sprintf(final, "[%2d] ", ++iReset);

	switch (pReset->command) {
	default:
	    sprintf(buf, "Bad reset command: %c.", pReset->command);
	    strcat(final, buf);
	    break;
	case 'M':
	    if (!(pMobIndex = get_mob_index(pReset->arg1))) {
		sprintf(buf, "Load Mobile - Bad Mob %u\n\r", pReset->arg1);
		strcat(final, buf);
		continue;
	    }
	    if (!(pRoomIndex = get_room_index(pReset->arg3))) {
		sprintf(buf, "Load Mobile - Bad Room %u\n\r", pReset->arg3);
		strcat(final, buf);
		continue;
	    }

	    pMob = pMobIndex;
	    sprintf(buf, "M[%5u] %-24.24s %-8s R[%5u] %2d-%2d %-15.15s{x\n\r",
		      pReset->arg1, 
		      russian_case(pMob->short_descr, '1').colourStrip( ).c_str( ),
		      "in room", 
		      pReset->arg3,
		      pReset->arg2, 
		      pReset->arg4, 
		      pRoomIndex->name);
	    strcat(final, buf);
	    break;

	case 'O':
	    if (!(pObjIndex = get_obj_index(pReset->arg1))) {
		sprintf(buf, "Load Object - Bad Object %u\n\r", pReset->arg1);
		strcat(final, buf);
		continue;
	    }
	    pObj = pObjIndex;
	    if (!(pRoomIndex = get_room_index(pReset->arg3))) {
		sprintf(buf, "Load Object - Bad Room %u\n\r", pReset->arg3);
		strcat(final, buf);
		continue;
	    }
	    sprintf(buf, "O[%5u] %-24.24s %-8s R[%5u]       %-15.15s{x\n\r",
		      pReset->arg1, 
		      russian_case(pObj->short_descr, '1').colourStrip( ).c_str( ),
		      "in room",
		      pReset->arg3, 
		      pRoomIndex->name);
	    strcat(final, buf);
	    break;

	case 'P':
	    if (!(pObjIndex = get_obj_index(pReset->arg1))) {
		sprintf(buf, "Put Object - Bad Object %u\n\r", pReset->arg1);
		strcat(final, buf);
		continue;
	    }

	    pObj = pObjIndex;

	    if (!(pObjToIndex = get_obj_index(pReset->arg3))) {
		sprintf(buf, "Put Object - Bad To Object %u\n\r", pReset->arg3);
		strcat(final, buf);
		continue;
	    }

	    sprintf(buf, "O[%5u] %-24.24s %-8s O[%5u] %2d-%2d %-15.15s{x\n\r",
		      pReset->arg1,
		      russian_case(pObj->short_descr, '1').colourStrip( ).c_str( ),
		      "inside",
		      pReset->arg3,
		      pReset->arg2,
		      pReset->arg4,
		      russian_case(pObjToIndex->short_descr, '1').colourStrip( ).c_str( ));
	    strcat(final, buf);
	    break;

	case 'G':
	case 'E':
	    if (!(pObjIndex = get_obj_index(pReset->arg1))) {
		sprintf(buf, "Give/Equip Object - Bad Object %u\n\r", pReset->arg1);
		strcat(final, buf);
		continue;
	    }

	    pObj = pObjIndex;

	    if (!pMob) {
		sprintf(buf, "Give/Equip Object - No Previous Mobile\n\r");
		strcat(final, buf);
		break;
	    }

	    sprintf(buf,
		      "O[%5u] %-24.24s %-8.8s M[%5u]       %-15.15s{x\n\r",
		      pReset->arg1,
		      russian_case(pObj->short_descr, '1').colourStrip( ).c_str( ),
		      (pReset->command == 'G') ?
			  wear_none.getName( ).c_str( )
			  : wearlocationManager->find( pReset->arg3 )->getName( ).c_str( ),
		      pMob->vnum,
		      russian_case(pMob->short_descr, '1').colourStrip( ).c_str( ));
	    strcat(final, buf);
	    break;

	case 'D':
	    pRoomIndex = get_room_index(pReset->arg1);
	    sprintf(buf, "R[%5u] %s door of %-19s reset to %s{x\n\r",
		      pReset->arg1,
		      DLString(dirs[pReset->arg2].name).capitalize( ).c_str( ),
		      pRoomIndex->name,
		      door_resets_table.name(pReset->arg3).c_str());
	    strcat(final, buf);

	    break;
	case 'R':
	    if (!(pRoomIndex = get_room_index(pReset->arg1))) {
		sprintf(buf, "Randomize Exits - Bad Room %u\n\r", pReset->arg1);
		strcat(final, buf);
		continue;
	    }

	    sprintf(buf, "R[%5u] Exits are randomized in %s{x\n\r",
		      pReset->arg1, pRoomIndex->name);
	    strcat(final, buf);
	    break;
	}
	stc(final, ch);
    }
}

struct editor_table_entry {
    const char *arg, *cmd;
} editor_table[] = {
    {"area",   "aedit"},
    {"room",   "redit"},
    {"object", "oedit"},
    {"mobile", "medit"},
    {NULL, 0,}
};

CMD(edit, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
	"Online editor.")
{
    char command[MAX_INPUT_LENGTH];
    int cmd;

    argument = one_argument(argument, command);

    if (command[0] == '\0') {
//	do_help(ch, "olc");
	return;
    }

    for (cmd = 0; editor_table[cmd].arg != NULL; cmd++) {
	if (!str_prefix(command, editor_table[cmd].arg)) {
	    interpret_raw(ch, editor_table[cmd].cmd, "%s", argument);
	    return;
	}
    }

//    do_help(ch, "olc");
}

void add_reset(Room * room, RESET_DATA * pReset, int index)
{
    RESET_DATA *reset;
    int iReset = 0;

    if (!room->reset_first) {
	room->reset_first = pReset;
	room->reset_last = pReset;
	pReset->next = NULL;
	return;
    }
    index--;

    if (index == 0) {		/* First slot (1) selected. */
	pReset->next = room->reset_first;
	room->reset_first = pReset;
	return;
    }

    // If negative slot( <= 0 selected) then this will find the last.
    for (reset = room->reset_first; reset->next; reset = reset->next) {
	if (++iReset == index)
	    break;
    }

    pReset->next = reset->next;
    reset->next = pReset;
    if (!pReset->next)
	room->reset_last = pReset;
}

CMD(resets, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
	"Online resets editor.")
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    char arg6[MAX_INPUT_LENGTH];
    char arg7[MAX_INPUT_LENGTH];
    RESET_DATA *pReset = NULL;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);
    argument = one_argument(argument, arg5);
    argument = one_argument(argument, arg6);
    argument = one_argument(argument, arg7);
    
    if (!OLCState::can_edit( ch, ch->in_room->vnum )) {
	stc("Resets: Invalid security for editing this area.\n\r", ch);
	return;
    }

    if (arg1[0] == '\0') {
	if (ch->in_room->reset_first) {
	    stc("Resets: M = mobile, R = room, O = object\n\r", ch);
	    display_resets(ch);
	}
	else
	    stc("No resets in this room.\n\r", ch);
    }

    if (is_number(arg1)) {
	Room *pRoom = ch->in_room;

	if (!str_cmp(arg2, "delete")) {
	    int insert_loc = atoi(arg1);

	    if (!ch->in_room->reset_first) {
		stc("No resets in this area.\n\r", ch);
		return;
	    }
	    if (insert_loc - 1 <= 0) {
		pReset = pRoom->reset_first;
		pRoom->reset_first = pRoom->reset_first->next;
		if (!pRoom->reset_first)
		    pRoom->reset_last = NULL;
	    }
	    else {
		int iReset = 0;
		RESET_DATA *prev = NULL;

		for (pReset = pRoom->reset_first; pReset; pReset = pReset->next) {
		    if (++iReset == insert_loc)
			break;
		    prev = pReset;
		}
		if (!pReset) {
		    stc("Reset not found.\n\r", ch);
		    return;
		}
		if (prev)
		    prev->next = prev->next->next;
		else
		    pRoom->reset_first = pRoom->reset_first->next;

		for (pRoom->reset_last = pRoom->reset_first;
		     pRoom->reset_last->next;
		     pRoom->reset_last = pRoom->reset_last->next);
	    }
	    free_reset_data(pReset);
	    SET_BIT(ch->in_room->area->area_flag, AREA_CHANGED);
	    stc("Reset deleted.\n\r", ch);
	}
	else if ((!str_cmp(arg2, "mob") && is_number(arg3))
		 || (!str_cmp(arg2, "obj") && is_number(arg3))) {
	    if (!str_cmp(arg2, "mob")) {
		pReset = new_reset_data();
		pReset->command = 'M';
		if (get_mob_index(is_number(arg3) ? atoi(arg3) : 1) == NULL) {
		    stc("������ �� ����������.\n\r", ch);
		    return;
		}
		pReset->arg1 = atoi(arg3);
		pReset->arg2 = is_number(arg4) ? atoi(arg4) : 1;	/* Max # */
		pReset->arg3 = ch->in_room->vnum;
		pReset->arg4 = is_number(arg5) ? atoi(arg5) : 1;	/* Min # */
	    }
	    else if (!str_cmp(arg2, "obj")) {
		pReset = new_reset_data();
		pReset->arg1 = atoi(arg3);
		if (!str_prefix(arg4, "inside")) {
		    pReset->command = 'P';
		    pReset->arg2 = 0;
		    if ((get_obj_index(is_number(arg5) ? atoi(arg5) : 1))->item_type != ITEM_CONTAINER) {
			stc("������� �� �������� �����������.\n\r", ch);
			return;
		    }
		    pReset->arg2 = is_number(arg6) ? atoi(arg6) : 1;
		    pReset->arg3 = is_number(arg5) ? atoi(arg5) : 1;
		    pReset->arg4 = is_number(arg7) ? atoi(arg7) : 1;
		}
		else if (!str_cmp(arg4, "room")) {
		    pReset = new_reset_data();
		    pReset->command = 'O';
		    if (get_obj_index(atoi(arg3)) == NULL) {
			stc("��������� � ����� ������� �� ����������.\n\r", ch);
			return;
		    }
		    pReset->arg1 = atoi(arg3);
		    pReset->arg2 = 0;
		    pReset->arg3 = ch->in_room->vnum;
		    pReset->arg4 = 0;
		}
		else {
		    Wearlocation *wl;
		    
		    if (!( wl = wearlocationManager->findExisting( arg4 ) )) {
			stc("Resets: '? wear-loc'\n\r", ch);
			return;
		    }
		    pReset = new_reset_data();
		    if (get_obj_index(atoi(arg3)) == NULL) {
			stc("��������� � ����� ������� �� ����������.\n\r", ch);
			return;
		    }
		    pReset->arg1 = atoi(arg3);
		    pReset->arg3 = wl->getIndex( );
		    if (pReset->arg3 == wear_none)
			pReset->command = 'G';
		    else
			pReset->command = 'E';
		}
	    }
	    add_reset(ch->in_room, pReset, atoi(arg1));
	    SET_BIT(ch->in_room->area->area_flag, AREA_CHANGED);
	    stc("Reset added.\n\r", ch);
	}
	else {
	    stc("Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r", ch);
	    stc("        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]\n\r", ch);
	    stc("        RESET <number> OBJ <vnum> room\n\r", ch);
	    stc("        RESET <number> MOB <vnum> [max # area] [max # room]\n\r", ch);
	    stc("        RESET <number> DELETE\n\r", ch);
	}
    }
}

CMD(alist, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
	"List areas.")
{
    AREA_DATA *pArea;

    ptc(ch, "%3s Sec [%27s] (%5s-%5s) %15s [%-10s]\n\r",
      "Num", "Area Name", "lvnum", "uvnum", "Filename", "Authors");

    for (pArea = area_first; pArea; pArea = pArea->next) {
	ptc(ch, "%3d [%d] {W%-29s {x(%5u-%5u) %15s [%10s]\n\r",
	    pArea->vnum, pArea->security, pArea->name,
	    pArea->min_vnum, pArea->max_vnum,
	    pArea->area_file->file_name, pArea->authors);
    }
}

static int get_obj_reset_level( AREA_DATA *pArea, int keyVnum )
{
    int mobVnum = -1;
    int level = 200;
    
    if (keyVnum <= 0 || !get_obj_index( keyVnum ))
	return pArea->low_range;
	
    for (Room *room = room_list; room; room = room->rnext) 
	for(RESET_DATA *pReset = room->reset_first;pReset;pReset = pReset->next)
	    switch(pReset->command) {
		case 'M':
		    mobVnum = pReset->arg1;
		    break;
		case 'G':
		case 'E':
		    if (pReset->arg1 == keyVnum && mobVnum > 0)
			level = min( get_mob_index(mobVnum)->level, level );
		    break;
		case 'O':
		    if (pReset->arg1 == keyVnum) 
			level = min( pArea->low_range, level );
		    break; 
		case 'P':
		    if (pReset->arg1 == keyVnum) {
			OBJ_INDEX_DATA *in = get_obj_index( pReset->arg3 );

			if (in->item_type == ITEM_CONTAINER
			    && IS_SET(in->value[1], CONT_LOCKED))
			{
			    level = min( get_obj_reset_level( room->area, in->value[2] ), 
			                 level );
			}
			else
			    level = min( pArea->low_range, level );
		    }
		    break;
	    }
    
    return (level == 200 ? pArea->low_range : level);
}	    

static bool get_obj_resets( int vnum, AREA_DATA *&pArea, DLString &where )
{
    // Scan resets for each room.
    for (Room *room = room_list; room; room = room->rnext) {
        int mobVnum = -1;
	for(RESET_DATA *pReset = room->reset_first;pReset;pReset = pReset->next)
	    switch(pReset->command) {
		case 'M':
                    // Remember potential carrier in the room.
		    mobVnum = pReset->arg1;
		    break;
		case 'G':
		case 'E':
                    // Found who carries the object.
		    if (pReset->arg1 == vnum && mobVnum > 0) {
                        MOB_INDEX_DATA *pMob = get_mob_index( mobVnum );
                        if (pMob) {
                            // Return success
                            pArea = room->area;
                            where = russian_case(pMob->short_descr, '1');
                            return true;
                        }
                    }
		    break;
		case 'O':
		    if (pReset->arg1 == vnum) { 
                        // Object is on the floor, return success.
                        pArea = room->area;
                        where = room->name;
                        return true;
                    }
		    break; 
		case 'P':
                    // Found where the object is placed.
		    if (pReset->arg1 == vnum) {
			OBJ_INDEX_DATA *in = get_obj_index( pReset->arg3 );
                        if (in) {
                            // Return success.
                            pArea = room->area;
                            where = room->name;
                            return true;
                        }
		    }
		    break;
	    }
     }

    // No candidates have been found, return a failure.
    return false;
}            

static void csv_escape( DLString &name ) {
    name.colourstrip( );
    name.replaces( "\"", "'");
//    name.replaces( "\'", "\\\'");
}

static DLString trim(const DLString& str, const string& chars = "\t\n\v\f\r ")
{
    DLString line = str;
    line.erase(line.find_last_not_of(chars) + 1);
    line.erase(0, line.find_first_not_of(chars));
    return line;
}

CMD(abc, 50, "", POS_DEAD, 110, LOG_ALWAYS, "")
{
    if (!ch->isCoder( ))
	return;

    DLString args = argument;
    DLString arg = args.getOneArgument();
    ostringstream buf;

    if (arg == "magic") {
	for (AREA_DATA *area = area_first; area; area = area->next) {
	    buf << "{C       ******   {C" << area->name << "      ********{x" << endl;
	    for (int i = area->min_vnum; i <= area->max_vnum; i++) {
		OBJ_INDEX_DATA *pObj = get_obj_index( i );
		if (!pObj)
		    continue;
		if (!IS_SET(pObj->extra_flags, ITEM_MAGIC))
		    continue;
		if (pObj->item_type == ITEM_PORTAL)
		    continue;
		
		REMOVE_BIT(pObj->extra_flags, ITEM_MAGIC);
		SET_BIT(pObj->area->area_flag, AREA_CHANGED);

		buf << dlprintf("[%5d] %s\n", 
			pObj->vnum, russian_case(pObj->short_descr, '1').c_str( ));
	    }
	}

	page_to_char(buf.str().c_str(), ch);
	return;
    }
    
    if (arg == "nomagic") {
	for (Object *obj = object_list; obj; obj = obj->next) {
	    if (!IS_SET(obj->extra_flags, ITEM_MAGIC))
		continue;
	    
	    if (IS_SET(obj->pIndexData->extra_flags, ITEM_MAGIC))
		continue;
	    
	    if (obj->enchanted) {
		if (obj->affected 
		    && (obj->affected->affect_find( gsn_enchant_armor )
		       || obj->affected->affect_find( gsn_enchant_weapon )))
		    continue;
	    }

	    Room *r = obj->getRoom( );

	    REMOVE_BIT( obj->extra_flags, ITEM_MAGIC );
	    buf << "nomagic [" << obj->pIndexData->vnum << "] " 
		<< "[" << obj->getID( ) << "] "
		<< obj->getShortDescr( '1' ) << " "
		<< (obj->getRealShortDescr( ) ? "*" : "")
		<< " " << (obj->getOwner( ) ? obj->getOwner( ) : "")
		<< " " << "[" << r->vnum << "] " 
		<< r->name 
		<< endl;
	    save_items( r );
	}

	page_to_char( buf.str( ).c_str( ), ch );
	return;
    }

    if (arg == "gender") {
	DLString prefix = args.getOneArgument();
	DLString mg = args.getOneArgument();

	ch->printf("Prefix=[%s] mg=[%s]\n", prefix.c_str(), mg.c_str());

	for (int i = 0; i < MAX_KEY_HASH; i++)
	    for(OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) {
		DLString oname = russian_case(pObj->short_descr, '1').colourStrip().toLower();
		
		if (oname.isName(prefix)) {
		    buf << dlprintf("[%5d] %s\n", pObj->vnum, oname.c_str());

		    if (!mg.empty()) {
			pObj->gram_gender = Grammar::MultiGender(mg.c_str());
			SET_BIT(pObj->area->area_flag, AREA_CHANGED);
		    }
		}
	    }

	page_to_char(buf.str().c_str(), ch);
	return;
    }
    
    if (arg == "swim") {
	for (Character *wch = char_list; wch; wch = wch->next) {
	    if (!wch->is_npc())
		continue;
	    if (!IS_WATER(wch->in_room))
		continue;
	    if (IS_AFFECTED(wch, AFF_FLYING) || IS_AFFECTED(wch, AFF_SWIM))
		continue;
	    if (boat_get_type(wch) != BOAT_NONE)
		continue;

	    buf << dlprintf("[%5d] (%14s) %s\n", 
			    wch->getNPC()->pIndexData->vnum, 
			    wch->getRace()->getName().c_str(),
			    wch->getNameP('1').c_str());
	}

	page_to_char(buf.str().c_str(), ch);
	return;
    }

    if (arg == "petshops") {
	for (Room *r = room_list; r; r = r->rnext) {
	    if (r->behavior 
		&& (r->behavior->getType( ) == "ClanPetShopStorage"
		    || r->behavior->getType( ) == "PetShopStorage")) 
	    {
		buf << "[" << r->vnum << "] " << r->name << endl;
		for(RESET_DATA *pReset = r->reset_first;pReset;pReset = pReset->next)
		    switch(pReset->command) {
		    case 'M':
			pReset->arg2 = -1;
			SET_BIT(r->area->area_flag, AREA_CHANGED);
			buf << "    [" << pReset->arg1 << "] " << russian_case(get_mob_index(pReset->arg1)->short_descr, '1') << endl;
			break;
		    }
	    }
	}

	page_to_char(buf.str().c_str(), ch);
	return;
    }

    if (arg == "searcher") {
        buf << "vnum,name,level,wearloc,itemtype,hr,dr,hp,mana,move,saves,armor,str,int,wis,dex,con,cha,size,affects,area,where,limit" << endl;

	for (int i = 0; i < MAX_KEY_HASH; i++)
	for (OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) {
	    bitstring_t wear = pObj->wear_flags;
	    REMOVE_BIT(wear, ITEM_TAKE|ITEM_NO_SAC);
            DLString wearloc;
            
            // Limit obj level by 100.
            if (pObj->level > LEVEL_MORTAL)
                continue;

            // Ignore items you can't even take.
            if (!IS_SET(pObj->wear_flags, ITEM_TAKE))
                continue;
            // Ignore items without wear flags.
	    if (wear == 0 && pObj->item_type != ITEM_LIGHT)
		continue;
            // Ignore weapons, they're shown in another table.
            if (pObj->item_type == ITEM_WEAPON) 
                continue;

            // Quirk with light wearlocation.
            if (wear == 0) 
                wearloc = "light";
            else
                wearloc = wear_flags.messages(wear);

            // Format object item type and damage roll.
            DLString itemtype = item_table.message( pObj->item_type );
            
            // Format object name.
	    DLString name = russian_case(pObj->short_descr, '1').toLower( );
            csv_escape( name );
        
            // Find all bonuses.
	    int hr=0, dr=0, hp=0, svs=0, mana=0, move=0, ac=0;
	    int str=0, inta=0, wis=0, dex=0, con=0, cha=0, size=0;
	    DLString aff,det,imm,res,vuln;

	    for (Affect *paf = pObj->affected; paf; paf = paf->next) {
		int m = paf->modifier;

		switch (paf->location) {
		case APPLY_STR: str+=m; break;
		case APPLY_INT: inta+=m; break;
		case APPLY_WIS: wis+=m; break;
		case APPLY_DEX: dex+=m; break;
		case APPLY_CON: con+=m; break;
		case APPLY_CHA: cha+=m; break;
		case APPLY_HIT: hp+=m; break;
		case APPLY_MANA: mana+=m; break;
		case APPLY_MOVE: move+=m; break;
		case APPLY_AC: ac+=m; break;
		case APPLY_HITROLL: hr+=m; break;
		case APPLY_DAMROLL: dr+=m; break;
		case APPLY_SIZE: size+=m; break;
		case APPLY_SAVES:         
		case APPLY_SAVING_ROD:    
		case APPLY_SAVING_PETRI:  
		case APPLY_SAVING_BREATH: 
		case APPLY_SAVING_SPELL:  svs+=m; break;
		}
		
		if (paf->bitvector) {
		    bitstring_t b = paf->bitvector;

		    switch(paf->where) {
		    case TO_DETECTS: det << detect_flags.names(b) << " "; break;
		    case TO_AFFECTS: aff << affect_flags.names(b) << " "; break;
		    case TO_IMMUNE:  imm << imm_flags.names(b) << " "; break;
		    case TO_RESIST:  res << res_flags.names(b) << " "; break;
		    case TO_VULN:    vuln << vuln_flags.names(b) << " "; break;
		    }
		}
	    }

            if (pObj->item_type == ITEM_ARMOR) {
                ac -= pObj->value[0];
            }


            
            // Potions, containers etc often can be held but do nothing - ignore them.
            bool useless = false;
            if (wear == ITEM_HOLD) {
                useless = str == 0 && inta == 0 && wis == 0 && dex == 0 && con == 0 && cha == 0
                    && hp == 0 && mana == 0 && move == 0 && ac == 0 && hr == 0 && dr == 0 && size == 0
                    && svs == 0;
                // TODO check affects
            }

            if (useless)
                continue;

            // Find item resets and ignore items without resets and from clan areas.
            DLString where;
            AREA_DATA *pArea;
            useless = !get_obj_resets( pObj->vnum, pArea, where );
            if (useless)
                continue;
            static const DLString CLAN_AREA_TYPE = DLString("ClanArea");
            if (pArea->behavior && CLAN_AREA_TYPE.strPrefix( pArea->behavior->getType( ) ))
                continue;
            if (IS_SET(pArea->area_flag, AREA_WIZLOCK|AREA_HIDDEN))
                continue;
            DLString area = pArea->name;
            csv_escape( area );
            csv_escape( where );
                
            // TODO show affects
	    buf << pObj->vnum << ","
		<< "\"" << name << "\","
		<< pObj->level << ","
		<< "\"" << wearloc << "\","
		<< "\"" << itemtype << "\","
	        << hr << "," << dr << "," << hp << "," 
                << mana << "," << move << "," 
                << svs << "," << ac << ","
	        << str << "," << inta << "," << wis << ","
	        << dex << "," << con << "," << cha << ","
	        << size << "," << "\"\"," 
                << "\"" << area << "\","  << "\"" << where << "\","  
                << pObj->limit << endl;
	}

	try {
	    DLFileStream( "db_armor.csv" ).fromString( buf.str( ) );
	} catch (const ExceptionDBIO &ex) {
	    ch->println( ex.what( ) );
	    return;
	}
	
	ch->println("Armor CSV file dumped.");
	return;
    }

    if (arg == "searcherMagic" || arg == "sm") {
        buf << "vnum,name,level,itemtype,spellLevel,charges,spells,area,where,limit" << endl;
        // Collect distinct list of spells.
        std::set<DLString> allSpells;

	for (int i = 0; i < MAX_KEY_HASH; i++)
	for (OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) {
            // Limit obj level by 100.
            if (pObj->level > LEVEL_MORTAL)
                continue;

            // Ignore weird items.
            if (!IS_SET(pObj->wear_flags, ITEM_TAKE))
                continue;
             
            DLString itemtype = item_table.name( pObj->item_type );
            int spellLevel = 0;
            int charges = 0;
            DLString spells = "";
            Skill *skill;
            // Only dump magic items; collect properties.
            switch (pObj->item_type) {
            case ITEM_SCROLL:
            case ITEM_POTION:
            case ITEM_PILL:
                spellLevel = pObj->value[0];
                charges = 1;

                for (int i = 1; i <= 4; i++) 
                    if ((skill = SkillManager::getThis( )->find( pObj->value[i] ) ))
                        if (skill->getIndex( ) != gsn_none) {
                            spells += "'" + skill->getName( ) + "' ";
                            allSpells.insert( skill->getName( ) );
                        }
                
                break;

            case ITEM_WAND:
            case ITEM_STAFF:
                spellLevel = pObj->value[0];
                charges = pObj->value[2];
                
                if ((skill = SkillManager::getThis( )->find( pObj->value[3] ) ))
                    if (skill->getIndex( ) != gsn_none) {
                        spells = "'" + skill->getName( ) + "'";
                        allSpells.insert( skill->getName( ) );
                    }

                break;

            case ITEM_WARP_STONE:
                // Warpstones don't have any fields (although might consider 
                // introducing 'charges' field).
                break;

            case ITEM_SPELLBOOK:
                // For spellbooks, display max quality as spell level for now.
                // Show 'total pages' as 'charges' field.
                spellLevel = pObj->value[2];
                charges = pObj->value[0];
                break;

            default:
                continue;
            }

            // Ignore useless stuff. Leave it commented for debugging.
//            if (spells.empty( ) || charges == 0)
//                continue;

            // Format object name.
	    DLString name = russian_case(pObj->short_descr, '1').toLower( );
            csv_escape( name );

            // Find item resets and ignore items without resets and from clan areas.
            DLString where;
            AREA_DATA *pArea;
            bool useless = !get_obj_resets( pObj->vnum, pArea, where );
            if (useless)
                continue;
            static const DLString CLAN_AREA_TYPE = DLString("ClanArea");
            if (pArea->behavior && CLAN_AREA_TYPE.strPrefix( pArea->behavior->getType( ) ))
                continue;
            if (IS_SET(pArea->area_flag, AREA_WIZLOCK|AREA_HIDDEN))
                continue;
            DLString area = pArea->name;
            csv_escape( area );
            csv_escape( where );
                
            // Header; "vnum,name,level,type,spellLevel,charges,spells,area,where" 
	    buf << pObj->vnum << ","
		<< "\"" << name << "\","
		<< pObj->level << ","
		<< "\"" << itemtype << "\","
                << spellLevel << "," 
                << charges << "," 
		<< "\"" << spells << "\","
                << "\"" << area << "\","  << "\"" << where << "\","
                << pObj->limit << endl;
	}

	try {
	    DLFileStream( "db_magic.csv" ).fromString( buf.str( ) );
	} catch (const ExceptionDBIO &ex) {
	    ch->println( ex.what( ) );
	    return;
	}
	
	ch->println("Magic CSV file dumped.");
        for (std::set<DLString>::iterator s = allSpells.begin( ); s != allSpells.end( ); s++)
            ch->println( *s );

        ch->printf("Found %d unique spells.\r\n", allSpells.size( ));
	return;
    }
    
    if (arg == "searcherWeapons" || arg == "sw") {
        buf << "vnum,name,level,wclass,special,d1,d2,ave,hr,dr,hp,mana,saves,armor,str,int,wis,dex,con,area,where,limit" << endl;

	for (int i = 0; i < MAX_KEY_HASH; i++)
	for (OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) {
	    bitstring_t wear = pObj->wear_flags;
	    REMOVE_BIT(wear, ITEM_TAKE|ITEM_NO_SAC);
            DLString wearloc;
            
            // Limit obj level by 100.
            if (pObj->level > LEVEL_MORTAL)
                continue;

            // Ignore items you can't even take.
            if (!IS_SET(pObj->wear_flags, ITEM_TAKE))
                continue;
            // Only dump weapons. An arrow can be 'held' so can't ignore non-wieldable ones.
	    if (wear == 0 || pObj->item_type != ITEM_WEAPON)
		continue;

            // Format weapon class, special flags and damage.
            DLString weaponClass = weapon_class.name( pObj->value[0] );
            DLString special = weapon_type2.messages( pObj->value[4] );
            int d1 = pObj->value[1];
            int d2 = pObj->value[2];
            int ave = (1 + pObj->value[2]) * pObj->value[1] / 2; 
            
            // Format object name.
	    DLString name = russian_case(pObj->short_descr, '1').toLower( );
            csv_escape( name );
        
            // Find all bonuses.
	    int hr=0, dr=0, hp=0, svs=0, mana=0, move=0, ac=0;
	    int str=0, inta=0, wis=0, dex=0, con=0, cha=0, size=0;

	    for (Affect *paf = pObj->affected; paf; paf = paf->next) {
		int m = paf->modifier;

		switch (paf->location) {
		case APPLY_STR: str+=m; break;
		case APPLY_INT: inta+=m; break;
		case APPLY_WIS: wis+=m; break;
		case APPLY_DEX: dex+=m; break;
		case APPLY_CON: con+=m; break;
		case APPLY_CHA: cha+=m; break;
		case APPLY_HIT: hp+=m; break;
		case APPLY_MANA: mana+=m; break;
		case APPLY_MOVE: move+=m; break;
		case APPLY_AC: ac+=m; break;
		case APPLY_HITROLL: hr+=m; break;
		case APPLY_DAMROLL: dr+=m; break;
		case APPLY_SIZE: size+=m; break;
		case APPLY_SAVES:         
		case APPLY_SAVING_ROD:    
		case APPLY_SAVING_PETRI:  
		case APPLY_SAVING_BREATH: 
		case APPLY_SAVING_SPELL:  svs+=m; break;
		}
	    }
            
            // Find item resets and ignore items without resets and from clan areas.
            DLString where;
            AREA_DATA *pArea;
            bool useless = !get_obj_resets( pObj->vnum, pArea, where );
            if (useless)
                continue;
            static const DLString CLAN_AREA_TYPE = DLString("ClanArea");
            if (pArea->behavior && CLAN_AREA_TYPE.strPrefix( pArea->behavior->getType( ) ))
                continue;
            if (IS_SET(pArea->area_flag, AREA_WIZLOCK|AREA_HIDDEN))
                continue;
            DLString area = pArea->name;
            csv_escape( area );
            csv_escape( where );
                
            // Header: "vnum,name,level,wclass,special,d1,d2,ave,hr,dr,hp,mana,saves,ac,str,int,wis,dex,con,area,where"
	    buf << pObj->vnum << ","
		<< "\"" << name << "\","
		<< pObj->level << ","
		<< "\"" << weaponClass << "\","
		<< "\"" << special << "\","
                << d1 << "," 
                << d2 << "," 
                << ave << "," 
	        << hr << "," << dr << "," 
                << hp << "," << mana << "," 
                << svs << "," << ac << ","
	        << str << "," << inta << "," << wis << ","
	        << dex << "," << con << "," 
                << "\"" << area << "\","  << "\"" << where << "\","
                << pObj->limit << endl;
	}

	try {
	    DLFileStream( "db_weapon.csv" ).fromString( buf.str( ) );
	} catch (const ExceptionDBIO &ex) {
	    ch->println( ex.what( ) );
	    return;
	}
	
	ch->println("Weapon CSV file dumped.");
	return;
    }

    if (arg == "linkwrapper") {
	DLString vnumStr = args.getOneArgument();
	if (vnumStr.isNumber()) {
	    Room *r = get_room_index(vnumStr.toInt());
	    if (r) {
		if (FeniaManager::wrapperManager)
		    FeniaManager::wrapperManager->linkWrapper(r);
		ch->println("Room wrapped.");
	    }
	    else 
		ch->println("Room not found.");
	}
	else
	    ch->println("Usage: abc linkwrapper <room vnum>");

    }

    if (arg == "objdup") {
	typedef list<Object *> ObjectList;
	ObjectList::iterator o;
	map<long long, ObjectList> ids;
	map<long long, ObjectList>::iterator i;

	for (Object *obj = object_list; obj; obj = obj->next) {
	    ids[obj->getID( )].push_back( obj );
	}

	for (i = ids.begin( ); i != ids.end( ); i++)
	    if (i->second.size( ) > 1)
		for (o = i->second.begin( ); o != i->second.end( ); o++)
		    buf << "[" << i->first << "] " 
			<< (*o)->getShortDescr( '1' ) 
			<< " [" << (*o)->getRoom( )->vnum << "]" << endl;
	    
	page_to_char( buf.str( ).c_str( ), ch );
	return;
    }


    if (arg == "mobdup") {
	typedef list<NPCharacter *> MobileList;
	MobileList::iterator m;
	map<long long, MobileList> ids;
	map<long long, MobileList>::iterator i;

	for (Character *wch = char_list; wch; wch = wch->next) {
	    if (wch->is_npc( ))
		ids[wch->getID( )].push_back( wch->getNPC( ) );
	}

	for (i = ids.begin( ); i != ids.end( ); i++)
	    if (i->second.size( ) > 1)
		for (m = i->second.begin( ); m != i->second.end( ); m++)
		    buf << "[" << i->first << "] " 
			<< (*m)->getNameP( '1' ) 
			<< " [" << (*m)->in_room->vnum << "]" << endl;
	    
	page_to_char( buf.str( ).c_str( ), ch );
	return;
    }

    if (arg == "objname") {
        int cnt = 0, hcnt = 0, rcnt = 0;
        ostringstream buf, hbuf, rbuf;

	for (int i = 0; i < MAX_KEY_HASH; i++)
	for (OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) {
            DLString longd = pObj->description;
            longd.colourstrip( );

            static RegExp pattern_rus("[�-�]");
            static RegExp pattern_longd("^.*\\(([-a-z ]+)\\).*$");
            
            if (!pattern_rus.match( longd )) 
                continue;

            if (IS_SET(pObj->area->area_flag, AREA_NOQUEST|AREA_WIZLOCK|AREA_HIDDEN))
                continue;
            
            {
                DLString names = DLString( pObj->name );
                RussianString rshortd(pObj->short_descr, pObj->gram_gender );
                DLString shortd = rshortd.decline( '7' ).colourStrip( );
                if (!arg_contains_someof( shortd, pObj->name )) {
                    rcnt ++;
                    rbuf << pObj->vnum << ": [" << rshortd.decline( '1' ) << "] [" << pObj->name << "]" <<  endl;
                }
            }
            if (!pattern_longd.match( longd )) {
                buf << pObj->vnum << ": [" << longd << "] [" << pObj->name << "]" <<  endl;
                cnt++;
            } else {
                RegExp::MatchVector matches = pattern_longd.subexpr( longd.c_str( ) );
                if (matches.size( ) < 1) {
                    buf << pObj->vnum << ": [" << longd << "] [" << pObj->short_descr << "]" <<  endl;
                    cnt++;
                } else {
                    
                    DLString hint = matches.front( );
                    if (!is_name( hint.c_str( ), pObj->name )) {
                        hbuf << dlprintf( "%6d : [%35s] hint [{G%10s{x] [{W%s{x]\r\n",
                                pObj->vnum, longd.c_str( ), hint.c_str( ), pObj->name );
                        hcnt++;
                    }
                }
            }
        }

//        ch->printf("������� %d ������� ���� ��������� ��� ��������� (������).\r\n", cnt);
//	page_to_char( buf.str( ).c_str( ), ch );
//        ch->printf("������� %d �������������� ��������� � ������� ����� ���������.\r\n", hcnt);
//	page_to_char( hbuf.str( ).c_str( ), ch );
        ch->printf("������� %d ��������� ��� � short descr �� ����������� �����.\r\n", rcnt);
	page_to_char( rbuf.str( ).c_str( ), ch );
        return;
    }

    if (arg == "rlim") {
        
        if (args.empty( )) {
            for (Object *obj = object_list; obj; obj = obj->next) {
                    if (obj->pIndexData->limit < 0)
                        continue;
                    if (obj->in_room == NULL)
                        continue;
                    if (obj->pIndexData->area == obj->in_room->area)
                        continue;
                    if (obj->timestamp > 0)
                        continue;
                    
                    ch->printf("Found %s [%d] in [%d] %s\r\n", 
                            obj->getShortDescr('1').c_str( ), obj->pIndexData->vnum,
                            obj->in_room->vnum, obj->in_room->area->name);
            }

            return;
        }

        if (!is_number( args.c_str( )))
            return;

        Room *r = get_room_index( atoi( args.c_str( ) ) );
        if (!r) {
            ch->println("Room vnum not found.");
            return;
        }

        for (Object *obj = r->contents; obj; obj = obj->next_content) {
                if (obj->pIndexData->limit < 0)
                    continue;
                if (obj->in_room == NULL)
                    continue;
                if (obj->pIndexData->area == obj->in_room->area)
                    continue;
                if (obj->timestamp > 0)
                    continue;

                obj->timestamp = 1531034011;
                save_items( obj->in_room );
                ch->printf("Marking %s [%d] in [%d] %s\r\n", 
                        obj->getShortDescr('1').c_str( ), obj->pIndexData->vnum,
                            obj->in_room->vnum, obj->in_room->area->name);
        }
        ch->println("Done marking limits.");
        return;
    }

    if (arg == "mobname") {
        int cnt = 0, hcnt = 0, rcnt = 0;
        ostringstream buf, hbuf, rbuf;

	for (int i = 0; i < MAX_KEY_HASH; i++)
	for (MOB_INDEX_DATA *pMob = mob_index_hash[i]; pMob; pMob = pMob->next) {
            DLString names = DLString(pMob->player_name).toLower();
            DLString longd = trim(pMob->long_descr).toLower();
            longd.colourstrip( );

            static RegExp pattern_rus("[�-�]");
            static RegExp pattern_longd("^.*\\(([-a-z ]+)\\).*$");
            
            if (!pattern_rus.match( longd )) 
                continue;

            if (IS_SET(pMob->area->area_flag, AREA_WIZLOCK|AREA_HIDDEN))
                continue;
            
            {
                RussianString rshortd(pMob->short_descr, MultiGender(pMob->sex, pMob->gram_number));
                DLString shortd = rshortd.decline( '7' ).colourStrip( ).toLower();
                if (!arg_contains_someof( shortd, pMob->player_name )) {
                    rcnt ++;
                    rbuf << pMob->vnum << ": [" << rshortd.decline( '1' ) << "] [" << pMob->player_name << "]" <<  endl;
                }
            }

	    RegExp::MatchVector matches = pattern_longd.subexpr( longd.c_str( ) );
	    if (matches.size( ) < 1) {
		buf << pMob->vnum << ": [" << longd << "] [" << pMob->short_descr << "]" <<  endl;
		cnt++;
	    } else {
		
		DLString hint = matches.front( );
		if (!is_name(hint.c_str(), names.c_str())) {
		    hbuf << dlprintf( "%6d : [%35s] hint [{G%10s{x] [{W%s{x]\r\n",
			    pMob->vnum, longd.c_str( ), hint.c_str( ), pMob->player_name );
		    hcnt++;
		}
	    }
        }

        ch->printf("\r\n{R������� %d �������������� ��������� � ������� ����� ����.{x\r\n", hcnt);
	page_to_char( hbuf.str( ).c_str( ), ch );
        return;
    }

}


