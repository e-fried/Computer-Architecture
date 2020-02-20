#ifndef CACHE_H
#define CACHE_H
#include <math.h>

#include "TagUnit.h"

class Cache {

private:
	int size_;
	int ways_;
	int block_;
	int access_time_;
	TagUnit** tag_table_;
	int num_of_rows_;
	int Hit_counter_;
	int access_counter_;
	unsigned int time_stamp;//to add the set
public:
	Cache(int size,int ways, int block,int access_time);
	Cache() {};
	~Cache();
	int getAccessTime();
	bool SearchTag(unsigned long int address, unsigned int time_stamp);
	void SetDirty(unsigned int address, bool bit);
	void WriteToCache(int tag, unsigned int time_stamp);
	unsigned int tagCalculator(unsigned long int address);
	unsigned int setCalculator(unsigned long int address);
    int FindLru(int set_index);//return the wat to the tag
	void HitIncrement();
	void AccessIncrement();
	TagUnit CacheGetTag(unsigned int set_index, unsigned int way_index);
	void UpdateWriteBack(unsigned long int tag, unsigned int time_stamp);
	double getMissRate();
	unsigned int long rebuildAddress(unsigned int tag, unsigned int set);
	bool checkLRUIfValid(unsigned long int address);
	void setValid(unsigned long int address, bool valid_bit);
	bool getDirty(unsigned long int address);
    unsigned int getWays();



};


#endif //CASH_H
