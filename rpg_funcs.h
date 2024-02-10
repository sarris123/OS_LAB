
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
void init_pid_list(void);
int add_to_list(pid_t pid);
int pid_in_list(pid_t pid);
void remove_from_list(pid_t pid);



/* create player struct*/
struct player {
	pid_t player_pid;
	int player_level;
	int cclass;
	list_t my_list;
};

struct rpg_stats{
	int cclass;
	int level;
	int party_size;
	int fighter_levels;
	int mage_levels;
};



