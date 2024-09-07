#include "../../../include/types.h"

void* memsetq(uint64_t* bufptr, uint64_t value, size_t size_of_dest_in_qwords) {
	uint64_t* buf = (uint64_t*) bufptr;
	for (size_t i = 0; i < size_of_dest_in_qwords; i++)
		buf[i] = value;
	return bufptr;
}
