/*
 * Goodix Gesture Module
 *
 * Copyright (C) 2019 - 2020 Goodix, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be a reference
 * to you, when you are integrating the GOODiX's CTP IC into your system,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/atomic.h>
#include "goodix_ts_core.h"
int gesture_flag = 0;
int highrate_data = 0;
int rate_data = 0;
int levelregion_data = 0;
int separation_data = 0;
int open_prevention_data = 0;
int prevention_data = 0;
int open_sensitivity_data = 0;
int sensitivity_data = 0;
 
int NX709_highrate_data = 0;
int NX709_separation_data = 0; 
int NX709_game_data  = 0;
int NX709_sensitivity_data = 0;
int NX709_prevention_data = 0;
	
 
#define QUERYBIT(longlong, bit) (!!(longlong[bit/8] & (1 << bit%8)))

#define GSX_GESTURE_TYPE_LEN	32

/*
 * struct gesture_module - gesture module data
 * @registered: module register state
 * @sysfs_node_created: sysfs node state
 * @gesture_type: valid gesture type, each bit represent one gesture type
 * @gesture_data: store latest gesture code get from irq event
 * @gesture_ts_cmd: gesture command data
 */
struct gesture_module {
	atomic_t registered;
	rwlock_t rwlock;
	u8 gesture_type[GSX_GESTURE_TYPE_LEN];
	u8 gesture_data;
	struct goodix_ext_module module;
};

static struct gesture_module *gsx_gesture; /*allocated in gesture init module*/
static bool module_initialized;


int goodix_gesture_enable(int enable)
{
	int ret = 0;

	if (!module_initialized)
		return 0;		

	if (enable) {
		if (atomic_read(&gsx_gesture->registered))
			ts_info("gesture module has been already registered");
		else
			ret = goodix_register_ext_module_no_wait(&gsx_gesture->module);
	} else {
		if (!atomic_read(&gsx_gesture->registered))
			ts_info("gesture module has been already unregistered");
		else
			ret = goodix_unregister_ext_module(&gsx_gesture->module);
	}

	return ret;
}

/**
 * gsx_gesture_type_show - show valid gesture type
 *
 * @module: pointer to goodix_ext_module struct
 * @buf: pointer to output buffer
 * Returns >=0 - succeed,< 0 - failed
 */
static ssize_t gsx_gesture_type_show(struct goodix_ext_module *module,
				char *buf)
{
	int count = 0, i, ret = 0;
	unsigned char *type;

	type = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!type)
		return -ENOMEM;
	read_lock(&gsx_gesture->rwlock);
	for (i = 0; i < 256; i++) {
		if (QUERYBIT(gsx_gesture->gesture_type, i)) {
			count += scnprintf(type + count,
					   PAGE_SIZE, "%02x,", i);
		}
	}
	if (count > 0)
		ret = scnprintf(buf, PAGE_SIZE, "%s\n", type);
	read_unlock(&gsx_gesture->rwlock);

	kfree(type);
	return ret;
}

/**
 * gsx_gesture_type_store - set vailed gesture
 *
 * @module: pointer to goodix_ext_module struct
 * @buf: pointer to valid gesture type
 * @count: length of buf
 * Returns >0 - valid gestures, < 0 - failed
 */
static ssize_t gsx_gesture_type_store(struct goodix_ext_module *module,
		const char *buf, size_t count)
{
	int i;

	if (count <= 0 || count > 256 || buf == NULL) {
		ts_err("Parameter error");
		return -EINVAL;
	}

	write_lock(&gsx_gesture->rwlock);
	memset(gsx_gesture->gesture_type, 0, GSX_GESTURE_TYPE_LEN);
	for (i = 0; i < count; i++)
		gsx_gesture->gesture_type[buf[i]/8] |= (0x1 << buf[i]%8);
	write_unlock(&gsx_gesture->rwlock);

	return count;
}

static ssize_t gsx_gesture_enable_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n",
			 atomic_read(&gsx_gesture->registered));
}

static ssize_t gsx_gesture_enable_store(struct goodix_ext_module *module,
		const char *buf, size_t count)
{
	bool val;
	int ret;
	ret = strtobool(buf, &val);
	ts_info(" gesture val:%x", val);
	gesture_flag = val;
	ts_info("enable_gesture:%x", core_data->enable_wakeup_gesture);
	ts_info("ts_core_data->fp_switch:%x", core_data->fp_switch);
	if (ret < 0)
		return ret;

	if (val) {
		ret = goodix_gesture_enable(1);
		core_data->enable_wakeup_gesture = true;
		return ret ? ret : count;
	} else if(!core_data->fp_switch) {
		ret = goodix_gesture_enable(0);
		core_data->enable_wakeup_gesture = false;
		return ret ? ret : count;
	}else {
    ts_info("enable_gesture:%x", core_data->enable_wakeup_gesture);
	ts_info("core_data->fp_switch:%x", core_data->fp_switch);
    return ret ? ret : count;
	}
	return ret;
}

static ssize_t gsx_fp_enable_show(struct goodix_ext_module *module,
				char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n",
			atomic_read(&gsx_gesture->registered));
}
static ssize_t gsx_fp_enable_store(struct goodix_ext_module *module,
			const char *buf, size_t count)
{
	bool val;
	int ret;
	bool flag_gesture;			
	ret = strtobool(buf, &val);
	if(core_data->bUpdateOver){
		msleep(4000);
		ts_info("success bUpdateOverk");
	}	
	flag_gesture = val;
	ts_info("core_data->bUpdateOver:%x", core_data->bUpdateOver);
	ts_info("enable_gesture:%x", core_data->enable_wakeup_gesture);
	ts_info("flag_gesture:%x", flag_gesture);
	ts_info("ts_core_data->fp_switch:%x", core_data->fp_switch);
	if (ret < 0)
	return ret;			
	if (flag_gesture) {
	ret = goodix_gesture_enable(1);
	core_data->enable_wakeup_gesture = true;
	return ret ? ret : count;
	} else if(!gesture_flag) {
	ret = goodix_gesture_enable(0);
	core_data->enable_wakeup_gesture = false;
	return ret ? ret : count;
	} else {
    ts_info("enable_gesture:%x", core_data->enable_wakeup_gesture);
	ts_info("flag_gesture:%x", flag_gesture);
    return ret ? ret : count;
	}
	return ret;
}

static ssize_t gsx_nubia_open_prevention_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", open_prevention_data);
}
static ssize_t gsx_nubia_rate_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", rate_data);
}
static ssize_t gsx_nubia_levelregion_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", levelregion_data);
}
static ssize_t gsx_nubia_separation_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", separation_data);
}
static ssize_t gsx_nubia_prevention_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", prevention_data);
}
static ssize_t gsx_nubia_open_sensitivity_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", open_sensitivity_data);
}
static ssize_t gsx_nubia_sensitivity_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", sensitivity_data);
}
static ssize_t gsx_gesture_data_show(struct goodix_ext_module *module,
				char *buf)
{
	ssize_t count;

	read_lock(&gsx_gesture->rwlock);
	count = scnprintf(buf, PAGE_SIZE, "gesture type code:0x%x\n",
			  gsx_gesture->gesture_data);
	read_unlock(&gsx_gesture->rwlock);

	return count;
}

 

static ssize_t gsx_nubia_NX709_highrate_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n",  NX709_highrate_data );
}
static ssize_t gsx_nubia_game_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n",  NX709_game_data );
}
static ssize_t gsx_nubia_NX709_sensitivity_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n",  NX709_sensitivity_data );
}
static ssize_t gsx_nubia_NX709_prevention_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n",  NX709_prevention_data );
}
static ssize_t gsx_nubia_NX709_separation_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n",  NX709_separation_data );
}
/* show chip infoamtion */
static ssize_t gsx_nubia_highrate_show(struct goodix_ext_module *module,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n",  highrate_data );
}
/* show chip infoamtion */
static ssize_t goodix_ts_chip_info_show(struct goodix_ext_module *module,
		char *buf)
{
	//struct goodix_ts_core *core_data = dev_get_drvdata(dev);
	struct goodix_ts_core *cd = module->core_data;
	struct goodix_ts_hw_ops *hw_ops = cd->hw_ops;
	struct goodix_fw_version chip_ver;
	u8 temp_pid[8] = {0};
	int ret;
	int cnt = -EINVAL;

	if (hw_ops->read_version) {
		ret = hw_ops->read_version(cd, &chip_ver);
		if (!ret) {
			memcpy(temp_pid, chip_ver.rom_pid, sizeof(chip_ver.rom_pid));
			cnt = snprintf(&buf[0], PAGE_SIZE,
				"rom_pid:%s\nrom_vid:%02x%02x%02x\n",
				temp_pid, chip_ver.rom_vid[0],
				chip_ver.rom_vid[1], chip_ver.rom_vid[2]);
			cnt += snprintf(&buf[cnt], PAGE_SIZE,
				"patch_pid:%s\npatch_vid:%02x%02x%02x%02x\n",
				chip_ver.patch_pid, chip_ver.patch_vid[0],
				chip_ver.patch_vid[1], chip_ver.patch_vid[2],
				chip_ver.patch_vid[3]);
			cnt += snprintf(&buf[cnt], PAGE_SIZE,
				"sensorid:%d\n", chip_ver.sensor_id);
		}
	}

	if (hw_ops->get_ic_info) {
		ret = hw_ops->get_ic_info(cd, &cd->ic_info);
		if (!ret) {
			cnt += snprintf(&buf[cnt], PAGE_SIZE,
				"config_id:%x\n", cd->ic_info.version.config_id);
			cnt += snprintf(&buf[cnt], PAGE_SIZE,
				"config_version:%x\n", cd->ic_info.version.config_version);
		}
	}

	return cnt;
}

/* reset chip */
static	ssize_t gsx_nubia_open_prevention_store(struct goodix_ext_module *module,
			const char *buf, size_t count)
{	
	struct goodix_ts_cmd open_preventioopn_cmd;
	struct goodix_ts_core *cd = module->core_data;
	int en;
	int ret = 0;
	ts_info("goodix_suspend:%d", core_data->goodix_suspend);
	if(core_data->goodix_suspend)
	return count;
	sscanf(buf, "%x", &en);
	open_preventioopn_cmd.cmd	=  0xB5;
	open_preventioopn_cmd.data [0] = en;
	open_prevention_data = en;
	open_preventioopn_cmd.len = 5;
	ts_info("open_prevention:%d", en);
	ret = cd->hw_ops->send_cmd(cd ,&open_preventioopn_cmd);
	if(!ret){
	ts_err("open prevention cmd");
	}	    					
	return count;
}

static	ssize_t gsx_nubia_rate_store(struct goodix_ext_module *module,
			const char *buf, size_t count)
{	
	struct goodix_ts_cmd rate;
	struct goodix_ts_core *cd = module->core_data;
	int en;
	int ret = 0;
	sscanf(buf, "%x", &en);
	rate.cmd	=  0xB6;
	rate.data [0] = en;
    rate_data = en;
	rate.len = 5;
	ts_info("rate:%d", en);
	ret = cd->hw_ops->send_cmd(cd ,&rate);
	if(!ret){
	ts_err("open rate cmd");
	}	    					
	return count;
}

static	ssize_t gsx_nubia_levelregion_store(struct goodix_ext_module *module,
			const char *buf, size_t count)
{
	struct goodix_ts_cmd levelregion;
	struct goodix_ts_core *cd = module->core_data;
	int en;
	int ret = 0;
	sscanf(buf, "%x", &en);
	levelregion.cmd	=  0x20;
	//sensitivity.data [4] ={69,00,50,00};
	levelregion.data [0] =en;
	//levelregion.data [1] =en;
	//levelregion.data [2] =en;
    levelregion_data = en;
	levelregion.len = 5;
	ts_info("levelregion:%d", en);
	ret = cd->hw_ops->send_cmd(cd ,&levelregion);
	if(!ret){
	ts_err("levelregion cmd");
	}
	return count;
}
static	ssize_t gsx_nubia_highrate_store(struct goodix_ext_module *module,
			const char *buf, size_t count)
{	
	struct goodix_ts_core *cd = module->core_data;
	struct goodix_ts_cmd highrate;
	struct goodix_ts_cmd highrate_noral;
	int en;
	int ret = 0;
	sscanf(buf, "%x", &en);
	highrate.cmd	=  0x1A;
	//sensitivity.data [4] ={69,00,50,00};
	highrate.data [0] =01;
	highrate.data [1] =00;
	highrate_noral.cmd	=  0x1A;
	highrate_noral.data [0] =00;
	highrate_noral.data [1] =00;
	highrate.len = 6;
	highrate_data = en;
	highrate_noral.len = 6;
	ts_info("highrate:%d", en);
	if(en){
	ret = cd->hw_ops->send_cmd(cd ,&highrate);
	}else{
	ret = cd->hw_ops->send_cmd(cd ,&highrate_noral);
	}
	if(!ret){
	ts_err("highrate cmd");
	}							
	return count;
}

static	ssize_t gsx_nubia_game_store(struct goodix_ext_module *module,
			const char *buf, size_t count)
{	
	struct goodix_ts_core *cd = module->core_data;
	struct goodix_ts_cmd NX709_game;
	struct goodix_ts_cmd NX709_game_noral;
	int en;
	int ret = 0;
	sscanf(buf, "%x", &en);
	NX709_game.cmd	=  0xC2;
	//sensitivity.data [4] ={69,00,50,00};
	NX709_game.data [0] =01;
	NX709_game.data [1] =00;
	NX709_game_noral.cmd	=  0xC2;
	NX709_game_noral.data [0] =00;
	NX709_game_noral.data [1] =00;
	NX709_game.len = 6;
	NX709_game_data = en;
	NX709_game_noral.len = 6;
	ts_info("game:%d", en);
	if(en){
	ret = cd->hw_ops->send_cmd(cd ,&NX709_game);
	}else{
	ret = cd->hw_ops->send_cmd(cd ,&NX709_game_noral);
	}
	if(!ret){
	ts_err("game cmd");
	}							
	return count;
}
static	ssize_t gsx_nubia_NX709_highrate_store(struct goodix_ext_module *module,
			const char *buf, size_t count)
{	
	struct goodix_ts_core *cd = module->core_data;
	struct goodix_ts_cmd NX709_highrate;
	struct goodix_ts_cmd NX709_highrate_noral;
	int en;
	int ret = 0;
	sscanf(buf, "%x", &en);
	NX709_highrate.cmd	=  0xC1;
	//sensitivity.data [4] ={69,00,50,00};
	NX709_highrate.data [0] =01;
	NX709_highrate.data [1] =00;
	NX709_highrate_noral.cmd	=  0xC1;
	NX709_highrate_noral.data [0] =00;
	NX709_highrate_noral.data [1] =00;
	NX709_highrate.len = 6;
	NX709_highrate_data = en;
	NX709_highrate_noral.len = 6;
	ts_info("NX709_highrate:%d", en);
	if(en){
	ret = cd->hw_ops->send_cmd(cd ,&NX709_highrate);
	}else{
	ret = cd->hw_ops->send_cmd(cd ,&NX709_highrate_noral);
	}
	if(!ret){
	ts_err("NX709_highrate cmd");
	}							
	return count;
}

static	ssize_t gsx_nubia_NX709_sensitivity_store(struct goodix_ext_module *module,
			const char *buf, size_t count)
{	
	struct goodix_ts_core *cd = module->core_data;
	struct goodix_ts_cmd NX709_sensitivity;
	int en;
	int ret = 0;
	sscanf(buf, "%x", &en);
	NX709_sensitivity.cmd	=  0x27;
	//sensitivity.data [4] ={69,00,50,00};
	NX709_sensitivity.data [0] =en;
	NX709_sensitivity.data [1] =00;
	
	NX709_sensitivity.len = 6;
	NX709_sensitivity_data = en;

	ts_info("NX709_sensitivity:%d", en);

	ret = cd->hw_ops->send_cmd(cd ,&NX709_sensitivity);

	if(!ret){
	ts_err("NX709_sensitivity cmd");
	}							
	return count;
}

static	ssize_t gsx_nubia_NX709_separation_store(struct goodix_ext_module *module,
			const char *buf, size_t count)
{	
	struct goodix_ts_core *cd = module->core_data;
	struct goodix_ts_cmd NX709_separation;
	int en;
	int ret = 0;
	sscanf(buf, "%x", &en);
	NX709_separation.cmd	=  0x28;
	//sensitivity.data [4] ={69,00,50,00};
	NX709_separation.data [0] =en;
	NX709_separation.data [1] =00;
	
	NX709_separation.len = 6;
	NX709_sensitivity_data = en;

	ts_info("NX709_separation:%d", en);
	
	ret = cd->hw_ops->send_cmd(cd ,&NX709_separation);

	if(!ret){
	ts_err("NX709_separation cmd");
	}							
	return count;
}

static	ssize_t gsx_nubia_NX709_prevention_store(struct goodix_ext_module *module,
			const char *buf, size_t count)
{	
	struct goodix_ts_core *cd = module->core_data;
	struct goodix_ts_cmd NX709_prevention;
	int en;
	int ret = 0;
	sscanf(buf, "%x", &en);
	ts_info("goodix_suspend:%d", core_data->goodix_suspend);
	if(core_data->goodix_suspend)
	return count;
	NX709_prevention.cmd	=  0x17;
	//sensitivity.data [4] ={69,00,50,00};
	switch(en) {
	case closeprevention:
	NX709_prevention.data [0] =00;
	NX709_prevention.data [1] =00;
    break;
	case right0_prevention:
	NX709_prevention.data [0] =0x40;
	NX709_prevention.data [1] =0x00;
	break;
	case right1_prevention:
	NX709_prevention.data [0] =0x40;
	NX709_prevention.data [1] =0x40;
	break;
	case right2_prevention:
	NX709_prevention.data [0] =0x40;
	NX709_prevention.data [1] =0x80;
	break;
	case right3_prevention:
	NX709_prevention.data [0] =0x40;
	NX709_prevention.data [1] =0xC0;
	break;
	case left0_prevention:
	NX709_prevention.data [0] =0x80;
	NX709_prevention.data [1] =00;
	break;
	case left1_prevention:
	NX709_prevention.data [0] =0x80;
	NX709_prevention.data [1] =0x40;
	break;
	case left2_prevention:
	NX709_prevention.data [0] =0x80;
	NX709_prevention.data [1] =0x80;
	break;
	case left3_prevention:
	NX709_prevention.data [0] =0x80;
	NX709_prevention.data [1] =0xC0;
	break;
	}

	NX709_prevention.len = 6;
    NX709_prevention_data = en;
	ts_info("NX709_prevention:%d", en);
	
	ret = cd->hw_ops->send_cmd(cd ,&NX709_prevention);

	if(!ret){
	ts_err("NX709_prevention cmd");
	}							
	return count;
}
static	ssize_t gsx_nubia_sensitivity_store(struct goodix_ext_module *module,
			const char *buf, size_t count)
{	
	struct goodix_ts_cmd sensitivity;
	struct goodix_ts_cmd sensitivity_noral;
	struct goodix_ts_core *cd = module->core_data;
	int en;
	int ret = 0;
	sscanf(buf, "%x", &en);
	sensitivity.cmd	=  0xB7;
	//sensitivity.data [4] ={69,00,50,00};
	sensitivity.data [0] =105;
	sensitivity.data [1] =00;
	sensitivity.data [2] =80;
	sensitivity.data [3] =00;
	sensitivity_noral.cmd	=  0xB7;
	sensitivity_noral.data [0] =00;
	sensitivity_noral.data [1] =00;
	sensitivity_noral.data [2] =00;
	sensitivity_noral.data [3] =00;
	sensitivity.len = 8;
	sensitivity_noral.len = 8;
	ts_info("sensitivity:%d", en);
	sensitivity_data = en;
	if(en){
	ret = cd->hw_ops->send_cmd(cd ,&sensitivity);
	}else{
	ret = cd->hw_ops->send_cmd(cd ,&sensitivity_noral);
	}
	if(!ret){
	ts_err("sensitivity cmd");
	}							
	return count;
}
static	ssize_t gsx_nubia_open_sensitivity_store(struct goodix_ext_module *module,
			const char *buf, size_t count)
{      struct goodix_ts_cmd open_sensitivity;
	struct goodix_ts_cmd open_sensitivity_noral;
	struct goodix_ts_core *cd = module->core_data;
	unsigned int x, y;
	int ret = 0;

	sscanf(&buf[0], "%d", &x);
	sscanf(&buf[5], "%d", &y);
	printk("%s: x = %d, y = %d", __func__, x, y);
	open_sensitivity.cmd	=  0xB7;
	//sensitivity.data [4] ={69,00,50,00};
	open_sensitivity.data [0] =x;
	open_sensitivity.data [1] =00;
	open_sensitivity.data [2] =y;
	open_sensitivity.data [3] =00;
	open_sensitivity_noral.cmd	=  0xB7;
	open_sensitivity_noral.data [0] =00;
	open_sensitivity_noral.data [1] =00;
	open_sensitivity_noral.data [2] =00;
	open_sensitivity_noral.data [3] =00;
	open_sensitivity.len = 8;
	open_sensitivity_noral.len = 8;
    open_sensitivity_data = x;
	ret = cd->hw_ops->send_cmd(cd ,&open_sensitivity);
	if(!ret){
	ts_err("open_sensitivity cmd");
	}
	return count;
}
static	ssize_t gsx_nubia_separation_store(struct goodix_ext_module *module,
    const char *buf, size_t count)
{	
    struct goodix_ts_cmd separation_cmd;
	struct goodix_ts_core *cd = module->core_data;
	int en;
	int ret = 0;
	sscanf(buf, "%x", &en);
	separation_cmd.cmd	=  0xB3;
	separation_cmd.data [0] = en;
	separation_cmd.len = 6;
	separation_data = en;
	ts_info("open_separation:%d", en);
	ret = cd->hw_ops->send_cmd(cd ,&separation_cmd);
	if(!ret){
	ts_err("separation cmd");
	}										
	    return count;
}
static	ssize_t gsx_nubia_prevention_store(struct goodix_ext_module *module,
	const char *buf, size_t count)
{   
    struct goodix_ts_cmd prevention_cmd;
	struct goodix_ts_cmd close_prevention_cmd;
	struct goodix_ts_core *cd = module->core_data;
	int en;
	int ret = 0;
	sscanf(buf, "%x", &en);
	ts_info("goodix_suspend:%d", core_data->goodix_suspend);
	if(core_data->goodix_suspend)
	return count;
	//core_data->prevention_flg = en ;
	prevention_cmd.cmd  =  0x17;
	prevention_cmd.data [0] = en;
	prevention_cmd.len = 5;

	close_prevention_cmd.cmd  =  0x18;
	close_prevention_cmd.len = 4;
	//close_prevention_cmd.data [0] = 0X00;	
	prevention_data = en;
	ts_info("prevention:%d", en);
	switch(en) {
		case close_prevention:
			 ret = cd->hw_ops->send_cmd(cd ,&close_prevention_cmd);
			ts_err("close prevention cmd");
			break;
		case left_prevention:
			ret = cd->hw_ops->send_cmd(cd ,&prevention_cmd);
			if(!ret){
			ts_err("left prevention cmd");
			}
			break;
		case right_prevention:
		    ret =cd->hw_ops->send_cmd(cd ,&prevention_cmd);
			if(!ret){
	        ts_err("right prevention cmd");
			}
			break;
		default:return 0;
		    }
return count;
}

 
 
	
const struct goodix_ext_attribute gesture_attrs[] = {
	__EXTMOD_ATTR(type, 0666, gsx_gesture_type_show,
		gsx_gesture_type_store),
	__EXTMOD_ATTR(enable, 0666, gsx_gesture_enable_show,
		gsx_gesture_enable_store),
	__EXTMOD_ATTR(fp, 0666, gsx_fp_enable_show,
		gsx_fp_enable_store),
	__EXTMOD_ATTR(data, 0444, gsx_gesture_data_show, NULL),
	__EXTMOD_ATTR(highrate, 0666, gsx_nubia_highrate_show,
		gsx_nubia_highrate_store),
	__EXTMOD_ATTR(NX709_highrate, 0666, gsx_nubia_NX709_highrate_show,
		gsx_nubia_NX709_highrate_store),
	__EXTMOD_ATTR(NX709_separation, 0666, gsx_nubia_NX709_separation_show,
		gsx_nubia_NX709_separation_store),
	__EXTMOD_ATTR(NX709_prevention, 0666, gsx_nubia_NX709_prevention_show,
		gsx_nubia_NX709_prevention_store),
	__EXTMOD_ATTR(NX709_sensitivity, 0666, gsx_nubia_NX709_sensitivity_show,
		gsx_nubia_NX709_sensitivity_store),
	__EXTMOD_ATTR(game, 0666, gsx_nubia_game_show,
		gsx_nubia_game_store),
	__EXTMOD_ATTR(chip_info, 0666, goodix_ts_chip_info_show,
		NULL),
	__EXTMOD_ATTR(prevention, 0666, gsx_nubia_prevention_show,
		gsx_nubia_prevention_store),
	__EXTMOD_ATTR(open_prevention, 0666, gsx_nubia_open_prevention_show,
		gsx_nubia_open_prevention_store),
	__EXTMOD_ATTR(rate, 0666, gsx_nubia_rate_show,
		gsx_nubia_rate_store),
	__EXTMOD_ATTR(levelregion, 0666, gsx_nubia_levelregion_show,
		gsx_nubia_levelregion_store),
	__EXTMOD_ATTR(sensitivity, 0666, gsx_nubia_sensitivity_show,
		gsx_nubia_sensitivity_store),
	__EXTMOD_ATTR(open_sensitivity, 0666, gsx_nubia_open_sensitivity_show,
		gsx_nubia_open_sensitivity_store),		
	__EXTMOD_ATTR(separation, 0666, gsx_nubia_separation_show,
		gsx_nubia_separation_store),
};

static int gsx_gesture_init(struct goodix_ts_core *cd,
		struct goodix_ext_module *module)
{
	if (!cd || !cd->hw_ops->gesture) {
		ts_err("gesture unsupported");
		return -EINVAL;
	}

	ts_info("gesture switch: ON");
	ts_debug("enable all gesture type");
	/* set all bit to 1 to enable all gesture wakeup */
	memset(gsx_gesture->gesture_type, 0xff, GSX_GESTURE_TYPE_LEN);
	atomic_set(&gsx_gesture->registered, 1);

	return 0;
}

static int gsx_gesture_exit(struct goodix_ts_core *cd,
		struct goodix_ext_module *module)
{
	if (!cd || !cd->hw_ops->gesture) {
		ts_err("gesture unsupported");
		return -EINVAL;
	}

	ts_info("gesture switch: OFF");
	ts_debug("disable all gesture type");
	memset(gsx_gesture->gesture_type, 0x00, GSX_GESTURE_TYPE_LEN);
	atomic_set(&gsx_gesture->registered, 0);
	
	return 0;
}

/**
 * gsx_gesture_ist - Gesture Irq handle
 * This functions is excuted when interrupt happended and
 * ic in doze mode.
 *
 * @cd: pointer to touch core data
 * @module: pointer to goodix_ext_module struct
 * return: 0 goon execute, EVT_CANCEL_IRQEVT  stop execute
 */
static int gsx_gesture_ist(struct goodix_ts_core *cd,
	struct goodix_ext_module *module)
{
	struct goodix_ts_hw_ops *hw_ops = cd->hw_ops;
	struct goodix_ts_event gs_event = {0};
	int ret;

	if (atomic_read(&cd->suspended) == 0)
		return EVT_CONTINUE;

	ret = hw_ops->event_handler(cd, &gs_event);
	if (ret) {
		ts_err("failed get gesture data");
		goto re_send_ges_cmd;
	}
	
	if (!(gs_event.event_type & EVENT_GESTURE)) {
		ts_err("invalid event type: 0x%x",
			cd->ts_event.event_type);
		goto re_send_ges_cmd;
	}

	if (QUERYBIT(gsx_gesture->gesture_type,
		     gs_event.gesture_type)) {
		gsx_gesture->gesture_data = gs_event.gesture_type;
		/* do resume routine */
		ts_info("got valid gesture type 0x%x",
			gs_event.gesture_type);
		if((gsx_gesture->gesture_data == 0xcc)&&(gesture_flag)){
		input_report_key(cd->input_dev, KEY_F10, 1);
		input_sync(cd->input_dev);
		input_report_key(cd->input_dev, KEY_F10, 0);
		input_sync(cd->input_dev);
		}
		if(gsx_gesture->gesture_data == 0x4c){
		input_report_key(cd->input_dev, KEY_F9, 1);
		ts_info("got valid gesture type 0x%x",gs_event.gesture_type);
		input_sync(cd->input_dev);
		input_report_key(cd->input_dev, KEY_F9, 0);
		input_sync(cd->input_dev);
		}
		if((gsx_gesture->gesture_data == 0x46)&&(core_data->fp_switch)){
		input_report_key(cd->input_dev, KEY_F11, 1);
		ts_info("got valid gesture type 0x%x",gs_event.gesture_type);
		ts_info("got valid ts_core_data->fp_switch 0x%x",core_data->fp_switch);
		input_sync(cd->input_dev);
		input_report_key(cd->input_dev, KEY_F11, 0);
		input_sync(cd->input_dev);
		}
		if((gsx_gesture->gesture_data == 0x55)&&(core_data->fp_switch)){
		input_report_key(cd->input_dev, KEY_F12, 1);
		ts_info("got valid gesture type 0x%x",gs_event.gesture_type);
		ts_info("got valid ts_core_data->fp_switch 0x%x",core_data->fp_switch);
		input_sync(cd->input_dev);
		input_report_key(cd->input_dev, KEY_F12, 0);
		input_sync(cd->input_dev);
		}
		goto gesture_ist_exit;
	} else {
		ts_info("unsupported gesture:%x", gs_event.gesture_type);
	}

re_send_ges_cmd:
	if (hw_ops->gesture(cd, 0))
		ts_info("warning: failed re_send gesture cmd");
gesture_ist_exit:
	if (!cd->tools_ctrl_sync)
		hw_ops->after_event_handler(cd);
	return EVT_CANCEL_IRQEVT;
}

/**
 * gsx_gesture_before_suspend - execute gesture suspend routine
 * This functions is excuted to set ic into doze mode
 *
 * @cd: pointer to touch core data
 * @module: pointer to goodix_ext_module struct
 * return: 0 goon execute, EVT_IRQCANCLED  stop execute
 */
static int gsx_gesture_before_suspend(struct goodix_ts_core *cd,
	struct goodix_ext_module *module)
{
	int ret;
	const struct goodix_ts_hw_ops *hw_ops = cd->hw_ops;

	ret = hw_ops->gesture(cd, 0);
	if (ret)
		ts_err("failed enter gesture mode");
	else
		ts_info("enter gesture mode");

	hw_ops->irq_enable(cd, true);
	enable_irq_wake(cd->irq);

	return EVT_CANCEL_SUSPEND;
}

static int gsx_gesture_before_resume(struct goodix_ts_core *cd,
	struct goodix_ext_module *module)
{
	const struct goodix_ts_hw_ops *hw_ops = cd->hw_ops;

	//hw_ops->irq_enable(cd, false);
	disable_irq_wake(cd->irq);
	hw_ops->reset(cd, GOODIX_NORMAL_RESET_DELAY_MS);

	return EVT_CANCEL_RESUME;
}

static struct goodix_ext_module_funcs gsx_gesture_funcs = {
	.irq_event = gsx_gesture_ist,
	.init = gsx_gesture_init,
	.exit = gsx_gesture_exit,
	.before_suspend = gsx_gesture_before_suspend,
	.before_resume = gsx_gesture_before_resume,
};

int gesture_module_init(struct goodix_ts_core *cd)
{
	int ret;
	int i;
	struct kobject *def_kobj = goodix_get_default_kobj();
	struct kobj_type *def_kobj_type = goodix_get_default_ktype();

	gsx_gesture = kzalloc(sizeof(struct gesture_module), GFP_KERNEL);
	if (!gsx_gesture)
		return -ENOMEM;

	gsx_gesture->module.funcs = &gsx_gesture_funcs;
	gsx_gesture->module.priority = EXTMOD_PRIO_GESTURE;
	gsx_gesture->module.name = "Goodix_gsx_gesture";
	gsx_gesture->module.priv_data = gsx_gesture;
	gsx_gesture->module.core_data = cd;

	atomic_set(&gsx_gesture->registered, 0);
	rwlock_init(&gsx_gesture->rwlock);

	/* gesture sysfs init */
	ret = kobject_init_and_add(&gsx_gesture->module.kobj,
			def_kobj_type, def_kobj, "gesture");
	if (ret) {
		ts_err("failed create gesture sysfs node!");
		goto err_out;
	}

	for (i = 0; i < ARRAY_SIZE(gesture_attrs) && !ret; i++)
		ret = sysfs_create_file(&gsx_gesture->module.kobj,
				&gesture_attrs[i].attr);
	if (ret) {
		ts_err("failed create gst sysfs files");
		while (--i >= 0)
			sysfs_remove_file(&gsx_gesture->module.kobj,
					&gesture_attrs[i].attr);

		kobject_put(&gsx_gesture->module.kobj);
		goto err_out;
	}

	module_initialized = true;
	ts_info("gesture module init success");

	return 0;

err_out:
	ts_err("gesture module init failed!");
	kfree(gsx_gesture);
	return ret;
}

void gesture_module_exit(void)
{
	int i;

	ts_info("gesture module exit");
	if (!module_initialized)
		return;

	goodix_gesture_enable(0);

	/* deinit sysfs */
	for (i = 0; i < ARRAY_SIZE(gesture_attrs); i++)
		sysfs_remove_file(&gsx_gesture->module.kobj,
					&gesture_attrs[i].attr);

	kobject_put(&gsx_gesture->module.kobj);
	kfree(gsx_gesture);
	module_initialized = false;
}