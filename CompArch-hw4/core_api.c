/* 046267 Computer Architecture - Spring 2019 - HW #4 */



#include "core_api.h"
#include "sim_api.h"

#include <stdio.h>

typedef struct threadData_ {
	tcontext regs_context;
	int PC;
	unsigned int thread_id;
	int sleep_count;
	unsigned int finished;
	unsigned  int sleep_on;

}threadData, *pthreadData;


// globals
pthreadData blocked_thread_arr;
pthreadData finegrained_thread_arr;
unsigned int block_cycle_count = 0;
unsigned int block_inst_num = 0;
unsigned int finegrained_cycle_count = 0;
unsigned int finegrained_inst_num = 0;
unsigned int block_finished_thread = 0;
unsigned int finegrained_finished_thread = 0;

Status run_thread(pthreadData blocked_thread_arr, unsigned int threadId, int load_latency, int store_latency);
void updateSleep(pthreadData thread_arr, int dec_val);
Status run_thread_fineGrained(pthreadData fingrained_thread_arr, unsigned int threadId, int load_latency, int store_latency);

/*
Status Core_Context_switch_NoOverhead(int newthread){// not in pdf
	return Success;
}
 */


Status Core_blocked_Multithreading() {
	unsigned int thread_number = Get_thread_number();
	int prog_latency[2];
	Mem_latency(prog_latency);
	int store_latency = prog_latency[1];
	int load_latency = prog_latency[0];
	int switch_cycles = Get_switch_cycles();

	blocked_thread_arr = (pthreadData)malloc(thread_number * sizeof(threadData));
	if (blocked_thread_arr == NULL) {
		return Failure;
	}
	// initalize variables
     block_cycle_count = 0;
     block_inst_num = 0;
     block_finished_thread = 0;

	for (unsigned int i = 0; i < thread_number; i++) {
		blocked_thread_arr[i].PC = 0;
		blocked_thread_arr[i].thread_id = i;
		blocked_thread_arr[i].sleep_count = 0;
		blocked_thread_arr[i].finished = 0;
		blocked_thread_arr[i].sleep_on = 0;
		for (int j = 0; j < REGS; ++j) {
			blocked_thread_arr[i].regs_context.reg[j] = 0;

		}
	}


	// function run
	int not_finished = 1;
	int last_cycle_flag=0;

	while (not_finished) {

		for (unsigned int i = 0; i < thread_number; i++) {// round robin
			//function to run instructions of current thread run_thread()
			if (blocked_thread_arr[i].finished == 0) {
				if(run_thread(blocked_thread_arr, i, load_latency, store_latency)==Failure){
				   return Failure;
				}
				if (block_finished_thread < thread_number) {
				    if (( block_finished_thread==(thread_number -1)) && (last_cycle_flag == 1 || thread_number==1)){

                        //block_cycle_count += 1;
                        //updateSleep(blocked_thread_arr, 1);

				    }
				    else if(block_finished_thread== (thread_number-1 ) && last_cycle_flag==0){
				        last_cycle_flag=1;
                        block_cycle_count += switch_cycles;
                        updateSleep(blocked_thread_arr, switch_cycles);

				    }
				    else {
                        block_cycle_count += switch_cycles;
                        updateSleep(blocked_thread_arr, switch_cycles);
                    }
				}// must be in a condition



			}
			if (block_finished_thread == thread_number) {// if all threads are halted
				not_finished = 0;
				 // we coun
				break;
			}


		}

	}

	return Success;
}


Status Core_fineGrained_Multithreading() {

	int thread_number = Get_thread_number();
	int prog_latency[2];
	Mem_latency(prog_latency);
	int store_latency = prog_latency[1];
	int load_latency = prog_latency[0];

	finegrained_thread_arr = (pthreadData)malloc(thread_number * sizeof(threadData));
	if (finegrained_thread_arr == NULL) {
		return Failure;
	}
	// initalize variables


     finegrained_cycle_count = 0;
     finegrained_inst_num = 0;
     finegrained_finished_thread = 0;

	for (int i = 0; i < thread_number; i++) {
		finegrained_thread_arr[i].PC = 0;
		finegrained_thread_arr[i].thread_id = i;
		finegrained_thread_arr[i].sleep_count = 0;
		finegrained_thread_arr[i].finished = 0;
		finegrained_thread_arr[i].sleep_on = 0;
		for (int j = 0; j < REGS; ++j) {
			finegrained_thread_arr[i].regs_context.reg[j] = 0;

		}
	}


	// function run
	int not_finished = 1;

	while (not_finished) {

		for (int i = 0; i < thread_number; i++) {// round robin
			//function to run instructions of current thread run_thread()
			if (finegrained_thread_arr[i].finished == 0) {
				if(run_thread_fineGrained(finegrained_thread_arr, i, load_latency, store_latency)==Failure){
				    return Failure;
				}
			}

			//printf("thread_number: %d \n",finegrained_finished_thread);

			if (finegrained_finished_thread == thread_number) {// if all threads are halted
				not_finished = 0;
				break;
			}


		}

	}

	return Success;
}


double Core_finegrained_CPI() {

    return (double)finegrained_cycle_count / (double)finegrained_inst_num;
}
double Core_blocked_CPI() {
    //printf("\n block cycles %d instructions %d \n", block_cycle_count, block_inst_num);
	return (double)block_cycle_count / (double)block_inst_num;
}

Status Core_blocked_context(tcontext* bcontext, int threadid) {
	for (int i = 0; i < REGS; i++) {
		bcontext[threadid].reg[i] = blocked_thread_arr[threadid].regs_context.reg[i];

	}
	return Success;
}

Status Core_finegrained_context(tcontext* finegrained_context, int threadid) {
	for (int i = 0; i < REGS; i++) {
		finegrained_context[threadid].reg[i] = finegrained_thread_arr[threadid].regs_context.reg[i];

	}
	return Success;
}

Status run_thread_fineGrained(pthreadData fingrained_thread_arr, unsigned int threadId, int load_latency, int store_latency) {
	Instuction current_inst;
	if (finegrained_thread_arr[threadId].sleep_count > 0) {
		finegrained_cycle_count +=1;
		updateSleep(finegrained_thread_arr,1);
		return Success;
	}
	if (finegrained_thread_arr[threadId].sleep_on) { //commit
		finegrained_cycle_count += 1;
		finegrained_thread_arr[threadId].sleep_on = 0;
		updateSleep(finegrained_thread_arr, 1);
		return Success;

	}

	SIM_MemInstRead(finegrained_thread_arr[threadId].PC, &current_inst, threadId);
	uint32_t addr_to_store;
	uint32_t addr_to_load;
	switch (current_inst.opcode) {
		case  CMD_NOP:
			finegrained_cycle_count += 1;

			break;
		case CMD_ADD: // dst <- src1 + src2
			finegrained_cycle_count += 1;
			finegrained_thread_arr[threadId].regs_context.reg[current_inst.dst_index] =
				finegrained_thread_arr[threadId].regs_context.reg[current_inst.src1_index] +
				finegrained_thread_arr[threadId].regs_context.reg[current_inst.src2_index_imm];
			break;

		case CMD_SUB:// dst <- src1 - src2
			finegrained_cycle_count += 1;
			finegrained_thread_arr[threadId].regs_context.reg[current_inst.dst_index] =
				finegrained_thread_arr[threadId].regs_context.reg[current_inst.src1_index] -
				finegrained_thread_arr[threadId].regs_context.reg[current_inst.src2_index_imm];
			break;
		case CMD_ADDI:    // dst <- src1 + imm
			finegrained_cycle_count += 1;
			finegrained_thread_arr[threadId].regs_context.reg[current_inst.dst_index] =
				finegrained_thread_arr[threadId].regs_context.reg[current_inst.src1_index] +
				current_inst.src2_index_imm;

			break;
		case CMD_SUBI:    // dst <- src1 - imm
			finegrained_cycle_count += 1;
			finegrained_thread_arr[threadId].regs_context.reg[current_inst.dst_index] =
				finegrained_thread_arr[threadId].regs_context.reg[current_inst.src1_index] -
				current_inst.src2_index_imm;
			break;
		case CMD_LOAD: // dst <- Mem[src1 + src2]  (src2 may be an immediate)
			finegrained_cycle_count += 1;
			// because we need to enter the command in the start of the sleep and in the end of it
			if (current_inst.isSrc2Imm) {
				addr_to_load = (uint32_t)(finegrained_thread_arr[threadId].regs_context.reg[current_inst.src1_index] + current_inst.src2_index_imm);
			}
			else {
				addr_to_load = (uint32_t)(finegrained_thread_arr[threadId].regs_context.reg[current_inst.src1_index] +
					finegrained_thread_arr[threadId].regs_context.reg[current_inst.src2_index_imm]);
			}
			int32_t dst;
			if (SIM_MemDataRead(addr_to_load, &dst) == -1) {
				printf("failed to read from memory");
				return Failure;
			}
			finegrained_thread_arr[threadId].regs_context.reg[current_inst.dst_index] = (int)dst;
			if (load_latency) {
				finegrained_thread_arr[threadId].sleep_count = load_latency;// +1;
				finegrained_thread_arr[threadId].sleep_on = 1;

			}

			break;
		case CMD_STORE:  // Mem[dst + src2] <- src1  (src2 may be an immediate)
			finegrained_cycle_count += 1;
			if (current_inst.isSrc2Imm) {
				addr_to_store = (uint32_t)(finegrained_thread_arr[threadId].regs_context.reg[current_inst.dst_index] + current_inst.src2_index_imm);
			}
			else {
				addr_to_store = (uint32_t)(finegrained_thread_arr[threadId].regs_context.reg[current_inst.dst_index] + finegrained_thread_arr[threadId].regs_context.reg[current_inst.src2_index_imm]);
			}
			SIM_MemDataWrite(addr_to_store, (int32_t) (finegrained_thread_arr[threadId].regs_context.reg[current_inst.src1_index]));
			if (store_latency) {
				finegrained_thread_arr[threadId].sleep_count = store_latency;// +1;
				finegrained_thread_arr[threadId].sleep_on = 1;

			}
			break;
		case CMD_HALT:
			finegrained_cycle_count += 1;
			finegrained_thread_arr[threadId].finished = 1;
			finegrained_finished_thread++;
			break;


	}
	finegrained_thread_arr[threadId].PC++;// make sure for halt this does not bring problems
	updateSleep(finegrained_thread_arr, 1);
	finegrained_inst_num++;



	return Success;


}

Status run_thread(pthreadData blocked_thread_arr, unsigned int threadId, int load_latency, int store_latency) {
	int switch_ = 0;
	Instuction current_inst;
	if (blocked_thread_arr[threadId].sleep_count > 0) { // idle
        //printf("cycle is - %d   , thread is %d pc is %d CMD_LD-St-Sleep\n", block_cycle_count, threadId, blocked_thread_arr[threadId].PC);

        block_cycle_count+=1;
		updateSleep(blocked_thread_arr,1);
		return Success;
	}
	if (blocked_thread_arr[threadId].sleep_on) { // commit
        //printf("cycle is - %d   , thread is %d pc is %d CMD_LOAD-Store\n", block_cycle_count, threadId, blocked_thread_arr[threadId].PC);
		block_cycle_count += 1;
		blocked_thread_arr[threadId].sleep_on = 0;
		updateSleep(blocked_thread_arr, 1);

	}
	while (!switch_) {
		SIM_MemInstRead(blocked_thread_arr[threadId].PC, &current_inst, threadId);
		uint32_t addr_to_store;
		uint32_t addr_to_load;
		switch (current_inst.opcode) {
		case  CMD_NOP:
			block_cycle_count += 1;
			break;
		case CMD_ADD: // dst <- src1 + src2
           //printf("cycle is - %d   , thread is %d pc is %d isAdd\n", block_cycle_count, threadId, blocked_thread_arr[threadId].PC);

                block_cycle_count += 1;
			blocked_thread_arr[threadId].regs_context.reg[current_inst.dst_index] =
				blocked_thread_arr[threadId].regs_context.reg[current_inst.src1_index] +
				blocked_thread_arr[threadId].regs_context.reg[current_inst.src2_index_imm];
			break;

		case CMD_SUB:// dst <- src1 - src2
            //printf("cycle is - %d   , thread is %d pc is %d isSub\n", block_cycle_count, threadId, blocked_thread_arr[threadId].PC);

                block_cycle_count += 1;
			blocked_thread_arr[threadId].regs_context.reg[current_inst.dst_index] =
				blocked_thread_arr[threadId].regs_context.reg[current_inst.src1_index] -
				blocked_thread_arr[threadId].regs_context.reg[current_inst.src2_index_imm];
			break;
		case CMD_ADDI:    // dst <- src1 + imm
            //printf("cycle is - %d   , thread is %d pc is %d isAddi\n", block_cycle_count, threadId, blocked_thread_arr[threadId].PC);

                block_cycle_count += 1;
			blocked_thread_arr[threadId].regs_context.reg[current_inst.dst_index] =
				blocked_thread_arr[threadId].regs_context.reg[current_inst.src1_index] +
				current_inst.src2_index_imm;

			break;
		case CMD_SUBI:    // dst <- src1 - imm
            //printf("cycle is - %d   , thread is %d pc is %d isSubi\n", block_cycle_count, threadId, blocked_thread_arr[threadId].PC);

                block_cycle_count += 1;
			blocked_thread_arr[threadId].regs_context.reg[current_inst.dst_index] =
				blocked_thread_arr[threadId].regs_context.reg[current_inst.src1_index] -
				current_inst.src2_index_imm;
			break;
		case CMD_LOAD: // dst <- Mem[src1 + src2]  (src2 may be an immediate)
            //printf("cycle is - %d   , thread is %d pc is %d CMD_LOAD\n", block_cycle_count, threadId, blocked_thread_arr[threadId].PC);

                block_cycle_count += 1;
			// because we need to enter the command in the start of the sleep and in the end of it
			if (current_inst.isSrc2Imm) {
				addr_to_load = (uint32_t)(blocked_thread_arr[threadId].regs_context.reg[current_inst.src1_index] + current_inst.src2_index_imm);
			}
			else {
				addr_to_load = (uint32_t)(blocked_thread_arr[threadId].regs_context.reg[current_inst.src1_index] +
					blocked_thread_arr[threadId].regs_context.reg[current_inst.src2_index_imm]);
			}
			int32_t dst;
			if (SIM_MemDataRead(addr_to_load, &dst) == -1) {
				printf("failed to read from memory");
				return Failure;
			}
			blocked_thread_arr[threadId].regs_context.reg[current_inst.dst_index] = (int)dst;
			if (load_latency) {
				blocked_thread_arr[threadId].sleep_count = load_latency;// +1; chen said it was the problam
				blocked_thread_arr[threadId].sleep_on = 1;

			}
			switch_ = 1;

			break;
		case CMD_STORE:  // Mem[dst + src2] <- src1  (src2 may be an immediate)
            //printf("cycle is - %d   , thread is %d pc is %d isStore\n", block_cycle_count, threadId, blocked_thread_arr[threadId].PC);

                block_cycle_count += 1;
			if (current_inst.isSrc2Imm) {
				addr_to_store = (uint32_t)(blocked_thread_arr[threadId].regs_context.reg[current_inst.dst_index] + current_inst.src2_index_imm);
			}
			else {
				addr_to_store = (uint32_t)(blocked_thread_arr[threadId].regs_context.reg[current_inst.dst_index] + blocked_thread_arr[threadId].regs_context.reg[current_inst.src2_index_imm]);
			}
			SIM_MemDataWrite(addr_to_store, (int32_t) (blocked_thread_arr[threadId].regs_context.reg[current_inst.src1_index]));
			if (store_latency) {
				blocked_thread_arr[threadId].sleep_count = store_latency;// + 1; chen said it was the problam
				blocked_thread_arr[threadId].sleep_on = 1;

			}
			switch_ = 1;
			break;
		case CMD_HALT:
            //printf("cycle is - %d   , thread is %d pc is %d isHalt\n", block_cycle_count, threadId, blocked_thread_arr[threadId].PC);
			block_cycle_count += 1;
			blocked_thread_arr[threadId].finished = 1;
			switch_ = 1;
			block_finished_thread++;
			break;


		}

		blocked_thread_arr[threadId].PC++;// make sure for halt this does not bring problems
		updateSleep(blocked_thread_arr, 1);
		block_inst_num++;


	}
	return Success;
}
void updateSleep(pthreadData thread_arr, int dec_val) {
	for (int i = 0; i < Get_thread_number(); i++) {
		thread_arr[i].sleep_count -= dec_val;
	}
}
