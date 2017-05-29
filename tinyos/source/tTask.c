#include "tinyOS.h"

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
	
	tTaskSchedRdy(task); 
}

void tTaskSuspend (tTask * task) 
{
    // 进入临界区
    uint32_t status = tTaskEnterCritical();

    // 不允许对已经进入延时状态的任务挂起
    if (!(task->state & TINYOS_TASK_STATE_DELAYED)) 
    {
        // 增加挂起计数，仅当该任务被执行第一次挂起操作时，才考虑是否
        // 要执行任务切换操作
        if (++task->suspendCount <= 1)
        {
            // 设置挂起标志
            task->state |= TINYOS_TASK_STATE_SUSPEND;

            // 挂起方式很简单，就是将其从就绪队列中移除，这样调度器就不会发现他
            // 也就没法切换到该任务运行
            tTaskSchedUnRdy(task);

            // 当然，这个任务可能是自己，那么就切换到其它任务
            if (task == currentTask)
            {
                tTaskSched();
            }
        }
    }

    // 退出临界区
    tTaskExitCritical(status); 
}

/**********************************************************************************************************
** Function name        :   tTaskWakeUp
** Descriptions         :   唤醒被挂起的任务
** parameters           :   task        待唤醒的任务
** Returned value       :   无
***********************************************************************************************************/
void tTaskWakeUp (tTask * task)
{
    // 进入临界区
    uint32_t status = tTaskEnterCritical();

     // 检查任务是否处于挂起状态
    if (task->state & TINYOS_TASK_STATE_SUSPEND)
    {
        // 递减挂起计数，如果为0了，则清除挂起标志，同时设置进入就绪状态
        if (--task->suspendCount == 0) 
        {
            // 清除挂起标志
            task->state &= ~TINYOS_TASK_STATE_SUSPEND;

            // 同时将任务放回就绪队列中
            tTaskSchedRdy(task);

            // 唤醒过程中，可能有更高优先级的任务就绪，执行一次任务调度
            tTaskSched();
        }
    }

    // 退出临界区
    tTaskExitCritical(status); 
}


void tTaskSetCleanCallFunc (tTask * task, void (*clean)(void * param), void * param) 
{
    task->clean = clean;
    task->cleanParam = param;
}

void tTaskForceDelete (tTask * task) 
{
    // 进入临界区
    uint32_t status = tTaskEnterCritical();

     // 如果任务处于延时状态，则从延时队列中删除
    if (task->state & TINYOS_TASK_STATE_DELAYED) 
    {
        tTimeTaskRemove(task);
    }
		// 如果任务不处于挂起状态，那么就是就绪态，从就绪表中删除
    else if (!(task->state & TINYOS_TASK_STATE_SUSPEND))
    {
        tTaskSchedRemove(task);
    }

    // 删除时，如果有设置清理函数，则调用清理函数
    if (task->clean) 
    {
        task->clean(task->cleanParam);
    }

    // 如果删除的是自己，那么需要切换至另一个任务，所以执行一次任务调度
    if (currentTask == task) 
    {
        tTaskSched();
    }

    // 退出临界区
    tTaskExitCritical(status); 
}

void tTaskRequestDelete (tTask * task)
{
   // 进入临界区
    uint32_t status = tTaskEnterCritical();

    // 设置清除删除标记
    task->requestDeleteFlag = 1;

    // 退出临界区
    tTaskExitCritical(status); 
}

uint8_t tTaskIsRequestedDelete (void)
{
    uint8_t delete;

   // 进入临界区
    uint32_t status = tTaskEnterCritical();

    // 获取请求删除标记
    delete = currentTask->requestDeleteFlag;

    // 退出临界区
    tTaskExitCritical(status); 

    return delete;
}

void tTaskDeleteSelf (void)
{
    // 进入临界区
    uint32_t status = tTaskEnterCritical();

    // 任务在调用该函数时，必须是处于就绪状态，不可能处于延时或挂起等其它状态
    // 所以，只需要从就绪队列中移除即可
    tTaskSchedRemove(currentTask);

    // 删除时，如果有设置清理函数，则调用清理函数
    if (currentTask->clean)
    {
        currentTask->clean(currentTask->cleanParam);
    }

    // 接下来，肯定是切换到其它任务去运行
    tTaskSched();

    // 退出临界区
    tTaskExitCritical(status);
}

void tTaskGetInfo (tTask * task, tTaskInfo * info)
{
   // 进入临界区
    uint32_t status = tTaskEnterCritical();

    info->delayTicks = task->delayTicks;                // 延时信息
    info->prio = task->prio;                            // 任务优先级
    info->state = task->state;                          // 任务状态
    info->slice = task->slice;                          // 剩余时间片
    info->suspendCount = task->suspendCount;            // 被挂起的次数

    // 退出临界区
    tTaskExitCritical(status); 
}


