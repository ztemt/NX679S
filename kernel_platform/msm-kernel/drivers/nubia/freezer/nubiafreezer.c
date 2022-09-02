// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2011, 2014-2016, 2018, 2020-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/cpuidle.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/irqchip/arm-gic-v3.h>
#include <trace/hooks/cpuidle_psci.h>
#include <trace/hooks/gic.h>

#include <stdbool.h>
#include <linux/signal.h>
#include <linux/sched/task.h>
#include <trace/hooks/signal.h>

#include <linux/cgroup.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/freezer.h>

#define UID_SIZE	100
#define STATE_SIZE	10

static struct device *dev = NULL;
static int state = 1;
static struct workqueue_struct *unfreeze_eventqueue = NULL;
static struct send_event_data
{
	char *type;
	int sig;
	unsigned int uid;
	int pid;
	struct work_struct sendevent_work;
} *wsdata;


enum freezer_state_flags {
	CGROUP_FREEZER_ONLINE	= (1 << 0), /* freezer is fully online */
	CGROUP_FREEZING_SELF	= (1 << 1), /* this freezer is freezing */
	CGROUP_FREEZING_PARENT	= (1 << 2), /* the parent freezer is freezing */
	CGROUP_FROZEN		= (1 << 3), /* this and its descendants frozen */

	/* mask for all FREEZING flags */
	CGROUP_FREEZING		= CGROUP_FREEZING_SELF | CGROUP_FREEZING_PARENT,
};

struct nbfreezer {
	struct cgroup_subsys_state	css;
	unsigned int			state;
};


static inline struct nbfreezer *css_freezer(struct cgroup_subsys_state *css)
{
	return css ? container_of(css, struct nbfreezer, css) : NULL;
}

static inline struct nbfreezer *task_freezer(struct task_struct *task)
{
	return css_freezer(task_css(task, freezer_cgrp_id));
}


bool cgroup_freezing(struct task_struct *task)
{
	bool ret;

	rcu_read_lock();
	ret = task_freezer(task)->state & CGROUP_FREEZING;
	rcu_read_unlock();

	return ret;
}

bool cgroup_frozen(struct task_struct *task)
{
	bool ret;

	rcu_read_lock();
	ret = task_freezer(task)->state & CGROUP_FROZEN;
	rcu_read_unlock();

	return ret;
}

static void sendevent_handler(struct work_struct *work)
{
	struct send_event_data *temp = container_of(work, struct send_event_data, sendevent_work);
	char uid_buf[UID_SIZE] = {0};
	char sig_buf[UID_SIZE] = {0};
	char pid_buf[UID_SIZE] = {0};
	int sig = 0;
	int pid = 0;
	char *envp[4] = {uid_buf, sig_buf, pid_buf, NULL};
	char *type = NULL;
	int uid = 0;

	sig = temp->sig;
	type = temp->type;
	uid = temp->uid;
	pid = temp->pid;

	snprintf(uid_buf, UID_SIZE, "%sUID=%u", type, uid);
	snprintf(sig_buf, UID_SIZE, "SIG=%d", sig);
	snprintf(pid_buf, UID_SIZE, "PID=%d", pid);
	kobject_uevent_env(&dev->kobj, KOBJ_CHANGE, envp);
	kfree(temp);
	temp = NULL;
	//pr_info("have send event uid is %u, reason is %s\n", uid, type);
}

void send_unfreeze_event(char *type, int sig, unsigned int uid, int pid)
{
//	pr_info("Need send event uid is %u, reason is %s\n", uid, type);

	if (state == 0) {
		pr_err("cgroup event module state is %d, not send uevent!!!\n", state);
		return;
	}

	wsdata = kzalloc(sizeof(struct send_event_data), GFP_ATOMIC);
	if (wsdata == NULL) {
		pr_err("send event malloc workqueue data is error!!!\n");
		return;
	}
	wsdata->sig = sig;
	wsdata->type = type;
	wsdata->uid = uid;
	wsdata->pid = pid;
	INIT_WORK(&(wsdata->sendevent_work), sendevent_handler);
	queue_work(unfreeze_eventqueue, &(wsdata->sendevent_work));
}

static void send_unfreeze_event_test(char *buf)
{
	char *s_c[2] = {buf, NULL};

	if (state == 0) {
		pr_err("cgroup event module state is %d, not send uevent!!!\n", state);
		return;
	}

	kobject_uevent_env(&dev->kobj, KOBJ_CHANGE, s_c);
}



static ssize_t unfreeze_show_state(struct device *dev, struct device_attribute *attr, char *buf)
{
	pr_info("show unfreeze event state is %d\n", state);
	return snprintf(buf, STATE_SIZE, "%d\n", state);
}

static ssize_t unfreeze_set_state(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	size_t ret = -EINVAL;

	ret = kstrtoint(buf, STATE_SIZE, &state);
	if (ret < 0)
		return ret;

	pr_info("set unfreeze event state is %d\n", state);
	return count;
}

static ssize_t send(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	char unfreeze_uid[UID_SIZE] = {0};

	snprintf(unfreeze_uid, sizeof(unfreeze_uid), "UID=%s", buf);
	send_unfreeze_event_test(unfreeze_uid);
	pr_info("send unfreeze event %s\n", unfreeze_uid);
	return count;
}




static struct class unfreeze_event_class = {
	.name = "unfreezer",
	.owner = THIS_MODULE,
};


static DEVICE_ATTR(test, S_IRUGO|S_IWUSR, NULL, send);
static DEVICE_ATTR(state, S_IRUGO|S_IWUSR, unfreeze_show_state, unfreeze_set_state);


static const struct attribute *unfreeze_event_attr[] = {
	&dev_attr_test.attr,
	&dev_attr_state.attr,
	NULL,
};
void signal_catch_for_freeze(void *data, int sig, struct task_struct *killer, struct task_struct *dst)
{
//	bool is_freezing = cgroup_freezing(dst);
//	bool is_frozen =  cgroup_frozen(dst);
	if (sig == SIGQUIT || sig == SIGABRT || sig == SIGKILL || sig == SIGSEGV) {
		//if(is_freezing || is_frozen) {
			if(dst->real_cred->uid.val < 10000)
				return ;
			send_unfreeze_event("KILL", sig, (unsigned int)(dst->real_cred->uid.val), dst->pid);
//			pr_info("sig:%d uid:%d pid:%d freezer state(CGROUP_FREEZING:%d CGROUP_FROZEN:%d)\n", sig, dst->real_cred->uid.val, dst->pid,
//			   is_freezing ? 1 : 0, is_frozen ? 1 : 0);
		//}
	}
}
static const struct attribute_group unfreeze_event_attr_group = {
	.attrs = (struct attribute **) unfreeze_event_attr,
};

static int __init nubia_freezer_init(void)
{
	int ret = 0;

	pr_info("cpufreezer uevent init\n");

	ret = class_register(&unfreeze_event_class);
	if (ret < 0) {
		pr_err("cpufreezer unfreezer event: class_register failed!!!\n");
		return ret;
	}
	dev = device_create(&unfreeze_event_class, NULL, MKDEV(0, 0), NULL, "unfreezer");
	if (IS_ERR(dev)) {
		pr_err("cpufreezer:device_create failed!!!\n");
		ret = IS_ERR(dev);
		goto unregister_class;
	}
	ret = sysfs_create_group(&dev->kobj, &unfreeze_event_attr_group);
	if (ret < 0) {
		pr_err("cpufreezer:sysfs_create_group failed!!!\n");
		goto destroy_device;
	}

	unfreeze_eventqueue = create_workqueue("send_unfreeze_event");
	if (unfreeze_eventqueue == NULL) {
		pr_err("unfreeze event module could not create workqueue!!!");
		ret = -ENOMEM;
		goto destroy_device;
	}
	register_trace_android_vh_do_send_sig_info(signal_catch_for_freeze, NULL);
	return 0;

destroy_device:
	device_destroy(&unfreeze_event_class, MKDEV(0, 0));
unregister_class:
	class_unregister(&unfreeze_event_class);
	return 0;
}

module_init(nubia_freezer_init);

MODULE_DESCRIPTION("Nubia Technologies, Inc. Signal unfreeze driver");
MODULE_LICENSE("GPL v2");
