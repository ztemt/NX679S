/*
 * Synaptics DSX touchscreen driver
 *
 * Copyright (C) 2012-2016 Synaptics Incorporated. All rights reserved.
 *
 * Copyright (C) 2021 liangchuan <liangchuan@nubia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND SYNAPTICS
 * EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * IN NO EVENT SHALL SYNAPTICS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, PUNITIVE, OR CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED
 * AND BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF SYNAPTICS WAS ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF COMPETENT JURISDICTION DOES
 * NOT PERMIT THE DISCLAIMER OF DIRECT DAMAGES OR ANY OTHER DAMAGES, SYNAPTICS'
 * TOTAL CUMULATIVE LIABILITY TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S.
 * DOLLARS.
 */

#include <linux/slab.h>		/* For kmalloc. */
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/string_helpers.h>
#include <linux/sysfs.h>
#include <linux/ctype.h>
#include <linux/module.h>

#include "nblog/nblog.h"

static struct kobject *nblog_kobj = NULL;
struct kset *nblog_kset;
//struct nubia_log_system *log_system;
struct mutex log_lock;
char *kdata;

ssize_t log_system_report_log(enum NUBIA_MODULE_NAME module, char *key, char *argv, uint32_t value)
{
	char *envp[] = {kdata, NULL};

	mutex_lock(&log_lock);
	memset(kdata, 0, strlen(kdata));
	if(!argv)
	{
		sprintf(kdata, "event=%s,%s=%d", nubia_module_type_text[module], key, value);
	}
	else
	{
		sprintf(kdata, "event=%s,%s=%s", nubia_module_type_text[module], key, argv);
	}

	printk("---[nubia_log]--kdata=%s\n", kdata);
	kobject_uevent_env(nblog_kobj, KOBJ_CHANGE, envp);
	
	mutex_unlock(&log_lock);
	return 0;
}

EXPORT_SYMBOL(log_system_report_log);

static ssize_t log_system_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t log_system_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t size)
{
	uint32_t val = 0;

	sscanf(buf, "%d", &val);

	switch (val){
		case 1:	
			log_system_report_log(NUBIA_MODULE_CHARGE, "charge_type", "QC3.0", 0);
			break;

		case 2:
			log_system_report_log(NUBIA_MODULE_TOUCH_PANEL, "tp_type", "goodix", 0);
			break;

		case 3:
			log_system_report_log(NUBIA_MODULE_DISPLAY_LCD, "lcd_lenght", NULL, 1920);
			break;

		case 4:
			log_system_report_log(NUBIA_MODULE_DISPLAY_DP, "dp_version", "dp_3.0", 0);
			break;

		case 5:
			log_system_report_log(NUBIA_MODULE_LIGHT_SENSOR, "light_vale", NULL, 500);
			break;
		default:
			break;

	}

	return size;
}

static struct kobj_attribute nubia_log_attrs[] = {
	__ATTR(nblog, 0664, log_system_show, log_system_store),
};

int __init nubia_log_system_init(void)
{
	int retval = 0;

	nblog_kset = kset_create_and_add("nblog_kset", NULL, NULL);
	if (!nblog_kset)
	{
		pr_err("failed to create kset\n");
		return -ENOMEM;
	}

	nblog_kobj = kobject_create_and_add("nblog", kernel_kobj);
	nblog_kobj->kset = nblog_kset;

	/* Create attribute files associated with this kobject */
	retval = sysfs_create_file(nblog_kobj, &nubia_log_attrs[0].attr);
	if (retval < 0) {
		pr_err("failed to create sysfs attributes\n");
		goto err_sys_creat;
	}

	kdata = (char *)kzalloc(NUBIA_MODULE_LOG_SIZE, GFP_KERNEL);
	if(!kdata){
		pr_err("%s: create buffer fail \n", __func__);
		goto err_sys_creat;
	}
	
	mutex_init(&log_lock);
	return retval;

err_sys_creat:
	sysfs_remove_file(nblog_kobj, &nubia_log_attrs[0].attr);
	kobject_put(nblog_kobj);
	return retval;
}

static void __exit nubia_log_system_exit(void)
{
	sysfs_remove_file(nblog_kobj, &nubia_log_attrs[0].attr);
	kobject_put(nblog_kobj);
}

module_init(nubia_log_system_init);
module_exit(nubia_log_system_exit);

MODULE_DESCRIPTION("nubia_log driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:nubia_log" );



