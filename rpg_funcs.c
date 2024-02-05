#include <linux/errno.h>
#include <linux/rpg_funcs.h>
#include <asm/current.h>



#define ERROR -1
#define SUCCESS 0
#define LOSE 0
#define WIN 1
#define CREATURE_ORC 0
#define CREATURE_DEMON 1
#define MAGE 1
#define FIGHTER 0
#define NOT_A_MEMBER 0
#define MEMBER 1



/* user POV , need to implement wrapper function */

int sys_rpg_create_character(int cclass){
	//check if process has a character
	struct task_struct *current_task = current;
    struct player *character;
	if(current_task->player){
		//process already has a character
		errno = EEXIST;
		return ERROR;
	}
	else{
		//player does not have a character, create one
		character = kmalloc(sizeof(struct player), GFP_KERNEL);
		if(!character){
			// allocation failed
			errno = -ENOMEM;
			return ERROR;
		}
		character->player_level = 1;
		character->party_member = NOT_A_MEMBER;
		if(cclass == MAGE){
			//mage charecter
			charecter->cclass = MAGE;
			
		}else if(cclass == FIGHTER){
			//fighter charecter
			character->cclass = FIGHTER;
		}else{
			kfree(character);
			errno = EINVAL;
			return ERROR;
		}
		//initialize the list head
		LIST_HEAD_INIT(&character->my_list);
		//add character to its own list
		list_add_tail(&character->my_list, &character->my_list);
		pcurrent_task->player = character;
		return SUCCESS; 
		
	}
	
}

/******************************************************************************************************/

int sys_rpg_fight(int type , int level){
	// check if arguments are valid
	if( level < 0 || !(type == CREATURE_ORC || type == CREATURE_DEMON)){
		errno = EINVAL;
		return ERROR;
	}
	//check if process has a character
	struct task_struct *p = current;
	if(p->ptr){
		//process already has a character
		errno = EINVAL;
		return ERROR;
	}
	
    
	struct rpg_stats * my_party;
	my_party = malloc(sizeof(struct rpg_stat));
	if( my_party =! NULL){
		// allocation failed
		errno = -ENOMEM;
		return ERROR;
		
	}
	if(rpg_get_stats(my_party) == -1){
		free(my_party);
		return ERROR;
	}
		
	int strength = calc_strength(type,my_party->fighter_levels,my_party->mage_levels);
	if(strength >= level){
		//party wins
		struct player *entry;
		list_for_each_entry(entry,&(p->ptr->my_list),p->ptr->my_list){
			(entry->player_level)++;
		}
		free(my_party);
		return WIN;
			
			
	}
	else{
		//party lost
		struct player *entry;
		list_for_each_entry(entry,&(p->ptr->my_list),p->ptr->my_list){
			(entry->player_level)--;
			if(entry->player_level < 0){
				entry->player_level = 0;
					
			}
		}
		free(my_party);
		return LOSE;
			
	}
	
	
}


int calc_strength(int type, int fighter_level,int mage_level){
	if(type == CREATURE_ORC){
		int strength = 2*(fighter_level) + mage_level;
		
	}
	if(type == CREATURE_DEMON){
		int strength = fighter_level + 2*(mage_level);
	}
	return strength;
}
/******************************************************************************************************/

int sys_rpg_get_stats(struct rpg_stats* stats){
	if(stats == NULL){
		errno = EINVAL;
		return ERROR;
	}

	struct task_struct *p = current;
	if(p->ptr == NULL){
		//process does not havs a character
		errno = EINVAL;
		return ERROR;
	}
	//checking if a user-space pointer is valid
	if (access_ok(stats, sizeof(struct rpg_stats), VERIFY_WRITE) == 0) {
		errno = EFAULT;  // Bad address
		return ERROR;
	}
	
	struct player *entry;
	stats->cclass = p->ptr->cclass;
	stats->level = p->ptr->level;
	stats->party_size = 0;
	stats->fighter_levels = 0;
	stats->mage_levels = 0;
	
	
	list_for_each_entry(entry,&(p->ptr->my_list),p->ptr->my_list){
		(stats->party_size)++;
		if(entry->cclass == MAGE ){
			stats->mage_levels += entry->player_level;
		}
		if( entry->cclass == FIGHTER){
			stats->fighter_levels += entry->player_level;
		}		
		
	}
	return SUCCESS;
	
}

/******************************************************************************************************/

int sys_rpg_join(pid_t player){
	struct task_struct *p = current;
	struct task_struct *player_task;
	player_task = find_task_by_pid(player); 
	if(!player_task){
		//player doesn't exist
		errno = ESRCH;
		return ERROR;
	}
	if(p->ptr == NULL || player_task->ptr == NULL){
		//process or player does not havs a character
		errno = EINVAL;
		return ERROR;
	}
	
	//add process to player list
	list_del(&p->ptr->my_list);
	list_add_tail(&p->ptr->my_list, &player_task->ptr->my_list);
	p->ptr->party_member = MEMBER;
	player_task->ptr->party_member = MEMBER;
	
	return SUCCESS;
	
}















