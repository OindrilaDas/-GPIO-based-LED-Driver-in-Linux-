#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/fcntl.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/errno.h>
#include <linux/ktime.h>
# include <linux/hrtimer.h>
# include <linux/timer.h>
#include "query_ioctl.h"
#include <linux/proc_fs.h>


#define DEVICE_NAME                 "RGBLed"  // device name to be created and registered

static struct hrtimer pwm_timer;
unsigned long on_time = 0, off_time = 0;

/* lookup table for IO pins  */
int lookup_table[14][4]=  {
                                {11,32,-1,-1},         // IO0
                                {12,28,45,-1},         // IO1
                                {13,34,77,-1},         // IO2
                                {14,16,76,64},         // IO3
                                {6,36,-1,-1},          // IO4
                                {0,18,66,-1},          // IO5
                                {1,20,68,-1},          // IO6
                                {38,-1,-1,-1},         // IO7
                                {40,-1,-1,-1},         // IO8
                                {4,22,70,-1},          // IO9
                                {10,26,74,-1},         // IO10
                                {5,24,44,72},          // IO11
                                {15,42,-1,-1},         // IO12
                                {7,30,46,-1}           // IO13
                             };


/* per device structure */
struct RGBLed_dev {
	struct cdev cdev;                       /* The cdev structure */
	int PWM;
   int IO_R, IO_G, IO_B, R_in, G_in, B_in;
} *RGBLed_devp;

static dev_t RGBLed_dev_number;            /* Allotted device number */
struct class *RGBLed_dev_class;            /* The device-driver class struct pointer */
static struct device *RGBLed_dev_device;   /* The device-driver device struct pointer */


//calculating on time & off time for timer
int timer_count=0;

//timer callback function
static enum hrtimer_restart timer_resetter(struct hrtimer *pwm_timer)
{
	if(timer_count <= 48)
	
	{
		if(timer_count%2 == 0)
	{
		ktime_t off_period;
		off_period = ktime_set(0, off_time);

		gpio_set_value(lookup_table[RGBLed_devp->IO_R][0],0);
		gpio_set_value(lookup_table[RGBLed_devp->IO_G][0],0);
		gpio_set_value(lookup_table[RGBLed_devp->IO_B][0],0);
		 
		hrtimer_forward_now(pwm_timer, off_period);
		timer_count++;
		return HRTIMER_RESTART;
	}
	if(timer_count%2 != 0)
	{
		ktime_t on_period;
		on_period = ktime_set(0, on_time);

		gpio_set_value(lookup_table[RGBLed_devp->IO_R][0],RGBLed_devp->R_in);
		gpio_set_value(lookup_table[RGBLed_devp->IO_G][0],RGBLed_devp->G_in);
		gpio_set_value(lookup_table[RGBLed_devp->IO_B][0],RGBLed_devp->B_in);
		 
		hrtimer_forward_now(pwm_timer, on_period);
		timer_count++;
		return HRTIMER_RESTART;
	}
}

	return HRTIMER_NORESTART;
}


/* The prototype functions for the character driver -- must come before the struct definition */

static int     RGBLed_driver_open(struct inode *, struct file *);
static int     RGBLed_driver_release(struct inode *, struct file *);
static ssize_t RGBLed_driver_write(struct file *, const char *, size_t, loff_t *);
static long    RGBLed_driver_ioctl(struct file *, unsigned int, unsigned long);

/*
* Open RGBLed driver
*/
int RGBLed_driver_open(struct inode *inode, struct file *file)
{
	printk("Opening RGBLed driver\n");
	return 0;
}

/*
 * Release RGBLed driver
 */
int RGBLed_driver_release(struct inode *inode, struct file *file)
{
	 int i;    
       
        /* Checking if the input values are valid */
        if(RGBLed_devp->PWM<0 || RGBLed_devp->PWM>100)          
         {
				printk("\nReleasing RGBLed driver\n");
				return 0;
         }
         if(!(RGBLed_devp->IO_R==0 || RGBLed_devp->IO_R==1 || RGBLed_devp->IO_R==2 || RGBLed_devp->IO_R==3 || RGBLed_devp->IO_R==10 || RGBLed_devp->IO_R==12 ) )        
         {
				printk("\nReleasing RGBLed driver\n");
				return 0;
         }
         if(!(RGBLed_devp->IO_G==0 || RGBLed_devp->IO_G==1 || RGBLed_devp->IO_G==2 || RGBLed_devp->IO_G==3 || RGBLed_devp->IO_G==10 || RGBLed_devp->IO_G==12 ) )
         {
				printk("\nReleasing RGBLed driver\n");
				return 0;
         }
         if(!(RGBLed_devp->IO_B==0 || RGBLed_devp->IO_B==1 || RGBLed_devp->IO_B==2 || RGBLed_devp->IO_B==3 || RGBLed_devp->IO_B==10 || RGBLed_devp->IO_B==12 ) )
         {
				printk("\nReleasing RGBLed driver\n");
				return 0;
         }
        
   		gpio_set_value(lookup_table[RGBLed_devp->IO_R][0],0);
			gpio_set_value(lookup_table[RGBLed_devp->IO_G][0],0);
			gpio_set_value(lookup_table[RGBLed_devp->IO_B][0],0);
           
         /* Allocating GPIOs and setting their direction and values */
            for(i=0; i<3; i++)           // IO_R
            {
                if(lookup_table[RGBLed_devp->IO_R][i]!=-1)
                {
                    gpio_free(lookup_table[RGBLed_devp->IO_R][i]);
                    gpio_unexport(lookup_table[RGBLed_devp->IO_R][i]);
                }
            }
            for(i=0; i<3; i++)           // IO_G
            {
                if(lookup_table[RGBLed_devp->IO_G][i]!=-1)
                {
                   gpio_free(lookup_table[RGBLed_devp->IO_G][i]);
                    gpio_unexport(lookup_table[RGBLed_devp->IO_G][i]);
                }
            }
            for(i=0; i<3; i++)           // IO_B
            {
                if(lookup_table[RGBLed_devp->IO_B][i]!=-1)
                {
                   gpio_free(lookup_table[RGBLed_devp->IO_B][i]);
                    gpio_unexport(lookup_table[RGBLed_devp->IO_B][i]);
                }
            }
            
   	hrtimer_cancel(&pwm_timer);         	
		printk("\nSuccessfully Released RGBLed driver\n");
		return 0;
}

/*
 * Write to RGBLed driver
 */
ssize_t RGBLed_driver_write(struct file *file, const char *buf,
           size_t count, loff_t *ppos)
{   
   int i;
   ktime_t settime;
   char rgb_input[strlen(buf)+1];
   int rgb_int;
   int rgb_bin[3];
   
   
   memset(rgb_input,0,2);
	copy_from_user(rgb_input, buf, count);
   rgb_int = rgb_input[0] - '0';
   
   
   for(i=0; i<=2; i++)
    {
    	rgb_bin[i] = rgb_int%2;
    	rgb_int = rgb_int/2;
    }
    
    RGBLed_devp->R_in = rgb_bin[2];
    RGBLed_devp->G_in = rgb_bin[1];
    RGBLed_devp->B_in = rgb_bin[0];
   

   //setting timer 
   
   settime = ktime_set(0,on_time);
   hrtimer_init(&pwm_timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS);
   pwm_timer.function = &timer_resetter;
   
   //starting the timer
   if(on_time == 0)
   {
   	gpio_set_value(lookup_table[RGBLed_devp->IO_R][0], 0);
		gpio_set_value(lookup_table[RGBLed_devp->IO_G][0], 0);
   	gpio_set_value(lookup_table[RGBLed_devp->IO_B][0], 0);
   }
   else{
   	gpio_set_value(lookup_table[RGBLed_devp->IO_R][0], rgb_bin[2]);
		gpio_set_value(lookup_table[RGBLed_devp->IO_G][0], rgb_bin[1]);
   	gpio_set_value(lookup_table[RGBLed_devp->IO_B][0], rgb_bin[0]);
   }
   timer_count = 0;
   
   if(on_time != 0 && on_time != 20000000)
   {        
  	 	hrtimer_start(&pwm_timer,settime, HRTIMER_MODE_REL);
   }
      
	return count;
}



/*
 * IOCTL
 */
 
   long RGBLed_driver_ioctl(struct file *file, unsigned int cmd , unsigned long arg)
    {
        query_arg_t q;
        copy_from_user(&q, (query_arg_t *)arg, sizeof(query_arg_t));   // copying the input values from user space to kernel module 
          
        RGBLed_devp->PWM = q.PWM;
        RGBLed_devp->IO_R = q.IO_R;
        RGBLed_devp->IO_G = q.IO_G;
        RGBLed_devp->IO_B = q.IO_B;
        on_time = (RGBLed_devp->PWM)*200000;
		  off_time = 20000000 - ((RGBLed_devp->PWM)*200000);
        printk("PWM: %d, Red IO Port: %d Green Port: %d, Blue Port: %d, on time= %lu, off time = %lu", RGBLed_devp->PWM, RGBLed_devp->IO_R, RGBLed_devp->IO_G, RGBLed_devp->IO_B,on_time, off_time);
       
        switch (cmd)
        {
        case CONFIG:
        {
         
            int i;    
 
        /* Checking if the input values are valid */
       if(RGBLed_devp->PWM<0 || RGBLed_devp->PWM>100)        
         {
           printk("\nERROR: PWM should be between 0 to 100");
           return -EINVAL;
         }
         if(!(RGBLed_devp->IO_R==0 || RGBLed_devp->IO_R==1 || RGBLed_devp->IO_R==2 || RGBLed_devp->IO_R==3 || RGBLed_devp->IO_R==10 || RGBLed_devp->IO_R==12 ) )       
         {
           printk("\nERROR: Pin number for Red Led can only take values from set {0,1,2,3,10,12}\n");
           return -EINVAL;
         }
          if(!(RGBLed_devp->IO_G==0 || RGBLed_devp->IO_G==1 || RGBLed_devp->IO_G==2 || RGBLed_devp->IO_G==3 || RGBLed_devp->IO_G==10 || RGBLed_devp->IO_G==12 ) )       
         {
           printk("\nERROR: Pin number for Green Led can only take values from set {0,1,2,3,10,12}\n");
           return -EINVAL;
         }
         if(!(RGBLed_devp->IO_B==0 || RGBLed_devp->IO_B==1 || RGBLed_devp->IO_B==2 || RGBLed_devp->IO_B==3 || RGBLed_devp->IO_B==10 || RGBLed_devp->IO_B==12 ) )
         {
           printk("\nERROR: Pin number for Blue Led can only take values from set {0,1,2,3,10,12}\n");
           return -EINVAL;
         }
        
           
           
         /* Allocating GPIOs and setting their direction and values */
            for(i=0; i<3; i++)           // IO_R
            {
                if(lookup_table[RGBLed_devp->IO_R][i]!=-1)
                {
                    if(gpio_request(lookup_table[RGBLed_devp->IO_R][i], "sysfs")<0)
                        printk("\nError in allocating GPIO %d", lookup_table[RGBLed_devp->IO_R][i]);
                    else
                    		printk("\nAllocating GPIO %d", lookup_table[RGBLed_devp->IO_R][i]);
                    
                    gpio_export(lookup_table[RGBLed_devp->IO_R][i], false);
                 
                    if(lookup_table[RGBLed_devp->IO_R][i]<64)
                    {
                        if(gpio_direction_output(lookup_table[RGBLed_devp->IO_R][i], 0)<0)
                            printk("\nError in giving GPIO directions");
                    }
                    else if(lookup_table[RGBLed_devp->IO_R][i]>=64)
                    {
                    		gpio_set_value_cansleep(lookup_table[RGBLed_devp->IO_R][i], 0);
                    }
                }
            }
            for(i=0; i<3; i++)           // IO_G
            {
                if(lookup_table[RGBLed_devp->IO_G][i]!=-1)
                {
                    if(gpio_request(lookup_table[RGBLed_devp->IO_G][i], "sysfs")<0)
                        printk("\nError in allocating GPIO %d", lookup_table[RGBLed_devp->IO_G][i]);
                     else
                    		printk("\nAllocating GPIO %d", lookup_table[RGBLed_devp->IO_G][i]);
                    gpio_export(lookup_table[RGBLed_devp->IO_G][i], false);
                 
                    if(lookup_table[RGBLed_devp->IO_G][i]<64)
                    {
                        if(gpio_direction_output(lookup_table[RGBLed_devp->IO_G][i], 0)<0)
                            printk("\nError in giving GPIO directions");
                    }
                    else if(lookup_table[RGBLed_devp->IO_G][i]>=64)
                    {
                    		gpio_set_value_cansleep(lookup_table[RGBLed_devp->IO_G][i], 0);
                    }

                }
            }
            for(i=0; i<3; i++)           // IO_B
            {
                if(lookup_table[RGBLed_devp->IO_B][i]!=-1)
                {
                    if(gpio_request(lookup_table[RGBLed_devp->IO_B][i], "sysfs")<0)
                        printk("\nError in allocating GPIO %d", lookup_table[RGBLed_devp->IO_B][i]);
               	  else
                    		printk("\nAllocating GPIO %d", lookup_table[RGBLed_devp->IO_B][i]);
                    gpio_export(lookup_table[RGBLed_devp->IO_B][i], false);
                 
                    if(lookup_table[RGBLed_devp->IO_B][i]<64)
                    {
                        if(gpio_direction_output(lookup_table[RGBLed_devp->IO_B][i], 0)<0)
                            printk("\nError in giving GPIO directions");
                    }
                    else if(lookup_table[RGBLed_devp->IO_B][i]>=64)
                    {
                    		gpio_set_value_cansleep(lookup_table[RGBLed_devp->IO_B][i], 0);
                    }

                }
            }
            
            break;
            default:
            printk("\nWrong command, exiting driver");
            return 0;
            }
    } 
    return 0;
 }
 
/* File operations structure. Defined in linux/fs.h */
static struct file_operations RGBLed_fops = {
    .owner		= THIS_MODULE,           /* Owner */
    .open		= RGBLed_driver_open,        /* Open method */
    .release	= RGBLed_driver_release,     /* Release method */
    .write		= RGBLed_driver_write,       /* Write method */
    .unlocked_ioctl = RGBLed_driver_ioctl   /* IOCTL method */
};

/*
 * Driver Initialization
 */
int __init RGBLed_driver_init(void)
{
	int ret;
	
	/* Request dynamic allocation of a device major number */
	if (alloc_chrdev_region(&RGBLed_dev_number, 0, 1, DEVICE_NAME) < 0) {
			printk(KERN_DEBUG "Can't register device\n"); 
			return -1;
	}

	/* Populate sysfs entries */
	RGBLed_dev_class = class_create(THIS_MODULE, DEVICE_NAME);

	/* Allocate memory for the per-device structure */
	RGBLed_devp = kmalloc(sizeof(struct RGBLed_dev), GFP_KERNEL);
		
	if (!RGBLed_devp) {
		printk("Bad Kmalloc\n"); 
		return -ENOMEM;
	}

	/* Request I/O region */

	/* Connect the file operations with the cdev */
	cdev_init(&RGBLed_devp->cdev, &RGBLed_fops);
	RGBLed_devp->cdev.owner = THIS_MODULE;

	/* Connect the major/minor number to the cdev */
	ret = cdev_add(&RGBLed_devp->cdev, (RGBLed_dev_number), 1);

	if (ret) {
		printk("Bad cdev\n");
		return ret;
	}

	/* Send uevents to udev, so it'll create /dev nodes */
	RGBLed_dev_device = device_create(RGBLed_dev_class, NULL, MKDEV(MAJOR(RGBLed_dev_number), 0), NULL, DEVICE_NAME);		
	

	printk("RGBLed driver initialized.\n");
	return 0;
}
/* Driver Exit */
void __exit RGBLed_driver_exit(void)
{
	
	/* Destroy device */
	device_destroy (RGBLed_dev_class, MKDEV(MAJOR(RGBLed_dev_number), 0));
	cdev_del(&RGBLed_devp->cdev);
	kfree(RGBLed_devp);
	
	
	/* Destroy driver_class */
	class_destroy(RGBLed_dev_class);
	/* Release the major number */
	unregister_chrdev_region((RGBLed_dev_number), 1);

	printk("RGBLed driver removed.\n");
}

module_init(RGBLed_driver_init);
module_exit(RGBLed_driver_exit);
MODULE_LICENSE("GPL v2");
