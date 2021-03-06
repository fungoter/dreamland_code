/* $Id$
 *
 * ruffina, 2004
 */
#include "directions.h"

#include "character.h"
#include "room.h"

#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

const char * extra_move_ru [] =
{
	"��%1$G��|��|��", "�������%1$G���|��|���", "��������%1$G��|�|��"
	, "������%1$G���|��|���", "�����%1$G��|�|��", "����%1$G��|�|��"
	, "�����%1$G��|�|��", "���������%1$G���|��|���", "�����%1$G��|�|��"
	, "�������%1$G��|�|��", "���%1$G���|�|���", "�������%1$G���|��|���"
};

const char * extra_move_rp [] =
{
	"����%1$G��|��|��", "������%1$G���|��|���", "��������%1$G��|�|��"
	, "���%1$G��|�|��", "�������%1$G��|�|��", "������%1$G��|�|��"
	, "�����%1$G��|�|��", "���������%1$G���|��|���", "�������%1$G��|�|��"
	, "�������%1$G��|�|��", "���%1$G���|�|���", "�������%1$G���|��|���"
};

const char * extra_move_rt [] =
{
		"�", "��", "������", "�����", "���"
	, "�����", "���", "�", "��", "��", "�� ���", "��"
};

char const extra_move_rtum [] =
{
	'4', '4', '4', '5', '5', '4', '5', '2', '2', '2', '2', '2', '2'
};

char const extra_move_rtpm [] =
{
	'2', '4', '4', '5', '5', '4', '5', '2', '2', '2', '2', '2', '2'
};

const struct direction_t dirs [] = {
    { DIR_NORTH, DIR_SOUTH, "north", "�����",  "�� �����",  "� ������",  "�� ������"  },
    { DIR_EAST,  DIR_WEST,  "east",  "������", "�� ������", "� �������", "�� �������" },
    { DIR_SOUTH, DIR_NORTH, "south", "��",     "�� ��",     "� ���",     "�� ���"     },
    { DIR_WEST,  DIR_EAST,  "west",  "�����",  "�� �����",  "� ������",  "�� ������"  },
    { DIR_UP,    DIR_DOWN,  "up",    "�����",  "�����",     "������",    "�������"    },
    { DIR_DOWN,  DIR_UP,    "down",  "����",   "����",      "�����",     "�����"      },
};

int direction_lookup( char c )
{
    char arg[2];
    arg[0] = c;
    arg[1] = 0;
    return direction_lookup( arg );
}

int direction_lookup( const char *arg )
{
    int door;
    
    if (!arg || !*arg)
	return -1;
	
    for (door = 0; door < DIR_SOMEWHERE; door++) {
	if (arg[0] == dirs[door].name[0] || arg[0] == dirs[door].rname[0]) {
	    if (arg[1] == 0) // neswup ����
		return door;
	    else if (arg[1] == dirs[door].rname[1] && arg[2] == 0) // �� ��
		return door;
	}

	if (!str_prefix( arg, dirs[door].name ))
	    return door;

	if (!str_prefix( arg, dirs[door].rname ))
	    return door;
    }
    
    return -1;
}

int find_exit( Character *ch, const char *arg, int flags )
{
    EXIT_DATA *pexit;
    int door = direction_lookup( arg );

    if (door < 0) {
	for (int d = 0; d < DIR_SOMEWHERE; d++) {
	    pexit = ch->in_room->exit[d];

	    if (!pexit || !pexit->u1.to_room)
		continue;

	    if (!ch->can_see( pexit ) && IS_SET(flags, FEX_NO_INVIS))
		continue;

	    if (!IS_SET( pexit->exit_info, EX_ISDOOR ) && IS_SET(flags, FEX_DOOR))
		continue;

	    if (pexit->keyword && is_name( arg, pexit->keyword ))
		return d;
	}

	if (IS_SET(flags, FEX_VERBOSE))
	    act( "�� �� ������ $T �����.", ch, 0, arg, TO_CHAR );
	
	return door;
    }
    
    pexit = ch->in_room->exit[door];
    
    if ((!pexit || !pexit->u1.to_room) && IS_SET(flags, FEX_NO_EMPTY)) {
	if (IS_SET(flags, FEX_VERBOSE))
	    act( "�� �� ������ ������ �� $T ������.", ch, 0, dirs[door].name, TO_CHAR);

	return -1;
    }

    if (pexit && !ch->can_see( pexit ) && IS_SET(flags, FEX_NO_INVIS)) {
	if (IS_SET(flags, FEX_VERBOSE))
	    act( "�� �� ������ ������ �� $T ������.", ch, 0, dirs[door].name, TO_CHAR);

	return -1;
    }

    if ((!pexit || !IS_SET(pexit->exit_info, EX_ISDOOR)) && IS_SET(flags, FEX_DOOR)) {
	if (IS_SET(flags, FEX_VERBOSE))
	    act( "�� �� ������ ����� �� $T ������.", ch, 0, dirs[door].name, TO_CHAR);

	return -1;
    }

    return door;
}

EXTRA_EXIT_DATA * get_extra_exit ( const char * name,EXTRA_EXIT_DATA * list )
{
	for( ; list != 0; list = list->next )
	{
		if ( is_name( (char *) name , list->keyword ) )
			return list;
	}

	return 0;
}

