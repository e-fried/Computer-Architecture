#ifndef VICTIM_H
#define VICTIM_H
#include <math.h>
#include "TagUnit.h"
#include <deque>


#define VICTIM_SIZE 4

using namespace std;

class VictimCache {

private:
	deque <TagUnit> victim_cache;
	//TagUnit victim_cache_[VICTIM_SIZE];
	unsigned int block_size_;


public:
	VictimCache(unsigned int block_size):block_size_(block_size) {

	}


	void queue(unsigned long int address, bool dirty) { //this qeue would remove last member in queue if full
		if (victim_cache.size() >= VICTIM_SIZE) {
			victim_cache.pop_back();
		}
		TagUnit newTag= TagUnit();
		newTag.setDirty_bit(dirty);
		newTag.setTag_entry(address);
		

		
		victim_cache.push_front(newTag);
		victim_cache.front().setUnit_time(0);// not relevant
		victim_cache.front().setValid_bit(1);

	}
	bool dequeue() {
		if (victim_cache.size() == 0) {
			return false;
		}
		victim_cache.pop_back();
        return  true;
	}

	bool is_full() {
		return  (victim_cache.size() == 4 ? true : false);
	}

	bool is_empty() {
		return (victim_cache.size() == 0 ? true : false);
	}

	bool check_if_exist(unsigned long int address) {
	    unsigned long mask=0xffffffff;
	    mask= mask<<block_size_;
	    address= address & mask;
		for (int i = 0; i < victim_cache.size(); i++) {
			if (address == victim_cache[i].getTag_entry()) {
				return true;
			}
			
		}
		return false;
	}

	void write_to_victim(unsigned long int address) {
        unsigned long mask=0xffffffff;
        mask= mask<<block_size_;
        address= address & mask;
		for (int i = 0; i < victim_cache.size(); i++) {
			if (address == victim_cache[i].getTag_entry()) {
				victim_cache[i].setDirty_bit(1);
			}

		}
	}
	TagUnit getVictimEntry(unsigned long int address) {
        unsigned long mask=0xffffffff;
        mask= mask<<block_size_;
        address= address & mask;
		for (int i = 0; i < victim_cache.size(); i++) {
			if (address == victim_cache[i].getTag_entry()) {
				return victim_cache[i];
			}
		}
		TagUnit tag=TagUnit();
        return tag;
	}

	void removeEntry_I(unsigned long int address) {
        unsigned long mask=0xffffffff;
        mask= mask<<block_size_;
        address= address & mask;
		for (int i = 0; i < victim_cache.size(); i++) {
			if (address == victim_cache[i].getTag_entry()) {
				victim_cache.erase(victim_cache.begin() + i);
			}
		}
	}

	bool getDirtyBit(unsigned long int address) {
        unsigned long mask=0xffffffff;
        mask= mask<<block_size_;
        address= address & mask;

		for (int i = 0; i < victim_cache.size(); i++) {
			if (address == victim_cache[i].getTag_entry()) {
				return victim_cache[i].getDirty_bit();
			}
		}
        return 0;
	}
};


#endif //VICTIM_H