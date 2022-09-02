#ifndef __SCHED_CTL_H__
#define __SCHED_CTL_H__

extern int isTargetPid(int pid);
extern int shouldPrint(int who);
extern void printed(int who);

extern int getDebugCode(void);
#endif

