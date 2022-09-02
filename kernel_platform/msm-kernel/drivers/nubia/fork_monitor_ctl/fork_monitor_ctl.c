#include "fork_monitor_ctl.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/regmap.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>

//监视最多6条进程产生子线程
#define MONITOR_MAX_PROCESS_COUNT 6

#define MONITOR_ADD_PID 43040
#define MONITOR_DELETE_PID 43041

int f_monitor_device_major = 4304;
int f_monitor_device_minor = 4303;
int number_of_device = 1;
static struct device *f_monitor_dev;
struct class *f_monitor_class;
dev_t dev = 0;
struct cdev monitorcdev;
struct file_operations monitor_fops = {
     .owner = THIS_MODULE,
};

struct monitor_data{
	struct mutex file_mutex;
	unsigned char *databuf;
	unsigned int monitor_pid[MONITOR_MAX_PROCESS_COUNT];
        int monitor_pid_position;
        bool enable;
        bool debug;
};

struct monitor_data *dev_data;
static struct kobject *fork_monitor_kobj;

//将char转成数字
int atoi(char *pstr){
  int sum = 0;
  int sign = 1;
  if(pstr == NULL){
    return 0;
  }
  pstr = strim(pstr);
  if(*pstr == '-'){
    sign = -1;
  }
  if(*pstr == '-' || *pstr == '+'){
    pstr++;
  }
  while(*pstr >= '0' && *pstr <= '9'){
    sum = sum * 10 + (*pstr - '0');
    pstr++;
  }
  sum = sign * sum;
  return sum;
}

void f_monitor_send_uevent(int pid, int tid){
  if(need_to_send_uevent(pid)){
     char event_string[128] = { 0 };
     char *envp[] = {event_string, NULL};
     snprintf(event_string, sizeof(event_string), "THREAD_INFO=%d:%d", pid, tid);
     kobject_uevent_env(&f_monitor_dev->kobj,KOBJ_CHANGE ,envp);
  }
}

bool need_to_send_uevent(int pid){
  bool isPidExists = false;
  int x = -1;
  bool needToSendUevent = false;
  if(!fork_monitor_kobj || !dev_data->enable)return false;
  mutex_lock(&(dev_data->file_mutex));
  for(x = 0; x < MONITOR_MAX_PROCESS_COUNT; x++){
      if(dev_data->monitor_pid[x] == pid){
         isPidExists = true;
      }
  }
  if(isPidExists){
     needToSendUevent = true;
  }
  mutex_unlock(&(dev_data->file_mutex));
  return needToSendUevent; 
}
static ssize_t opts_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
   ssize_t retval;
   char event_string[100] = { 0 };
   mutex_lock(&(dev_data->file_mutex));
   snprintf(event_string, sizeof(event_string), "MONITOR_PID=%d:%d:%d:%d:%d:%d\n", dev_data->monitor_pid[0],
        dev_data->monitor_pid[1], dev_data->monitor_pid[2],dev_data->monitor_pid[3],dev_data->monitor_pid[4],dev_data->monitor_pid[5]);
   retval = snprintf(buf, sizeof(event_string),"%s\n", event_string);
   mutex_unlock(&(dev_data->file_mutex));

   return retval;
}
static ssize_t opts_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	ssize_t retval = 0;
        int optType = -1;
        int pidValue = -1;
        const char *pchDilem = "@";
        char chBuffer[218];
        char *pchTmp = NULL;
        char *optStr = NULL;
        int pidArrayPosition = -1;
        int x = -1;

	if (IS_ERR(dev_data)) {
		pr_err("%s: Pointer of char device data is invalid", __func__);
		return -EBADF;
	}

	if (count == 0)
		return 0;

	mutex_lock(&(dev_data->file_mutex));
          
        strncpy(dev_data->databuf,buf,256);
        if(dev_data->debug)printk(KERN_DEBUG "fmonitor_opts_store buf:%s \n", buf);
        retval = count;
        if(strstr(dev_data->databuf, pchDilem)){
           strncpy(chBuffer, dev_data->databuf, sizeof(chBuffer)-1);
           pchTmp = chBuffer;
           optStr = strsep(&pchTmp, pchDilem);
           if(strnlen(optStr, 8) && strnlen(pchTmp, 8)){
                optType = atoi(optStr);
                pidValue = atoi(pchTmp);
                if(dev_data->debug)printk(KERN_DEBUG "fmonitor_addPidSucess optType:%d, pidValue:%d \n", optType, pidValue);
                if(optType > 0 && pidValue > 0){
                      if(optType == MONITOR_ADD_PID){
                           pidArrayPosition = dev_data->monitor_pid_position % MONITOR_MAX_PROCESS_COUNT;
                           dev_data->monitor_pid[pidArrayPosition] = pidValue;
                           dev_data->monitor_pid_position++;
                       }
                       if(optType == MONITOR_DELETE_PID){
                           for(x = 0; x < MONITOR_MAX_PROCESS_COUNT; x++){
                             if(dev_data->monitor_pid[x] == pidValue){
                                dev_data->monitor_pid[x] = -1;
                             }
                          }
                      }
                  }
           }
           pchTmp = NULL;
           optStr = NULL;
        }
	mutex_unlock(&(dev_data->file_mutex));

	return retval;
}

static ssize_t enable_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    mutex_lock(&(dev_data->file_mutex));
    ret = sprintf(buf, "%d\n", dev_data->enable);
    mutex_unlock(&(dev_data->file_mutex));
    return ret;
}
static ssize_t enable_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret = kstrtoul(buf, 0, &val);
	if (ret){
		printk("%s is not in hex or decimal form.\n", buf);
	}else{
                mutex_lock(&(dev_data->file_mutex));
		dev_data->enable = !!val;
                if(!dev_data->enable){
                    memset(dev_data->monitor_pid, -1, MONITOR_MAX_PROCESS_COUNT);
                }
                mutex_unlock(&(dev_data->file_mutex));
	}
	return count;
}
static ssize_t debug_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    mutex_lock(&(dev_data->file_mutex));
    ret = sprintf(buf, "%d\n", dev_data->debug);
    mutex_unlock(&(dev_data->file_mutex));
    return ret;
}
static ssize_t debug_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret = kstrtoul(buf, 0, &val);
	if (ret){
		printk("%s is not in hex or decimal form.\n", buf);
	}else{
                mutex_lock(&(dev_data->file_mutex));
		dev_data->debug= !!val;
                mutex_unlock(&(dev_data->file_mutex));
	}
	return count;
}

static struct kobj_attribute enable_attr = __ATTR(enable, 0664, enable_show, enable_store);
static struct kobj_attribute opts_attr = __ATTR(opts, 0664, opts_show, opts_store);
static struct kobj_attribute debug_attr = __ATTR(debug, 0664, debug_show, debug_store);

static struct attribute *fork_monitor_attributes[] = {
    &enable_attr.attr,
    &opts_attr.attr,
    &debug_attr.attr,
    NULL
};

static struct attribute_group fork_monitor_attribute_group = {
        .attrs = fork_monitor_attributes
};

static int __init monitor_init(void){
   int result;
   int error, devno;
   //init sys file
   fork_monitor_kobj = kobject_create_and_add("fork_monitor", NULL);
   if (!fork_monitor_kobj)
   {
	printk(KERN_ERR "%s: fork_monitor_ctl kobj create error\n", __func__);
	return -ENOMEM;
   }

   result = sysfs_create_group(fork_monitor_kobj, &fork_monitor_attribute_group);
   if(result)
	printk(KERN_ERR "%s: failed to create fork_monitor_ctl group attributes\n", __func__);

   dev_data = kzalloc(sizeof(*dev_data), GFP_KERNEL);
   mutex_init(&dev_data->file_mutex);
   memset(dev_data->monitor_pid, -1, MONITOR_MAX_PROCESS_COUNT);
   dev_data->monitor_pid_position = 0;
   dev_data->enable = false;
   dev_data->debug = false;
   dev_data->databuf = kzalloc(256, GFP_KERNEL);
   
   //init monitor driver
   dev = MKDEV(f_monitor_device_major, f_monitor_device_minor);
   result = register_chrdev_region(dev, number_of_device, "f_monitor" );
   if(result < 0){
     printk(KERN_DEBUG "monitor_device can not get major number %d\n", f_monitor_device_major);
     return result;
   }
   devno = MKDEV(f_monitor_device_major, f_monitor_device_minor);
   cdev_init(&monitorcdev, &monitor_fops);
   monitorcdev.owner = THIS_MODULE;
   monitorcdev.ops = &monitor_fops;
   error = cdev_add(&monitorcdev, devno, 1);
   if(error){
       printk(KERN_DEBUG "monitor_device_create cdev error :%d\n", error);
   }
   f_monitor_class = class_create(THIS_MODULE, "f_monitor_class");
   if(IS_ERR(f_monitor_class)){
      printk(KERN_DEBUG "monitor_device failed in creating class\n");
      return -1;
   }
   
   f_monitor_dev = device_create(f_monitor_class, NULL, dev,NULL, "f_monitor");
   if(!f_monitor_dev){
       printk(KERN_DEBUG "monitor_device_create failed\n");
   }
   return 0;
}
static void __exit monitor_exit(void){
   //release sys file res
   sysfs_remove_group(fork_monitor_kobj, &fork_monitor_attribute_group);
   kobject_put(fork_monitor_kobj);
   kfree(dev_data->databuf);
   kfree(dev_data);
   dev_data = NULL;
   //release driver res
   cdev_del(&monitorcdev);
   device_destroy(f_monitor_class, MKDEV(f_monitor_device_major, f_monitor_device_minor));
   unregister_chrdev_region(MKDEV(f_monitor_device_major, f_monitor_device_minor), number_of_device);
   printk(KERN_DEBUG "monitor_exit device clear");
}
module_init(monitor_init);
module_exit(monitor_exit);
MODULE_AUTHOR("gskou4304, <kou.guisen@swlab.com>");
MODULE_DESCRIPTION("work for fork monitor");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:fork_monitor" );
