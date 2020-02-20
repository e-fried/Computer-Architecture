#include "MemorySys.h"


#define VICTIM_TIME_ACCESS 1

void MemorySystem::ReadTag(unsigned long int address) {
	this->time_++;
	/*-----------------------------------L1_access---------------------------------------*/
	if (this->L1_.SearchTag(address,this->time_++)) {
		this->cycle_count_ += L1_.getAccessTime();
		this->L1_.AccessIncrement();
		this->L1_.HitIncrement();
		return;
	}

	/*-----------------------------------L2_access--(if we dont find tag in l1)----------*/
	else if (this->L2_.SearchTag(address, this->time_++)) {
		this->cycle_count_ += (L1_.getAccessTime() + L2_.getAccessTime());
		this->L1_.AccessIncrement();
		this->L2_.HitIncrement();
		this->L2_.AccessIncrement();
		this->L2WriteBack(address,time_++);
		L1_.WriteToCache(address,time_++);
		return;
	}


	/*----------------------------case the tag isnt in l1 and l2-------------------------*/
	else {
		this->L1_.AccessIncrement(); this->L2_.AccessIncrement();
		if (this->victim_cache_exist_ && this->victim_cache_.check_if_exist(address)) {
			this->cycle_count_ += (L1_.getAccessTime() + L2_.getAccessTime() + VICTIM_TIME_ACCESS);
			TagUnit tmp = victim_cache_.getVictimEntry(address);
			victim_cache_.removeEntry_I(address);
			if (this->L2_.checkLRUIfValid(address)) {

				this->snoop(address);
			}
			this->WriteBackToVictim(address);
			
			L2_.WriteToCache(address, time_++);
			L2_.SetDirty(address, 0);
			this->L2WriteBack(address, time_++);
			L1_.WriteToCache(address,time_++);
			
			if (tmp.getDirty_bit()==1) {
				L1_.SetDirty(address, 1);
			}
			return;
		}
		if (this->L2_.checkLRUIfValid(address)) {

			this->snoop(address);
		}
		
		if (this->victim_cache_exist_) {this->WriteBackToVictim(address);}
		L2_.WriteToCache(address, time_++);
		this->L2WriteBack(address, time_++);
		L1_.WriteToCache(address, time_++);
		L1_.SetDirty(address, 0); L2_.SetDirty(address, 0);
		this->cycle_count_ += (L1_.getAccessTime() + L2_.getAccessTime() + (VICTIM_TIME_ACCESS*victim_cache_exist_) + memory_access_);
			//write l1 l2 
	}
}



void MemorySystem::WriteTag(unsigned long int address) {
	this->time_++;
	/*-----------------------------------L1_access---------------------------------------*/

	if (this->L1_.SearchTag(address, this->time_++)) {//l1 search
		this->cycle_count_ += L1_.getAccessTime();
		this->L1_.SetDirty(address,1);
		this->L1_.HitIncrement();
		this->L1_.AccessIncrement();
		return;
	}

	/*-----------------------------------L2_access--(if we dont find tag in l1)----------*/

	else if (this->L2_.SearchTag(address,this->time_++)) {//l1 miss , l2 search
		this->cycle_count_ += (L1_.getAccessTime() + L2_.getAccessTime());
		this->L1_.AccessIncrement();
		this->L2_.AccessIncrement(); this->L2_.HitIncrement();
		this->L2_.SetDirty(address, 1);
		
		
		if (write_policy_) {
			this->L2WriteBack(address, time_++);
			this->L1_.WriteToCache(address, this->time_++);
			this->L1_.SetDirty(address, 1);
			this->L2_.SetDirty(address,0);
		}
		return;
	}


	/*----------------------------case the tag isnt in l1 and l2-------------------------*/
	else {
		this->L1_.AccessIncrement(); this->L2_.AccessIncrement();
		if (this->victim_cache_exist_ && this->victim_cache_.check_if_exist(address)) {
			this->cycle_count_ += (L1_.getAccessTime() + L2_.getAccessTime() + VICTIM_TIME_ACCESS);
			victim_cache_.write_to_victim(address);
			if (write_policy_) {

				TagUnit tmp = victim_cache_.getVictimEntry(address);
				victim_cache_.removeEntry_I(address);
				if (this->L2_.checkLRUIfValid(address)) {

					this->snoop(address);
				}
				this->WriteBackToVictim(address);
				L2_.WriteToCache(address, time_++);
				L2_.SetDirty(address, 0);
				this->L2WriteBack(address, time_++);
				L1_.WriteToCache(address, time_++);
				L1_.SetDirty(address, 1);
				return;
				
			}
			return;
		}

		this->cycle_count_ += (L1_.getAccessTime() + L2_.getAccessTime() + (VICTIM_TIME_ACCESS*victim_cache_exist_) + memory_access_);
		if (write_policy_) {

			if (this->L2_.checkLRUIfValid(address)) {

				this->snoop(address);
			}
			if (this->victim_cache_exist_) { this->WriteBackToVictim(address); }
			L2_.WriteToCache(address, time_++);
			this->L2WriteBack(address, time_++);
			L1_.WriteToCache(address, time_++);
			L1_.SetDirty(address, 1); L2_.SetDirty(address, 0);

		}

		//write l1 l2 
	}
}


void MemorySystem::L2WriteBack(unsigned long int address, unsigned int time_stamp) {
	int set_index = this->L1_.setCalculator(address);
	int way_index = this->L1_.FindLru(set_index);
	TagUnit tag_to_move = this->L1_.CacheGetTag(set_index, way_index);
	unsigned int long L2_address = L1_.rebuildAddress(tag_to_move.getTag_entry(),set_index);
	if (tag_to_move.getValid_bit() != 0 && tag_to_move.getDirty_bit() == 1) {
		L2_.UpdateWriteBack(L2_address,time_stamp);
	}
}
double MemorySystem::getL2MissRate(){
    return (this->L2_.getMissRate());
}
double MemorySystem::getL1MissRate(){
    return (this->L1_.getMissRate());
}

unsigned int  MemorySystem::getAccTime(){
	
	return (this->cycle_count_);
	
}



void MemorySystem::WriteBackToVictim( unsigned int address) {

	int set_index = this->L2_.setCalculator(address);
	int way_index = this->L2_.FindLru(set_index);
	TagUnit tmpTag = L2_.CacheGetTag(set_index, way_index);
	if (!tmpTag.getValid_bit()) {
		return;
	}
	this->victim_cache_.queue(L2_.rebuildAddress(tmpTag.getTag_entry(), set_index),tmpTag.getDirty_bit());

}
void MemorySystem::snoop(unsigned long int new_address) {
	int set_index = this->L2_.setCalculator(new_address);
	int way_index = this->L2_.FindLru(set_index);

	TagUnit tempTag = L2_.CacheGetTag(set_index, way_index);
	unsigned long int built_address = L2_.rebuildAddress(tempTag.getTag_entry(), set_index);// changed from L1 to L2
	if (L1_.getDirty(built_address) == 1) {
		int set_index_L1 = this->L1_.setCalculator(built_address);
		int tag_l1=this->L1_.tagCalculator(built_address);
		int way_index_l1;
		for (int i=0;i<L1_.getWays();i++){
		    if (tag_l1==L1_.CacheGetTag(set_index,i).getTag_entry()){
		        way_index_l1=i;
		    }
		}
		TagUnit tag_to_move = this->L1_.CacheGetTag(set_index_L1, way_index_l1);
		unsigned int long L2_address = L1_.rebuildAddress(tag_to_move.getTag_entry(), set_index_L1);
		if (tag_to_move.getValid_bit() != 0) {
			L2_.UpdateWriteBack(L2_address, time_++);
		}
	}
	L1_.setValid(built_address, 0);// if exist in L2 and not L1, this will do nothing
	L1_.SetDirty(built_address, 0);
	
}