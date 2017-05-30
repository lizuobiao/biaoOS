#ifndef TTASK_H
#define TTASK_H

struct _tEvent;


#define	TINYOS_TASK_STATE_RDY					0
#define TINYOS_TASK_STATE_DESTROYED             (1 << 0)
#define	TINYOS_TASK_STATE_DELAYED				(1 << 1)
#define TINYOS_TASK_STATE_SUSPEND               (1 << 2)

#define TINYOS_TASK_WAIT_MASK                   (0xFF << 16)

typedef uint32_t tTaskStack;

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
	
    struct _tEvent * waitEvent;

    void * eventMsg;

    uint32_t waitEventResult;
}tTask;

typedef struct _tTaskInfo {
    // 任务延时计数器
    uint32_t delayTicks;

    // 任务的优先级
    uint32_t prio;

    // 任务当前状态
    uint32_t state;

    // 当前剩余的时间片
    uint32_t slice;

    // 被挂起的次数
    uint32_t suspendCount;
	
}tTaskInfo;

void tTaskInit(tTask * task, void (*entry)(void *), void *param, uint32_t prio,uint32_t * stack);
void tTaskSuspend (tTask * task) ;
void tTaskWakeUp (tTask * task);

void tTaskSetCleanCallFunc (tTask * task, void (*clean)(void * param), void * param);
void tTaskForceDelete (tTask * task);
void tTaskRequestDelete (tTask * task);
uint8_t tTaskIsRequestedDelete (void);
void tTaskDeleteSelf (void);

void tTaskGetInfo (tTask * task, tTaskInfo * info);

#endif /* TTASK_H */ 
