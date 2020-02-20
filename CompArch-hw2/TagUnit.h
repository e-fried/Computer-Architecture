#ifndef TAG_UNIT_H
#define TAG_UNIT_H

class TagUnit {

private:
	bool valid_bit_;
	bool dirty_bit_;
	unsigned long int tag_entry_;
	unsigned int unit_time_stampe_;
	
public:

	TagUnit() : valid_bit_(false),dirty_bit_(false),tag_entry_(0),unit_time_stampe_(0) {}
	bool getValid_bit() {return this->valid_bit_;}
	bool getDirty_bit() { return this->dirty_bit_; }
	unsigned long int getTag_entry() { return this->tag_entry_; }
	unsigned int getUnit_time() { return this->unit_time_stampe_; }
	void setValid_bit(bool input) { valid_bit_ = input; }
	void setDirty_bit(bool input) { dirty_bit_ = input; }
	void setTag_entry(int input) { tag_entry_ = input; }
	void setUnit_time(unsigned int input) { unit_time_stampe_ = input; }




	




};











#endif // !CACHE_H

