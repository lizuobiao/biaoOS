#ifndef _TIMYOS_H
#define _TIMYOS_H

#include <stdint.h>
#include "tLib.h"
#include "tconfig.h"
typedef uint32_t tTaskStack;

#define TINYOS_TASK_STATE_RDY                   0
#define TINYOS_TASK_STATE_DELAYED               (1 << 1)
#define TINYOS_TASK_STATE_SUSPEND               (1 << 2)

typedef struct tTask{

	tTaskStack * stack;
	
	tNode linkNode;
	
	uint32_t delayTicks;
	
	tNode delayNode;
	
	uint32_t prio;
	
	uint32_t state;
	
	uint32_t slice;
	
	uint32_t suspendCount;
	
	void (*clean) (void * param);
	
	void * cleanParam;
	
	uint8_t requestDeleteFlag;
}tTask;

extern tTask * currentTask;

extern tTask * nextTask;

//void tTaskSystemTickHandler();
void tTaskSchedUnRdy (tTask * task);
void tTaskRunFirst (void); 
void tTaskSwitch (void);
void tTaskSchedInit (void);
void tTaskSchedDisable (void);
void tTaskSchedEnable (void);
void tTaskSched (void);
void tTaskSystemTickHandler (void);
void tTaskDelay (uint32_t delay);
void tSetSysTickPeriod(uint32_t ms);
void tTaskSchedRdy (tTask * task);
void tTaskInit(tTask * task, void (*entry)(void *), void *param, uint32_t prio,uint32_t * stack);

void tTaskSchedRemove (tTask * task);
void tInitApp (void);
void tTimeTaskWait (tTask * task, uint32_t ticks);
void tTaskSuspend (tTask * task) ;
void tTaskWakeUp (tTask * task);
void tTaskSchedRemove (tTask * task);
void tTimeTaskRemove (tTask * task);
	
void tTaskSetCleanCallFunc (tTask * task, void (*clean)(void * param), void * param);
void tTaskForceDelete (tTask * task);
void tTaskRequestDelete (tTask * task);

uint8_t tTaskIsRequestedDelete (void);
void tTaskDeleteSelf (void);
uint8_t tTaskIsRequestedDelete (void);

uint32_t tTaskEnterCritical (void);
void tTaskExitCritical (uint32_t status);
#endif

