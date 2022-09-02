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

#ifndef _NUBIA_BIG_DATA_LOG_
#define _NUBIA_BIG_DATA_LOG_

#define NUBIA_MODULE_LOG_SIZE	PAGE_SIZE

/**
** the  macro for log module, if you want to add log to big data,
** you can add macro the next before NUBIA_MODULE_MAX
**
** NUBIA_MODULE_CHARGE : log for charge
** NUBIA_MODULE_TOUCH_PANEL : log for touch panel (tp)
** NUBIA_MODULE_DISPLAY_LCD : log for lcd
** NUBIA_MODULE_DISPLAY_DP : log for display port or hdmi
** NUBIA_MODULE_LIGHT_SENSOR : log for light sensor
**/
enum NUBIA_MODULE_NAME{
	NUBIA_MODULE_CHARGE = 0,
	NUBIA_MODULE_TOUCH_PANEL,
	NUBIA_MODULE_DISPLAY_LCD,
	NUBIA_MODULE_DISPLAY_DP,
	NUBIA_MODULE_LIGHT_SENSOR,
	NUBIA_MODULE_MAX = 0xFF
};

static const char * const nubia_module_type_text[] = {
	"kernel_charge", "kernel_touch_panel", "kernel_display_lcd", "kernel_display_dp", 
	"kernel_light_sensor", "Unknown", 
};

struct nubia_log_key_value{
	const char *key;
	const char *args;
	double value;
	struct nubia_log_key_value * next;
};

/**
**  module_name: the module name which you want to send
** module_name: which module you will report
** nb_log_key_value: the detail log
**/
struct nubia_log_module{
	enum NUBIA_MODULE_NAME module_name;
	struct nubia_log_key_value *nb_log_key_value;
};


struct nubia_log_system{
	//struct nubia_log_module *log_module;
	char *kdata;
	struct mutex log_lock;
	
};

ssize_t log_system_report_log(enum NUBIA_MODULE_NAME module, char *key, char *argv, uint32_t value);
#endif


