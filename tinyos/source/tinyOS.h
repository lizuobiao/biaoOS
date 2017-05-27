#ifndef _TIMYOS_H
#define _TIMYOS_H

#include <stdint.h>
#include "tLib.h"
#include "tconfig.h"
typedef uint32_t tTaskStack;

typedef struct tTask{

	tTaskStack * stack;
	
	uint32_t delayTicks;
	
	uint32_t prio;
}tTask;

extern tTask * currentTask;

extern tTask * nextTask;

void tTaskRunFirst (void); 
void tTaskSwitch (void);
void tTaskSchedInit (void);
void tTaskSchedDisable (void);
void tTaskSchedEnable (void);
void tTaskSched (void);

uint32_t tTaskEnterCritical (void);
void tTaskExitCritical (uint32_t status);
#endif

