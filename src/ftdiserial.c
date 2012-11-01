#include <stdio.h>

#include "serial.h"

#include <ftdi.h>

#ifdef __MINGW32_VERSION

#include <windows.h>

void zzz(long time) {
  Sleep(time);
}

#else

#include <unistd.h>

void zzz(long time) {
  usleep(time*1000);
}

#endif

#include <usb.h>

void resetFtdi(struct ftdi_context *ftdi) {
  printf("Cycling FT232R\n");
  //TODO: link with libusb in win32
  //usb_reset(ftdi->usb_dev);
  zzz(1000);
}

// Default VID:PID=0x0403:0x6001
int vid=0x0403;
int pid=0x6001;

#define RESET_BIT (0x44)
#define BM_BIT (0x22)


struct ftdi_context *fc = NULL;

void ftdi_checksum(unsigned char *buf) {
  unsigned short checksum = 0xAAAA;
  int i;
  for (i = 0; i < (FTDI_DEFAULT_EEPROM_SIZE)/2-1; i++)
  {
    unsigned short value = buf[i*2];
    value |= buf[(i*2)+1] << 8;
    checksum = value^checksum;
    checksum = (checksum << 1) | (checksum >> 15);
  }
  
  buf[(FTDI_DEFAULT_EEPROM_SIZE)-2] = checksum;
  buf[(FTDI_DEFAULT_EEPROM_SIZE)-1] = checksum >> 8;
  
  printf("checksum=%04x\n", checksum);

}

void printFtdiString(unsigned char *eebuf, int offset) {
  offset = eebuf[offset] - 0x80;
  eebuf += offset;
  int len = ((*eebuf)/2)-1;
  
  while (len>0) {
    eebuf+=2;
    printf("%c", *eebuf);
    len--;
  }
  
}

int ftdiListAll() {
  
    fc = ftdi_new();
    
    if (fc == NULL) { fprintf(stderr, "ftdi_init failed\n"); return -1; }
    
    struct ftdi_device_list * devlist = NULL;
    struct ftdi_device_list * ptr;
    
    int count = ftdi_usb_find_all(fc, &devlist, vid, pid);
    
    if (count<0) {
      fprintf(stderr, "unable to enumerate ftdi devices: %s",
	      ftdi_get_error_string(fc)
	     );
      return -1;
    }
 
    ptr=devlist;
    
    char mnf[33];
    char desc[33];
    char serial[33];
      
    for (int i=0; i<count; i++) {
      int result = ftdi_usb_get_strings(fc, ptr->dev, mnf, 32,  desc, 32, serial, 32);
      
      printf("(%d) [%x:%x] %s %s %s\n", i, vid, pid, serial, mnf, desc);
      ptr = ptr->next;
    }
    
    if (devlist != NULL) ftdi_list_free(&devlist);
    
}

void closeFtdi(){
    ftdi_usb_close(fc);
    ftdi_deinit(fc);
}

int bindFtdi(const char *addr) {
  if (fc==NULL) fc = ftdi_new();
  
  int result;
  if (addr == NULL) {
    result = ftdi_usb_open( fc, vid, pid);
  } else {
    result = ftdi_usb_open_string( fc, addr);
  }
  
  if (result<0) {
    fprintf(stderr, "unable to open ftdi device: %s\n",
	    ftdi_get_error_string(fc)
	    );
    return -1;
  }
  
}

int flushFtdi(){
  zzz(100);
  ftdi_usb_purge_buffers(fc);
  zzz(100);
  return 1;
}

int setFtdiBaudRate(int baudrate) {
  ftdi_set_line_property(fc, BITS_8, STOP_BIT_1, NONE);
  return ftdi_set_baudrate(fc, baudrate);
}

int printFtdiInfo() {
      unsigned char eebuf[FTDI_DEFAULT_EEPROM_SIZE];
      ftdi_read_eeprom(fc, eebuf);
      
      printf ("Manufacturer: "); printFtdiString(eebuf, 0x0e);
      printf ("\nProduct:      "); printFtdiString(eebuf, 0x10);
      printf ("\nSerial:       "); printFtdiString(eebuf, 0x12);
      printf("\n");
  
}

int aducFtdiReset(int pm) {
    int bmMask = 0;
  
    if (pm) {
      bmMask = BM_BIT;
      printf("Holding down BM...\n");
      ftdi_set_bitmode(fc, bmMask, BITMODE_CBUS);
      zzz(100);
    }

    printf("Pulsing RESET ~\\__/~\n");

    ftdi_set_bitmode(fc, bmMask | RESET_BIT, BITMODE_CBUS);
    zzz(250);
    ftdi_set_bitmode(fc, bmMask, BITMODE_CBUS);
    
    if (pm) {
      printf("Entering bootloader mode...\n");
      zzz(2000);

      ftdi_set_bitmode(fc, 0, BITMODE_CBUS);
      zzz(250);
    }
  
    //ftdi_disable_bitbang(fc);
}
  
#include "timeout.h"

int serial_read_fully(uint8_t *buffer, int count, int timeout)
{
  uint8_t *ptr = buffer;
  
  setTimeout(timeout);

  while (count>0) {
    int r=ftdi_read_data(fc, ptr, count);
    count-=r; ptr+=r;
    if (isTimeout()) break;
  }

  cancelTimeout();
  return ptr - buffer;
}

int serial_write_fully(const uint8_t *buffer, int count, int timeout)
{
  uint8_t const *ptr = buffer;
  setTimeout(timeout);

  while (count>0) {
    int w=ftdi_write_data(fc, ptr, count);
    
    count-=w; ptr+=w;
    if (isTimeout()) break;
  }
  
  cancelTimeout();
  return ptr - buffer;
}

static const ReadWrite ftdiRW = {
  serial_read_fully,
  serial_write_fully
};

ReadWrite const * getFtdiReadWrite() {
  return &ftdiRW;
}