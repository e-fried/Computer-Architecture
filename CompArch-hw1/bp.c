/* 046267 Computer Architecture - Spring 2019 - HW #1 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include "math.h"
#include <stdio.h>

#define MAXROW 32
#define MAXHISTORY 8
#define MAXTAG 30


typedef enum { SNT, WNT, WT, ST }STATES;
typedef enum { FAILURE, SUCCESS } Result;
/*-------------------------------------------functions-----------------------------------------------------------------------*/

typedef struct HISTORY_ {
	int size;
	uint8_t history_taken;// data structure to keep track of history bits
}hist,*Phist;

// data structure to hold the state machines
typedef struct STATE_TABLE_{

	STATES bimodal[256]; // state Machines
	int size; //the length of the dynamic size

}state_table,*Pstate_table;

// data structure to hold a row in the BTB table
typedef struct ROW_ {
	uint32_t tag;
	int tag_size;
	uint32_t address;
	hist  history;
	state_table local_fsm_table;
}btb_row,*Pbtb_row;

// data structure to hold the BTB table and structures for global cases

typedef struct BTB_TABLE {

	unsigned btb_size;
	btb_row row[MAXROW];
	hist global_history;
	state_table global_state_table;
	unsigned tag_size;

}btb_table,*Pbtb_table;

/*function declarations*/
Result initState(int history_size, int state, Pstate_table current_table);

void initrow(int history_size, Pbtb_row row, int tag_size, int state);

int btbIndexFind(uint32_t pc);

bool ifExist(uint32_t pc, int index);

int findFsmIndex(hist history, uint32_t pc);

bool taken(hist history, state_table fsm_table, uint32_t pc);

Result fsm_update(Pstate_table fsm_table, hist history, bool taken, uint32_t  pc);

void history_update(Phist history, bool taken);

void calculateSize(unsigned btbSize, unsigned tagSize, unsigned historySize, bool isGlobalHist, bool isGlobalTable);

int findBtbCase(bool isGlobalHist, bool isGlobalTable, int Shared);

/*-----------------------------------------globalVar-----------------------------------------------------------------------*/

//global table
btb_table my_predictor;

int predictor_case;
/*
 predictor case:
case 1: Global table and global history with gshare 
case 2: Global table and global history with no share
case 3: Global table and local history with Lshare
case 4: Global Table and local history with no share
case 5: Local Table and local history with no share
case 6: local table and global history with no share

*/
int mid_or_lsb;
unsigned fsmStartState;
SIM_stats global_Stats;


/*int function*/
//function to initalize btb structures after recieving input how the structures should be built


int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
	if (findBtbCase(isGlobalHist, isGlobalTable, Shared) == -1) {
		printf("This case is not logical \n");
		return -1;
	}
	calculateSize(btbSize, tagSize, historySize, isGlobalHist, isGlobalTable);
	/*size calculation*/
	
	fsmStartState = fsmState;
	my_predictor.tag_size = tagSize;
	my_predictor.btb_size = btbSize;
	my_predictor.global_history.size = historySize;
	my_predictor.global_history.history_taken = 0;
	for (unsigned int i = 0; i < btbSize; i++)
	{
		initrow(historySize,&my_predictor.row[i],tagSize,fsmState);//stopped here
	}
	initState(historySize, fsmState, &my_predictor.global_state_table);

	
	return 0;
}

// function to predict wheter we need to take PC+4 or the junp address in the BTB table
bool BP_predict(uint32_t pc, uint32_t *dst){
	int index = btbIndexFind(pc);
	if (!ifExist(pc, index)) {
		*dst = pc + 4;
		return false;
	}
    if(my_predictor.row[index].address==0xFFFFFFFF){
        *dst = pc + 4;
        return false;
    }
	switch (predictor_case)
	{
	case 1:
		if (taken(my_predictor.global_history, my_predictor.global_state_table, pc)) {
			*dst = my_predictor.row[index].address;
			return true;
		}
		*dst = pc + 4;
		return false;

	case 2:
		if (taken(my_predictor.global_history, my_predictor.global_state_table, pc)) {
			*dst = my_predictor.row[index].address;
			return true;
		}
		*dst = pc + 4;
		return false;

	case 3:
		if (taken(my_predictor.row[index].history, my_predictor.global_state_table, pc)) {
			*dst = my_predictor.row[index].address;
			return true;
		}
		*dst = pc + 4;
		return false;
	case 4:
		if (taken(my_predictor.row[index].history, my_predictor.global_state_table, pc)) {
			*dst = my_predictor.row[index].address;
			return true;
		}
		*dst = pc + 4;
		return false;

	case 5:
		if (taken(my_predictor.row[index].history, my_predictor.row[index].local_fsm_table, pc)) {
			*dst = my_predictor.row[index].address;
			return true;
		}
		*dst = pc + 4;
		return false;
	case 6:
		if (taken(my_predictor.global_history, my_predictor.row[index].local_fsm_table, pc)) {
			*dst = my_predictor.row[index].address;
			return true;
		}
		*dst = pc + 4;
		return false;

	default:
		printf("problam in case function BP_predict");
		break;
	}
	return false;
}

// function to update the btb table based on the calculations made in EXE

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	int index = btbIndexFind(pc);
	global_Stats.br_num++;
	if ((pred_dst != (pc + 4) && !taken) || (pred_dst != targetPc && taken)) {
		global_Stats.flush_num++;
	}
	 if (ifExist(pc, index)) {
		switch (predictor_case) {
		case 1:
			fsm_update(&my_predictor.global_state_table,my_predictor.global_history, taken, pc);
			history_update(&my_predictor.global_history, taken);
			break;
		case 2:
			fsm_update(&my_predictor.global_state_table, my_predictor.global_history, taken, pc);
			history_update(&my_predictor.global_history, taken);
			break;
		case 3:
			fsm_update(&my_predictor.global_state_table, my_predictor.row[index].history, taken, pc);
			history_update( &my_predictor.row[index].history, taken);
			break;
		case 4:
			fsm_update(&my_predictor.global_state_table, my_predictor.row[index].history, taken, pc);
			history_update(&my_predictor.row[index].history, taken);
			break;
		case 5:
			fsm_update(&my_predictor.row[index].local_fsm_table, my_predictor.row[index].history, taken, pc);
			history_update(&my_predictor.row[index].history, taken);
			break;
		case 6:
			fsm_update(&my_predictor.row[index].local_fsm_table, my_predictor.global_history, taken, pc);
			history_update(&my_predictor.global_history, taken);
			break;

		}
		
	}
	else {
		 initrow(my_predictor.global_history.size, &my_predictor.row[index], my_predictor.tag_size, fsmStartState);
		 my_predictor.row[index].address = targetPc;
		 uint32_t tmp_pc = pc >> 2;
		 uint32_t mask = 0xFFFFFFFF;
		 mask = mask >> (MAXROW - my_predictor.tag_size);
		 tmp_pc = tmp_pc & mask;
		 my_predictor.row[index].tag =tmp_pc;
		 if (predictor_case == 1 || predictor_case == 2) {
			 fsm_update(&my_predictor.global_state_table, my_predictor.global_history, taken, pc);
			 history_update(&my_predictor.global_history, taken);
		 }
		 else if (predictor_case == 3 || predictor_case == 4) {
			 fsm_update(&my_predictor.global_state_table, my_predictor.row[index].history, taken, pc);
			 history_update(&my_predictor.row[index].history, taken);
		 }
		 else if (predictor_case==5){
			 fsm_update(&my_predictor.row[index].local_fsm_table, my_predictor.row[index].history, taken, pc);
			 history_update(&my_predictor.row[index].history, taken);
		 }
		 else{
			 fsm_update(&my_predictor.row[index].local_fsm_table, my_predictor.global_history, taken, pc);
			 history_update(&my_predictor.global_history, taken);
		 }

		
	}
	return;
}

// function to return the status of how good our predictor is, based on quality of prediction
void BP_GetStats(SIM_stats *curStats) {
	if (curStats == NULL) {
		return;
	}
	curStats->br_num = global_Stats.br_num;
	curStats->flush_num = global_Stats.flush_num;
	curStats->size = global_Stats.size;
	return;
}


// initalize state data structure
Result initState(int history_size, int state, Pstate_table current_table) {
	current_table->size = pow(2, history_size);
	for (int i = 0; i < current_table->size; i++) {
		current_table->bimodal[i] = state;
	}
	return SUCCESS;
}

// initalize Row data structure

void initrow(int history_size, Pbtb_row row, int tag_size, int state) {
	row->tag_size = tag_size;
	row->tag = 0xFFFFFFFF;
	initState(history_size, state, &row->local_fsm_table);
	row->history.size = history_size;
	row->history.history_taken = 0;//should be init function for history? becuse history can be in diffrent lenght
	row->address = 0xFFFFFFFF;
}

// function to decide based on input what case for predictor build is needed

int findBtbCase(bool isGlobalHist, bool isGlobalTable, int Shared) {
	if (isGlobalHist) {
		if (isGlobalTable) {
			if (Shared == 1) {//
				mid_or_lsb = 1;// this case for LSB
				predictor_case = 1; // global table and global history with gshare
				return 0;
			}
			else if (Shared == 2) {
				predictor_case = 1; // global table and global history with gshare
				mid_or_lsb = 2;// this case for mid bits
				return 0;

			}
			else {// not gshare nor lshare
				predictor_case = 2;// this case is for global table and global history with no share
				mid_or_lsb = -1;
				return 0;
			}
		}
		else {
			predictor_case = 6;// this case is for local table and global history
			mid_or_lsb = -1;
			return 0;
		}
	}
	else {
		if (isGlobalTable) {
			if (Shared == 1) {
				predictor_case = 3;// this case is for a local history table and global table with lshare
				mid_or_lsb = 1;
				return 0;
			}
			else if (Shared == 2) {
				predictor_case = 3;// this case is for a local history table and global table with lshare
				mid_or_lsb = 2;
				return 0;
			}
			else {// not gshare nor lshare
				predictor_case = 4;// this case is for a local history table and global table with no share
				mid_or_lsb = -1;
				return 0;
			}
		}
		else {
			if (Shared == 1 || Shared == 2) {
				return -1; // shared can't occur in this mode
			}
			predictor_case = 5;// this case if for Local table and local history with no share
			mid_or_lsb = -1;
			return 0;

		}
	}
	return -1;
}

// function to calculate index of PC given in the BTB table
int btbIndexFind(uint32_t pc) {
	int  logsize_of_btb;
	if (my_predictor.btb_size == 1) {
		return 0;
	}
	else{
		logsize_of_btb = log2(my_predictor.btb_size);
	}
	uint32_t pc_new = pc >> 2;
	uint32_t mask= 0xFFFFFFFF;
	mask = mask >> (MAXROW - logsize_of_btb);
	return (mask & pc_new);
	
}

// function to check wheater the pc address given equals the tag in the row pointed ny index
bool ifExist(uint32_t pc,int index) {
	pc = pc >> 2;
	uint32_t mask = 0xFFFFFFFF;
	mask = mask >> (MAXROW -my_predictor.tag_size);
	pc = pc & mask;
	if (my_predictor.row[index].tag == pc) {
		return true;
	}
	return false;
}

// function to calculate what index in the state machine fits the history for current PC

int findFsmIndex(hist history,uint32_t pc) {
	switch (mid_or_lsb)
	{
	case -1:
		return history.history_taken;
	case 2://case for mid
		pc = pc >> 16;
		uint16_t mask = 0xFFFF; 
		mask = mask >> (16 - history.size);
		pc = pc & mask;
		return history.history_taken ^ pc;
	case 1://case for lsb
		pc = pc >> 2;
		uint32_t mask1 = 0xFFFFFFFF;
		mask1 = mask1 >> (32 - history.size);
		pc = mask1 & pc;
		return (pc ^ history.history_taken);
	default:
		return -1;
	}
	return -1;
}//finding the index in the FSM

// function to calculate wheater we need to jump or not based on value in state machine, at given index
bool taken(hist history, state_table fsm_table, uint32_t pc) {
	int index = findFsmIndex(history, pc);
	if (fsm_table.bimodal[index] == WT || fsm_table.bimodal[index] == ST) {
		return true;
	}
	return false;

}

// function to update the state machine in appropriate index based on values recieved from EXE
Result fsm_update(Pstate_table fsm_table, hist history, bool taken,uint32_t  pc) {
	int index = findFsmIndex(history, pc);
	if (taken) {
		if (fsm_table->bimodal[index] == 3) {
			return true;
		}
		fsm_table->bimodal[index]++;
	}
	else {
		if (fsm_table->bimodal[index] == 0) {
			return true;
		}
		fsm_table->bimodal[index]--;
		return true;
	}
	return false;
}
// function to update history for current PC based on values recieved from EXE
void history_update(Phist history, bool taken) {
	history->history_taken = history->history_taken << 1;
	if (taken) {
		history->history_taken++;
	}
	uint8_t mask = 0xFF;
	mask = mask >>( MAXHISTORY - history->size);
	history->history_taken = (history->history_taken & mask);
}

// function to calculate memory size of the predictor
void calculateSize(unsigned btbSize, unsigned tagSize, unsigned historySize, bool isGlobalHist, bool isGlobalTable) {
	global_Stats.size = btbSize * (tagSize + 30);
	if (isGlobalHist == true) {
		global_Stats.size += historySize;
	}
	else
		global_Stats.size += btbSize * historySize;
	int tableSize = 2;
	for (unsigned int i = 0; i < historySize; i++) {
		tableSize *= 2;
	}

	if (isGlobalTable) {
		global_Stats.size += tableSize;
	}
	else
		global_Stats.size += btbSize * tableSize;

}
