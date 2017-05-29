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

tList tTaskDelayedList;

void tTaskInit(tTask * task, void (*entry)(void *), void *param, uint32_t prio,uint32_t * stack)
{
	
	*(--stack) = (unsigned long)(1<<24);                // XPSR, 设置了Thumb模式，恢复到Thumb状态而非ARM状态运行
    *(--stack) = (unsigned long)entry;                  // 程序的入口地址
    *(--stack) = (unsigned long)0x14;                   // R14(LR), 任务不会通过return xxx结束自己，所以未用
    *(--stack) = (unsigned long)0x12;                   // R12, 未用
    *(--stack) = (unsigned long)0x3;                    // R3, 未用
    *(--stack) = (unsigned long)0x2;                    // R2, 未用
    *(--stack) = (unsigned long)0x1;                    // R1, 未用
    *(--stack) = (unsigned long)param;                  // R0 = param, 传给任务的入口函数
    *(--stack) = (unsigned long)0x11;                   // R11, 未用
    *(--stack) = (unsigned long)0x10;                   // R10, 未用
    *(--stack) = (unsigned long)0x9;                    // R9, 未用
    *(--stack) = (unsigned long)0x8;                    // R8, 未用
    *(--stack) = (unsigned long)0x7;                    // R7, 未用
    *(--stack) = (unsigned long)0x6;                    // R6, 未用
    *(--stack) = (unsigned long)0x5;                    // R5, 未用
    *(--stack) = (unsigned long)0x4;                    // R4, 未用
	
	task->slice = TINYOS_SLICE_MAX; 
	task->stack = stack; 
	task->delayTicks = 0;
    task->prio = prio;                                  // 设置任务的优先级
	task->state = TINYOS_TASK_STATE_RDY;                // 设置任务为就绪状态

	 tNodeInit(&(task->linkNode)); 
    tNodeInit(&(task->delayNode));
	
    tListAddLast(&taskTable[prio], &(task->linkNode));   // 填入任务优先级表
    tBitmapSet(&taskPrioBitmap, prio);                  // 标记优先级位置中的相应位
}

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
	
	tTaskExitCritical(status);
    // 这个过程中可能有任务延时完毕(delayTicks = 0)，进行一次调度。
    tTaskSched();
}

void tTaskDelay (uint32_t delay) {
	
	uint32_t status = tTaskEnterCritical();

	tTimeTaskWait(currentTask, delay);
	
	tTaskSchedUnRdy(currentTask);

	tTaskExitCritical(status);

	
    tTaskSched();
}


void tSetSysTickPeriod(uint32_t ms)
{
  SysTick->LOAD  = ms * SystemCoreClock / 1000 - 1; 
  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
  SysTick->VAL   = 0;                           
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk; 
}

void SysTick_Handler () 
{
    tTaskSystemTickHandler();
}



int task1Flag;
void task1Entry (void * param) 
{
	tSetSysTickPeriod(10);
    for (;;) 
    {
		task1Flag = 1;
        tTaskDelay(1);
        task1Flag = 0;
        tTaskDelay(1);
//      tTaskSched();
    }
}

void delay ()
{
    int i;
    for (i = 0; i < 0xFF; i++) {}
}


int task2Flag;
void task2Entry (void * param) 
{
    for (;;) 
    {
		
        task2Flag = 1;
        delay();
        task2Flag = 0;
        delay();
  //      tTaskSched();
    }
}

int task3Flag;
void task3Entry (void * param)
{
    for (;;)
    {
        task3Flag = 1;
        delay();
        task3Flag = 0;
        delay();
    }
}

int flag;

tTask tTask1;
tTask tTask2;
tTask tTask3;
tTaskStack task1Env[1024];     
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];

// 用于空闲任务的任务结构和堆栈空间
tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];
int idle_count = 0;

void idleTaskEntry (void * param) {
    for (;;)
    {
		idle_count++;
        // 空闲任务什么都不做
    }
}

int main()
{
	tTaskSchedInit();
	
	tTaskDelayedInit();
	tTaskInit(&tTask1,task1Entry,(void *)0x11111111,0, &task1Env[1024]);
	tTaskInit(&tTask2,task2Entry,(void *)0x22222222,1,&task2Env[1024]);
	tTaskInit(&tTask3, task3Entry, (void *)0x33333333, 1, &task3Env[1024]);
	
	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0,TINYOS_PRO_COUNT-1,&idleTaskEnv[1024]);
	
	nextTask = tTaskHighestReady();
	
  // 切换到nextTask， 这个函数永远不会返回
    tTaskRunFirst();
    return 0;
}
