#include "Cache.h"
#include <math.h>


Cache::Cache(int size, int ways, int block, int access_time):
	size_(size),
	ways_(ways),
	block_(block),
	access_time_(access_time),
	tag_table_(0), // for some reason does not work otherwise
	//tag_table_(nullptr),
	num_of_rows_(0),
	Hit_counter_(0),
	access_counter_(0)
{
	this->num_of_rows_ = (size_) / (block_*ways_);
	this->tag_table_ = new TagUnit* [ways_];

	for (unsigned int i = 0; i < this->ways_; ++i) {
		this->tag_table_[i] = new TagUnit[num_of_rows_];
	}
}




Cache::~Cache(){
/*
	for (int i = 0; i < this->num_of_rows_; ++i) {
		delete[] (this->tag_table_[i]);
}*/
	delete[] (this->tag_table_);
}




int Cache::getAccessTime() {
	return this->access_time_;
}

bool Cache::SearchTag(unsigned long int address, unsigned int time_stamp) {
	int tag = tagCalculator(address);
	int set_index = setCalculator(address);
	for (int i = 0; i < ways_; i++) {
		if ((tag_table_[i][set_index].getTag_entry() == tag) && (tag_table_[i][set_index].getValid_bit() == true)) {
			tag_table_[i][set_index].setUnit_time(time_stamp);
			return true;
		}

	}
	return false;
}

void Cache::WriteToCache(int address,unsigned int time_stamp) {
	int tag = tagCalculator(address);
	int set_index = setCalculator(address);
	int way_to_replace_the_set = FindLru(set_index);
	this->tag_table_[way_to_replace_the_set][set_index].setTag_entry(tag);
	this->tag_table_[way_to_replace_the_set][set_index].setUnit_time(time_stamp);
	this->tag_table_[way_to_replace_the_set][set_index].setValid_bit(1);
	return;
}

void Cache::SetDirty(unsigned int address, bool bit) {
	int tag = tagCalculator(address);
	int set_index = setCalculator(address);
	for (int i = 0; i < ways_; i++) {
		if ((tag_table_[i][set_index].getTag_entry() == tag) && (tag_table_[i][set_index].getValid_bit() == true)) {
			tag_table_[i][set_index].setDirty_bit(bit);
			return ;
		}
	}
}



unsigned int Cache::tagCalculator(unsigned long int address) {
	return address >> (unsigned int)(log2(block_) + log2(num_of_rows_));

}

unsigned int Cache::setCalculator(unsigned long int address) {
	unsigned int setWithTag = address >> (unsigned int)log2(this->block_);
	unsigned int Mask = this->num_of_rows_ - 1;
	return setWithTag & Mask;
}



int Cache::FindLru(int set_index) {
	unsigned int min = 4294836225;
	int min_way=0;
	for (int i = 0; i < ways_; i++) {
		if (tag_table_[i][set_index].getUnit_time() < min) {
			min = tag_table_[i][set_index].getUnit_time();
			min_way = i;
		}
	}
	return min_way;
}


void Cache::HitIncrement() {
	this->Hit_counter_++;
}

void Cache::AccessIncrement() {
	this->access_counter_++;
}


TagUnit Cache::CacheGetTag(unsigned int set_index, unsigned int way_index) {
	return tag_table_[way_index][set_index];
		
}

void Cache::UpdateWriteBack(unsigned long int address, unsigned int time_stamp) {
	int tag = tagCalculator(address);
	int set_index = setCalculator(address);
	for (int i = 0; i < ways_; i++) {
		if ((tag_table_[i][set_index].getTag_entry() == tag)) {
			tag_table_[i][set_index].setUnit_time(time_stamp);
			tag_table_[i][set_index].setDirty_bit(1);
			return ;
			
		}

	}
	return ;
}


double Cache::getMissRate(){
	if (this->access_counter_ != 0) {
		return ((double)(this->Hit_counter_) /(double) this->access_counter_); 
	}
	return -1;
}


unsigned int long Cache::rebuildAddress(unsigned int tag, unsigned int set) {
	set = (unsigned int long)set;
	unsigned int long address_to_return = 0;
	address_to_return = (tag << (unsigned int)(log2(this->num_of_rows_)));
	address_to_return +=set;
	address_to_return = address_to_return << (unsigned int)log2(this->block_);
	return address_to_return;


}
bool Cache::checkLRUIfValid(unsigned long int address) {
	int tag = tagCalculator(address);
	int set_index = setCalculator(address);
	int way = FindLru(set_index);

	if (tag_table_[way][set_index].getValid_bit() == true) {
		return true;
	}
	return false;
}

void Cache::setValid(unsigned long int address, bool valid_bit) {
	int tag = tagCalculator(address);
	int set_index = setCalculator(address);
	for (int i = 0; i < ways_; i++) {
		if ((tag_table_[i][set_index].getTag_entry() == tag)) {
			tag_table_[i][set_index].setValid_bit(valid_bit);
			return;

		}

	}
}
bool Cache::getDirty(unsigned long int address) {
	int tag = tagCalculator(address);
	int set_index = setCalculator(address);
	for (int i = 0; i < ways_; i++) {
		if ((tag_table_[i][set_index].getTag_entry() == tag) && tag_table_[i][set_index].getValid_bit()) {
			return (tag_table_[i][set_index].getDirty_bit());

		}
	}
	return 0;// if tag doesn't exist in L1
}

unsigned int Cache::getWays(){
    return this->ways_;
}
