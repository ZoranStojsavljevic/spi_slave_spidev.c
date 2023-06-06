## spi_slave_spidev.c

Nowadays, the linux spi framework does fully support the spi slave
framework, so no need to make own spi slave driver, since this is
the waist of the effort and time.

Here are the facts, in the modern kernels 5.x+, how to make it, no
matter that similar spi slave driver exists and it is is presented
here:

https://github.com/pmezydlo/SPI_slave_driver_implementation

This driver is written in Y2016., but I do believe that the feature I
am going to present existed even before (maybe I am over exadurating).

Patryk Mezydlo did, in Y2016. as 21y old kid, a great job giving to
us this spi slave driver.

### The things to think about

The main source of knowledge is outlined here:

https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git

If you look into the modern master branches linux-stable 5.x+
nowadays, you'll notice that spidev.c has everything which needs to
support the spi slave.

SPI HW wise has as a bare minimum only four (4) HW lines (one master
and a single slave - could be more than one slave).

Master Clock (driving both master and slave)
MISO (Master IN, Slave OUT)
MOSI (Master OUT, Slave IN)
CSx (Chip Select for selecting the spi slave), in this case x is 1.

The variable mode itself has four (4) SPI "clock modes":

From the croot/Documentation/spi/spi-summary.rst

#### What are these four SPI "clock modes"?

It's easy to be confused here, and the vendor documentation you'll
find isn't necessarily helpful.  The four modes combine two mode bits:

 - CPOL indicates the initial clock polarity.  CPOL=0 means the
   clock starts low, so the first (leading) edge is rising, and
   the second (trailing) edge is falling.  CPOL=1 means the clock
   starts high, so the first (leading) edge is falling.

 - CPHA indicates the clock phase used to sample data; CPHA=0 says
   sample on the leading edge, CPHA=1 means the trailing edge.

   Since the signal needs to stabilize before it's sampled, CPHA=0
   implies that its data is written half a clock before the first
   clock edge.  The chipselect may have made it become available.

Chip specs won't always say "uses SPI mode X" in as many words,
but their timing diagrams will make the CPOL and CPHA modes clear.

In the SPI mode number, CPOL is the high order bit and CPHA is the
low order bit.  So when a chip's timing diagram shows the clock
starting low (CPOL=0) and data stabilized for sampling during the
trailing clock edge (CPHA=1), that's SPI mode 1.

Note that the clock mode is relevant as soon as the chipselect goes
active.  So the master must set the clock to inactive before selecting
a slave, and the slave can tell the chosen polarity by sampling the
clock level when its select line goes active.  That's why many devices
support for example both modes 0 and 3:  they don't care about
polarity, and always clock data in/out on rising clock edges.

### SPI framework

SPI framework is positioned here: croot/include/linux/spi/spi.h

The file is: spi.h . I strongly advise to do here git blame spi.h to
get glimpse of a history, since this does help.

#### The four (4) very important structures

The four (4) very important structures you need to know about from spi
framework are outlined below:

	struct spi_device {
	struct spi_controller {	<<<======= underlying controller!
	struct spi_transfer {
	struct spi_message {

Please, do note that struct spi_message contains struct spi_device :

	struct spi_message {
		struct list_head	transfers;
		struct spi_device	*spi;		<<<=======

Please, read very carefully the description above all four structures.

It could be really helpful, but not instantly. Time requires for the
deffered brain processing.

#### struct spi_driver - Host side "protocol" driver

The comments about this struct spi_driver speaks for themselves.
Please, read it very carefully!

	/**
	 * struct spi_driver - Host side "protocol" driver
	 * @id_table: List of SPI devices supported by this driver
	 * @probe: Binds this driver to the spi device.  Drivers can verify
	 *	that the device is actually present, and may need to configure
	 *	characteristics (such as bits_per_word) which weren't needed for
	 *	the initial configuration done during system setup.
	 * @remove: Unbinds this driver from the spi device
	 * @shutdown: Standard shutdown callback used during system state
	 *	transitions such as powerdown/halt and kexec
	 * @driver: SPI device drivers should initialize the name and owner
	 *	field of this structure.
	 *
	 * This represents the kind of device driver that uses SPI messages to
	 * interact with the hardware at the other end of a SPI link.  It's called
	 * a "protocol" driver because it works through messages rather than talking
	 * directly to SPI hardware (which is what the underlying SPI controller
	 * driver does to pass those messages).  These protocols are defined in the
	 * specification for the device(s) supported by the driver.
	 *
	 * As a rule, those device protocols represent the lowest level interface
	 * supported by a driver, and it will support upper level interfaces too.
	 * Examples of such upper levels include frameworks like MTD, networking,
	 * MMC, RTC, filesystem character device nodes, and hardware monitoring.
	 */
	struct spi_driver {
		const struct spi_device_id *id_table;
		int			(*probe)(struct spi_device *spi);
		void			(*remove)(struct spi_device *spi);
		void			(*shutdown)(struct spi_device *spi);
		struct device_driver	driver;
	};

### croot/drivers/spi/spidev.c driver functions to be presented here

#### Underlying common spi framework for all spi drivers

Inherited from spi.h - struct spi_driver spidev_spi_driver .

Bare minimum spi framework for all spi drivers:

	static struct spi_driver spidev_spi_driver = {
		.driver = {
			.name =		"spidev",
			.of_match_table = spidev_dt_ids,
			.acpi_match_table = spidev_acpi_ids,
		},
		.probe =	spidev_probe,
		.remove =	spidev_remove,
		.id_table =	spidev_spi_ids,

		/*
		 * NOTE:  suspend/resume methods are not necessary here.
		 * We don't do anything except pass the requests to/from
		 * the underlying controller.  The refrigerator handles
		 * most issues; the controller driver handles the rest.
		 */
	};

##### croot/drivers/spi/spidev.c spidev_probe()

	static int spidev_probe(struct spi_device *spi)

This function allocates and imitializes the device structure and
associated with the device driver initialization. It also stars
parsing the device tree on several layers, to be able to bind the
slave drivers to the correct matching device tree hierarchy.

Here is shown first level of matching for the spi slave driver
(chapter: The solution).

##### croot/drivers/spi/spidev.c spidev_remove()

	static void spidev_remove(struct spi_device *spi)

Does the opposite to probe. Clears the driver's and device's data
structures.

#### croot/drivers/spi/spidev.c struct file_operations spidev_fops

This list comes from the structure file_operations, as well as two
important ones: spidev_init() and spidev_exit() .

	static const struct file_operations spidev_fops = {
		.owner =	THIS_MODULE,
		/* REVISIT switch to aio primitives, so that userspace
		 * gets more complete API coverage.  It'll simplify things
		 * too, except for the locking.
		 */
		.write =		spidev_write,
		.read =			spidev_read,
		.unlocked_ioctl =	spidev_ioctl,
		.compat_ioctl =		spidev_compat_ioctl,
		.open =			spidev_open,
		.release =		spidev_release,
		.llseek =		no_llseek,
	};

##### croot/drivers/spi/spi.c spi_init() (from croot/drivers/spi/spi.c)

This function comes from the spi.c bus and bus class framework, which
is an underlying foundation for supporting device and driver models.

There is only a function spi_init(), no spi_exit(), since spi_init()
stays alive for the existance of the kernel.

	static int __init spi_init(void)
	{
		int	status;

		buf = kmalloc(SPI_BUFSIZ, GFP_KERNEL);
		if (!buf) {
			status = -ENOMEM;
			goto err0;
		}

		status = bus_register(&spi_bus_type);
		if (status < 0)
			goto err1;

		status = class_register(&spi_master_class);
		if (status < 0)
			goto err2;

		if (IS_ENABLED(CONFIG_SPI_SLAVE)) {
			status = class_register(&spi_slave_class);
			if (status < 0)
				goto err3;
		}

		if (IS_ENABLED(CONFIG_OF_DYNAMIC))
			WARN_ON(of_reconfig_notifier_register(&spi_of_notifier));
		if (IS_ENABLED(CONFIG_ACPI))
			WARN_ON(acpi_reconfig_notifier_register(&spi_acpi_notifier));

		return 0;

	err3:
		class_unregister(&spi_master_class);
	err2:
		bus_unregister(&spi_bus_type);
	err1:
		kfree(buf);
		buf = NULL;
	err0:
		return status;
	}

##### croot/drivers/spi/spidev.c spidev_init()

Classical character device init:

	static int __init spidev_init(void)
	{
		int status;

		/* Claim our 256 reserved device numbers.  Then register a class
		 * that will key udev/mdev to add/remove /dev nodes.  Last, register
		 * the driver which manages those device numbers.
		 */
		status = register_chrdev(SPIDEV_MAJOR, "spi", &spidev_fops);
		spidev_class = class_create("spidev");
		status = spi_register_driver(&spidev_spi_driver);

		return status;
	}
	module_init(spidev_init);

##### croot/drivers/spi/spidev.c spidev_exit()

Classical character device exit, unregistration in reverse order:

	static void __exit spidev_exit(void)
	{
		spi_unregister_driver(&spidev_spi_driver);
		class_destroy(spidev_class);
		unregister_chrdev(SPIDEV_MAJOR, spidev_spi_driver.driver.name);
	}
	module_exit(spidev_exit);

##### croot/drivers/spi/spidev.c spidev_open()

	static int spidev_open(struct inode *inode, struct file *filp)

##### croot/drivers/spi/spidev.c spidev_read()

User space read(): copy_to_user(buf, spidev->rx_buffer, status)

	/* Read-only message with current device setup */
	static ssize_t
	spidev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
	{
		struct spidev_data	*spidev;
		ssize_t			status;

		/* chipselect only toggles at start or end of operation */
		if (count > bufsiz)
			return -EMSGSIZE;

		spidev = filp->private_data;

		mutex_lock(&spidev->buf_lock);
		status = spidev_sync_read(spidev, count);
		if (status > 0) {
			unsigned long	missing;

			missing = copy_to_user(buf, spidev->rx_buffer, status);
			if (missing == status)
				status = -EFAULT;
			else
				status = status - missing;
		}
		mutex_unlock(&spidev->buf_lock);

		return status;
	}

#### croot/drivers/spi/spidev.c spidev_ioctl()

	static long spidev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)

Some of the constants used in spidev_ioctl():

	SPI_IOC_RD_MODE			Mode of spi operation: RD mode

	SPI_IOC_RD_LSB_FIRST		ReaD Low Significant Bit of SPI data FIRST
				OR	ReaD High Significant Bit of SPI data FIRST

	SPI_IOC_RD_BITS_PER_WORD	Number of bits per spi data word

	SPI_IOC_RD_MAX_SPEED_HZ		Maximum clock speed for RD xfer
					(usually the same clock speed for WR xfer)

	SPI_IOC_WR_MODE			Mode of spi operation: WR mode

	SPI_IOC_WR_LSB_FIRST		WRite Low Significant Bit of SPI data FIRST
				OR	WRite High Significant Bit of SPI data FIRST

	SPI_IOC_WR_BITS_PER_WORD	Number of bits per spi data word

	SPI_IOC_WR_MAX_SPEED_HZ		Maximum clock speed for WR xfer
					(usually the same clock speed for RD xfer)

##### croot/drivers/spi/spidev.c spidev_write()

User space write(): copy_from_user(spidev->tx_buffer, buf, count)

	/* Write-only message with current device setup */
	static ssize_t
	spidev_write(struct file *filp, const char __user *buf,
			size_t count, loff_t *f_pos)
	{
		struct spidev_data	*spidev;
		ssize_t			status;
		unsigned long		missing;

		/* chipselect only toggles at start or end of operation */
		if (count > bufsiz)
			return -EMSGSIZE;

		spidev = filp->private_data;

		mutex_lock(&spidev->buf_lock);
		missing = copy_from_user(spidev->tx_buffer, buf, count);
		if (missing == 0)
			status = spidev_sync_write(spidev, count);
		else
			status = -EFAULT;

		mutex_unlock(&spidev->buf_lock);

		return status;
	}

##### croot/drivers/spi/spidev.c spidev_release()

	static int spidev_release(struct inode *inode, struct file *filp)
	{
		struct spidev_data	*spidev;
		int			dofree;

		mutex_lock(&device_list_lock);
		spidev = filp->private_data;
		filp->private_data = NULL;

		mutex_lock(&spidev->spi_lock);
		/* ... after we unbound from the underlying device? */
		dofree = (spidev->spi == NULL);
		mutex_unlock(&spidev->spi_lock);

		/* last close? */
		spidev->users--;
		if (!spidev->users) {
			kfree(spidev->tx_buffer);
			spidev->tx_buffer = NULL;

			kfree(spidev->rx_buffer);
			spidev->rx_buffer = NULL;

			if (dofree)
				kfree(spidev);
			else
				spidev->speed_hz = spidev->spi->max_speed_hz;
		}
	#ifdef CONFIG_SPI_SLAVE
		if (!dofree)
			spi_slave_abort(spidev->spi);
	#endif
		mutex_unlock(&device_list_lock);

		return 0;
	}

### Source of the Confusion

The confusion comes from croot/drivers/spi/Kconfig, from the following
statement:

	config SPI_MASTER
	.
	.
	.
	comment "SPI Protocol Masters"

	config SPI_SPIDEV
		tristate "User mode SPI device driver support"
		help
		  This supports user mode SPI protocol drivers.
	.
	.
	.
	endif # SPI_MASTER

CONFIG_SPI_SPIDEV includes spidev.c. From croot/drivers/spi/Makefile

	obj-$(CONFIG_SPI_SPIDEV)		+= spidev.o

In fact, spidev.c also does support user mode spi slave driver!

Now I will try to prove this statement!

### The solution (for now just as a concept)

The solution is in the device tree source definition for the targeted
silicon. The correct spi slave device tree source definition.

The directory where the dtses exist is the following:
croot/arch/arm64/boot/dts/[vendor]/[device]

And here is the first level of match device tree source (spi 0 slave):

	&lpspi0 {
		#address-cells = <1>;
		#size-cells = <0>;
		fsl,spi-num-chipselects = <1>;
		pinctrl-names = "default";
		pinctrl-0 = <&pinctl_spi0>;
		cs-gpios = <&lsio_gpio3 5 GPIO_ACTIVE_LOW>;
		status = "okay";
		spi-slave;				<<<======= SPI SLAVE definition

		slave@0 {				<<<======= mandatory word "slave"
			compatible = "var,spidev";	<<<======= compatible to match string
			reg = <0>;
			status = "okay";
		};
	};

The entry "spi-max-frequency = xy000000" is deleted, since master is
the one who dictates the frequency, slave has the maximum sampling
rate given to resample (usually lower than the master, but these two
master and slave frequences must match!

### The results (for Android 12)

#### /sys/class directory

	<A12 device>:/sys/class # ls -al spi*
	spi_master:
	lrwxrwxrwx  1 root root 0 2023-05-11 13:19 spi1 -> ../../devices/platform/bus@5a000000/5a020000.spi/spi_master/spi1

	spi_slave:
	lrwxrwxrwx  1 root root 0 2023-05-11 13:19 spi0 -> ../../devices/platform/bus@5a000000/5a000000.spi/spi_slave/spi0

	spidev:
	lrwxrwxrwx  1 root root 0 2023-05-11 13:19 spidev0.0 -> ../../devices/platform/bus@5a000000/5a000000.spi/spi_slave/spi0/spi0.0/spidev/spidev0.0
	lrwxrwxrwx  1 root root 0 2023-05-11 13:19 spidev1.0 -> ../../devices/platform/bus@5a000000/5a020000.spi/spi_master/spi1/spi1.0/spidev/spidev1.0

#### /dev/spi*

	<A12 device>/ # ls -al /dev/spi*
	crw------- 1 root root 153,   0 1970-01-01 01:00 /dev/spidev0.0		<<===== spi slave device
	crw------- 1 root root 153,   1 1970-01-01 01:00 /dev/spidev1.0		<<===== spi master device

#### Debug tools (example shown)

Please, you should use debug tools to debug any driver.

Example given for i.MX8 where lpspi0 is @ address 0x5a000000
(from /dev/spi*):

	busybox devmem 0x5a000000 32	// lpspi version number 32 bit reg 0
	0x01010004

Please, consult NXP i.MX8QM documentation.

