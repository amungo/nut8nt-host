/* DMA Proxy
 *
 * This module is designed to be a small example of a DMA device driver that is
 * a client to the DMA Engine using the AXI DMA driver. It serves as a proxy for
 * kernel space DMA control to a user space application.
 *
 * A zero copy scheme is provided by allowing user space to mmap a kernel allocated
 * memory region into user space, referred to as a proxy channel interface. The
 * ioctl function is provided to start a DMA transfer which then blocks until the
 * transfer is complete. No input arguments are being used in the ioctl function.
 *
 * There is an associated user space application, dma_proxy_test.c, and dma_proxy.h
 * that works with this device driver.
 *
 * The hardware design was tested with an AXI DMA without scatter gather and
 * with the transmit channel looped back to the receive channel.
 *
 * This driver is character driver which creates 2 devices that user space can
 * access for each DMA channel, /dev/dma_proxy_rx and /dev/dma_proxy_tx.

 * An internal test mode is provided to allow it to be self testing without the 
 * need for a user space application and this mode is good for making bigger
 * changes to this driver.
 *
 * This driver is designed to be simple to help users get familiar with how to 
 * use the DMA driver provided by Xilinx which uses the Linux DMA Engine. The 
 * performance of this example is not expected to be high without more work.
 *
 * To use this driver a node must be added into the device tree.  Add the 
 * following node while adjusting the dmas property to match the name of 
 * the AXI DMA node.
 * 
 * dma_proxy {
 *    compatible ="xlnx,dma_proxy";
 *    dmas = <&axi_dma_0 0
 *            &axi_dma_0 1>;
 *    dma-names = "dma_proxy_tx", "dma_proxy_rx";
 * }
 *
 */

#include <linux/dmaengine.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kfifo.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/of_dma.h>
#include <linux/of_reserved_mem.h>

#include "dma-proxy.h"

//#define USE_OLD_FUNC

MODULE_LICENSE("GPL");

#define DRIVER_NAME 		"dma_proxy"
#define CHANNEL_COUNT 		1
#define RX_CHANNEL		0
#define TX_CHANNEL		1
#define ERROR 			-1
#define NOT_LAST_CHANNEL 	0
#define LAST_CHANNEL 		0
//#define EXP_DATA_SIZE       8192
#define BUFFER_NUM	8000
//#define SGLIST_SIZE         BUFFER_NUM * (BUFFER_SIZE / EXP_DATA_SIZE)

/* The following data structure represents a single channel of DMA, transmit or receive in the case
 * when using AXI DMA.  It contains all the data to be maintained for the channel.
 */

struct dma_proxy_channel {
    struct dma_proxy_channel_interface *interface_p;	/* user to kernel space interface */
    dma_addr_t interface_phys_addr;

    struct device *proxy_device_p;				/* character device support */
    struct device *dma_device_p;
    dev_t dev_node;
    struct cdev cdev;
    struct class *class_p;

    struct dma_chan *channel_p;				/* dma support */
    struct completion cmp;
    dma_cookie_t cookie;
    dma_addr_t dma_handle;
    u32 direction;						/* DMA_MEM_TO_DEV or DMA_DEV_TO_MEM */
    struct scatterlist sglist;
    struct scatterlist* sglistArray;
    uint8_t curret_buffer;
    volatile bool loop;
    pid_t signal_pid;
    uint32_t signal_data;
    uint32_t sig_per;
    uint32_t loopcount;
    uint32_t maxloops;
    int dmaBufFd;
};

/* Allocate the channels for this example statically rather than dynamically for simplicity.
 */
static struct dma_proxy_channel channels[CHANNEL_COUNT];
/* Handle a callback and indicate the DMA transfer is complete to another
 * thread of control
 */

struct task_struct	*task;
struct kernel_siginfo sinfo;
static void send_signal(struct dma_proxy_channel *pchannel_p, uint32_t signal_data){
    if(pchannel_p->signal_pid) {
        sinfo.si_signo = signal_data;
        //printk(KERN_INFO "DMA send_signal %d %d\n", (pchannel_p->loopcount) % pchannel_p->interface_p->buf_num, signal_data);

        if(task != NULL) {
            send_sig_info(SIGIO, &sinfo, task);
        }
    }
}


static void sync_callback(void *completion) {
    printk(KERN_INFO "[DMA] sync_callback\n");
    complete(completion);
}

static void cyclic_callback(void *data) {
    struct dma_proxy_channel *pchannel_p = data;
    pchannel_p->loopcount++;
    if ((pchannel_p->loopcount % pchannel_p->sig_per) == pchannel_p->sig_per - 1) {
        send_signal(pchannel_p, (pchannel_p->loopcount) % pchannel_p->interface_p->buf_num);
    }

    if (pchannel_p->maxloops != 0  && pchannel_p->loopcount == pchannel_p->maxloops) {
        pchannel_p->channel_p->device->device_terminate_all(pchannel_p->channel_p);
    }
}

#ifdef USE_OLD_FUNC

static void transfer_loop(struct dma_proxy_channel *pchannel_p);
static void sync_callback_loop(void *completion)
{
    struct dma_proxy_channel *pchannel_p = completion;
    //printk(KERN_INFO "DMA sync_callback_loop\n");
    pchannel_p->loopcount++;
    pchannel_p->interface_p->status = PROXY_NO_ERROR;
    if (pchannel_p->loop) {
        //printk(KERN_INFO "DMA sync_callback_loop loop == true\n");
        pchannel_p->curret_buffer = (pchannel_p->curret_buffer + 1)%BUFFER_NUM;

        transfer_loop(pchannel_p);

        //printk(KERN_INFO "DMA sync_callback_loop send signal\n");
        send_signal(pchannel_p, 0);
        //pchannel_p->callback_f();
    }
}

/* Prepare a DMA buffer to be used in a DMA transaction, submit it to the DMA engine
 * to ibe queued and return a cookie that can be used to track that status of the
 * transaction
 */
static void start_transfer(struct dma_proxy_channel *pchannel_p)
{
	enum dma_ctrl_flags flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
	struct dma_async_tx_descriptor *chan_desc;
	struct dma_proxy_channel_interface *interface_p = pchannel_p->interface_p;
	struct dma_device *dma_device = pchannel_p->channel_p->device;

	/* For now use a single entry in a scatter gather list just for future
	 * flexibility for scatter gather.
	 */
	sg_init_table(&pchannel_p->sglist, 1);
	sg_dma_address(&pchannel_p->sglist) = pchannel_p->dma_handle;
	sg_dma_len(&pchannel_p->sglist) = interface_p->length;

	chan_desc = dma_device->device_prep_slave_sg(pchannel_p->channel_p, &pchannel_p->sglist, 1, 
						pchannel_p->direction, flags, NULL);

	/* Make sure the operation was completed successfully
	 */
	if (!chan_desc) {
		printk(KERN_ERR "dmaengine_prep*() error\n");
	} else {
		chan_desc->callback = sync_callback;
		chan_desc->callback_param = &pchannel_p->cmp;

		/* Initialize the completion for the transfer and before using it
		 * then submit the transaction to the DMA engine so that it's queued
		 * up to be processed later and get a cookie to track it's status
		 */
		init_completion(&pchannel_p->cmp);

		pchannel_p->cookie = dmaengine_submit(chan_desc);
		if (dma_submit_error(pchannel_p->cookie)) {
			printk("Submit error\n");
	 		return;
		}

		/* Start the DMA transaction which was previously queued up in the DMA engine
		 */
		dma_async_issue_pending(pchannel_p->channel_p);
	}
}

#endif

/* Wait for a DMA transfer that was previously submitted to the DMA engine
 */
static void wait_for_transfer(struct dma_proxy_channel *pchannel_p)
{
	unsigned long timeout = msecs_to_jiffies(3000);
	enum dma_status status;

	pchannel_p->interface_p->status = PROXY_BUSY;

	/* Wait for the transaction to complete, or timeout, or get an error
	 */
	timeout = wait_for_completion_timeout(&pchannel_p->cmp, timeout);
	status = dma_async_is_tx_complete(pchannel_p->channel_p, pchannel_p->cookie, NULL, NULL);

	if (timeout == 0)  {
		pchannel_p->interface_p->status  = PROXY_TIMEOUT;
        printk(KERN_ERR "[DMA] timed out\n");
	} else if (status != DMA_COMPLETE) {
		pchannel_p->interface_p->status = PROXY_ERROR;
		printk(KERN_ERR "DMA returned completion callback status of: %s\n",
			   status == DMA_ERROR ? "error" : "in progress");
	} else
		pchannel_p->interface_p->status = PROXY_NO_ERROR;
}

/* Perform the DMA transfer for the channel, starting it then blocking to wait for completion.
 */

#ifdef USE_OLD_FUNC
static void transfer(struct dma_proxy_channel *pchannel_p)
{
	/* The physical address of the buffer in the interface is needed for the dma transfer
	 * as the buffer may not be the first data in the interface
	 */
	pchannel_p->dma_handle = (dma_addr_t)(pchannel_p->interface_phys_addr + 
                    offsetof(struct dma_proxy_channel_interface, buffer[0]));
    start_transfer(pchannel_p);
	wait_for_transfer(pchannel_p);
}

static void transfer_loop(struct dma_proxy_channel *pchannel_p)
{
    /* The physical address of the buffer in the interface is needed for the dma transfer
     * as the buffer may not be the first data in the interface
     */
    pchannel_p->dma_handle = (dma_addr_t)(pchannel_p->interface_phys_addr +
                    offsetof(struct dma_proxy_channel_interface, buffer[pchannel_p->curret_buffer]));


    enum dma_ctrl_flags flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
    struct dma_async_tx_descriptor *chan_desc;
    struct dma_proxy_channel_interface *interface_p = pchannel_p->interface_p;
    struct dma_device *dma_device = pchannel_p->channel_p->device;

    /* For now use a single entry in a scatter gather list just for future
     * flexibility for scatter gather.
     */
    sg_init_table(pchannel_p->sglistArray, interface_p->buf_num);
    unsigned int i;
    for (i=0; i < interface_p->buf_num; i++) {
        sg_dma_address(pchannel_p->sglistArray + i) = pchannel_p->dma_handle + i * interface_p->length;//(dma_addr_t)(pchannel_p->interface_phys_addr + offsetof(struct dma_proxy_channel_interface, buffer[i]));
        sg_dma_len(pchannel_p->sglistArray + i) = interface_p->length;
    }

    chan_desc = dma_device->device_prep_slave_sg(pchannel_p->channel_p, pchannel_p->sglistArray, interface_p->buf_num, pchannel_p->direction, flags, NULL);


    /* Make sure the operation was completed successfully
     */
    if (!chan_desc) {
        printk(KERN_ERR "dmaengine_prep*() error\n");
    } else {
        chan_desc->callback = sync_callback_loop;
        chan_desc->callback_param = pchannel_p;

        /* Initialize the completion for the transfer and before using it
         * then submit the transaction to the DMA engine so that it's queued
         * up to be processed later and get a cookie to track it's status
         */
        init_completion(&pchannel_p->cmp);

        pchannel_p->cookie = dmaengine_submit(chan_desc);
        if (dma_submit_error(pchannel_p->cookie)) {
            printk("Submit error\n");
            return;
        }

        /* Start the DMA transaction which was previously queued up in the DMA engine
         */
        dma_async_issue_pending(pchannel_p->channel_p);
    }
}

#endif

static void transfer_allb(struct dma_proxy_channel *pchannel_p)
{
    /* The physical address of the buffer in the interface is needed for the dma transfer
     * as the buffer may not be the first data in the interface
     */
    pchannel_p->dma_handle = (dma_addr_t)(pchannel_p->interface_phys_addr +
                    offsetof(struct dma_proxy_channel_interface, buffer));


    enum dma_ctrl_flags flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
    struct dma_async_tx_descriptor *chan_desc;
    struct dma_proxy_channel_interface *interface_p = pchannel_p->interface_p;
    struct dma_device *dma_device = pchannel_p->channel_p->device;

    /* For now use a single entry in a scatter gather list just for future
     * flexibility for scatter gather.
     */


    sg_init_table(pchannel_p->sglistArray, interface_p->buf_num);
    unsigned int i;
    for (i=0; i < interface_p->buf_num; i++) {
        sg_dma_address(pchannel_p->sglistArray + i) = pchannel_p->dma_handle + i * interface_p->length;//(dma_addr_t)(pchannel_p->interface_phys_addr + offsetof(struct dma_proxy_channel_interface, buffer[i]));
        sg_dma_len(pchannel_p->sglistArray + i) = interface_p->length;
    }

    chan_desc = dma_device->device_prep_slave_sg(pchannel_p->channel_p, pchannel_p->sglistArray, interface_p->buf_num, pchannel_p->direction, flags, NULL);

    if (!chan_desc) {
        printk(KERN_ERR "dmaengine_prep*() error\n");
    } else {
        chan_desc->callback = sync_callback;
        chan_desc->callback_param = &pchannel_p->cmp;

        init_completion(&pchannel_p->cmp);

        pchannel_p->cookie = dmaengine_submit(chan_desc);
        if (dma_submit_error(pchannel_p->cookie)) {
            printk("Submit error\n");
            return;
        }

        dma_async_issue_pending(pchannel_p->channel_p);
    }
    wait_for_transfer(pchannel_p);
}


static void transfer_cyclic(struct dma_proxy_channel *pchannel_p)
{
    /* The physical address of the buffer in the interface is needed for the dma transfer
     * as the buffer may not be the first data in the interface
     */
    printk("0\n");
    pchannel_p->dma_handle = (dma_addr_t)(pchannel_p->interface_phys_addr +
                    offsetof(struct dma_proxy_channel_interface, buffer));

    printk("1\n");
    enum dma_ctrl_flags flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
    struct dma_async_tx_descriptor *chan_desc;
    struct dma_proxy_channel_interface *interface_p = pchannel_p->interface_p;
    struct dma_device *dma_device = pchannel_p->channel_p->device;

    /* For now use a single entry in a scatter gather list just for future
     * flexibility for scatter gather.
     */

    printk("2\n");
    chan_desc = dma_device->device_prep_dma_cyclic(pchannel_p->channel_p, pchannel_p->dma_handle, interface_p->length * interface_p->buf_num, interface_p->length, pchannel_p->direction, flags);
    //chan_desc = dma_device->device_prep_slave_sg(pchannel_p->channel_p, pchannel_p->sglistArray, SGLIST_SIZE, pchannel_p->direction, flags, NULL);

    if (!chan_desc) {
        printk(KERN_ERR "dmaengine_prep*() error\n");
    } else {
	printk("3\n");
        chan_desc->callback = cyclic_callback;
        chan_desc->callback_param = pchannel_p;

	printk("4\n");
        init_completion(&pchannel_p->cmp);

        pchannel_p->cookie = dmaengine_submit(chan_desc);
        if (dma_submit_error(pchannel_p->cookie)) {
            printk("Submit error\n");
            return;
        }
	printk("5\n");

        dma_async_issue_pending(pchannel_p->channel_p);

	printk("6\n");
    }
}

/* Map the memory for the channel interface into user space such that user space can
 * access it using coherent memory which will be non-cached for s/w coherent systems
 * such as Zynq 7K or the current default for Zynq MPSOC. MPSOC can be h/w coherent
 * when set up and then the memory will be cached.
 */
static int mmap(struct file *file_p, struct vm_area_struct *vma)
{
	struct dma_proxy_channel *pchannel_p = (struct dma_proxy_channel *)file_p->private_data;

	return dma_mmap_coherent(pchannel_p->dma_device_p, vma,
					   pchannel_p->interface_p, pchannel_p->interface_phys_addr,
					   vma->vm_end - vma->vm_start);
}

/* Open the device file and set up the data pointer to the proxy channel data for the
 * proxy channel such that the ioctl function can access the data structure later.
 */
static int local_open(struct inode *ino, struct file *file)
{
	file->private_data = container_of(ino->i_cdev, struct dma_proxy_channel, cdev);
	return 0;
}

/* Close the file and there's nothing to do for it
 */
static int release(struct inode *ino, struct file *file)
{
	struct dma_proxy_channel *pchannel_p = (struct dma_proxy_channel *)file->private_data;
	struct dma_device *dma_device = pchannel_p->channel_p->device;

	/* Stop all the activity when the channel is closed assuming this
	 * may help if the application is aborted without normal closure
	 */

	dma_device->device_terminate_all(pchannel_p->channel_p);
	return 0;
}

/* Perform I/O control to start a DMA transfer.
 */
static long ioctl(struct file *file, unsigned int req , unsigned long arg)
{
    int ret = 0;
	struct dma_proxy_channel *pchannel_p = (struct dma_proxy_channel *)file->private_data;


    switch (req) {
#ifdef USE_OLD_FUNC
    case PROXY_DMA_SYNC_READ: {
            printk(KERN_INFO "DMA ioctl PROXY_DMA_SYNC_READ\n");
            if (pchannel_p->interface_p->status != PROXY_BUSY && pchannel_p->loop != true)
                transfer(pchannel_p);
            else
                printk(KERN_ERR "DMA ioctl PROXY_DMA_SYNC_READ PROXY_BUSY\n");
            }
        break;

    case PROXY_DMA_LOOP_START: {
            printk(KERN_INFO "DMA ioctl PROXY_DMA_LOOP_START\n");
            if (pchannel_p->loop != true) {
                pchannel_p->loop = true;
                pchannel_p->loopcount = 0;
                transfer_loop(pchannel_p);
            }
            else {
                printk(KERN_ERR "DMA ioctl PROXY_DMA_LOOP_START PROXY_BUSY\n");
            }
        }
        break;

    case PROXY_DMA_LOOP_STOP: {
            printk(KERN_INFO "DMA ioctl PROXY_DMA_LOOP_STOP %u\n", pchannel_p->loopcount);
            pchannel_p->loop = false;
        }
        break;
#endif

    case PROXY_DMA_SYNC_READ_ALL_BUFFERS: {
            printk(KERN_INFO "[DMA ioctl] PROXY_DMA_SYNC_READ_ALL_BUFFERS\n");
            transfer_allb(pchannel_p);

        }
        break;

    case PROXY_DMA_CYCLIC_START: {
            printk(KERN_INFO "[DMA ioctl] PROXY_DMA_CYCLIC_START\n");
            pchannel_p->loopcount = 0;
            pchannel_p->maxloops = arg;
            transfer_cyclic(pchannel_p);
        }
        break;

    case PROXY_DMA_CYCLIC_STOP: {
            pchannel_p->channel_p->device->device_terminate_all(pchannel_p->channel_p);
            printk(KERN_INFO "[DMA ioctl] PROXY_DMA_CYCLIC_STOP %u\n", pchannel_p->loopcount);
            ret = pchannel_p->loopcount;
        }
        break;

    case PROXY_DMA_SET_SIGNAL: {
            signal_parameters* sig_params = (signal_parameters*)arg;
            printk(KERN_INFO "[DMA ioctl] PROXY_DMA_SET_SIGNAL period = %d\n", sig_params->period);

            if(sig_params->on){
                pchannel_p->signal_pid = current->pid;
                pchannel_p->sig_per = sig_params->period;
                pchannel_p->signal_data = 0xff & (sig_params->data);
            } else {
                pchannel_p->signal_pid = 0;
            }

            memset(&sinfo, 0, sizeof(struct siginfo));
            sinfo.si_signo = SIGIO;
            sinfo.si_code  = SI_QUEUE;
            task = pid_task(find_vpid(pchannel_p->signal_pid), PIDTYPE_PID);
        }
        break;

#ifndef USE_OLD_FUNC
    case PROXY_DMA_SYNC_READ:
    case PROXY_DMA_LOOP_START:
    case PROXY_DMA_LOOP_STOP:
        printk(KERN_INFO "[DMA ioctl] Not enable req\n");
#endif
    default:
        printk(KERN_INFO "[DMA ioctl] what is it???\n");
        break;
    }

    return ret;
}
static struct file_operations dm_fops = {
	.owner    = THIS_MODULE,
	.open     = local_open,
	.release  = release,
	.unlocked_ioctl = ioctl,
	.mmap	= mmap
};


/* Initialize the driver to be a character device such that is responds to
 * file operations.
 */
static int cdevice_init(struct dma_proxy_channel *pchannel_p, char *name)
{
	int rc;
	char device_name[32] = "dma_proxy";
	static struct class *local_class_p = NULL;

	/* Allocate a character device from the kernel for this driver.
	 */
	rc = alloc_chrdev_region(&pchannel_p->dev_node, 0, 1, "dma_proxy");

	if (rc) {
		dev_err(pchannel_p->dma_device_p, "unable to get a char device number\n");
		return rc;
	}

	/* Initialize the device data structure before registering the character 
	 * device with the kernel.
	 */
	cdev_init(&pchannel_p->cdev, &dm_fops);
	pchannel_p->cdev.owner = THIS_MODULE;
	rc = cdev_add(&pchannel_p->cdev, pchannel_p->dev_node, 1);

	if (rc) {
		dev_err(pchannel_p->dma_device_p, "unable to add char device\n");
		goto init_error1;
	}

	/* Only one class in sysfs is to be created for multiple channels,
	 * create the device in sysfs which will allow the device node
	 * in /dev to be created
	 */
	if (!local_class_p) {
		local_class_p = class_create(THIS_MODULE, DRIVER_NAME);

		if (IS_ERR(pchannel_p->dma_device_p->class)) {
			dev_err(pchannel_p->dma_device_p, "unable to create class\n");
			rc = ERROR;
			goto init_error2;
		}
	}
	pchannel_p->class_p = local_class_p;

	/* Create the device node in /dev so the device is accessible
	 * as a character device
	 */
	strcat(device_name, name);
	pchannel_p->proxy_device_p = device_create(pchannel_p->class_p, NULL,
					  	 pchannel_p->dev_node, NULL, name);

	if (IS_ERR(pchannel_p->proxy_device_p)) {
		dev_err(pchannel_p->dma_device_p, "unable to create the device\n");
		goto init_error3;
	}

    pchannel_p->sglistArray = kmalloc(sizeof(struct scatterlist) * 1024 * 4, GFP_KERNEL);
    if (pchannel_p->sglistArray == 0) {
        printk("kmalloc for sglistArray error!\n");
        goto init_error3;
    }

	return 0;

init_error3:
	class_destroy(pchannel_p->class_p);

init_error2:
	cdev_del(&pchannel_p->cdev);

init_error1:
	unregister_chrdev_region(pchannel_p->dev_node, 1);
	return rc;
}

/* Exit the character device by freeing up the resources that it created and
 * disconnecting itself from the kernel.
 */
static void cdevice_exit(struct dma_proxy_channel *pchannel_p, int last_channel)
{
	/* Take everything down in the reverse order
	 * from how it was created for the char device
	 */
	if (pchannel_p->proxy_device_p) {
		device_destroy(pchannel_p->class_p, pchannel_p->dev_node);
		if (last_channel)
			class_destroy(pchannel_p->class_p);

		cdev_del(&pchannel_p->cdev);
		unregister_chrdev_region(pchannel_p->dev_node, 1);
	}
}

/* Create a DMA channel by getting a DMA channel from the DMA Engine and then setting
 * up the channel as a character device to allow user space control.
 */
static int create_channel(struct platform_device *pdev, struct dma_proxy_channel *pchannel_p, char *name, u32 direction)
{
	int rc;

	/* Request the DMA channel from the DMA engine and then use the device from
	 * the channel for the proxy channel also.
	 */
	pchannel_p->channel_p = dma_request_slave_channel(&pdev->dev, name);
	if (!pchannel_p->channel_p) {
		dev_err(pchannel_p->dma_device_p, "DMA channel request error\n");
		return ERROR;
	}
	pchannel_p->dma_device_p = &pdev->dev; 

	/* Initialize the character device for the dma proxy channel
	 */
	rc = cdevice_init(pchannel_p, name);
	if (rc) 
		return rc;


	pchannel_p->direction = direction;

	/* Allocate DMA memory for the proxy channel interface.
	 */

    pchannel_p->interface_p = (struct dma_proxy_channel_interface *)dmam_alloc_coherent(pchannel_p->dma_device_p, sizeof(struct dma_proxy_channel_interface), &pchannel_p->interface_phys_addr, GFP_KERNEL);
    printk(KERN_INFO "Allocating uncached memory at virtual address 0x%p, physical address 0x%p\n",  pchannel_p->interface_p, (void *)pchannel_p->interface_phys_addr);


	if (!pchannel_p->interface_p) {
		dev_err(pchannel_p->dma_device_p, "DMA allocation error\n");
		return ERROR;
	}
	return 0;
}

/* Initialize the dma proxy device driver module.
 */
static int dma_proxy_probe(struct platform_device *pdev)
{
	int rc;

	printk(KERN_INFO "dma_proxy module initialized\n");

	
	rc = create_channel(pdev, &channels[RX_CHANNEL], "dma_proxy_rx", DMA_DEV_TO_MEM);
	if (rc) 
		printk(KERN_INFO "create_channel_rx error %d\n", rc);
		//return rc;

	return 0;

}
 
/* Exit the dma proxy device driver module.
 */
static int dma_proxy_remove(struct platform_device *pdev)
{
	int i;

	printk(KERN_INFO "dma_proxy module exited\n");

	/* Take care of the char device infrastructure for each
	 * channel except for the last channel. Handle the last
	 * channel seperately.
	 */
	for (i = 0; i < CHANNEL_COUNT - 1; i++) 
		if (channels[i].proxy_device_p)
			cdevice_exit(&channels[i], NOT_LAST_CHANNEL);

	cdevice_exit(&channels[i], LAST_CHANNEL);

	/* Take care of the DMA channels and the any buffers allocated
	 * for the DMA transfers. The DMA buffers are using managed
	 * memory such that it's automatically done.
	 */
	for (i = 0; i < CHANNEL_COUNT; i++)
		if (channels[i].channel_p) {
			channels[i].channel_p->device->device_terminate_all(channels[i].channel_p);
			dma_release_channel(channels[i].channel_p);
		}
	return 0;
}

static const struct of_device_id dma_proxy_of_ids[] = {
	{ .compatible = "xlnx,dma_proxy",},
	{}
};

static struct platform_driver dma_proxy_driver = {
	.driver = {
		.name = "dma_proxy_driver",
		.owner = THIS_MODULE,
		.of_match_table = dma_proxy_of_ids,
	},
	.probe = dma_proxy_probe,
	.remove = dma_proxy_remove,
};

static int __init dma_proxy_init(void)
{
	return platform_driver_register(&dma_proxy_driver);

}

static void __exit dma_proxy_exit(void)
{
	platform_driver_unregister(&dma_proxy_driver);
}

module_init(dma_proxy_init)
module_exit(dma_proxy_exit)

MODULE_AUTHOR("Xilinx, Inc.");
MODULE_DESCRIPTION("DMA Proxy Prototype");
MODULE_LICENSE("GPL v2");
