
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



/* check if a proccess has created a character
	returns 1 if it has , 0 otherwise */
int has_character(struct task_struct *pros){
	if(list_empty(&(pros->party_list))){
		if(pros->party_member){
			return 1;
		}else{
			return 0;
		}
	}else{
		return 1;
	}
}

/* user POV , need to implement wrapper function */

int sys_rpg_create_character(int cclass){
	
	//check if process has a character
	struct task_struct *current_task = current;
	if(has_character(current_task)){
		//process already has a character
		//errno = EEXIST;
		return -EEXIST;
	}
	else{
		//player does not have a character, create one
		struct player *character = (struct player *)kmalloc(sizeof(struct player), GFP_KERNEL);
		if(character == NULL){
			//allocation failed
			//errno = -ENOMEM;
			return -ENOMEM;
		}
		character->player_level = 1;
		pid_t my_id = current_task->pid;
		character->player_pid = my_id;
		if(cclass == MAGE){
			//mage charecter
			character->cclass = MAGE;	
		}else if(cclass == FIGHTER){
			//fighter charecter
			character->cclass = FIGHTER;
		}else{
			kfree(character);
			//errno = EINVAL;
			return -EINVAL;
		}
		//player is not a part of a party, he is his own group leader
		current_task -> group_leader = current_task;
		//add player to his party list
		list_add_tail(&character->my_list, &(current_task->party_list));
		return SUCCESS; 
	}
	
}

/******************************************************************************************************/

int sys_rpg_fight(int type , int level){
	struct task_struct *current_task = current;
	// check if arguments are valid
	if( level < 0 ){
		//errno = EINVAL;
		return -EINVAL;
	}
	if(!(type == CREATURE_ORC)){
		//errno = EINVAL;
		return -EINVAL;
	}
	if(!(type == CREATURE_DEMON)){
		//errno = EINVAL;
		return -EINVAL;
	}
	//check if process has a character
	if(!has_character(current_task)){
		//player does not have a character
		//errno = EINVAL;
		return -EINVAL;	
	}

	//getting the party info
	struct task_struct* leader = current -> group_leader;
	if(!leader){
		//pointer is NULL
		return ERROR;
	}
	int strength = calc_strength(type,leader);
	if(strength >= level){
		//party wins
		struct player *entry;
		struct list_head* tmp;
		struct list_head* position;
		list_for_each_safe (position, tmp, &(leader->party_list)){
			entry = list_entry (position, struct player, my_list);
			(entry->player_level)++;
		}
		return WIN;		
	}
	else{
		//party lost
		struct player *entry;
		struct list_head* tmp;
		struct list_head* position;
		list_for_each_safe (position, tmp, &(leader->party_list)){
			entry = list_entry (position, struct player, my_list);
			(entry->player_level)--;
			if(entry->player_level < 0){
				entry->player_level = 0;
					
			}
		}
		return LOSE;
	}
			
}
	



int calc_strength(int type, struct task_struct * leader){
	int strength = 0;
	if(type == CREATURE_ORC){
		struct player *entry;
		struct list_head* tmp;
		struct list_head* position;
		list_for_each_safe(position,tmp,&(leader->party_list)){
			entry = list_entry (position, struct player, my_list);
			if(entry->cclass == FIGHTER ){
				strength += 2*(entry->player_level);
			}else if(entry->cclass == MAGE){
				strength += (entry->player_level);
			}
		}
	}
	if(type == CREATURE_DEMON){
		struct player *entry;
		struct list_head* tmp;
		struct list_head* position;
		list_for_each_safe(position,tmp,&(leader->party_list)){
			entry = list_entry (position, struct player, my_list);
			if(entry->cclass == FIGHTER ){
				strength += (entry->player_level);
			}else if(entry->cclass == MAGE){
				strength += 2*(entry->player_level);
			}
		}
	}
	return strength;
}
/******************************************************************************************************/

int sys_rpg_get_stats(struct rpg_stats* stats){
	//checking if stats is a NULL
	if(stats == NULL){
		//errno = EINVAL;
		return -EINVAL;
	}

	struct task_struct *current_task = current;
	//check if proccess has a character
	if(!has_character(current_task)){
		//player does not have a character
		//errno = EINVAL;
		return -EINVAL;	
	}
	struct rpg_stats* my_stats = (struct rpg_stats*)kmalloc(sizeof(struct rpg_stats), GFP_KERNEL);
	if(my_stats == NULL){
		//allocation failed
		//errno = -ENOMEM;
		return -ENOMEM;
	}
	//getting the party info
	pid_t my_id = current_task->pid;
	struct task_struct* leader = current -> group_leader;
	int cclass = get_cclass(leader,my_id); 
	int player_level = get_player_level(leader,my_id);
	int size = 0;
	int fighter_levels = 0;
	int mage_levels = 0;
	struct player *entry;
	struct list_head* tmp;
	struct list_head* position;
	list_for_each_safe(position,tmp,&(leader->party_list)){
		entry = list_entry (position, struct player, my_list);
		size++;
		if(entry->cclass == FIGHTER){
			fighter_levels += entry->player_level;
		}
		else if(entry->cclass == MAGE){
			mage_levels += entry->player_level;
		}
	}
	//filling the party info
	my_stats->cclass = cclass;
	my_stats->level = player_level;
	my_stats->party_size = size;
	my_stats->fighter_levels = fighter_levels;
	my_stats->mage_levels = mage_levels;

	//sending info back to user 
	if(copy_to_user(&stats,&my_stats,sizeof(struct rpg_stats))){
		//errno = EFAULT;  // Bad address
		kfree(my_stats);
		return -EFAULT;
	}
	kfree(my_stats);
	return SUCCESS;
}

int get_cclass(struct task_struct* leader,pid_t pid){
	struct player *entry;
	struct list_head* tmp;
	struct list_head* position;
    list_for_each_safe(position,tmp, &(leader->party_list)) {
		entry = list_entry (position, struct player, my_list);
        if (entry->player_pid == pid)
            return (entry->cclass);
    }
	return ERROR;
}

int get_player_level(struct task_struct* leader,pid_t pid){
	struct player *entry;
	struct list_head* tmp;
	struct list_head* position;
    list_for_each_safe(position,tmp, &(leader->party_list)) {
		entry = list_entry (position, struct player, my_list);
        if (entry->player_pid == pid)
            return (entry->player_level);
    }
	return ERROR;
}

/******************************************************************************************************/

int sys_rpg_join(pid_t player){
	struct task_struct *current_task = current;
	struct task_struct *player_task;
	player_task = find_task_by_pid(player); 
	if(!player_task){
		//player doesn't exist
		//errno = ESRCH;
		return -ESRCH;
	}
	if(!has_character(current_task) || !has_character(player_task)){
		//process or player does not havs a character
		//errno = EINVAL;
		return -EINVAL;
	}
	if(current_task->party_member == NOT_A_MEMBER){
		//proccess in not a party member
		struct player* tmp;
		//obtain the first node, proccess node
		tmp = list_entry(current_task->party_list.next, struct player, my_list);
		//remove proccess node from his list
		list_del(&tmp->my_list);
		//add node to the player list
		struct task_struct* leader = player_task->group_leader;
		if(leader == NULL){
			return ERROR;
		}
		list_add_tail(&tmp->my_list, &(leader->party_list));
		current_task->group_leader = leader;
		current_task->party_member = MEMBER;
		player_task->party_member = MEMBER;
	}else{
		//proccess is a member of a party. check if proccess is a leader
		if(current_task->group_leader == current_task){
			//assign a different player as leader, moving the party to his party_list head.
			moving_list(current_task,player_task);
			player_task->party_member = MEMBER;

		}else{
			//proccess is not a leader
			pid_t my_id = current_task->pid;
			move_my_node(my_id,current_task->group_leader,player_task->group_leader);
			current_task->group_leader = player_task->group_leader;
			player_task->party_member = MEMBER;

		}
	}
	
	return SUCCESS;
	
}


void moving_list(struct task_struct* source,struct task_struct* new){
	pid_t source_id = source->pid;
	pid_t dest_id;
	struct player *entry;
	struct list_head* tmp;
	struct list_head* position;
	//find a new leader
    list_for_each_safe(position,tmp, &(source->party_list)){
		entry = list_entry (position, struct player, my_list);
		if(entry->player_pid != source_id){
			dest_id = entry->player_pid;
			break;
		}
	}
	struct task_struct* dest = find_task_by_pid(dest_id);
	//move list to new leader head
	list_splice(&source->party_list, &dest->party_list);
	//delete source player node from dest list, add to new list and update leader
	struct player *entry1;
	struct list_head* tmp1;
	struct list_head* position1;
	list_for_each_safe(position1,tmp1, &(dest->party_list)){
		entry1= list_entry (position, struct player, my_list);
		if(entry1->player_pid == source_id){
			list_del(&entry1->my_list);
			list_add_tail(&entry1->my_list, &new->group_leader->party_list);
			source->group_leader = new->group_leader;
			break;
		}
	}
	//update leader in dest nodes
	update_leader(dest);

}

void update_leader(struct task_struct* dest){
	struct player *entry;
	struct list_head* tmp;
	struct list_head* position;
	list_for_each_safe(position,tmp, &(dest->party_list)){
		entry= list_entry (position, struct player, my_list);
		struct task_struct* node = find_task_by_pid(entry->player_pid);
		node->group_leader = dest;
	}
}


void move_my_node(pid_t my_id,struct task_struct* old_leader,struct task_struct* new_leader){
	//locate my node in old leader list
	struct player* entry;
	struct list_head* tmp;
	struct list_head* position;
	list_for_each_safe(position,tmp, &(old_leader->party_list)){
		entry= list_entry (position, struct player, my_list);
		if(entry->player_pid == my_id){
			list_del(&entry->my_list);
			list_add_tail(&entry->my_list, &new_leader->party_list);
			break;
		}
	}

}


/**************************************************************/
int rpg_fork(struct task_struct* son){
	son->party_member = NOT_A_MEMBER;
	son->group_leader = son;
	struct list_head *cursor,*tmp;
	if (list_empty(&(son->party_list))){
		return 0;
	}
	struct player *entry;
	list_for_each_safe (cursor, tmp, &(son->party_list)){
		entry = list_entry(cursor,struct player,my_list);
		list_del(cursor);
		kfree (entry);
	}
	return 0;

}


int rpg_exit(struct task_struct* proc){
	struct list_head *cursor,*tmp;
	struct player *entry;
	if (list_empty(&(proc->party_list))){
		return 0;
	}
	list_for_each_safe (cursor, tmp, &(proc->party_list)){
		entry = list_entry(cursor,struct player,my_list);
		list_del(cursor);
		kfree (entry);
	}
	return 0;
}














