### User space program example

This example of this user space program is taken from:

https://github.com/pmezydlo/SPI_slave_driver_implementation

https://github.com/pmezydlo/SPI_slave_driver_implementation/tree/master/slave_app

TO DO: to fix this program, since it does not comply 100% with the
recent croot/drivers/spi/spidev.c

Namely, it does not comply with ioctl defined constants.

Please, do note that some code updates are done. The function:

	static int get_setting(int fd)

is defined, but not used in main() program.

Also, I disabled the transfer_8bit(fd) (write to spi-slave buffer)
function, since I am testing only spi-slave read() .

I placed spi-slave read() in endless while loop, since my use case is
read endless (still problems with the FW of the STM32 controller).

I replaced "spi-slave-dev.h" with croot/include/uapi/linux/spi/spidev.c
