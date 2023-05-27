### User space program example

This example of this user space program is taken from:

https://github.com/pmezydlo/SPI_slave_driver_implementation

https://github.com/pmezydlo/SPI_slave_driver_implementation/tree/master/slave_app

And then I did a quick hack to rework it to comply to the latest spi
kernel framework for the spi slave cases.

It does comply with spidev.h ioctl defined constants.

Please, do note that some code updates are done. The function:

	static int get_setting(int fd)

is defined as a debug function, used on the beginng of the setup.

Also, I disabled the transfer_8bit(fd) (write to spi-slave buffer)
function, since I am testing only spi-slave read() .

I placed spi-slave read() in endless while loop, since my use case is
to read endless back-to-back SPI master bursts via DMA engine (STM32
controller now sends correct patterns).

I replaced "spi-slave-dev.h" with croot/include/uapi/linux/spi/spidev.h

And for me as is, shown in this repo, does work as a proof of concept!

Please, do understand that this is a DVT slave_app user space program,
which must be run as root. SELinux must be set to the permissive mode.

### TO DO

Still, it does NOT comply to DMA transfer, but shows the bytes read.
