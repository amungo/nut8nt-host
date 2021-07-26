/*
 * nutdma module -- Simple zero-copy DMA to/from userspace for
 * dmaengine-compatible hardware.
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/param.h>  /* HZ */
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>

#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/wait.h>

#include "ntdma.h"
#include "ntdma_ioctl.h"

static int ntdma_open(struct inode* inode, struct file* filp);
static int ntdma_release(struct inode* inode, struct file* filp);
static long ntdma_ioctl(struct file* file, unsigned int cmd, unsigned long arg);
static int ntdma_mmap(struct file* file, struct vm_area_struct* vma);

static void teardown_devices( struct ntdma_pdev_drvdata* p_pdev_info, struct platform_device* pdev);
static void ntdma_transfer_destroy(struct ntdma_drvdata* p_info, struct ntdma_transfer* xfer);

static int ntdma_cyclic_transfer_teardown( struct ntdma_drvdata* p_info );

static const struct file_operations ntdma_fops = {
    .owner          = THIS_MODULE,
    .open           = ntdma_open,
    .unlocked_ioctl = ntdma_ioctl,
    .release        = ntdma_release,
    .mmap           = ntdma_mmap,
};

#define NUM_DEVICE_NUMBERS_TO_ALLOCATE (8)
static dev_t base_devno;
static int devno_in_use[NUM_DEVICE_NUMBERS_TO_ALLOCATE];
static struct class* ntdma_class;
static DEFINE_SEMAPHORE(devno_lock);

static inline int get_free_devno(dev_t* p_dev)
{
    int i;
    int rv = -ENODEV;

    down( &devno_lock );

    for (i = 0; i < NUM_DEVICE_NUMBERS_TO_ALLOCATE; i++ )
    {
        if ( !devno_in_use[i] )
        {
            *p_dev = MKDEV( MAJOR(base_devno), i );
            devno_in_use[i] = 1;
            rv = 0;
            break;
        }
    }

    up( &devno_lock );

    return rv;
}

static inline int put_devno(dev_t dev)
{
    down( &devno_lock );

    BUG_ON( 0 == devno_in_use[ MINOR(dev) ] );
    devno_in_use[ MINOR(dev) ] = 0;

    up( &devno_lock );

    return 0;
}

static bool ntdma_access_ok(const void __user* arg, size_t size, bool readonly)
{
    if(!readonly && !access_ok(VERIFY_WRITE, arg, size)) {
        pr_err("Argument address %p, size %zu cannot be written to. \n", arg, size);
        return false;
    } else if(!access_ok(VERIFY_READ, arg, size)) {
        pr_err("Argument address %p, size %zu cannot be read from. \n", arg, size);
        return false;
    }

    return true;
}

static int engine_ring_process(struct ntdma_drvdata* p_info)
{
    int start;
    bool wake_up = false;

    /* where we start receiving in the ring buffer */
    start = p_info->rx_head;

    /* check for waiting data  */
    wake_up = (p_info->rx_head == p_info->rx_tail) || (p_info->wait_rx_data == true);

    /* increment head pointer */
    p_info->rx_head = (p_info->rx_head + 1) % p_info->cyclic_desc_count;

    pr_debug("%s, head %d, tail %d.\n", p_info->name, p_info->rx_head, p_info->rx_tail);

    /* overrun? */
    if (p_info->rx_head == p_info->rx_tail) {
            printk("%s: overrun. rx_head: %d, rx_tail: %d \n", p_info->name, p_info->rx_head, p_info->rx_tail);
            /* flag to user space that overrun has occurred */
            p_info->rx_overrun += 1;
            p_info->rx_tail = (p_info->rx_tail + 1) % p_info->cyclic_desc_count;
    }

    if(wake_up) {
        wake_up_interruptible(&p_info->transfer_wq);
    }

    return 0;
}

// this runs in tasklet (interrupt) context -- no sleeping!
static void ntdma_dmaengine_callback_func(void* data)
{
    struct ntdma_drvdata* p_info = (struct ntdma_drvdata*)data;
    unsigned long iflags;
    struct ntdma_transfer* xfer;
    int rc;

    //printk( KERN_ERR KBUILD_MODNAME "*** : %s: callback fired for %s ***\n", p_info->name, "TX" );
    spin_lock_irqsave(&p_info->state_lock, iflags);

    if(p_info->current_transfer != &p_info->cyclic_transfer_list)
    {
        pr_debug("ntdma_dmaengine_callback_func() set next current transfer [engine->current_transfer] \n");
        xfer = list_entry(p_info->current_transfer, struct ntdma_transfer, entry);
        xfer->state = TRANSFER_STATE_COMPLETED;

        if( p_info->state == DMA_IN_FLIGHT)
        {
            //--------------------------------------------------------------------------------
            xfer->txn_desc = dmaengine_prep_slave_sg(
                        p_info->chan,
                        &xfer->sglist,
                        1,
                        DMA_DEV_TO_MEM,
                        DMA_CTRL_ACK | DMA_PREP_INTERRUPT);    // run callback after this one

            if ( !xfer->txn_desc )
            {
                printk( KERN_ERR KBUILD_MODNAME ": %s: dmaengine_prep_slave_sg() failed\n", p_info->name);
                rc = -ENOMEM;
                goto err_out;
            }

            xfer->txn_desc->callback = ntdma_dmaengine_callback_func;
            xfer->txn_desc->callback_param = p_info;

            xfer->cookie = dmaengine_submit(xfer->txn_desc);
            if( (rc = dma_submit_error(xfer->cookie )) ) {
                printk( KERN_ERR KBUILD_MODNAME ": %s: dmaengine_submit() returned %d\n", p_info->name, xfer->cookie);
                goto err_out;
            }

            dma_async_issue_pending( p_info->chan );
            engine_ring_process(p_info);
        }
        else {
            if(++p_info->xfer_complited == p_info->cyclic_desc_count)
                complete(&p_info->cmp);
        }

        p_info->current_transfer = (p_info->current_transfer->next != &p_info->cyclic_transfer_list) ? p_info->current_transfer->next : p_info->cyclic_transfer_list.next;
    }

        //p_info->state = DMA_COMPLETING;
        //wake_up_interruptible( &p_info->wq );
    // else: well, nevermind then...
err_out:
    spin_unlock_irqrestore(&p_info->state_lock, iflags);
}

static int ntdma_open(struct inode* inode, struct file* filp)
{
    struct ntdma_drvdata* p_info = container_of(inode->i_cdev, struct ntdma_drvdata, ntdma_cdev);
    int rv = 0;

    if( down_interruptible( &p_info->sem ))
        return -ERESTARTSYS;

    if( p_info->in_use )
    {
        rv = -EBUSY;
    }
    else
    {
        p_info->in_use = 1;
        filp->private_data = p_info;
        atomic_set( &p_info->accepting, 1 );
    }
    up( &p_info->sem );

    return rv;
}


static int ntdma_release(struct inode* inode, struct file* filp)
{
    struct ntdma_drvdata* p_info = container_of(inode->i_cdev, struct ntdma_drvdata, ntdma_cdev);

    atomic_set( &p_info->accepting, 0 );    // disallow new reads/writes

    if ( down_interruptible( &p_info->sem ) )
        return -ERESTARTSYS;

    ntdma_cyclic_transfer_teardown(p_info);

    // TODO: wake up any sleeping threads?

    p_info->in_use = 0;

    up( &p_info->sem );

    return 0;
}

static int ntdma_create_device( struct ntdma_drvdata* p_info )
{
    int rv;

    if ( (rv = get_free_devno( &p_info->ntdma_devt )) )
    {
        printk( KERN_ERR KBUILD_MODNAME ": get_free_devno() returned %d\n", rv);
        return rv;
    }

    cdev_init( &p_info->ntdma_cdev, &ntdma_fops );
    p_info->ntdma_cdev.owner = THIS_MODULE;

    if ( (rv = cdev_add( &p_info->ntdma_cdev, p_info->ntdma_devt, 1 )) )
    {
        printk(KERN_ERR KBUILD_MODNAME ": cdev_add() returned %d\n", rv);
        put_devno(p_info->ntdma_devt);
        p_info->ntdma_devt = MKDEV(0,0);
        return rv;
    }

    if ( NULL == (p_info->ntdma_dev = device_create( ntdma_class,
                              &p_info->pdev->dev,
                              p_info->ntdma_devt,
                              p_info,
                              p_info->name)))
    {
        printk(KERN_ERR KBUILD_MODNAME ": device_create() failed\n");
        cdev_del( &p_info->ntdma_cdev );
        put_devno( p_info->ntdma_devt );
        p_info->ntdma_devt = MKDEV(0,0);
        return -ENOMEM;
    }

    return 0;
}

static void ntdma_teardown_device( struct ntdma_drvdata* p_info )
{
    device_destroy( ntdma_class, p_info->ntdma_devt );
    cdev_del( &p_info->ntdma_cdev );
    put_devno( p_info->ntdma_devt );
    p_info->ntdma_devt = MKDEV(0,0);
}

static int ntdma_cyclic_transfer_teardown( struct ntdma_drvdata* p_info )
{
    struct list_head* pos, *cur_pos;
    struct ntdma_transfer* xfer;
    unsigned long flags;
    enum dma_status status;
    unsigned long timeout = msecs_to_jiffies(3000);

    spin_lock_irqsave(&p_info->state_lock, flags);

    //---------------------  Clear transactions  ------------------------
    if(!list_empty(&p_info->cyclic_transfer_list)) {
        p_info->state = DMA_COMPLETING;
        init_completion(&p_info->cmp);
        p_info->xfer_complited = 0;
        spin_unlock_irqrestore(&p_info->state_lock, flags);
        timeout = wait_for_completion_timeout(&p_info->cmp, timeout);
        spin_lock_irqsave(&p_info->state_lock, flags);
        if(timeout == 0) {
            printk(KERN_ERR "--- Complited DMA timedout ---\n");
        }

        list_for_each_safe(pos, cur_pos, &p_info->cyclic_transfer_list) {
            xfer = list_entry(pos, struct ntdma_transfer, entry);
            status = dma_async_is_tx_complete(p_info->chan, xfer->cookie, NULL, NULL);
            if (status != DMA_COMPLETE) {
                printk(KERN_ERR "DMA returned completion callback status of: %s \n", status == DMA_ERROR ? "error" : "in progress");
            }
            else
                printk("Xfer DMA Complited %p \n", xfer);

            list_del(pos);
            ntdma_transfer_destroy(p_info, xfer);
        }
    }

    p_info->current_transfer = NULL;
    pr_debug("Transfer_teardown, set engine->current_transfer: NULL\n");
    //-------------------------------------------------------------------
    p_info->state = DMA_IDLE;
    dmaengine_terminate_sync(p_info->chan);


    spin_unlock_irqrestore(&p_info->state_lock, flags);

    return 0;
}

static void teardown_devices( struct ntdma_pdev_drvdata* p_pdev_info, struct platform_device* pdev);

static int create_devices( struct ntdma_pdev_drvdata* p_pdev_info, struct platform_device* pdev)
{
    /*
     * read number of "dma-names" in my device tree entry
     * for each
     *   allocate ezdma_drvdata
     *   create devices
     *   acquire slave channel
     *   add to list
     */

    int num_dma_names = of_property_count_strings(pdev->dev.of_node, "dma-names");
    int dma_name_idx;

    int outer_rv = 0;

    if ( 0 == num_dma_names )
    {
        printk( KERN_ERR KBUILD_MODNAME ": no DMAs specified in ezdma \"dma-names\" property\n");
        return -ENODEV;
    }
    else if ( num_dma_names < 0 )
    {
        printk( KERN_ERR KBUILD_MODNAME ": got %d when trying to count the elements of \"dma-names\" property\n", num_dma_names);
        return num_dma_names;   // contains error code
    }

    for(dma_name_idx = 0; dma_name_idx < num_dma_names; dma_name_idx++)
    {
        struct ntdma_drvdata* p_info;
        const char* p_dma_name;
        int rv;

        p_info = devm_kzalloc( &pdev->dev, sizeof(*p_info), GFP_KERNEL );

        if ( !p_info )
        {
            printk( KERN_ERR KBUILD_MODNAME ": failed to allocate ezdma_drvdata\n");
            outer_rv = -ENOMEM;
            break;
        }

        /* Initialize fields */
        p_info->pdev = pdev;
        p_info->in_use = 0;
        p_info->state = DMA_IDLE;
        spin_lock_init( &p_info->state_lock );
        list_add_tail( &p_info->node, &p_pdev_info->ntdma_list );
        sema_init( &p_info->sem, 1 );
        init_waitqueue_head( &p_info->transfer_wq );
        atomic_set( &p_info->packets_rcvd, 0 );

        /* Read the dma name for the current index */
        rv = of_property_read_string_index(
                pdev->dev.of_node, "dma-names",
                dma_name_idx, &p_dma_name);

        if ( rv )
        {
            printk( KERN_ERR KBUILD_MODNAME
                    ": of_property_read_string_index() returned %d\n", rv);

            outer_rv = rv;
            break;
        }
        else
        {
            strncpy( p_info->name, p_dma_name, NTDMA_DEV_NAME_MAX_CHARS-1 );
            p_info->name[NTDMA_DEV_NAME_MAX_CHARS-1] = '\0';

            //printk( KERN_DEBUG KBUILD_MODNAME ": setting up %s\n", p_info->name);
        }

        if ( (rv = ntdma_create_device( p_info )) )
        {
            outer_rv = rv;
            break;
        }

        /* Get the named DMA channel */
        p_info->chan = dma_request_slave_channel(
                    &pdev->dev, p_dma_name);

        if ( !p_info->chan )
        {
            printk( KERN_ERR KBUILD_MODNAME
                    ": couldn't find dma channel: %s, deferring...\n",
                    p_info->name);

            outer_rv = -EPROBE_DEFER;
        }

        p_info->dma_dev = &pdev->dev;

        INIT_LIST_HEAD( &p_info->cyclic_transfer_list );
        p_info->current_transfer = NULL;

        p_info->cyclic_desc_count = 0;
        p_info->cyclic_desc_len = 0;

        p_info->rx_head = 0;
        p_info->rx_tail = 0;
        p_info->rx_overrun = 0;
        p_info->wait_rx_data = false;
        p_info->xfer_complited = 0;

        printk( KERN_ALERT KBUILD_MODNAME ": %s (%s) available\n", p_info->name, "RX" );
    }

    if ( outer_rv )
    {
        // Unroll what we've done here
        teardown_devices( p_pdev_info, pdev );
    }

    return outer_rv;
}

static void teardown_devices( struct ntdma_pdev_drvdata* p_pdev_info, struct platform_device* pdev)
{
    struct ntdma_drvdata* p_info;

    list_for_each_entry( p_info, &p_pdev_info->ntdma_list, node )
    {
        // p_info might be partially-initialized, so check pointers and be careful
        printk( KERN_DEBUG KBUILD_MODNAME ": tearing down %s\n",
                p_info->name );    // name can only be all null-bytes or a valid string

        if ( p_info->chan )
        {
            dmaengine_terminate_sync(p_info->chan);
            dma_release_channel(p_info->chan);
            p_info->chan = 0;
        }

        if ( p_info->ntdma_dev )
            ntdma_teardown_device(p_info);
    }

    /* Note: we don't bother with the deallocations here, since they'll be
     * cleaned up by devm_* unrolling. */
}

static int ntdma_probe(struct platform_device* pdev)
{
    struct ntdma_pdev_drvdata* p_pdev_info;
    int rv;

    printk(KERN_INFO "probing ntdma\n");

    p_pdev_info = devm_kzalloc( &pdev->dev, sizeof(*p_pdev_info), GFP_KERNEL );

    if (!p_pdev_info)
        return -ENOMEM;

    INIT_LIST_HEAD( &p_pdev_info->ntdma_list );

    if ( (rv = create_devices( p_pdev_info, pdev )) )
        return rv;  // devm_* unrolls automatically

    platform_set_drvdata( pdev, p_pdev_info );

    return 0;
}

static int ntdma_remove(struct platform_device* pdev)
{
    struct ntdma_pdev_drvdata* p_pdev_info = (struct ntdma_pdev_drvdata *)platform_get_drvdata(pdev);

    teardown_devices( p_pdev_info, pdev );

    return 0;
}

static struct ntdma_transfer* ntdma_transfer_create(struct ntdma_drvdata* p_info, size_t dma_size)
{
    struct ntdma_transfer* xfer;
    u32 size = sizeof(struct ntdma_transfer);
    xfer = kzalloc(size, GFP_KERNEL);
    if(!xfer) {
        xfer = vmalloc(size);
        if(xfer)
            memset(xfer, 0, size);
    }
    if(!xfer) {
        pr_info("OOM, sw_desc, %u.\n", size);
        return NULL;
    }

    xfer->dma_buffer = dma_alloc_coherent(p_info->dma_dev, dma_size*sizeof(u8), &xfer->dma_buffer_bus, GFP_KERNEL);
    if(!xfer->dma_buffer) {
        pr_err("--- Error. %s (dev: %p) pre-alloc dma memory OOM. desc_size:%ld \n", p_info->name, p_info->ntdma_dev, dma_size);
        goto err_out;
    }
    xfer->dma_buffer_size = dma_size*sizeof(u8);

    sg_init_table(&xfer->sglist, 1);
    sg_dma_address(&xfer->sglist) = xfer->dma_buffer_bus;
    sg_dma_len(&xfer->sglist) = xfer->dma_buffer_size;

    return xfer;

err_out:
    ntdma_transfer_destroy(p_info, xfer);

    return NULL;
}

static void ntdma_transfer_destroy(struct ntdma_drvdata* p_info, struct ntdma_transfer* xfer)
{
    if( xfer->dma_buffer )
        dma_free_coherent(p_info->ntdma_dev, xfer->dma_buffer_size, xfer->dma_buffer, xfer->dma_buffer_bus);

    if(((unsigned long)xfer) >= VMALLOC_START && ((unsigned long)xfer) < VMALLOC_END)
           vfree(xfer);
    else
           kfree(xfer);

}

static int ntdma_get_buffer_params(struct ntdma_drvdata* p_info, struct ntdma_buffer_params* params)
{
    BUG_ON(!p_info);

    params->buf_size = p_info->cyclic_desc_len;
    params->buf_count = p_info->cyclic_desc_count;

    return 0;
}

static int ntdma_set_buffer_params(struct ntdma_drvdata* p_info, struct ntdma_buffer_params* params)
{
    int rc = -EINVAL;
    int i;

    //u32 desc_size_old = p_info->cyclic_desc_len;
    //u32 desc_count_old = p_info->cyclic_desc_count;

    BUG_ON(!p_info);

    if( p_info->cyclic_desc_len > NTDMA_MAX_BUFFER_SIZE)
        return -EINVAL;

    rc = ntdma_cyclic_transfer_teardown(p_info);

    p_info->cyclic_desc_len = params->buf_size;
    p_info->cyclic_desc_count = params->buf_count;

    for(i = 0; i < p_info->cyclic_desc_count; i++)
    {
        struct ntdma_transfer* xfer = ntdma_transfer_create(p_info, p_info->cyclic_desc_len);
        xfer->index = i;
        if(xfer != NULL) {
            /* add transfer to the tail of the engine transfer queue */
            list_add_tail(&xfer->entry, &p_info->cyclic_transfer_list);
        }
    }

    //pr_debug("ntdma_set_buffer_params. Set current transfer. [engine->current_transfer]\n");
    p_info->current_transfer = NULL;

    return rc;
}


static int ntdma_cyclic_transfer_setup(struct ntdma_drvdata* p_info)
{
    unsigned long flags;
    struct ntdma_transfer* xfer;
    struct list_head* pos;
    int rc = 0;

    spin_lock_irqsave(&p_info->state_lock, flags);

    if(p_info->current_transfer != NULL ) {
        spin_unlock_irqrestore(&p_info->state_lock, flags);
        pr_info("%s: exclusive access already taken. \n", p_info->name);
        return -EBUSY;
    }

    p_info->rx_head = 0;
    p_info->rx_tail = 0;
    p_info->rx_overrun = 0;
    p_info->wait_rx_data = false;

    if(!list_empty(&p_info->cyclic_transfer_list))
    {
        list_for_each(pos, &p_info->cyclic_transfer_list)
        { 
            xfer = list_entry(pos, struct ntdma_transfer, entry);
            xfer->txn_desc = dmaengine_prep_slave_sg(
                        p_info->chan,
                        &xfer->sglist,
                        1,
                        DMA_DEV_TO_MEM,
                        DMA_CTRL_ACK | DMA_PREP_INTERRUPT);    // run callback after this one

            if ( !xfer->txn_desc )
            {
                printk( KERN_ERR KBUILD_MODNAME ": %s: dmaengine_prep_slave_sg() failed\n", p_info->name);
                rc = -ENOMEM;
                goto err_out;
            }

            xfer->txn_desc->callback = ntdma_dmaengine_callback_func;
            xfer->txn_desc->callback_param = p_info;

            /* start cyclic transfer */
            xfer->cookie = dmaengine_submit(xfer->txn_desc);
            if( (rc = dma_submit_error(xfer->cookie )) ) {
                printk( KERN_ERR KBUILD_MODNAME ": %s: dmaengine_submit() returned %d\n", p_info->name, xfer->cookie);
                goto err_out;
            }
        }
    }

    printk( KERN_DEBUG KBUILD_MODNAME "hfr4_init_request() Set current transfer. [engine->current_transfer]\n");
    p_info->current_transfer = p_info->cyclic_transfer_list.next;
    p_info->state = DMA_IN_FLIGHT;

    dma_async_issue_pending( p_info->chan );

    spin_unlock_irqrestore(&p_info->state_lock, flags);

    return 0;


err_out:

    spin_unlock_irqrestore(&p_info->state_lock, flags);

    return rc;
}

static int transfer_monitor_cyclic(struct ntdma_drvdata* p_info, int timeout_ms)
{
    int rc = 0;

    BUG_ON(!p_info);

    rc = wait_event_interruptible_timeout( p_info->transfer_wq, p_info->rx_head != p_info->rx_tail, msecs_to_jiffies(timeout_ms));
    pr_debug("%s: wait returns %d.\n", p_info->name, rc);

    return rc;
}

static int ntdma_get_buffer_data(struct ntdma_drvdata* p_info, struct ntdma_transaction* data)
{
    unsigned long flags;
    int rc = 0;

    BUG_ON(!p_info);
    BUG_ON(!p_info->current_transfer);

    /* lock the engine */
    spin_lock_irqsave(&p_info->state_lock, flags);
    p_info->wait_rx_data = (p_info->rx_head == p_info->rx_tail);
    /* unlock the engine */
    spin_unlock_irqrestore(&p_info->state_lock, flags);

    if(p_info->wait_rx_data) {

        rc = transfer_monitor_cyclic(p_info, 5000);
        if (rc == 0) {
            pr_err(" timeout for read dma data \n");
            return -ETIMEDOUT;
        }
        else if(rc < 0 && rc != -ERESTARTSYS) {
            pr_err(" Error for read dma data. Errno: %d \n", rc);
            return rc;
        }
        else if(rc == -ERESTARTSYS) {
            pr_err(" Read dma data was interrupted by signal \n");
            return rc;
        }
    }

    /* lock the engine */
    spin_lock_irqsave(&p_info->state_lock, flags);

    data->head = p_info->rx_head;
    data->tail = p_info->rx_tail;
    data->buf_overrun = p_info->rx_overrun;

    p_info->rx_overrun = 0;
    p_info->rx_tail = (p_info->rx_tail + 1) % p_info->cyclic_desc_count;
    p_info->wait_rx_data = false;

    /* unlock the engine */
    spin_unlock_irqrestore(&p_info->state_lock, flags);

    return 0;
}

static int ntdma_clear_buffers(struct ntdma_drvdata* p_info)
{
    unsigned long flags;
    BUG_ON(!p_info);

    /* lock the engine */
    spin_lock_irqsave(&p_info->state_lock, flags);
    p_info->rx_tail = p_info->rx_head;
    p_info->rx_overrun = 0;

    /* unlock the engine */
    spin_unlock_irqrestore(&p_info->state_lock, flags);

    return 0;
}


static long ntdma_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
    struct ntdma_drvdata* p_info = (struct ntdma_drvdata*)file->private_data;
    void* __user arg_ptr;
    struct ntdma_buffer_params buf_params;
    struct ntdma_transaction read_buf;
    int start;
    int rv = 0;

    if(_IOC_TYPE(cmd) != NTDMA_IOC_MAGIC) {
           pr_err("cmd %u, bad magic 0x%x/0x%x.\n", cmd, _IOC_TYPE(cmd), NTDMA_IOC_MAGIC);
           return -ENOTTY;
    }

    arg_ptr = (void __user*)arg;

    // Verify the input argument
    if(_IOC_DIR(cmd) & _IOC_READ) {
        if(!ntdma_access_ok(arg_ptr, _IOC_SIZE(cmd), false)) {
            return -EFAULT;
        }
    } else if(_IOC_DIR(cmd) & _IOC_WRITE) {
        if(!ntdma_access_ok(arg_ptr, _IOC_SIZE(cmd), true)) {
            return -EFAULT;
        }
    }

    // Perform the specified command
    switch(cmd)
    {
    case IOCTL_NTDMA_GET_BUFFERS:
        rv = ntdma_get_buffer_params(p_info, &buf_params);
        if(rv < 0)
            return rv;

        if(copy_to_user(arg_ptr, &buf_params, sizeof(buf_params)) != 0) {
            pr_err("Unable to copy buffer parameters to userspace for IOCTL_NUTDMA_GET_BUFFERS. \n");
            return -EFAULT;
        }
        break;

    case IOCTL_NTDMA_SET_BUFFERS:
        if(copy_from_user(&buf_params, arg_ptr, sizeof(buf_params)) != 0) {
            pr_err("Unable to copy buffer parameters from userspace for IOCTL_NUTDMA_SET_BUFFERS. \n");
            return -EFAULT;
        }

        rv = ntdma_set_buffer_params(p_info, &buf_params);
        break;

    case IOCTL_NTDMA_DMA_READ:
        rv = ntdma_get_buffer_data(p_info, &read_buf);
        if(rv < 0)
            return rv;
        if(copy_to_user(arg_ptr, &read_buf, sizeof(read_buf)) != 0) {
                        pr_err("Unable to copy dma buffer pointers to userspace for IOCTL_NUTDMA_DMA_READ. \n");
                        return -EFAULT;
        }
        break;

    case IOCTL_NTDMA_DMA_START:
        start = (int)arg;
        if(start) {
            // Start reading data
            ntdma_cyclic_transfer_setup(p_info);
        }
        else {
            // Stop reading data
            ntdma_cyclic_transfer_teardown(p_info);
        }
        break;

    case IOCTL_NTDMA_DMA_RESET:
        rv = ntdma_clear_buffers(p_info);
        break;

    default:
        pr_err("Unsupported operation\n");
        rv = -EINVAL;
        break;
    }

    return rv;
}

static int ntdma_mmap(struct file* file, struct vm_area_struct* vma)
{
    int rv;
    unsigned long offset;
    unsigned long vsize;
    struct list_head *pos;
    struct ntdma_transfer* xfer, *cur_xfer;

    struct ntdma_drvdata* p_info = (struct ntdma_drvdata*)file->private_data;

    offset = (vma->vm_pgoff << PAGE_SHIFT) / p_info->cyclic_desc_len;
    vma->vm_pgoff = 0;

    if(offset >= p_info->cyclic_desc_count) {
        pr_err("Invalid offset value: offset:%lu . Max index = %u \n", offset, p_info->cyclic_desc_count);
        return -EINVAL;
    }
    vsize = vma->vm_end - vma->vm_start;

    // Find buffer
    xfer = 0;
    if(!list_empty(&p_info->cyclic_transfer_list)) {
        list_for_each(pos, &p_info->cyclic_transfer_list) {
            cur_xfer = list_entry(pos, struct ntdma_transfer, entry);
            if(cur_xfer->index == offset)
                xfer = cur_xfer;
        }
    }

    if(!xfer)
        return -EINVAL;

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    rv = dma_mmap_coherent(p_info->dma_dev, vma, xfer->dma_buffer, xfer->dma_buffer_bus, vsize);

    return rv;
}

/* Match table for of_platform binding */
static const struct of_device_id ntdma_of_match[] = {
    { .compatible = "ntdma" /*, .data = &... */ },
    { /* end of list */ },
};
MODULE_DEVICE_TABLE(of, ntdma_of_match);

static struct platform_driver ntdma_driver = {
    .driver = {
        .name = KBUILD_MODNAME,
        .owner = THIS_MODULE,
        .of_match_table = ntdma_of_match,
    },
    .probe      = ntdma_probe,
    .remove     = ntdma_remove,
};


static int __init ntdma_driver_init(void)
{
    int rv;
    ntdma_class = class_create(THIS_MODULE, "ntdma");

    if ( (rv = alloc_chrdev_region( &base_devno, 0, NUM_DEVICE_NUMBERS_TO_ALLOCATE, "ntdma" )) )
    {
        printk(KERN_ERR KBUILD_MODNAME ": alloc_chrdev_region() returned %d!\n", rv);
        return rv;
    }
    else
    {
        printk(KERN_INFO KBUILD_MODNAME ": allocated chrdev region: Major: %d, Minor: %d-%d\n",
                   MAJOR(base_devno),
                   MINOR(base_devno),
                   MINOR(base_devno) + NUM_DEVICE_NUMBERS_TO_ALLOCATE);
    }

    if ( (rv = platform_driver_register(&ntdma_driver)) )
    {
        unregister_chrdev_region( base_devno, NUM_DEVICE_NUMBERS_TO_ALLOCATE );
        class_destroy(ntdma_class);
        return rv;
    }
    return 0;
}

static void __exit ntdma_driver_exit(void)
{
    platform_driver_unregister(&ntdma_driver);
    class_destroy(ntdma_class);
    unregister_chrdev_region( base_devno, NUM_DEVICE_NUMBERS_TO_ALLOCATE );
}


module_init(ntdma_driver_init);
module_exit(ntdma_driver_exit);

MODULE_AUTHOR("Amungo OU");
MODULE_DESCRIPTION("NT DMA Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
