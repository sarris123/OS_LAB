
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
	int player_level;
	int cclass;
	int party_member;
	list_t my_list;
	
};

/* create a struct to store the pid's of the processes
   that have a characters */
struct pid_node {
    pid_t pid;
    struct list_head list;
};



