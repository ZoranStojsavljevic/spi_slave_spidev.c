### Kernel used

All the patches are related to the following NXP Android kernel:

	$ uname -a
	Linux localhost 5.4.47 #49 SMP PREEMPT

### LPSPI Driver used for i.MX8 silicon

All the patches to be applied are related to the following SPI driver:

	croot/drivers/spi/spi-fsl-lpspi.c

The spi driver spi-fsl-lpspi.c from kernel 5.4.47 is presented in the
directory as a complete file.

### LPSPI Driver settings

#### DMA mode

	- DMA is used for bursts (Slave Select always active low)
	- Related lpspi slave mode INT interrupts disabled
	- lpspi Configuration Register's 1 (CFGR1) bits 25-24 to
	  be set to 00b (PINCFG bits) described in the shown patch
	- lpspi Transmit Command Register (TCR) bit 21 to be set
	  to 1b (CONT bit)

#### INT mode

Please, do note that there is an another mode of operation: INT mode!

This is the mode for the single 8 bit transfer, using rising edge of
Slave Select (SS).

	- DMA disabled
	- Related lpspi slave mode INT interrupts enabled
	- lpspi Configuration Register's 1 (CFGR1) bits 25-24 to
	  be set to 00b (PINCFG bits) described in the shown patch
	- lpspi Transmit Command Register (TCR) bit 21 to be set
	  to 0b (CONT bit)

### Patches for DMA burst transfer

There are three patches for SPI Slave driver to properly work to be
applied for DMA mode:

Given patch for the lpspi Configuration Register's 1 (CFGR1):

	CFGR1_PINCFG.patch

Two patches for the bit TCR_CONT (BIT(21)) are NOT given here. It is
up to readers of this repo to find what needs to be changed for the
TCR_CONT.

To recap (from NXP's i.MX8 documentation):

	17.7.3.1.15 Transmit Command Register (TCR)

	Bit 21 CONT Continuous Transfer

	• In master mode, continuous transfer will keep the CS
	  asserted at the end of the frame size, until a command
	  word is received that starts a new frame.

	• In slave mode, when continuous transfer is enabled, the
	  LPSPI will only transmit the first FRAMESZ bits; after
	  which the LPSPI will transmit received data (assuming a
	  32-bit shift register) until the next CS negation.

	0b - Continuous transfer is disabled
	1b - Continuous transfer is enabled

### Latest development on the LPSPI Driver settings

DMA mode is disabled, and some PIN_CTRL definitions in .dtsi are
changed accordingly for the drive controller:

	croot/drivers/spi/spi-fsl-lpspi.c

and:

	croot/arch/arm64/boot/dts/freescale/*.dtsi

These changes are NOT to be shown here. It is a deep dig proprietary
job done!
