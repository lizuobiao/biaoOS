#ifndef _TIMYOS_H
#define _TIMYOS_H

#include <stdint.h>
#include "tLib.h"
#include "tconfig.h"
typedef uint32_t tTaskStack;

#define TINYOS_TASK_STATE_RDY                   0
#define TINYOS_TASK_STATE_DELAYED               (1 << 1)

typedef struct tTask{

	tTaskStack * stack;
	
	tNode linkNode;
	
	uint32_t delayTicks;
	
	tNode delayNode;
	
	uint32_t prio;
	
	uint32_t state;
	
	uint32_t slice;
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

