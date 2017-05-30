#include "tinyOS.h"

void tEventInit (tEvent * event, tEventType type)
{
	event->type = type;
	tListInit(&event->waitList);
}

void tEventWait (tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout)
{
  // 进入临界区
    uint32_t status = tTaskEnterCritical();

  task->state |= state;         // 标记任务处于等待某种事件的状态
  task->waitEvent = event;        // 设置任务等待的事件结构
  task->eventMsg = msg;         // 设置任务等待事件的消息存储位置 
                      // 因有时候需要接受消息，所以需要接受区
  task->waitEventResult = tErrorNoError;  // 清空事件的等待结果

    // 将任务从就绪队列中移除
    tTaskSchedUnRdy(task);

  // 将任务插入到等待队列中
  tListAddLast(&event->waitList, &task->linkNode);

  // 如果发现有设置超时，在同时插入到延时队列中
  // 当时间到达时，由延时处理机制负责将任务从延时列表中移除，同时从事件列表中移除
  if (timeout) 
  {
    tTimeTaskWait(task, timeout);
  }

  // 退出临界区
    tTaskExitCritical(status); 
}

tTask * tEventWakeUp (tEvent * event, void * msg, uint32_t result)
{
    tNode  * node;
    tTask  * task = (tTask * )0;
    
    // 进入临界区
    uint32_t status = tTaskEnterCritical();

    // 取出等待队列中的第一个结点
    if((node = tListRemoveFirst(&event->waitList)) != (tNode *)0)
    {                        
      // 转换为相应的任务结构                                          
        task = (tTask *)tNodeParent(node, tTask, linkNode);

        // 设置收到的消息、结构，清除相应的等待标志位
        task->waitEvent = (tEvent *)0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~TINYOS_TASK_WAIT_MASK;

        // 任务申请了超时等待，这里检查下，将其从延时队列中移除
        if (task->delayTicks != 0)
        { 
            tTimeTaskWakeUp(task);
        }

        // 将任务加入就绪队列
        tTaskSchedRdy(task);        
    }  

    // 退出临界区
    tTaskExitCritical(status); 
   
    return task;         
}

void tEventRemoveTask (tTask * task, void * msg, uint32_t result)
{     
 	  // 进入临界区
    uint32_t status = tTaskEnterCritical();

	 // 将任务从所在的等待队列中移除
	 // 注意，这里没有检查waitEvent是否为空。既然是从事件中移除，那么认为就不可能为空
	 tListRemove(&task->waitEvent->waitList, &task->linkNode);

  	// 设置收到的消息、结构，清除相应的等待标志位
    task->waitEvent = (tEvent *)0;
    task->eventMsg = msg;
   	task->waitEventResult = result;
	 task->state &= ~TINYOS_TASK_WAIT_MASK;

	// 退出临界区
    tTaskExitCritical(status); 
}

uint32_t tEventRemoveAll (tEvent * event, void * msg, uint32_t result)
{
    tNode  * node;
    uint32_t count;
    
    // 进入临界区
    uint32_t status = tTaskEnterCritical();

    // 获取等待中的任务数量
    count = tListCount(&event->waitList);

    // 遍历所有等待中的任务
    while ((node = tListRemoveFirst(&event->waitList)) != (tNode *)0)
    {                                                                   
        // 转换为相应的任务结构                                          
        tTask * task = (tTask *)tNodeParent(node, tTask, linkNode);
        
        // 设置收到的消息、结构，清除相应的等待标志位
        task->waitEvent = (tEvent *)0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~TINYOS_TASK_WAIT_MASK;

        // 任务申请了超时等待，这里检查下，将其从延时队列中移除
        if (task->delayTicks != 0)
        { 
            tTimeTaskWakeUp(task);
        }

        // 将任务加入就绪队列
        tTaskSchedRdy(task);        
    }  

    // 退出临界区
    tTaskExitCritical(status); 

    return  count;
}

uint32_t tEventWaitCount (tEvent * event)
{  
    uint32_t count = 0;

    // 进入临界区
    uint32_t status = tTaskEnterCritical();

    count = tListCount(&event->waitList);  

    // 退出临界区
    tTaskExitCritical(status);     

    return count;
} 


