#ifndef _TIMYOS_H
#define _TIMYOS_H

#include <stdint.h>
#include "tLib.h"
#include "tconfig.h"
#include "tTask.h"
#include "tEvent.h"


extern tTask * currentTask;
extern tTask * nextTask;

uint32_t tTaskEnterCritical (void);
void tTaskExitCritical (uint32_t status);

void tTaskRunFirst (void); 
void tTaskSwitch (void);

tTask * tTaskHighestReady (void);

void tTaskSchedInit (void);
void tTaskSchedDisable (void);
void tTaskSchedEnable (void);

void tTaskSchedRdy (tTask * task);
void tTaskSchedUnRdy (tTask * task);
void tTaskSchedRemove (tTask * task);
void tTaskSched (void);

void tTimeTaskWait (tTask * task, uint32_t ticks);
void tTimeTaskWakeUp (tTask * task);
void tTimeTaskRemove (tTask * task);

void tTaskDelay (uint32_t delay);
void tTaskSystemTickHandler (void);

void tInitApp (void);
void tSetSysTickPeriod(uint32_t ms);

#endif

