/*
 * aw8697.c
 *
 * Version: v1.4.16
 *
 * Copyright (c) 2020 AWINIC Technology CO., LTD
 *
 *  Author: Nick Li <liweilei@awinic.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include "aw8697_common.h"

int vibrator_read_file(char *file_path, char *read_buf, int count)
{
        struct file *file_p;
        mm_segment_t old_fs;
        int vfs_retval = -EINVAL;

        if (NULL == file_path) {
	    pr_err("%s: vibrator_read_file file_path is NULL\n", __func__);
	    return -EINVAL;
        }

        file_p = filp_open(file_path, O_RDONLY , 0666);
        if (IS_ERR(file_p)) {
	    pr_err("%s: vibrator_read_file filp_open return error\n", __func__);
	    return -EINVAL;
        }

        old_fs = get_fs();
        set_fs(KERNEL_DS);
        file_p->f_pos = 0;
        vfs_retval = vfs_read(file_p, (char*)read_buf, count, &file_p->f_pos);
        if (vfs_retval < 0) {
			pr_err("%s: vibrator_read_file vfs_write return error\n", __func__);
            goto file_close;
        }

file_close:
        set_fs(old_fs);
        filp_close(file_p, NULL);

        return vfs_retval;
}


int vibrator_write_file(char *file_path, const char *write_buf, int count)
{
        struct file *file_p;
        mm_segment_t old_fs;
        int vfs_retval = -EINVAL;

        if (NULL == file_path) {
			pr_err("%s: vibrator_write_file file_path is NULL\n", __func__);
            return -EINVAL;
        }

        file_p = filp_open(file_path, O_CREAT|O_RDWR|O_TRUNC , 0666);
        if (IS_ERR(file_p)) {
			pr_err("%s: vibrator_write_file filp_open return error\n", __func__);
            goto error;
        }

        old_fs = get_fs();
        set_fs(KERNEL_DS);

        vfs_retval = vfs_write(file_p, (char*)write_buf, count, &file_p->f_pos);
        if (vfs_retval < 0) {
			pr_err("%s: vibrator_write_file vfs_write return error\n", __func__);
            goto file_close;
        }

file_close:
        set_fs(old_fs);
        filp_close(file_p, NULL);
error:
        return vfs_retval;
}
