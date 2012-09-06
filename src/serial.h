#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

typedef int (*SERIAL_RDF)(uint8_t *buffer, int count, int timeout);
typedef int (*SERIAL_WRF)(const uint8_t *buffer, int count, int timeout);

typedef struct {
  SERIAL_RDF readBlock;
  SERIAL_WRF writeBlock;
} ReadWrite;

ReadWrite const * getFtdiReadWrite();

#endif //SERIAL_H