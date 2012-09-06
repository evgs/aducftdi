#include <stdio.h>
#include <ftdi.h>

#include "serial.h"

#include <ftdi.h>

#ifdef __MINGW32_VERSION

#include <windows.h>

void zzz(long time) {
  Sleep(time);
}

#else

void zzz(long time) {
  usleep(time*1000);
}

#endif
// Default VID:PID=0x0403:0x6001
int vid=0x0403;
int pid=0x6001;

int ftdi_list_all() {
    struct ftdi_context *fc = ftdi_new();
    
    if (fc == NULL) { fprintf(stderr, "ftdi_init failed\n"); return -1; }
    
    struct ftdi_device_list * devlist = NULL;
    struct ftdi_device_list * ptr;
    
    int count = ftdi_usb_find_all(fc, &devlist, vid, pid);
    
    switch (count) {
      case -1: fprintf(stderr, "usb_find_buses failed\n"); return -1; break;
      case -2: fprintf(stderr, "usb_find_devices failed\n"); return -1; break;
      case -3: fprintf(stderr, "out of memory\n"); return -1; break;
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
      printf("Entering bootloader mode...");
      zzz(2000);

      printf("[OK]\n");
      ftdi_set_bitmode(fc, 0, BITMODE_CBUS);
      zzz(250);
    }
  
    ftdi_disable_bitbang(fc);
}
  

int serial_read_fully(uint8_t *buffer, int count, int timeout)
{
/*	struct pollfd fds;
	int rc;
	int len;
	uint8_t *ptr = buffer;

	while (count > 0) {
		fds.fd = fd;
		fds.events = POLLIN;

		rc = poll(&fds, 1, timeout);
		if (rc == 0) {
			fprintf(stderr, "serial_read_fully: poll timeout\n");
			break;
		} else if (rc < 0) {
			fprintf(stderr, "serial_read_fully: poll failed %s\n", strerror(errno));
			return -1;
		}

		if (fds.revents & (POLLERR | POLLHUP | POLLNVAL)) {
			fprintf(stderr, "serial_read_fully: poll revents (%hx)\n", fds.revents);
			return -1;
		} else if (fds.revents & (POLLIN)) {
			len = read(fd, ptr, count);
			if (len < 0) {
				fprintf(stderr, "serial_read_fully: read failed %s\n", strerror(errno));
				return -1;
			}
			ptr += len;
			count -= len;
		}
	}

	return ptr - buffer;
*/
  
}

int serial_write_fully(const uint8_t *buffer, int count, int timeout)
{
/*	struct pollfd fds;
	int rc;
	int len;
	const uint8_t *ptr = buffer;

	while (count > 0) {
		fds.fd = fd;
		fds.events = POLLOUT;

		rc = poll(&fds, 1, timeout);
		if (rc == 0) {
			fprintf(stderr, "serial_write_fully: poll timeout\n");
			break;
		} else if (rc < 0) {
			fprintf(stderr, "serial_write_fully: poll failed %s\n", strerror(errno));
			return -1;
		}

		if (fds.revents & (POLLERR | POLLHUP | POLLNVAL)) {
			fprintf(stderr, "serial_write_fully: poll revents (%hx)\n", fds.revents);
			return -1;
		} else if (fds.revents & (POLLOUT)) {
			len = write(fd, ptr, count);
			if (len < 0) {
				fprintf(stderr, "serial_write_fully: write failed %s\n", strerror(errno));
				return -1;
			}
			ptr += len;
			count -= len;
		}
	}

	return ptr - buffer;
*/
  
}

static const ReadWrite ftdiRW = {
  serial_read_fully,
  serial_write_fully
};

ReadWrite const * getFtdiReadWrite() {
  return &ftdiRW;
}