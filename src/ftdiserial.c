#include "serial.h"

int serial_read_fully(uint8_t *buffer, int count, int timeout)
{
	struct pollfd fds;
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
}

int serial_write_fully(const uint8_t *buffer, int count, int timeout)
{
	struct pollfd fds;
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
}

static const ReadWrite ftdiRW {
  serial_read_fully,
  serial_write_fully
};

ReadWrite const * getFtdiReadWrite() {
  return &ftdiRW;
}