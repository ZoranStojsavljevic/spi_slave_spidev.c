### User space spidev_test.c program example

This example of this user space program is initially taken from:

https://github.com/STMicroelectronics/linux/blob/v5.15-stm32mp/tools/spi/spidev_test.c

### Android Android.bp file for spidev_test compiling

	cc_binary {
		name: "spidev_test",
		product_specific: true,
		// notice: "NOTICE",

		srcs: ["spidev_test.c"],

		shared_libs: ["liblog"],
	}

### Target user space commands' examples

The spidev_test help command:

	Usage: spidev_test [-DsbdlHOLC3vpNR24SI]
	  -D --device   device to use (default /dev/spidev1.1)
	  -s --speed    max speed (Hz)
	  -d --delay    delay (usec)
	  -b --bpw      bits per word
	  -i --input    input data from a file (e.g. "test.bin")
	  -o --output   output data to a file (e.g. "results.bin")
	  -l --loop     loopback
	  -H --cpha     clock phase
	  -O --cpol     clock polarity
	  -L --lsb      least significant bit first
	  -C --cs-high  chip select active high
	  -3 --3wire    SI/SO signals shared
	  -v --verbose  Verbose (show tx buffer)
	  -p            Send data (e.g. "1234\xde\xad")
	  -N --no-cs    no chip select
	  -R --ready    slave pulls low to pause
	  -2 --dual     dual transfer
	  -4 --quad     quad transfer
	  -S --size     transfer size
	  -I --iter     iterations

The command I am using for this example usage is the following:

	<device>:/ # spidev_test -D /dev/spidev0.0 -s 250000 -d 8 -b 8 -H 0 -O 0 -v -4 -S 256 -I 256

