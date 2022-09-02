/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/power_supply.h>
#include <linux/regmap.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include "sched_ctl.h"


#define ALL_MATCH       (99999999)

static int  enable = 0;
static struct mutex mMutex;
static int debug = 0;
static int printoption = 0;


typedef struct Node {
	int id;
	struct Node* next;
} Node;

typedef struct List {
	void (*addNode)(int id, struct List* self);
	void (*rmNode)(int id, struct List* self);
	void (*print)(char* buf, struct List* self);
	int (*is_in_list)(int current_id, struct List* self);
	struct Node* head;
} List;

int getDebugCode(void) {
	 return debug;
}

static void addNode(int id, struct List* self) {
	struct Node* node = (struct Node*)kmalloc(sizeof(Node), GFP_KERNEL);
	node->id = id;
	node->next = NULL;

	if(self->head == NULL) {
		self->head = node;
	} else {
		struct Node* tmp = self->head;
		self->head = node;
		node->next = tmp;
	}
}
static void rmNode(int id, struct List* self) {
	if(self->head == NULL) {
		return;
	} else {
		struct Node* pre = self->head;
		struct Node* curr = self->head;
		while(curr != NULL) {
			printk("wuzhibin curr:%p id:%d curr->id:%d\n", curr, id, curr->id);
			if(curr->id == id) {
				if(curr == self->head) {
					self->head = curr->next;
					kfree(curr);
					curr = self->head;
					pre = self->head;
				} else {
					pre->next = curr->next;
                    kfree(curr);
                    curr = pre->next;
				}
			} else {
				pre = curr;
				curr = curr->next;
			}
		}
	}
}
static void print(char* buf, List* self) {

	char s[64];
	if(self->head == NULL) {
		printk("list is null\n");
		return;
	} else {
		struct Node* tmp = self->head;
		while(tmp != NULL) {
			sprintf(s, "%d,",tmp->id);
			strcat(buf,s);
			tmp = tmp->next;
		}
		strcat(buf,"\n");
	}
}

static int is_in_list(int current_id, struct List* self) {
	if(self->head == NULL) {
		return 0;
	} else {
		struct Node* tmp = self->head;
		while(tmp != NULL) {
			if(tmp->id == current_id) {
				return 1;
			}
			tmp = tmp->next;
		}
	}
	return 0;
}

static struct List sched_id_list = {
	.addNode = addNode,
	.rmNode = rmNode,
	.print = print,
	.is_in_list = is_in_list,
	.head = NULL,
};


int isTargetPid(int pid) {
	if(!enable) {
		return 0;
	}
	return sched_id_list.is_in_list(pid, &sched_id_list);
}


extern void printed(int who) {
	mutex_lock(&mMutex);
	if(who != 9999) {
	    printoption = 0;
	}
	mutex_unlock(&mMutex);
}

int shouldPrint(int who) {
	int val = 0;
	mutex_lock(&mMutex);
	val = printoption;
	mutex_unlock(&mMutex);
	if(val == 9999) {
		return 1;
	}
	if(who == val) {
		return 1;
	}
	return 0;
}



extern void resetPrintOnce(int who);

static ssize_t enable_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    mutex_lock(&mMutex);
	ret = sprintf(buf, "%d\n", enable);
	mutex_unlock(&mMutex);
	return ret;
}
static ssize_t enable_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret = kstrtoul(buf, 0, &val);
	if (ret){
		printk("%s is not in hex or decimal form.\n", buf);
	}else{
        mutex_lock(&mMutex);
		enable = !!val;
		mutex_unlock(&mMutex);
	}
	return count;
}
static struct kobj_attribute enable_attr = __ATTR(enable, 0664, enable_show, enable_store);


static ssize_t list_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int len = 0;
    mutex_lock(&mMutex);
	sched_id_list.print(buf, &sched_id_list);
    mutex_unlock(&mMutex);
    len = strlen(buf);
	return len;
}

static ssize_t list_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret = kstrtoul(buf, 0, &val);
	if (ret){
		printk("%s is not in hex or decimal form.\n", buf);
	}else{
		mutex_lock(&mMutex);
		if(!sched_id_list.is_in_list((int)val, &sched_id_list)) {
		    sched_id_list.addNode((int)val, &sched_id_list);
		}
		mutex_unlock(&mMutex);
	}
	return count;
}

static struct kobj_attribute list_attr = __ATTR(list, 0664, list_show, list_store);

static ssize_t del_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret = kstrtoul(buf, 0, &val);
	if (ret){
		printk("%s is not in hex or decimal form.\n", buf);
	}else{
		printk("wuzhibin to del %d\n", (int)val);
		mutex_lock(&mMutex);
		sched_id_list.rmNode((int)val, &sched_id_list);
		mutex_unlock(&mMutex);
		printk("wuzhibin to del %d done.\n", (int)val);
	}
	return count;
}
static struct kobj_attribute del_attr = __ATTR(del, 0664, NULL, del_store);

static ssize_t print_option_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret = kstrtoul(buf, 0, &val);
	if (ret){
		printk("%s is not in hex or decimal form.\n", buf);
	}else{
		mutex_lock(&mMutex);
		printoption = (int)val;
		mutex_unlock(&mMutex);
	}
	return count;
}
static struct kobj_attribute print_option_attr = __ATTR(print_option, 0664, NULL, print_option_store);


static ssize_t debug_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int ret = 0;
	mutex_lock(&mMutex);
	ret = sched_id_list.is_in_list((int)debug, &sched_id_list);
	mutex_unlock(&mMutex);
	ret = sprintf(buf, "debug:%d is_in_list:%d \n", debug, ret);
	return ret;
}
static ssize_t debug_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret = kstrtoul(buf, 0, &val);
	if (ret){
		printk("%s is not in hex or decimal form.\n", buf);
	}else{
        mutex_lock(&mMutex);
		debug = (int)val;
		mutex_unlock(&mMutex);
	}
	return count;
}
static struct kobj_attribute debug_attr = __ATTR(debug, 0664, debug_show, debug_store);


static struct attribute *sched_ctl_attributes[] = {
    &enable_attr.attr,
    &list_attr.attr,
    &del_attr.attr,
	&debug_attr.attr,
	&print_option_attr.attr,
    NULL
};

static struct attribute_group sched_ctl_attribute_group = {
        .attrs = sched_ctl_attributes
};

static struct kobject *sched_ctl_kobj;
static int __init sched_ctll_init(void)
{
    int rc = 0;
    mutex_init(&mMutex);
	sched_ctl_kobj = kobject_create_and_add("sched_ctl", NULL);
	if (!sched_ctl_kobj)
	{
		printk(KERN_ERR "%s: sched_ctl kobj create error\n", __func__);
		return -ENOMEM;
	}

	rc = sysfs_create_group(sched_ctl_kobj, &sched_ctl_attribute_group);
	if(rc)
		printk(KERN_ERR "%s: failed to create sched_ctl group attributes\n", __func__);

	return rc;
}

static void __exit sched_ctl_exit(void)
{
	sysfs_remove_group(sched_ctl_kobj, &sched_ctl_attribute_group);
	kobject_put(sched_ctl_kobj);
}

module_init(sched_ctll_init);
module_exit(sched_ctl_exit);

MODULE_DESCRIPTION("sched_ctl driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:sched_ctl" );

