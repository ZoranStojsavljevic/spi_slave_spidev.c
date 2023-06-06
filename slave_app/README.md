### User space program example

This example of this user space program is initially taken from:

https://github.com/pmezydlo/SPI_slave_driver_implementation

https://github.com/pmezydlo/SPI_slave_driver_implementation/tree/master/slave_app

And then I did quite a lot of the changes to rework it to comply to
the latest spi kernel framework for the spi slave cases (including
the designated HW driver code as a 'controller' instance for i.MX8QM):

	croot/drivers/spi/spi-fsl-lpspi.c

It does comply with spidev.h ioctl defined constants (in the kernel
source tree, please, find the following):

	croot/include/uapi/linux/spi/spidev.h

Please, do note that some code updates are done. The function:

	static int get_setting(int fd)

is defined as a debug function, used on the begining of the setup.

Also, I disabled the transfer_8bit(fd) (write to spi-slave buffer)
function, since I am testing only spi-slave read() .

I placed spi-slave read() in endless while loop, since my use case
is to read endless single transfers SPI master bursts while disabling
DMA engine (STM32 controller finally sends correct patterns after a
correct Slave Select timing programmed on STM32 controller).

Replaced "spi-slave-dev.h" with croot/include/uapi/linux/spi/spidev.h

And for me as is, shown in this repo, does work as a very real app.
(please, again keep in mind that for other 'controllers' this app. is
a proof of concept (should be changed dictated for by real used HW
'controllers' accordingly)!

Please, do understand that this is STILL for the general users a DVT
slave_app user space program, which must be modified and tailored to
the users' use cases and must be run as a root.

The SELinux must be set to permissive mode.

The SELinux usage is NOT explained in this example!

### Android Android.bp file for slave_app compiling

	cc_binary {
		name: "slave_app",
		product_specific: true,
		// notice: "NOTICE",

		srcs: ["slave_app.c"],

		shared_libs: ["liblog"],
	}

### Target user space commands' examples

The slave_app help command:

	<device>:/ # slave_app -?
	Usage: /dev/spidev0.0 [-dbmlc?]
	  -d --device	device to use (default /dev/spidev0.0
	  -b --bpw	bits per word (default 8 bits)
	  -m --m	mode: 0, 1, 2 or 3
	  -l --l	lsb_first: 0-MSB, 1-LSB bit order
	  -c --c	maximum clock slave
	  -? --help	print help

The command I am using for this example usage is the following:

	<device>:/ # slave_app -d /dev/spidev0.0 -b 8 -m 0 -l 0 -c 1000000

	the device is /dev/spidev0.0
	the slave_app version is [1.07]
	Open:/dev/spidev0.0
	Device to use: /dev/spidev0.0
	LSB first [0-MSB, 1-LSB bit order]: 0
	MAX clock slave: 1000000
	Mode:0
	Bits per word: 8
	set SPI clock max to 1000000 Hz
	======= get_setting() =======
	set SPI mode to 0
	set SPI clock max to 1000000 Hz
	set SPI to MSB first
	set SPI bits per word to 8
	======= end get_setting() =======
	...


Please, go to source code of spi_slave_spidev.c/slave_app/slave_app.c
and get familiar with the DVT proof of concept!

Works (for my use case) like a charm!
