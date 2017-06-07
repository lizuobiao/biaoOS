#include "tinyOS.h"
#include "ARMCM3.h"

tTask * currentTask;

tTask * nextTask;

tTask * idleTask;

//tTask * taskTable[2];

// 任务优先级的标记位置结构
tBitmap taskPrioBitmap;

// 所有任务的指针数组：简单起见，只使用两个任务
tList taskTable[TINYOS_PRO_COUNT];

uint8_t schedLockCount;

uint32_t tickCount;

tList tTaskDelayedList;

uint32_t idleCount;
uint32_t idleMaxCount;

static void initCpuUsageStat (void);
static void checkCpuUsage (void);
static void cpuUsageSyncWithSysTick (void);

tTask * tTaskHighestReady (void) 
{
    uint32_t highestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
    tNode * node = tListFirst(&taskTable[highestPrio]);
    return (tTask *)tNodeParent(node, tTask, linkNode);
}

void tTaskSchedInit (void)
{
	int i;
    schedLockCount = 0;
	
	tBitmapInit(&taskPrioBitmap);
    for (i = 0; i < TINYOS_PRO_COUNT; i++)
    {
        tListInit(&taskTable[i]);
    }
}


void tTaskSchedDisable (void) 
{
    uint32_t status = tTaskEnterCritical();

    if (schedLockCount < 255) 
    {
        schedLockCount++;
    }

    tTaskExitCritical(status);
}

void tTaskSchedEnable (void) 
{
    uint32_t status = tTaskEnterCritical();

    if (schedLockCount > 0) 
    {
        if (--schedLockCount == 0) 
        {
            tTaskSched(); 
        }
    }

    tTaskExitCritical(status);
}

void tTaskSchedRdy (tTask * task)
{
    tListAddLast(&taskTable[task->prio], &(task->linkNode));
    tBitmapSet(&taskPrioBitmap, task->prio);
}

void tTaskSchedUnRdy (tTask * task)
{
    tListRemove(&taskTable[task->prio], &(task->linkNode));
    if (tListCount(&taskTable[task->prio]) == 0)
    {
        tBitmapClear(&taskPrioBitmap, task->prio);
    }
}

void tTimeTaskRemove (tTask * task)
{
    tListRemove(&tTaskDelayedList, &(task->delayNode));
}

void tTaskSchedRemove (tTask * task)
{
    tListRemove(&taskTable[task->prio], &(task->linkNode));
	
    if (tListCount(&taskTable[task->prio]) == 0) 
    {
        tBitmapClear(&taskPrioBitmap, task->prio);
    }
}

void tTaskSched () 
{
	tTask * tempTask;
	uint32_t status = tTaskEnterCritical();
    // 空闲任务只有在所有其它任务都不是延时状态时才执行
    // 所以，我们先检查下当前任务是否是空闲任务
	
	if (schedLockCount > 0) 
    {
        tTaskExitCritical(status);
        return;
    }
	
	tempTask = tTaskHighestReady();
    if (tempTask != currentTask) 
    {
        nextTask = tempTask;
        tTaskSwitch();   
    }
    
//    tTaskSwitch();
	
	tTaskExitCritical(status);
}

void tTaskDelayedInit (void) 
{
    tListInit(&tTaskDelayedList);
}

void tTimeTaskWait (tTask * task, uint32_t ticks)
{
    task->delayTicks = ticks;
    tListAddLast(&tTaskDelayedList, &(task->delayNode)); 
    task->state |= TINYOS_TASK_STATE_DELAYED;
}

void tTimeTaskWakeUp (tTask * task)
{
    tListRemove(&tTaskDelayedList, &(task->delayNode));
    task->state &= ~TINYOS_TASK_STATE_DELAYED;
}

void tTimeTickInit (void)
{
    tickCount = 0;
}


void tTaskSystemTickHandler () 
{
    // 检查所有任务的delayTicks数，如果不0的话，减1。
	tNode * node;
	uint32_t status = tTaskEnterCritical();
	
    for (node = tTaskDelayedList.headNode.nextNode; node != &(tTaskDelayedList.headNode); node = node->nextNode)
    {
        tTask * task = tNodeParent(node, tTask, delayNode);
        if (--task->delayTicks == 0) 
        {
			
			if (task->waitEvent) 
            {
                // 此时，消息为空，等待结果为超时
                tEventRemoveTask(task, (void *)0, tErrorTimeout);
            }
			
            // 将任务从延时队列中移除
            tTimeTaskWakeUp(task);

            // 将任务恢复到就绪状态
            tTaskSchedRdy(task);            
        }
    }
    
	if (--currentTask->slice == 0)
    {
        // 如果当前任务中还有其它任务的话，那么切换到下一个任务
        // 方法是将当前任务从队列的头部移除，插入到尾部
        // 这样后面执行tTaskSched()时就会从头部取出新的任务取出新的任务作为当前任务运行
        if (tListCount(&taskTable[currentTask->prio]) > 0)
        {
            tListRemoveFirst(&taskTable[currentTask->prio]);
            tListAddLast(&taskTable[currentTask->prio], &(currentTask->linkNode));

            // 重置计数器
            currentTask->slice = TINYOS_SLICE_MAX;
        }
    }
	
	tickCount++;

    // 检查cpu使用率
    checkCpuUsage();
	
	tTaskExitCritical(status);
	
#if TINYOS_ENABLE_TIMER == 1	
	tTimerModuleTickNotify();
#endif
	
    // 这个过程中可能有任务延时完毕(delayTicks = 0)，进行一次调度。
    tTaskSched();
}



static float cpuUsage;                      // cpu使用率统计
static uint32_t enableCpuUsageStat;         // 是否使能cpu统计

/**********************************************************************************************************
** Function name        :   initCpuUsageStat
** Descriptions         :   初始化cpu统计
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void initCpuUsageStat (void)
{
    idleCount = 0;
    idleMaxCount = 0;
    cpuUsage = 0;
    enableCpuUsageStat = 0;
}

/**********************************************************************************************************
** Function name        :   checkCpuUsage
** Descriptions         :   检查cpu使用率
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void checkCpuUsage (void)
{
    // 与空闲任务的cpu统计同步
    if (enableCpuUsageStat == 0)
    {
        enableCpuUsageStat = 1;
        tickCount = 0;
        return;
    }

    if (tickCount == TICKS_PER_SEC)
    {
        // 统计最初1s内的最大计数值
        idleMaxCount = idleCount;
        idleCount = 0;

        // 计数完毕，开启调度器，允许切换到其它任务
        tTaskSchedEnable();
    }
    else if (tickCount % TICKS_PER_SEC == 0)
    {
        // 之后每隔1s统计一次，同时计算cpu利用率
        cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
        idleCount = 0;
    }
}

/**********************************************************************************************************
** Function name        :   cpuUsageSyncWithSysTick
** Descriptions         :   为检查cpu使用率与系统时钟节拍同步
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void cpuUsageSyncWithSysTick (void)
{
    // 等待与时钟节拍同步
    while (enableCpuUsageStat == 0)
    {
        ;;
    }
}

/**********************************************************************************************************
** Function name        :   cpuUsageSyncWithSysTick
** Descriptions         :   为检查cpu使用率与系统时钟节拍同步
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
float tCpuUsageGet (void)
{
    float usage = 0;

    uint32_t status = tTaskEnterCritical();
    usage = cpuUsage;
    tTaskExitCritical(status);

    return usage;
}

// 用于空闲任务的任务结构和堆栈空间
tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];
int idle_count = 0;

void idleTaskEntry (void * param) {
	
	tTaskSchedDisable();
	
	tInitApp();
#if TINYOS_ENABLE_TIMER == 1
	tTimerInitTask();
#endif
	tSetSysTickPeriod(TINYOS_SYSTICK_MS);
	
	cpuUsageSyncWithSysTick();
    for (;;)
    {
		uint32_t status = tTaskEnterCritical();
        idleCount++;
        tTaskExitCritical(status);
    }
}

int main()
{
	tTaskSchedInit();
	
	tTaskDelayedInit();
#if TINYOS_ENABLE_TIMER == 1	
	tTimerModuleInit();
#endif	
	tTimeTickInit();

    // 初始化cpu统计
    initCpuUsageStat();
	
	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, TINYOS_PRO_COUNT - 1, idleTaskEnv, TINYOS_IDLETASK_STACK_SIZE);
	
	nextTask = tTaskHighestReady();
	
  // 切换到nextTask， 这个函数永远不会返回
    tTaskRunFirst();
    return 0;
}
