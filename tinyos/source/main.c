
#include "tinyOS.h"
#include "ARMCM3.h"

tTask * currentTask;

tTask * nextTask;

tTask * taskTable[2];



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
}

void tTaskSched () 
{    
    // 这里的算法很简单。
    // 一共有两个任务。选择另一个任务，然后切换过去
    if (currentTask == taskTable[0]) 
    {
        nextTask = taskTable[1];
    }
    else 
    {
        nextTask = taskTable[0];
    }
    
    tTaskSwitch();
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
    // 什么都没做，除了进行任务切换
    // 由于tTaskSched自动选择另一个任务切换过去，所以其效果就是
    // 两个任务交替运行，与上一次例子不同的是，这是由系统时钟节拍推动的
    // 如果说，上一个例子里需要每个任务主动去调用tTaskSched切换，那么这里就是不管任务愿不愿意，CPU
    // 的运行权都会被交给另一个任务。这样对每个任务就很公平了，不存在某个任务拒不调用tTaskSched而一直占用CPU的情况
    tTaskSched();
}


void delay(int count)
{
	while(--count);
}

int task1Flag;
void task1Entry (void * param) 
{
	tSetSysTickPeriod(10);
    for (;;) 
    {
        task1Flag = 1;
        delay(100);
        task1Flag = 0;
        delay(100);
 //       tTaskSched();
    }
}

int task2Flag;
void task2Entry (void * param) 
{
    for (;;) 
    {
        task2Flag = 1;
        delay(100);
        task2Flag = 0;
        delay(100);
  //      tTaskSched();
    }
}

int flag;

tTask tTask1;
tTask tTask2;
tTaskStack task1Env[1024];     
tTaskStack task2Env[1024];

int main()
{
	tTaskInit(&tTask1,task1Entry,(void *)0x11111111, &task1Env[1024]);
	tTaskInit(&tTask2,task2Entry,(void *)0x22222222, &task2Env[1024]);
	
	taskTable[0] = &tTask1;
    taskTable[1] = &tTask2;
	
	nextTask = taskTable[0];

  // 切换到nextTask， 这个函数永远不会返回
    tTaskRunFirst();
    return 0;
}
