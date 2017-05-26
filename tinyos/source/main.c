#include "tinyOS.h"
#include "ARMCM3.h"

tTask * currentTask;

tTask * nextTask;

tTask * idleTask;

tTask * taskTable[2];

uint8_t schedLockCount;

void tTaskInit(tTask * task, void (*entry)(void *), void *param, uint32_t * stack)
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
	uint32_t status = tTaskEnterCritical();
    // 空闲任务只有在所有其它任务都不是延时状态时才执行
    // 所以，我们先检查下当前任务是否是空闲任务
	
	if (schedLockCount > 0) 
    {
        tTaskExitCritical(status);
        return;
    }
	
    if (currentTask == idleTask) 
    {
        // 如果是的话，那么去执行task1或者task2中的任意一个
        // 当然，如果某个任务还在延时状态，那么就不应该切换到他。
        // 如果所有任务都在延时，那么就继续运行空闲任务，不进行任何切换了
        if (taskTable[0]->delayTicks == 0) 
        {
            nextTask = taskTable[0];
        }           
        else if (taskTable[1]->delayTicks == 0) 
        {
            nextTask = taskTable[1];
        } else 
        {
            return;
        }
    } 
    else 
    {
        // 如果是task1或者task2的话，检查下另外一个任务
        // 如果另外的任务不在延时中，就切换到该任务
        // 否则，判断下当前任务是否应该进入延时状态，如果是的话，就切换到空闲任务。否则就不进行任何切换
        if (currentTask == taskTable[0]) 
        {
            if (taskTable[1]->delayTicks == 0) 
            {
                nextTask = taskTable[1];
            }
            else if (currentTask->delayTicks != 0) 
            {
                nextTask = idleTask;
            } 
            else 
            {
                return;
            }
        }
        else if (currentTask == taskTable[1]) 
        {
            if (taskTable[0]->delayTicks == 0) 
            {
                nextTask = taskTable[0];
            }
            else if (currentTask->delayTicks != 0) 
            {
                nextTask = idleTask;
            }
            else 
            {
                return;
            }
        }
    }
    
    tTaskSwitch();
	
	tTaskExitCritical(status);
}

void tTaskSystemTickHandler () 
{
    // 检查所有任务的delayTicks数，如果不0的话，减1。
    int i;
	uint32_t status = tTaskEnterCritical();
    for (i = 0; i < 2; i++) 
    {
        if (taskTable[i]->delayTicks > 0)
        {
            taskTable[i]->delayTicks--;
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

int shareCount;

int task1Flag;
void task1Entry (void * param) 
{
	tSetSysTickPeriod(10);
    for (;;) 
    {
		int var;
		var = shareCount;
		
		task1Flag = 1;
        tTaskDelay(1);
		
		var++;
        shareCount = var;
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
		
		shareCount++;
		
        task2Flag = 1;
        tTaskDelay(100);
        task2Flag = 0;
        tTaskDelay(100);
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
	tTaskInit(&tTask1,task1Entry,(void *)0x11111111, &task1Env[1024]);
	tTaskInit(&tTask2,task2Entry,(void *)0x22222222, &task2Env[1024]);
	
	taskTable[0] = &tTask1;
    taskTable[1] = &tTask2;
	
	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, &idleTaskEnv[1024]);
    idleTask = &tTaskIdle;
	
	nextTask = taskTable[0];
	
	tTaskSchedInit();
  // 切换到nextTask， 这个函数永远不会返回
    tTaskRunFirst();
    return 0;
}
