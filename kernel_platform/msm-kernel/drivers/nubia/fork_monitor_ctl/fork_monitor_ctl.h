#ifndef __FORK_MONITOR_CTL_H__ 
#define __FORK_MONITOR_CTL_H__
#include <linux/types.h>
#include <linux/cdev.h>

void f_monitor_send_uevent(int pid, int tid);
bool need_to_send_uevent(int pid);

#endif
