/*
 * aducloader - A serial download program for ADuC702x microcontrollers
 * Copyright (C) 2006, 2007 Robert Fitzsimons
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  Look for the COPYING file in the top
 * level directory.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * This is an in-circuit serial download program for the Analog Devices
 * ADuC702x range of microcontrollers.  The relevant protocol details
 * are described in application note AN-724.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>

#include "hex.h"

#include "serial.h"

const int DATA_BUFFER_SIZE = 250;
uint8_t *io_buffer;
uint8_t *data_buffer;

int reset;
int follow;

int aducISPPacket(uint8_t *buffer, char cmd, uint32_t address, const uint8_t *data, int data_len);
void dump_buffer(FILE *f, const uint8_t *buffer, int len);
void dump_text(FILE *f, const uint8_t *buffer, int len);

int writeFlash(const ReadWrite *chn, struct HexRecord *root) {

  	io_buffer = malloc(sizeof(uint8_t) * 1024);
	data_buffer = malloc(sizeof(uint8_t) * 256);
	
	if (io_buffer != NULL || data_buffer !=NULL) {
		fprintf(stderr, "error allocating memory\n");
		return -1;
	}
	  

  
	int len;
	uint8_t *ptr;
	struct HexRecord *rec;
	const uint8_t BACKSPACE[] = {0x08};
	const uint8_t ERASE_ALL[] = {0x00};
	const uint8_t ACK = 0x06;
	ptr = io_buffer;

	fprintf(stdout, "Trying to synchronize with ADuC70xx: ");
	fflush(stdout);

	len = chn->writeBlock(BACKSPACE, sizeof(BACKSPACE), 1000);
	len = chn->readBlock(ptr, 24, 5000);

	if ((len > 6) && (memcmp(ptr, "ADuC70", 6) == 0)) {
		int product_len = (memchr(ptr, ' ', 24) - (void *)ptr);
		fprintf(stdout, "Found %.*s\n", product_len, ptr);

		fprintf(stdout, "Erasing: ");
		fflush(stdout);

		len = aducISPPacket(ptr, 'E', 0x00000000, ERASE_ALL, sizeof(ERASE_ALL));
		len = chn->writeBlock(ptr, len, 1000);
		len = chn->readBlock(ptr, 1, 10000);

		if ((len <= 0) || (ptr[0] != ACK)) {
			fprintf(stdout, "FAILED\n");
			return -1;
		}

		fprintf(stdout, "\nOK\n");

		rec = root;
		while (rec != NULL) {
			int count = 0;
			uint32_t address = rec->address;
			uint8_t *data = data_buffer;
			struct HexRecord *drec;

			for (drec = rec; ((drec != NULL) && ((address + count) == drec->address) && ((count + drec->count) < DATA_BUFFER_SIZE)); drec = drec->next) {
				memcpy(data, drec->data, drec->count);
				data += drec->count;
				count += drec->count;
			}
			rec = drec;

			fprintf(stdout, "Writting (%#x): ", address);
			fflush(stdout);

			len = aducISPPacket(ptr, 'W', address, data_buffer, count);
			len = chn->writeBlock(ptr, len, 1000);
			len = chn->readBlock(ptr, 1, 10000);

			if ((len <= 0) || (ptr[0] != ACK)) {
				fprintf(stdout, "FAILED\n");
				return -1;
			}

			fprintf(stdout, "OK\r");
		}
		fprintf(stdout, "OK\n");

		if (reset) {
			fprintf(stdout, "Resetting: ");
			fflush(stdout);

			len = aducISPPacket(ptr, 'R', 0x00000001, NULL, 0);
			len = chn->writeBlock(ptr, len, 1000);
			len = chn->readBlock(ptr, 1, 10000);

			if ((len <= 0) || (ptr[0] != ACK)) {
				fprintf(stdout, "FAILED\n");
				return -1;
			}

			fprintf(stdout, "OK\n");
		}
	} else {
		fprintf(stdout, "FAILED\n");
		return -1;
	}

	return 0;
}




int aducISPPacket(uint8_t *buffer, char command, uint32_t address, const uint8_t *data, int data_len)
{
	uint8_t *ptr = buffer;
	uint8_t checksum = 0;
	int i;

	address &= 0xFFFF;

	*(ptr++) = 0x07;
	*(ptr++) = 0x0E;

	checksum += *(ptr++) = 5 + data_len;

	checksum += *(ptr++) = command;

	checksum += *(ptr++) = (address >> 24) & 0xFF;
	checksum += *(ptr++) = (address >> 16) & 0xFF;
	checksum += *(ptr++) = (address >>  8) & 0xFF;
	checksum += *(ptr++) = (address >>  0) & 0xFF;

	for (i = 0; i < data_len; i++) {
		checksum += *(ptr++) = data[i];
	}

	*(ptr++) = 0x00 - checksum;

	return ptr - buffer;
}

void dump_buffer(FILE *f, const uint8_t *buffer, int len)
{
	int i;

	fprintf(f, "Buffer Length %d\n", len);
	for (i = 0; i < len; i++) {
		fprintf(f, "%2.2x", buffer[i]);
	}
	fprintf(f, "\n");
}

void dump_text(FILE *f, const uint8_t *buffer, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		uint8_t c = buffer[i];
		fputc(((isprint(c) || isspace(c)) ? (c) : ('.')), f);
	}
}


