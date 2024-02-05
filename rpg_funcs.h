
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <asm-i386/uaccess.h>
#include <linux/types.h>





/* define functions*/

int calc_strength(int type, int fighter_level,int mage_level);
int sys_rpg_create_character(int cclass);
int sys_rpg_fight(int type , int level);
int sys_rpg_get_stats(struct rpg_stats* stats);
int sys_rpg_join(pid_t player);


/* create player struct*/
struct player {
	int player_level;
	int cclass;
	int party_member;
	list_t my_list;
	
};



