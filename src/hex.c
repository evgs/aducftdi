#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "hex.h"


int hex1(uint8_t **buffer, uint8_t *value)
{
	uint8_t c = *((*buffer)++);
	if ((c >= '0') && (c <= '9'))
		*(value) = (c - '0');
	else if ((c >= 'A') && (c <= 'F'))
		*(value) = ((c - 'A') + 10);
	else if ((c >= 'a') && (c <= 'f'))
		*(value) = ((c - 'a') + 10);
	else
		return -1;
	return 0;
}

int hex2(uint8_t **buffer, uint8_t *value)
{
	uint8_t v;
	if ((hex1(buffer, &v)) < 0) {
		return -1;
	}
	*(value) = (v << 4);
	if ((hex1(buffer, &v)) < 0) {
		return -1;
	}
	*(value) |= v;
	return 0;
}

int hex4(uint8_t **buffer, uint16_t *value)
{
	uint8_t v;
	if ((hex2(buffer, &v)) < 0) {
		return -1;
	}
	*(value) = (v << 8);
	if ((hex2(buffer, &v)) < 0) {
		return -1;
	}
	*(value) |= v;
	return 0;
}

int read_fully(int fd, uint8_t *buffer, int count)
{
	int len;
	uint8_t *ptr = buffer;
	
	while (count > 0) {
		len = read(fd, buffer, count);
		if (len < 0) {
			fprintf(stderr, "read_fully: read failed %s\n", strerror(errno));
			return -1;
		} else if (len == 0) {
			return 0;
		}
		ptr += len;
		count -= len;
	}

	return ptr - buffer;
}

int process_ihexfile(const char *ihexfile, struct HexRecord **rec_p)
{
  
	uint8_t io_buffer[1024];
	
	int fd;
	int rc;
	int i;
	int len;
	uint8_t *ptr;
	uint8_t c;
	uint8_t ih_count;
	uint16_t ih_address;
	uint8_t ih_type;
	uint8_t ih_data[256];
	uint8_t ih_checksum;
	uint32_t ip = 0;
	struct HexRecord *rec;

	fd = open(ihexfile, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "process_ihexfile: open failed (%s) :%s\n", ihexfile, strerror(errno));
		return -1;
	}

	while (1) {
 		ptr = io_buffer;
		len = read_fully(fd, ptr, 1);
		if (len < 0)
			goto read_error;
		else if (len == 0)
			break;

		c = *(ptr++);
		if (c == '\n')
			continue;
		else if (c == '\r')
			continue;
		if (c != ':')
			goto data_error;

 		ptr = io_buffer;
		len = read_fully(fd, ptr, 8);
		if (len < 0)
			goto read_error;
		else if (len == 0)
			goto data_error;

		if (hex2(&ptr, &ih_count) < 0)
			goto data_error;
		if (hex4(&ptr, &ih_address) < 0)
			goto data_error;
		if (hex2(&ptr, &ih_type) < 0)
			goto data_error;

		if (ih_count > 0) {
			ptr = io_buffer;
			len = read_fully(fd, ptr, (ih_count * 2));
			if (len < 0)
				goto read_error;
			else if (len == 0)
				goto data_error;

			for (i = 0; i < ih_count; i++) {
				if (hex2(&ptr, &ih_data[i]) < 0)
					goto data_error;
			}
		}

 		ptr = io_buffer;
		len = read_fully(fd, ptr, 2);
		if (len < 0)
			goto read_error;
		else if (len == 0)
			goto data_error;

		if (hex2(&ptr, &ih_checksum) < 0)
			goto data_error;

		if (ih_type == 0x00) {
			rec = malloc(sizeof(struct HexRecord) + (sizeof(uint8_t) * ih_count));
			if (rec == NULL)
				goto memory_error;
			rec->next = NULL;
			rec->count = ih_count;
			rec->address = ip + ih_address;
			memcpy(rec->data, ih_data, ih_count);
			*(rec_p) = rec;
			rec_p = &rec->next;
		} else if (ih_type == 0x02) {
			ip = (((ih_data[0] << 8) | (ih_data[1] << 0)) << 4);
		} else if (ih_type == 0x04) {
			ip = (((ih_data[0] << 8) | (ih_data[1] << 0)) << 16);
		}
	}
	rc = 0;
	goto close;

memory_error:
	fprintf(stderr, "process_ihexfile: memory error\n");
	rc = -1;
	goto close;
read_error:
	fprintf(stderr, "process_ihexfile: read error\n");
	rc = -1;
	goto close;
data_error:
	fprintf(stderr, "process_ihexfile: data error\n");
	rc = -1;
	goto close;

close:
	if (close(fd) < 0) {
		fprintf(stderr, "process_ihexfile: close failed %s\n", strerror(errno));
		return -1;
	}

	return rc;
}


void hexdump(const char * buf, size_t length) {
  int addr=0;

  printf("\n");
    
  while (length>0) {
    int strlen = 16;
    int padding = 0;
    
    if (strlen>length) {
      strlen = length; 
      padding = 16-length;
    }
    
    printf("%04x  ",addr);
    
    for (int i=0; i<strlen; i++) {
      printf("%02x ", *(buf+addr+i) );
    }
    
    for (int i=0; i<padding; i++) { printf("   "); }
    
    printf("|");

    for (int i=0; i<strlen; i++) {
      unsigned char c = *((unsigned char *)(buf+addr+i));
      if ( c < 32 || c > 126 ) c='.';
      printf("%c", c );
    }

    for (int i=0; i<padding; i++) { printf(" "); }
    
    printf("|\n");
    
    addr+=strlen;
    length-=strlen;
  }
}
