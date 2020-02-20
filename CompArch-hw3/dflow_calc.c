/* 046267 Computer Architecture - Spring 2019 - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"

typedef struct inst_struct_
{
	InstInfo instruction;
	int source1_index;
	int source2_index;
	int depth_time;
	unsigned int instruction_time;

}instsStruct, *PinstsStruct;

typedef struct program_data_ {
	
	PinstsStruct inst_table;
	unsigned int false_dependencies[MAX_OPS];
	int          register_file[MAX_OPS];
	unsigned int program_depth;
	unsigned int total_instruction;

}programData, *PprogramData;

int getMax(int num1, int num2);

ProgCtx analyzeProg(const unsigned int opsLatency[],  InstInfo progTrace[],  int numOfInsts) {
	
	PprogramData program_data=(PprogramData)malloc(sizeof(programData));
	if (program_data == NULL) {
		return PROG_CTX_NULL;
	}
	program_data->inst_table = (PinstsStruct)malloc(sizeof(instsStruct)*numOfInsts);
	if (program_data->inst_table == NULL) {
		free(program_data);
		return PROG_CTX_NULL;
	}
	/*initialize structures-----------------------------------------------------------------------------*/
	program_data->total_instruction = numOfInsts;
	program_data->program_depth = 0;
	for (int i = 0; i < MAX_OPS; i++) {
		program_data->false_dependencies[i] = 0;
		program_data->register_file[i] = -1;
	}
	for (int i = 0; i < numOfInsts; i++) {
		program_data->inst_table[i].depth_time = 0;
		program_data->inst_table[i].source1_index = -1;
		program_data->inst_table[i].source2_index = -1;

	}
	/*-----------------------------------------------------------------------------*/

	for (int i = 0; i < numOfInsts; i++) {
		if (i >= 1) {
			if (progTrace[i].dstIdx == progTrace[i - 1].dstIdx ||
				progTrace[i].dstIdx == progTrace[i - 1].src1Idx ||
				progTrace[i].dstIdx == progTrace[i - 1].src2Idx) {

				program_data->false_dependencies[progTrace[i].dstIdx]++ ;

			}


		}

		program_data->inst_table[i].instruction.dstIdx = progTrace[i].dstIdx;
		program_data->inst_table[i].instruction.opcode = progTrace[i].opcode;
		program_data->inst_table[i].instruction.src1Idx = progTrace[i].src1Idx;
		program_data->inst_table[i].instruction.src2Idx = progTrace[i].src2Idx;
		
		program_data->inst_table[i].instruction_time = opsLatency[progTrace[i].opcode];
		int depth1=0, depth2=0;
		if (program_data->register_file[progTrace[i].src1Idx] != -1) {
			int index_to_source_1 = program_data->register_file[progTrace[i].src1Idx];
			depth1 = program_data->inst_table[index_to_source_1].depth_time + program_data->inst_table[index_to_source_1].instruction_time;
			program_data->inst_table[i].source1_index = index_to_source_1;
		}
		if (program_data->register_file[progTrace[i].src2Idx] != -1) {
			int index_to_source_2 = program_data->register_file[progTrace[i].src2Idx];
		    depth2 = program_data->inst_table[index_to_source_2].depth_time + program_data->inst_table[index_to_source_2].instruction_time;
			program_data->inst_table[i].source2_index = index_to_source_2;
		}
		program_data->inst_table[i].depth_time = getMax(depth1, depth2);
		program_data->register_file[progTrace[i].dstIdx] = i;
		if (program_data->inst_table[i].depth_time+program_data->inst_table[i].instruction_time >= program_data->program_depth) {
			program_data->program_depth = (program_data->inst_table[i].depth_time +program_data->inst_table[i].instruction_time) ;
		}
	}


	
    return program_data;
}

void freeProgCtx(ProgCtx ctx) {
	PprogramData program_data = (PprogramData)ctx;
	free(program_data->inst_table);
	free(program_data);
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
	PprogramData program_data = (PprogramData)ctx;
	if (theInst > program_data->total_instruction) {
		return -1;
	}
    return program_data->inst_table[theInst].depth_time;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
	PprogramData program_data = (PprogramData)ctx;
	if (theInst > program_data->total_instruction) {
		return -1;
	}
	*src1DepInst = program_data->inst_table[theInst].source1_index;
	*src2DepInst = program_data->inst_table[theInst].source2_index;
	return 0;
}

int getRegfalseDeps(ProgCtx ctx, unsigned int reg){
	PprogramData program_data = (PprogramData)ctx;
	if (reg > MAX_OPS) {
		return -1;
	}
	
	return program_data->false_dependencies[reg];
}

int getProgDepth(ProgCtx ctx) {
	PprogramData program_data = (PprogramData)ctx;

    return program_data->program_depth;
}

int getMax(int num1, int num2) {
	if (num1 > num2) {
		return num1;
	}
	return num2;
}


