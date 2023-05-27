#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#define FD_INVAL (-1)

#define TX_ARRAY_SIZE	8
#define RX_ARRAY_SIZE	1

static bool running = false;

static const char *device = "/dev/spidev0.0";

static uint32_t tx_actual_length;
static uint32_t rx_actual_length;

static uint8_t	bits_per_word = 8;
static uint8_t	mode;
static uint32_t	clk_max;
static uint32_t	bytes_per_load = 4;

static int transfer_8bit(int fd)
{
	int ret = 0;
	uint8_t tx[] = {'D', 'E', 'A', 'D', 'B', 'E', 'F', 'F'};
	int i;

	ret = write(fd, tx, TX_ARRAY_SIZE);

	if (ret < 0) {
		printf("Failed to write massage!\n");
		return -1;
	}

	printf("Transmit:\n");

	for (i = 0; i < ret; i++) {
		printf("0x%02X ", tx[i]);

		if (i%8 == 7)
			printf("\n");
	}

	return ret;
}

static int put_setting(int fd)
{
	int ret = 0;

	if (FD_INVAL == fd) {
		printf("SPI device '%s' not opened\n", device);
		ret = EBADF;
	} else {
		printf("set SPI clock max to %d Hz\n", clk_max);
		ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &(clk_max));
		if (0 > ret) {
			printf("unable to set SPI clock max to %d Hz: %s\n", clk_max,
					strerror(errno));
			return errno;
		}

		ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word);
		if (0 > ret) {
			printf("unable to set bits per word to %d, %s\n", bits_per_word,
					strerror(errno));
			return errno;
		}

		ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
		if (0 > ret) {
			printf("unable to set mode to %d, %s\n", mode,
					strerror(errno));
			return errno;
		}
	}

	return ret;
}

static int get_setting(int fd)
{
	char		buffer[33];
	int		ret;
	uint32_t	return_arg;

	printf("======= get_setting() =======\n");

	if (FD_INVAL == fd) {
		printf("SPI device '%s' not opened\n", device);
		ret = EBADF;
	} else {
		ret = ioctl(fd, SPI_IOC_RD_MODE, (uint8_t *)&return_arg);
		if (0 > ret) {
			printf("unable to read SPI mode, %s\n", strerror(errno));
			ret = errno;
			goto out;
		}
		printf("set SPI mode to %d\n", (uint8_t)return_arg);

		ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, (uint32_t *)&return_arg);
		if (0 > ret) {
			printf("unable to read SPI clock max, %s\n", strerror(errno));
			ret = errno;
			goto out;
		}
		sprintf(buffer, "%u", return_arg);
		printf("set SPI clock max to %s Hz\n", buffer);

		ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, (uint8_t *)&return_arg);
		if (0 > ret) {
			printf("unable to read SPI bit per word, %s\n", strerror(errno));
			ret = errno;
			goto out;
		}
		sprintf(buffer, "%d", (uint8_t)return_arg);
		printf("set SPI bits per word to %s\n", buffer);
	}

out:

	printf("======= end get_setting() =======\n");
	return ret;
}

static void print_setting(void)
{
	printf("TX length:%d, RX length:%d, Bits per word:%d\n",
	       tx_actual_length, rx_actual_length, bits_per_word);
	printf("Mode:%d\n", mode);
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-dbs?ewm]\n", prog);
	puts("  -d --device	device to use (default /dev/spidev0.0\n"
	     "  -b --bpw	bits per word (default 8 bits)\n"
	     "  -m --m		mode: 0, 1, 2 or 3\n"
	     "  -c --c		maximum clock slave\n"
	     "  -? --help	print help\n"
	     "\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",	required_argument,	0, 'd' },
			{ "bpw",    	required_argument,	0, 'b' },
			{ "mode",   	required_argument,	0, 'm' },
			{ "clk_max",	required_argument,	0, 'c' },
			{ "help",	no_argument,		0, '?' },
			{ NULL,		0,			0,  0  },
		};
		int c;

		c = getopt_long(argc, argv, "d:b:m:c:?", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'd':
			device = optarg;
			break;
		case 'b':
			bits_per_word = atoi(optarg);
			break;
		case 'm':
			mode = atoi(optarg);
			break;
		case 'c':
			clk_max = atoi(optarg);
			break;
		case '?':
			print_usage(device);
			break;
		default:
			break;
		}
	}
}

static int read_8bit(int fd)
{
	uint8_t rx[RX_ARRAY_SIZE];
	int ret;
	int i;
	uint32_t length;

	// Just for debugging purposes
	// get_setting(fd);

	printf("Receive:");

	length = sizeof(rx);
	// printf("the length is %d\n", length);

	ret = read(fd, rx, length);
	if (ret < 0) {
		printf("failed to read the message!\n");
		return -1;
	}

	for (i = 0; i < length; i++) {
		printf("0x%.2X\n", rx[i]);

//		if (i%8 == 7)
//			printf("\n");
	}
	return ret;
}

int main(int argc, char *argv[])
{
	int	ret = 0;
	int	fd = FD_INVAL;

	parse_opts(argc, argv);
	printf("the device is %s\n", device);
	printf("the slave_app version is [2]\n");

	fd = open(device, O_RDWR);

	if (fd < 0) {
		printf("Failed to open the device!\n");
		return -1;
	}

	printf("Open:%s\n", device);

	if (ret == -1)
		printf("Can't write bits per word\n");

	print_setting();

	ret = put_setting(fd);
	if (ret == -1)
		return -1;

	get_setting(fd);

	running = true;

	while (running) {
#if 0 // Testing only spi_slave read functionality
		ret = transfer_8bit(fd);
		if (ret == -1) {
			printf("Failed to write");
			return -1;
		}
#endif
		ret = read_8bit(fd);
		if (ret < 0) {
			printf("Failed to read!\n");
			return -1;
		}
	}

	close(fd);

	return ret;
}
