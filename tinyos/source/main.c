#include "tinyOS.h"
#include "ARMCM3.h"

tTask * currentTask;

tTask * nextTask;

tTask * idleTask;

//tTask * taskTable[2];

// 任务优先级的标记位置结构
tBitmap taskPrioBitmap;

// 所有任务的指针数组：简单起见，只使用两个任务
tTask * taskTable[TINYOS_PRO_COUNT];

uint8_t schedLockCount;

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
	
	task->stack = stack; 
	task->delayTicks = 0;
    task->prio = prio;                                  // 设置任务的优先级

    taskTable[prio] = task;                             // 填入任务优先级表
    tBitmapSet(&taskPrioBitmap, prio);                  // 标记优先级位置中的相应位
}

tTask * tTaskHighestReady (void) 
{
    uint32_t highestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
    return taskTable[highestPrio];
}

void tTaskSchedInit (void)
{
    schedLockCount = 0;
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

void tTaskSystemTickHandler () 
{
    // 检查所有任务的delayTicks数，如果不0的话，减1。
    int i;
	uint32_t status = tTaskEnterCritical();
    for (i = 0; i < TINYOS_PRO_COUNT; i++) 
    {
        if (taskTable[i]->delayTicks > 0)
        {
            taskTable[i]->delayTicks--;
        }else 
        {
            tBitmapSet(&taskPrioBitmap, i);
        }
    }
    
	
	tTaskExitCritical(status);
    // 这个过程中可能有任务延时完毕(delayTicks = 0)，进行一次调度。
    tTaskSched();
}

void tTaskDelay (uint32_t delay) {
	
	uint32_t status = tTaskEnterCritical();
    // 配置好当前要延时的ticks数
    currentTask->delayTicks = delay;
	tBitmapClear(&taskPrioBitmap, currentTask->prio);
	tTaskExitCritical(status);
    // 然后进行任务切换，切换至另一个任务，或者空闲任务
    // delayTikcs会在时钟中断中自动减1.当减至0时，会切换回来继续运行。
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


void delay(int count)
{
	while(--count);
}

int firstSet;
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

int task2Flag;
void task2Entry (void * param) 
{
    for (;;) 
    {
		
        task2Flag = 1;
        tTaskDelay(1);
        task2Flag = 0;
        tTaskDelay(1);
  //      tTaskSched();
    }
}

int flag;

tTask tTask1;
tTask tTask2;
tTaskStack task1Env[1024];     
tTaskStack task2Env[1024];

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
	tTaskInit(&tTask1,task1Entry,(void *)0x11111111,0, &task1Env[1024]);
	tTaskInit(&tTask2,task2Entry,(void *)0x22222222,1,&task2Env[1024]);
	
	taskTable[0] = &tTask1;
    taskTable[1] = &tTask2;
	
	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0,TINYOS_PRO_COUNT-1,&idleTaskEnv[1024]);
    idleTask = &tTaskIdle;
	
	nextTask = tTaskHighestReady();
	
  // 切换到nextTask， 这个函数永远不会返回
    tTaskRunFirst();
    return 0;
}
