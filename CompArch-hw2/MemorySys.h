#ifndef MEM_SYS_H
#define MEM_SYS_H
#include <math.h>

#include "Cache.h"
#include "VictimCache.h"

class MemorySystem {

private:
	Cache L1_;
	Cache L2_;
	VictimCache victim_cache_;
	bool victim_cache_exist_;
	int write_policy_; //write_no_allocate==0 ; write_allocate==1
	unsigned int cycle_count_;
	unsigned int time_;
	int memory_access_;
	
	
	
public:
	MemorySystem(unsigned int block_size, int  write_policy,
		int L1_size, int L1_ways, int L1_acc_time, int L2_size, int L2_ways,
		int L2_acc_time, bool victim_cache_exist, int memory_access) :
		victim_cache_exist_(victim_cache_exist), write_policy_(write_policy), cycle_count_(0), time_(0), memory_access_(memory_access),
		L1_(pow(2, L1_size), pow(2, L1_ways), pow(2, block_size), L1_acc_time),
		L2_(pow(2, L2_size), pow(2, L2_ways), pow(2, block_size), L2_acc_time),
		victim_cache_(block_size)
	
	{

	}

	void ReadTag(unsigned long int tag);
	void WriteTag(unsigned long int tag);
	void L2WriteBack(unsigned long int address, unsigned int time_stamp);
	double getL2MissRate();
    double getL1MissRate();
    unsigned int getAccTime();
	void WriteBackToVictim(unsigned int address);
	void snoop(unsigned long int address);



};


#endif //MEM_SYS_H