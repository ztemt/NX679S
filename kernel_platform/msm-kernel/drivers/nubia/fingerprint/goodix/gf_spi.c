/*
 * TEE driver for goodix fingerprint sensor
 * Copyright (C) 2016 Goodix
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#define pr_fmt(fmt)		KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/input.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/of_gpio.h>
#include <linux/timer.h>
#include <linux/pm_qos.h>
#include <linux/cpufreq.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <net/sock.h>
#include <net/netlink.h>
//#include <linux/wakelock.h>
#include "gf_spi.h"

#if defined(USE_SPI_BUS)
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#elif defined(USE_PLATFORM_BUS)
#include <linux/platform_device.h>
#endif
#ifdef GOODIX_NEED_FB_CALLBACK
#include <linux/notifier.h>
#include <linux/fb.h>
#endif

#define VER_MAJOR   1
#define VER_MINOR   2
#define PATCH_LEVEL 4
#define EXTEND_VER  2

#define WAKELOCK_HOLD_TIME 1000 /* in ms */

#define GF_SPIDEV_NAME     "goodix,fingerprint"
/*device name after register in charater*/
#define GF_DEV_NAME            "goodix_fp"
#define	GF_INPUT_NAME	    "goodix_fp" /* "gf9658" */

#define	CHRD_DRIVER_NAME	"goodix_fp" /* "goodix_fp_spi" */
#define	CLASS_NAME		    "goodix_fp"

#define N_SPI_MINORS		32	/* ... up to 256 */
static int SPIDEV_MAJOR;

static DECLARE_BITMAP(minors, N_SPI_MINORS);
static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);
//static struct wake_lock fp_wakelock;
static struct gf_dev gf;

static struct gf_key_map maps[] = {
	{ EV_KEY, GF_KEY_INPUT_HOME },
	{ EV_KEY, GF_KEY_INPUT_MENU },
	{ EV_KEY, GF_KEY_INPUT_BACK },
	{ EV_KEY, GF_KEY_INPUT_POWER },
#if defined(SUPPORT_NAV_EVENT)
	{ EV_KEY, GF_NAV_INPUT_UP },
	{ EV_KEY, GF_NAV_INPUT_DOWN },
	{ EV_KEY, GF_NAV_INPUT_RIGHT },
	{ EV_KEY, GF_NAV_INPUT_LEFT },
	{ EV_KEY, GF_KEY_INPUT_CAMERA },
	{ EV_KEY, GF_NAV_INPUT_CLICK },
	{ EV_KEY, GF_NAV_INPUT_DOUBLE_CLICK },
	{ EV_KEY, GF_NAV_INPUT_LONG_PRESS },
	{ EV_KEY, GF_NAV_INPUT_HEAVY },
#endif
};

#define NETLINK_TEST 25
#define MAX_MSGSIZE 32

static int pid = -1;
static struct sock *nl_sk;

int sendnlmsg(char *msg)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	int len = NLMSG_SPACE(MAX_MSGSIZE);
	int ret = 0;

	if (!msg || !nl_sk || !pid)
		return -ENODEV;

	skb = alloc_skb(len, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	nlh = nlmsg_put(skb, 0, 0, 0, MAX_MSGSIZE, 0);
	if (!nlh) {
		kfree_skb(skb);
		return -EMSGSIZE;
	}

	NETLINK_CB(skb).portid = 0;
	NETLINK_CB(skb).dst_group = 0;

	memcpy(NLMSG_DATA(nlh), msg, sizeof(char));
	pr_debug("send message: %d\n", *(char *)NLMSG_DATA(nlh));

	ret = netlink_unicast(nl_sk, skb, pid, MSG_DONTWAIT);
	if (ret > 0)
		ret = 0;

	return ret;
}

static void nl_data_ready(struct sk_buff *__skb)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	char str[100];

	skb = skb_get(__skb);
	if (skb->len >= NLMSG_SPACE(0)) {
		nlh = nlmsg_hdr(skb);

		memcpy(str, NLMSG_DATA(nlh), sizeof(str));
		pid = nlh->nlmsg_pid;

		kfree_skb(skb);
	}

}


int netlink_init(void)
{
	struct netlink_kernel_cfg netlink_cfg;

	memset(&netlink_cfg, 0, sizeof(struct netlink_kernel_cfg));

	netlink_cfg.groups = 0;
	netlink_cfg.flags = 0;
	netlink_cfg.input = nl_data_ready;
	netlink_cfg.cb_mutex = NULL;

	nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST,
			&netlink_cfg);

	if (!nl_sk) {
		pr_err("create netlink socket error\n");
		return 1;
	}

	return 0;
}

void netlink_exit(void)
{
	if (nl_sk != NULL) {
		netlink_kernel_release(nl_sk);
		nl_sk = NULL;
	}

	pr_info("self module exited\n");
}

int gf_pinctrl_init(struct gf_dev* gf_dev)
{
	int ret = 0;
	struct device *dev = &gf_dev->spi->dev;

	gf_dev->pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR_OR_NULL(gf_dev->pinctrl)) {
		FP_LOG_ERROR("Target does not use pinctrl\n");
		ret = PTR_ERR(gf_dev->pinctrl);
		goto err;
	}

	gf_dev->gpio_state_active = pinctrl_lookup_state(gf_dev->pinctrl, "gf_fp_active");
	if (IS_ERR_OR_NULL(gf_dev->gpio_state_active)) {
		FP_LOG_ERROR("Cannot get active pinstate\n");
		ret = PTR_ERR(gf_dev->gpio_state_active);
		goto err;
	}

	gf_dev->gpio_state_suspend = pinctrl_lookup_state(gf_dev->pinctrl, "gf_fp_suspend");
	if (IS_ERR_OR_NULL(gf_dev->gpio_state_suspend)) {
		FP_LOG_ERROR("Cannot get sleep pinstate\n");
		ret = PTR_ERR(gf_dev->gpio_state_suspend);
		goto err;
	}
	FP_LOG_INFO("success\n");
	return 0;
err:
	gf_dev->pinctrl = NULL;
	gf_dev->gpio_state_active = NULL;
	gf_dev->gpio_state_suspend = NULL;
	return ret;
}

int gf_pinctrl_select(struct gf_dev* gf_dev, bool on)
{
	int ret = 0;
	struct pinctrl_state *pins_state;

	pins_state = on ? gf_dev->gpio_state_active : gf_dev->gpio_state_suspend;
	if (IS_ERR_OR_NULL(pins_state)) {
		FP_LOG_ERROR("not a valid '%s' pinstate\n",
			on ? "gf_fp_active" : "gf_fp_suspend");
		return -1;
	}

	ret = pinctrl_select_state(gf_dev->pinctrl, pins_state);
	if (ret) {
		FP_LOG_ERROR("can not set %s pins\n",
			on ? "gf_fp_active" : "gf_fp_suspend");
	}
	FP_LOG_INFO("success\n");
	return ret;
}

int gf_parse_dts(struct gf_dev *gf_dev)
{
	int rc = 0;
	struct device *dev = &gf_dev->spi->dev;
	struct device_node *np = dev->of_node;

	/************avdd*************/
	gf_dev->pwr_avdd_gpio = of_get_named_gpio(np, "goodix,goodix_pwr_avdd", 0);
	if (gf_dev->pwr_avdd_gpio < 0) {
		FP_LOG_ERROR("falied to get goodix_pwr_avdd gpio!\n");
		return gf_dev->pwr_avdd_gpio;
	}
	FP_LOG_INFO("goodix_pwr_avdd gpio:%d\n", gf_dev->pwr_avdd_gpio);
	gpio_free(gf_dev->pwr_avdd_gpio);
	rc = devm_gpio_request(dev, gf_dev->pwr_avdd_gpio, "goodix_pwr_avdd");
	if (rc) {
		FP_LOG_ERROR("failed to request goodix_pwr_avdd gpio, rc = %d\n", rc);
		goto err_avdd;
	}
	gpio_direction_output(gf_dev->pwr_avdd_gpio, 1);

	/************reset*************/
	gf_dev->reset_gpio = of_get_named_gpio(np, "goodix,goodix_reset", 0);
	if (gf_dev->reset_gpio < 0) {
		FP_LOG_ERROR("falied to get reset gpio!\n");
		return gf_dev->reset_gpio;
	}
	FP_LOG_INFO("reset gpio:%d\n", gf_dev->reset_gpio);
	gpio_free(gf_dev->reset_gpio);
	rc = devm_gpio_request(dev, gf_dev->reset_gpio, "goodix_reset");
	if (rc) {
		FP_LOG_ERROR("failed to request reset gpio, rc = %d\n", rc);
		goto err_reset;
	}
	gpio_direction_output(gf_dev->reset_gpio, 1);

	/************init set*************/
	if (gpio_is_valid(gf_dev->pwr_avdd_gpio)) {
		gpio_set_value(gf_dev->pwr_avdd_gpio, 1);
	}
	if (gpio_is_valid(gf_dev->reset_gpio)) {
		gpio_set_value(gf_dev->reset_gpio, 1);
	}
	FP_LOG_INFO("avdd power %s\n", gpio_get_value(gf_dev->pwr_avdd_gpio)? "on" : "off");

	/************irq*************/
	gf_dev->irq_gpio = of_get_named_gpio(np, "goodix,goodix_irq", 0);
	if (gf_dev->irq_gpio < 0) {
		FP_LOG_ERROR("falied to get irq gpio!\n");
		return gf_dev->irq_gpio;
	}
	FP_LOG_INFO("irq_gpio:%d\n", gf_dev->irq_gpio);
	gpio_free(gf_dev->irq_gpio);
	rc = devm_gpio_request(dev, gf_dev->irq_gpio, "goodix_irq");
	if (rc) {
		FP_LOG_ERROR("failed to request irq gpio, rc = %d\n", rc);
		goto err_irq;
	}
	gpio_direction_input(gf_dev->irq_gpio);

	/************id*************/
	gf_dev->id_gpio = of_get_named_gpio(np, "goodix,goodix_id", 0);
	if (gf_dev->id_gpio < 0) {
		FP_LOG_ERROR("falied to get id gpio!\n");
		return gf_dev->id_gpio;
	}
	FP_LOG_INFO("id_gpio:%d\n", gf_dev->id_gpio);
	gpio_free(gf_dev->id_gpio);
	rc = devm_gpio_request(dev, gf_dev->id_gpio, "goodix_id");
	if (rc) {
		FP_LOG_ERROR("failed to request id gpio, rc = %d\n", rc);
		goto err_id;
	}
	gpio_direction_input(gf_dev->id_gpio);

	FP_LOG_INFO("parse success\n");


err_id:
    #ifndef CONFIG_NUBIA_FP_GOODIX_GKI
    devm_gpio_free(dev, gf_dev->irq_gpio);
    #endif
err_irq:
    #ifndef CONFIG_NUBIA_FP_GOODIX_GKI
    devm_gpio_free(dev, gf_dev->reset_gpio);
    #endif
err_reset:
    #ifndef CONFIG_NUBIA_FP_GOODIX_GKI
    devm_gpio_free(dev, gf_dev->pwr_avdd_gpio);
    #endif
err_avdd:
    return rc;
}

void gf_cleanup(struct gf_dev *gf_dev)
{
	FP_LOG_INFO("[info] %s\n", __func__);

	if (gpio_is_valid(gf_dev->pwr_avdd_gpio))
	{
		gpio_free(gf_dev->pwr_avdd_gpio);
		FP_LOG_INFO("remove pwr_avdd_gpio success\n");
	}

	if (gpio_is_valid(gf_dev->irq_gpio)) {
		gpio_free(gf_dev->irq_gpio);
		FP_LOG_INFO("remove irq_gpio success\n");
	}

	if (gpio_is_valid(gf_dev->reset_gpio)) {
		gpio_free(gf_dev->reset_gpio);
		FP_LOG_INFO("remove reset_gpio success\n");
	}
}

int gf_power_on(struct gf_dev *gf_dev)
{
	int rc = 0;

	if(!gf_dev) {
		FP_LOG_ERROR("gf_dev null\n");
	}
	if (gpio_is_valid(gf_dev->reset_gpio)) {
		gpio_set_value(gf_dev->reset_gpio, 1);
	}
	if (gpio_is_valid(gf_dev->pwr_avdd_gpio)) {
		gpio_set_value(gf_dev->pwr_avdd_gpio, 1);
	}
	msleep(10);
	FP_LOG_INFO("power on\n");

	return rc;
}

int gf_power_off(struct gf_dev *gf_dev)
{
	int rc = 0;

	if(!gf_dev) {
		FP_LOG_ERROR("gf_dev null\n");
	}

	if (gpio_is_valid(gf_dev->pwr_avdd_gpio)) {
		gpio_set_value(gf_dev->pwr_avdd_gpio, 0);
	}
	if (gpio_is_valid(gf_dev->reset_gpio)) {
		gpio_set_value(gf_dev->reset_gpio, 0);
	}
	gf_cleanup(gf_dev); //free gpio resource
	FP_LOG_INFO("power off\n");

	return rc;
}

int gf_hw_reset(struct gf_dev *gf_dev, unsigned int delay_ms)
{
	if (!gf_dev) {
		FP_LOG_ERROR("Input buff gf_dev is NULL.\n");
		return -ENODEV;
	}
	gpio_direction_output(gf_dev->reset_gpio, 1);
	gpio_set_value(gf_dev->reset_gpio, 0);
	mdelay(5);
	gpio_set_value(gf_dev->reset_gpio, 1);
	mdelay(delay_ms*10);
	return 0;
}

int gf_irq_num(struct gf_dev *gf_dev)
{
	if (!gf_dev) {
		FP_LOG_ERROR("Input buff gf_dev is NULL.\n");
		return -ENODEV;
	} else {
		return gpio_to_irq(gf_dev->irq_gpio);
	}
}

static void gf_enable_irq(struct gf_dev *gf_dev)
{
	if (gf_dev->irq_enabled) {
		FP_LOG_INFO("IRQ has been enabled.\n");
	} else {
		enable_irq(gf_dev->irq);
		gf_dev->irq_enabled = 1;
	}
}

static void gf_disable_irq(struct gf_dev *gf_dev)
{
	if (gf_dev->irq_enabled) {
		gf_dev->irq_enabled = 0;
		disable_irq(gf_dev->irq);
	} else {
		FP_LOG_INFO("IRQ has been disabled.\n");
	}
}

#ifdef AP_CONTROL_CLK
static long spi_clk_max_rate(struct clk *clk, unsigned long rate)
{
	long lowest_available, nearest_low, step_size, cur;
	long step_direction = -1;
	long guess = rate;
	int max_steps = 10;

	cur = clk_round_rate(clk, rate);
	if (cur == rate)
		return rate;

	/* if we got here then: cur > rate */
	lowest_available = clk_round_rate(clk, 0);
	if (lowest_available > rate)
		return -EINVAL;

	step_size = (rate - lowest_available) >> 1;
	nearest_low = lowest_available;

	while (max_steps-- && step_size) {
		guess += step_size * step_direction;
		cur = clk_round_rate(clk, guess);

		if ((cur < rate) && (cur > nearest_low))
			nearest_low = cur;
		/*
		 * if we stepped too far, then start stepping in the other
		 * direction with half the step size
		 */
		if (((cur > rate) && (step_direction > 0))
				|| ((cur < rate) && (step_direction < 0))) {
			step_direction = -step_direction;
			step_size >>= 1;
		}
	}
	return nearest_low;
}

static void spi_clock_set(struct gf_dev *gf_dev, int speed)
{
	long rate;
	int rc;

	rate = spi_clk_max_rate(gf_dev->core_clk, speed);
	if (rate < 0) {
		FP_LOG_INFO("no match found for requested clock frequency:%d",
				speed);
		return;
	}

	rc = clk_set_rate(gf_dev->core_clk, rate);
}

static int gfspi_ioctl_clk_init(struct gf_dev *data)
{
	FP_LOG_DEBUG("enter\n");

	data->clk_enabled = 0;
	data->core_clk = clk_get(&data->spi->dev, "core_clk");
	if (IS_ERR_OR_NULL(data->core_clk)) {
		FP_LOG_ERROR("fail to get core_clk\n");
		return -EPERM;
	}
	data->iface_clk = clk_get(&data->spi->dev, "iface_clk");
	if (IS_ERR_OR_NULL(data->iface_clk)) {
		FP_LOG_ERROR("fail to get iface_clk\n");
		clk_put(data->core_clk);
		data->core_clk = NULL;
		return -ENOENT;
	}
	return 0;
}

static int gfspi_ioctl_clk_enable(struct gf_dev *data)
{
	int err;

	FP_LOG_DEBUG("enter\n");

	if (data->clk_enabled)
		return 0;

	err = clk_prepare_enable(data->core_clk);
	if (err) {
		FP_LOG_ERROR("fail to enable core_clk\n");
		return -EPERM;
	}

	err = clk_prepare_enable(data->iface_clk);
	if (err) {
		FP_LOG_ERROR("fail to enable iface_clk\n");
		clk_disable_unprepare(data->core_clk);
		return -ENOENT;
	}

	data->clk_enabled = 1;

	return 0;
}

static int gfspi_ioctl_clk_disable(struct gf_dev *data)
{
	FP_LOG_DEBUG("enter\n");

	if (!data->clk_enabled)
		return 0;

	clk_disable_unprepare(data->core_clk);
	clk_disable_unprepare(data->iface_clk);
	data->clk_enabled = 0;

	return 0;
}

static int gfspi_ioctl_clk_uninit(struct gf_dev *data)
{
	FP_LOG_DEBUG("enter\n");

	if (data->clk_enabled)
		gfspi_ioctl_clk_disable(data);

	if (!IS_ERR_OR_NULL(data->core_clk)) {
		clk_put(data->core_clk);
		data->core_clk = NULL;
	}

	if (!IS_ERR_OR_NULL(data->iface_clk)) {
		clk_put(data->iface_clk);
		data->iface_clk = NULL;
	}

	return 0;
}
#endif

static void nav_event_input(struct gf_dev *gf_dev, gf_nav_event_t nav_event)
{
	uint32_t nav_input = 0;

	switch (nav_event) {
	case GF_NAV_FINGER_DOWN:
		FP_LOG_DEBUG("nav finger down\n");
		break;

	case GF_NAV_FINGER_UP:
		FP_LOG_DEBUG("nav finger up\n");
		break;

	case GF_NAV_DOWN:
		nav_input = GF_NAV_INPUT_DOWN;
		FP_LOG_DEBUG("nav down\n");
		break;

	case GF_NAV_UP:
		nav_input = GF_NAV_INPUT_UP;
		FP_LOG_DEBUG("nav up\n");
		break;

	case GF_NAV_LEFT:
		nav_input = GF_NAV_INPUT_LEFT;
		FP_LOG_DEBUG("nav left\n");
		break;

	case GF_NAV_RIGHT:
		nav_input = GF_NAV_INPUT_RIGHT;
		FP_LOG_DEBUG("nav right\n");
		break;

	case GF_NAV_CLICK:
		nav_input = GF_NAV_INPUT_CLICK;
		FP_LOG_DEBUG("nav click\n");
		break;

	case GF_NAV_HEAVY:
		nav_input = GF_NAV_INPUT_HEAVY;
		FP_LOG_DEBUG("nav heavy\n");
		break;

	case GF_NAV_LONG_PRESS:
		nav_input = GF_NAV_INPUT_LONG_PRESS;
		FP_LOG_DEBUG("nav long press\n");
		break;

	case GF_NAV_DOUBLE_CLICK:
		nav_input = GF_NAV_INPUT_DOUBLE_CLICK;
		FP_LOG_DEBUG("nav double click\n");
		break;

	default:
		FP_LOG_DEBUG("unknown nav event: %d\n",nav_event);
		break;
	}

	if ((nav_event != GF_NAV_FINGER_DOWN) &&
			(nav_event != GF_NAV_FINGER_UP)) {
		input_report_key(gf_dev->input, nav_input, 1);
		input_sync(gf_dev->input);
		input_report_key(gf_dev->input, nav_input, 0);
		input_sync(gf_dev->input);
	}
}

static irqreturn_t gf_irq(int irq, void *handle)
{
#if defined(GF_NETLINK_ENABLE)
	struct gf_dev *gf_dev = handle;
	struct device *dev = &gf_dev->spi->dev;
	char msg = GF_NET_EVENT_IRQ;

	pm_wakeup_event(dev, WAKELOCK_HOLD_TIME);
	//wake_lock_timeout(&fp_wakelock, msecs_to_jiffies(WAKELOCK_HOLD_TIME));
	sendnlmsg(&msg);
#elif defined(GF_FASYNC)
	struct gf_dev *gf_dev = &gf;

	if (gf_dev->async)
		kill_fasync(&gf_dev->async, SIGIO, POLL_IN);
#endif

	return IRQ_HANDLED;
}

static int irq_setup(struct gf_dev *gf_dev)
{
	int status;

	gf_dev->irq = gf_irq_num(gf_dev);
	status = request_threaded_irq(gf_dev->irq, NULL, gf_irq,
			IRQF_TRIGGER_RISING | IRQF_ONESHOT,
			"gf", gf_dev);

	if (status) {
		FP_LOG_ERROR("failed to request IRQ:%d\n", gf_dev->irq);
		return status;
	}
	enable_irq_wake(gf_dev->irq);
	gf_dev->irq_enabled = 1;

	return status;
}

static void irq_cleanup(struct gf_dev *gf_dev)
{
	if (gf_dev->irq_enabled) {
		gf_dev->irq_enabled = 0;
		disable_irq(gf_dev->irq);
		disable_irq_wake(gf_dev->irq);
		free_irq(gf_dev->irq, gf_dev);
	} else {
		FP_LOG_INFO("IRQ has been cleaned.\n");
	}
}

static void gf_kernel_key_input(struct gf_dev *gf_dev, struct gf_key *gf_key)
{
	uint32_t key_input = 0;

	if (gf_key->key == GF_KEY_HOME) {
		key_input = GF_KEY_INPUT_HOME;
	} else if (gf_key->key == GF_KEY_POWER) {
		key_input = GF_KEY_INPUT_POWER;
	} else if (gf_key->key == GF_KEY_CAMERA) {
		key_input = GF_KEY_INPUT_CAMERA;
	} else {
		/* add special key define */
		key_input = gf_key->key;
	}
	FP_LOG_INFO("received key event[%d], key=%d, value=%d\n", key_input, gf_key->key, gf_key->value);

	if ((GF_KEY_POWER == gf_key->key || GF_KEY_CAMERA == gf_key->key)
			&& (gf_key->value == 1)) {
		input_report_key(gf_dev->input, key_input, 1);
		input_sync(gf_dev->input);
		input_report_key(gf_dev->input, key_input, 0);
		input_sync(gf_dev->input);
	}

	if (gf_key->key == GF_KEY_HOME) {
		input_report_key(gf_dev->input, key_input, gf_key->value);
		input_sync(gf_dev->input);
	}
}

static long gf_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct gf_dev *gf_dev = &gf;
	struct gf_key gf_key;
#if defined(SUPPORT_NAV_EVENT)
	gf_nav_event_t nav_event = GF_NAV_NONE;
#endif
	int retval = 0;
	u8 netlink_route = NETLINK_TEST;
	struct gf_ioc_chip_info info;

	if (_IOC_TYPE(cmd) != GF_IOC_MAGIC)
		return -ENODEV;

	//if (_IOC_DIR(cmd) & _IOC_READ)
		//retval = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	//else if (_IOC_DIR(cmd) & _IOC_WRITE)
		//retval = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	retval = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
	if (retval)
		return -EFAULT;

	if (gf_dev->device_available == 0) {
		if ((cmd == GF_IOC_ENABLE_POWER) || (cmd == GF_IOC_DISABLE_POWER)) {
			FP_LOG_INFO("power cmd\n");
		} else {
			FP_LOG_INFO("Sensor is power off currently.\n");
			return -ENODEV;
		}
	}

	switch (cmd) {
	case GF_IOC_INIT:
		FP_LOG_INFO("GF_IOC_INIT\n");
		if (copy_to_user((void __user *)arg, (void *)&netlink_route, sizeof(u8))) {
			retval = -EFAULT;
			break;
		}
		break;
	case GF_IOC_EXIT:
		FP_LOG_INFO("GF_IOC_EXIT\n");
		break;
	case GF_IOC_DISABLE_IRQ:
		FP_LOG_INFO("GF_IOC_DISABEL_IRQ\n");
		gf_disable_irq(gf_dev);
		break;
	case GF_IOC_ENABLE_IRQ:
		FP_LOG_INFO("GF_IOC_ENABLE_IRQ\n");
		gf_enable_irq(gf_dev);
		break;
	case GF_IOC_RESET:
		FP_LOG_INFO("GF_IOC_RESET.\n");
		gf_hw_reset(gf_dev, 3);
		break;
	case GF_IOC_INPUT_KEY_EVENT:
		if (copy_from_user(&gf_key, (void __user *)arg, sizeof(struct gf_key))) {
			FP_LOG_ERROR("failed to copy input key event from user to kernel\n");
			retval = -EFAULT;
			break;
		}

		gf_kernel_key_input(gf_dev, &gf_key);
		break;
#if defined(SUPPORT_NAV_EVENT)
	case GF_IOC_NAV_EVENT:
		FP_LOG_DEBUG("GF_IOC_NAV_EVENT\n");
		if (copy_from_user(&nav_event, (void __user *)arg, sizeof(gf_nav_event_t))) {
			FP_LOG_ERROR("failed to copy nav event from user to kernel\n");
			retval = -EFAULT;
			break;
		}

		nav_event_input(gf_dev, nav_event);
		break;
#endif

	case GF_IOC_ENABLE_SPI_CLK:
		FP_LOG_DEBUG("GF_IOC_ENABLE_SPI_CLK\n");
#ifdef AP_CONTROL_CLK
		gfspi_ioctl_clk_enable(gf_dev);
#else
		FP_LOG_DEBUG("Doesn't support control clock.\n");
#endif
		break;
	case GF_IOC_DISABLE_SPI_CLK:
		FP_LOG_DEBUG("GF_IOC_DISABLE_SPI_CLK\n");
#ifdef AP_CONTROL_CLK
		gfspi_ioctl_clk_disable(gf_dev);
#else
		FP_LOG_DEBUG("Doesn't support control clock\n");
#endif
		break;
	case GF_IOC_ENABLE_POWER:
		FP_LOG_INFO("GF_IOC_ENABLE_POWER\n");
		if (gf_dev->device_available == 1)
			FP_LOG_INFO("Sensor has already powered-on.\n");
		else
			gf_power_on(gf_dev);
		gf_dev->device_available = 1;
		break;
	case GF_IOC_DISABLE_POWER:
		FP_LOG_INFO("GF_IOC_DISABLE_POWER\n");
		if (gf_dev->device_available == 0)
			FP_LOG_INFO("Sensor has already powered-off.\n");
		else
			gf_power_off(gf_dev);
		gf_dev->device_available = 0;
		break;
	case GF_IOC_ENTER_SLEEP_MODE:
		FP_LOG_INFO("GF_IOC_ENTER_SLEEP_MODE\n");
		break;
	case GF_IOC_GET_FW_INFO:
		FP_LOG_INFO("GF_IOC_GET_FW_INFO\n");
		break;
	case GF_IOC_REMOVE:
		FP_LOG_INFO("GF_IOC_REMOVE\n");
		irq_cleanup(gf_dev);
		gf_cleanup(gf_dev);
		break;
	case GF_IOC_CHIP_INFO:
		FP_LOG_DEBUG("GF_IOC_CHIP_INFO\n");
		if (copy_from_user(&info, (void __user *)arg, sizeof(struct gf_ioc_chip_info))) {
			retval = -EFAULT;
			break;
		}
		FP_LOG_INFO("vendor_id : 0x%x\n", info.vendor_id);
		FP_LOG_INFO("mode : 0x%x\n", info.mode);
		FP_LOG_INFO("operation: 0x%x\n", info.operation);
		break;
	default:
		FP_LOG_INFO("unsupport cmd:0x%x\n", cmd);
		break;
	}

	return retval;
}

#ifdef CONFIG_COMPAT
static long gf_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return gf_ioctl(filp, cmd, (unsigned long)compat_ptr(arg));
}
#endif /*CONFIG_COMPAT*/


static int gf_open(struct inode *inode, struct file *filp)
{
	struct gf_dev *gf_dev = &gf;
	int status = -ENXIO;

    FP_LOG_INFO("enter\n");
	mutex_lock(&device_list_lock);

	list_for_each_entry(gf_dev, &device_list, device_entry) {
		if (gf_dev->devt == inode->i_rdev) {
			FP_LOG_INFO("Found\n");
			status = 0;
			break;
		}
	}

	if (status == 0) {
		if (status == 0) {
			gf_dev->users++;
			filp->private_data = gf_dev;
			nonseekable_open(inode, filp);
			FP_LOG_INFO("Succeed to open device. irq = %d\n",
					gf_dev->irq);
			if (gf_dev->users == 1) {
				status = gf_parse_dts(gf_dev);
				if (status)
					goto err_parse_dt;

				status = gf_pinctrl_init(gf_dev);
				status = gf_pinctrl_select(gf_dev, true);
				if (gf_dev->irq) {
					FP_LOG_INFO("Already requested irq = %d\n", gf_dev->irq);
				} else {
					status = irq_setup(gf_dev);
					if (status)
					goto err_irq;
					FP_LOG_INFO("requested irq = %d\n", gf_dev->irq);
				}
			}
			gf_hw_reset(gf_dev, 3);
			gf_dev->device_available = 1;
		}
	} else {
		FP_LOG_INFO("No device for minor %d\n", iminor(inode));
	}
	mutex_unlock(&device_list_lock);
    FP_LOG_INFO("exit\n");

	return status;
err_irq:
	gf_cleanup(gf_dev);
err_parse_dt:
	return status;
}

#ifdef GF_FASYNC
static int gf_fasync(int fd, struct file *filp, int mode)
{
	struct gf_dev *gf_dev = filp->private_data;
	int ret;

	ret = fasync_helper(fd, filp, mode, &gf_dev->async);
	FP_LOG_INFO("ret = %d\n", ret);
	return ret;
}
#endif

static int gf_release(struct inode *inode, struct file *filp)
{
	struct gf_dev *gf_dev = &gf;
	int status = 0;

	mutex_lock(&device_list_lock);
	gf_dev = filp->private_data;
	filp->private_data = NULL;

	/*last close?? */
	gf_dev->users--;
	if (!gf_dev->users) {

		FP_LOG_INFO("disble_irq. irq = %d\n", gf_dev->irq);
		irq_cleanup(gf_dev);
		gf_disable_irq(gf_dev);
		/*power off the sensor*/
		gf_dev->device_available = 0;
		gf_power_off(gf_dev);
	}
	mutex_unlock(&device_list_lock);
	return status;
}

static const struct file_operations gf_fops = {
	.owner = THIS_MODULE,
	/* REVISIT switch to aio primitives, so that userspace
	 * gets more complete API coverage.  It'll simplify things
	 * too, except for the locking.
	 */
	.unlocked_ioctl = gf_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = gf_compat_ioctl,
#endif /*CONFIG_COMPAT*/
	.open = gf_open,
	.release = gf_release,
#ifdef GF_FASYNC
	.fasync = gf_fasync,
#endif
};

#ifdef GOODIX_NEED_FB_CALLBACK
static int goodix_fb_state_chg_callback(struct notifier_block *nb,
		unsigned long val, void *data)
{
#if (0) //depend on oled driver macro define
	struct gf_dev *gf_dev;
	struct fb_event *evdata = data;
	unsigned int blank;
	char msg = 0;

	FP_LOG_INFO("event value = %d\n", (int)val);
	FP_LOG_INFO("blank value = %d\n", *(int *)(evdata->data));

	if (val != FB_EARLY_EVENT_BLANK)
		return 0;

	gf_dev = container_of(nb, struct gf_dev, notifier);
	if (evdata && evdata->data && val == FB_EARLY_EVENT_BLANK && gf_dev) {
		blank = *(int *)(evdata->data);
		switch (blank) {
		case FB_BLANK_POWERDOWN:
			if (gf_dev->device_available == 1) {
				gf_dev->fb_black = 1;
#if defined(GF_NETLINK_ENABLE)
				msg = GF_NET_EVENT_FB_BLACK;
				sendnlmsg(&msg);
#elif defined(GF_FASYNC)
				if (gf_dev->async)
					kill_fasync(&gf_dev->async, SIGIO, POLL_IN);
#endif
			}
			break;
		case FB_BLANK_UNBLANK:
			if (gf_dev->device_available == 1) {
				gf_dev->fb_black = 0;
#if defined(GF_NETLINK_ENABLE)
				msg = GF_NET_EVENT_FB_UNBLACK;
				sendnlmsg(&msg);
#elif defined(GF_FASYNC)
				if (gf_dev->async)
					kill_fasync(&gf_dev->async, SIGIO, POLL_IN);
#endif
			}
			break;
		default:
			FP_LOG_INFO("%s defalut\n", __func__);
			break;
		}
	}
#endif //if (0)
	return NOTIFY_OK;
}

static struct notifier_block goodix_noti_block = {
	.notifier_call = goodix_fb_state_chg_callback,
};
#endif

static struct class *gf_class;
#if defined(USE_SPI_BUS)
static int gf_probe(struct spi_device *spi)
#elif defined(USE_PLATFORM_BUS)
static int gf_probe(struct platform_device *pdev)
#endif
{
	struct gf_dev *gf_dev = &gf;
	int status = -EINVAL;
	unsigned long minor;
	int i;
	struct device *dev = NULL;

	FP_LOG_INFO("probe\n");
	/* Initialize the driver data */
	INIT_LIST_HEAD(&gf_dev->device_entry);
#if defined(USE_SPI_BUS)
	gf_dev->spi = spi;
	dev = &spi->dev;
#elif defined(USE_PLATFORM_BUS)
	gf_dev->spi = pdev;
	dev = &pdev->dev;
#endif
	gf_dev->irq_gpio = -EINVAL;
	gf_dev->reset_gpio = -EINVAL;
	gf_dev->pwr_gpio = -EINVAL;
	gf_dev->pwr_avdd_gpio= -EINVAL;
	gf_dev->pwr_vddio_gpio= -EINVAL;
	gf_dev->id_gpio= -EINVAL;
	gf_dev->device_available = 0;
	gf_dev->fb_black = 0;

	/* If we can allocate a minor number, hook up this device.
	 * Reusing minors is fine so long as udev or mdev is working.
	 */
	mutex_lock(&device_list_lock);
	minor = find_first_zero_bit(minors, N_SPI_MINORS);
	if (minor < N_SPI_MINORS) {
		struct device *dev;

		gf_dev->devt = MKDEV(SPIDEV_MAJOR, minor);
		dev = device_create(gf_class, &gf_dev->spi->dev, gf_dev->devt,
				gf_dev, GF_DEV_NAME);
		status = IS_ERR(dev) ? PTR_ERR(dev) : 0;
	} else {
		dev_dbg(&gf_dev->spi->dev, "no minor number available!\n");
		status = -ENODEV;
		mutex_unlock(&device_list_lock);
		goto error_hw;
	}

	if (status == 0) {
		set_bit(minor, minors);
		list_add(&gf_dev->device_entry, &device_list);
	} else {
		gf_dev->devt = 0;
	}
	mutex_unlock(&device_list_lock);

	if (status == 0) {
		/*input device subsystem */
		gf_dev->input = input_allocate_device();
		if (gf_dev->input == NULL) {
			FP_LOG_ERROR("failed to allocate input device\n");
			status = -ENOMEM;
			goto error_dev;
		}
		for (i = 0; i < ARRAY_SIZE(maps); i++)
			input_set_capability(gf_dev->input, maps[i].type, maps[i].code);

		gf_dev->input->name = GF_INPUT_NAME;
		status = input_register_device(gf_dev->input);
		if (status) {
			FP_LOG_ERROR("failed to register input device\n");
			goto error_input;
		}
	}
	FP_LOG_INFO("input device init success\n");
#ifdef AP_CONTROL_CLK
	FP_LOG_INFO("Get the clk resource.\n");
	/* Enable spi clock */
	if (gfspi_ioctl_clk_init(gf_dev))
		goto gfspi_probe_clk_init_failed;

	if (gfspi_ioctl_clk_enable(gf_dev))
		goto gfspi_probe_clk_enable_failed;

	spi_clock_set(gf_dev, 1000000);
#endif

#ifdef GOODIX_NEED_FB_CALLBACK
	gf_dev->notifier = goodix_noti_block;
	fb_register_client(&gf_dev->notifier);
#endif

	//wake_lock_init(&fp_wakelock, WAKE_LOCK_SUSPEND, "fp_wakelock");
	device_init_wakeup(dev, true);


	FP_LOG_INFO("version V%d.%d.%02d.%02d\n", VER_MAJOR, VER_MINOR, PATCH_LEVEL, EXTEND_VER);
	FP_LOG_INFO("probe ok\n");
	return status;

#ifdef AP_CONTROL_CLK
gfspi_probe_clk_enable_failed:
	gfspi_ioctl_clk_uninit(gf_dev);
gfspi_probe_clk_init_failed:
#endif

error_input:
	if (gf_dev->input != NULL)
		input_free_device(gf_dev->input);
error_dev:
	if (gf_dev->devt != 0) {
		FP_LOG_INFO("Err: status = %d\n", status);
		mutex_lock(&device_list_lock);
		list_del(&gf_dev->device_entry);
		device_destroy(gf_class, gf_dev->devt);
		clear_bit(MINOR(gf_dev->devt), minors);
		mutex_unlock(&device_list_lock);
	}
error_hw:
	gf_dev->device_available = 0;

	return status;
}

#if defined(USE_SPI_BUS)
static int gf_remove(struct spi_device *spi)
#elif defined(USE_PLATFORM_BUS)
static int gf_remove(struct platform_device *pdev)
#endif
{
	struct gf_dev *gf_dev = &gf;

	//wake_lock_destroy(&fp_wakelock);
	/* make sure ops on existing fds can abort cleanly */
	if (gf_dev->irq)
		free_irq(gf_dev->irq, gf_dev);

	if (gf_dev->input != NULL)
		input_unregister_device(gf_dev->input);
	input_free_device(gf_dev->input);

	/* prevent new opens */
	mutex_lock(&device_list_lock);
	list_del(&gf_dev->device_entry);
	device_destroy(gf_class, gf_dev->devt);
	clear_bit(MINOR(gf_dev->devt), minors);
	if (gf_dev->users == 0)
		gf_cleanup(gf_dev);

#ifdef GOODIX_NEED_FB_CALLBACK
	fb_unregister_client(&gf_dev->notifier);
#endif
	mutex_unlock(&device_list_lock);

	return 0;
}

static const struct of_device_id gx_match_table[] = {
	{ .compatible = GF_SPIDEV_NAME },
	{},
};

#if defined(USE_SPI_BUS)
static struct spi_driver gf_driver = {
#elif defined(USE_PLATFORM_BUS)
static struct platform_driver gf_driver = {
#endif
	.driver = {
		.name = GF_DEV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = gx_match_table,
	},
	.probe = gf_probe,
	.remove = gf_remove,
};

static int __init gf_init(void)
{
	int status;

	/* Claim our 256 reserved device numbers.  Then register a class
	 * that will key udev/mdev to add/remove /dev nodes.  Last, register
	 * the driver which manages those device numbers.
	 */

	BUILD_BUG_ON(N_SPI_MINORS > 256);
	status = register_chrdev(SPIDEV_MAJOR, CHRD_DRIVER_NAME, &gf_fops);
	if (status < 0) {
		FP_LOG_ERROR("Failed to register char device!\n");
		return status;
	}
	SPIDEV_MAJOR = status;
	gf_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(gf_class)) {
		unregister_chrdev(SPIDEV_MAJOR, gf_driver.driver.name);
		FP_LOG_ERROR("Failed to create class.\n");
		return PTR_ERR(gf_class);
	}
#if defined(USE_PLATFORM_BUS)
	status = platform_driver_register(&gf_driver);
#elif defined(USE_SPI_BUS)
	status = spi_register_driver(&gf_driver);
#endif
	if (status < 0) {
		class_destroy(gf_class);
		unregister_chrdev(SPIDEV_MAJOR, gf_driver.driver.name);
		FP_LOG_ERROR("Failed to register SPI driver.\n");
	}

#ifdef GF_NETLINK_ENABLE
	netlink_init();
#endif
	FP_LOG_INFO("goodixfp status = 0x%x\n", status);
	return 0;
}
module_init(gf_init);

static void __exit gf_exit(void)
{
#ifdef GF_NETLINK_ENABLE
	netlink_exit();
#endif
#if defined(USE_PLATFORM_BUS)
	platform_driver_unregister(&gf_driver);
#elif defined(USE_SPI_BUS)
	spi_unregister_driver(&gf_driver);
#endif
	class_destroy(gf_class);
	unregister_chrdev(SPIDEV_MAJOR, gf_driver.driver.name);
}
module_exit(gf_exit);

MODULE_AUTHOR("Jiangtao Yi, <yijiangtao@goodix.com>");
MODULE_AUTHOR("Jandy Gou, <gouqingsong@goodix.com>");
MODULE_DESCRIPTION("goodix fingerprint sensor device driver");
MODULE_LICENSE("GPL");
