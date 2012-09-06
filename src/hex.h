#ifndef HEX_H
#define HEX_H

#include <stdint.h>

struct HexRecord {
	struct HexRecord *next;
	int count;
	uint32_t address;
	uint8_t data[0];
};

int process_ihexfile(const char *ihexfile, struct HexRecord **rec_p);

#endif