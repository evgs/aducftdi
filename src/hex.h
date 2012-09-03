#ifndef HEX_H
#define HEX_H

#include <stdint.h>

struct HexRecord {
	struct HexRecord *next;
	int count;
	uint32_t address;
	uint8_t data[0];
};

#endif