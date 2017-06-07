#ifndef TCONFIG_H
#define TCONFIG_H

#define	TINYOS_PRO_COUNT				32						// TinyOS任务的优先级序号
#define TINYOS_SLICE_MAX				10	

#define TINYOS_IDLETASK_STACK_SIZE		1024					// 空闲任务的堆栈单元数
#define TINYOS_TIMERTASK_STACK_SIZE		1024					// 定时器任务的堆栈单元数
#define TINYOS_TIMERTASK_PRIO           1                       // 定时器任务的优先级

#define TINYOS_SYSTICK_MS               10                      // 时钟节拍的周期，以ms为单位

#endif /* TCONFIG_H */
