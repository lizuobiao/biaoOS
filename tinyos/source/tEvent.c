#include "tinyOS.h"

void tEventInit (tEvent * event, tEventType type)
{
	event->type = type;
	tListInit(&event->waitList);
}

void tEventWait (tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout)
{
  // �����ٽ���
    uint32_t status = tTaskEnterCritical();

  task->state |= state;         // ��������ڵȴ�ĳ���¼���״̬
  task->waitEvent = event;        // ��������ȴ����¼��ṹ
  task->eventMsg = msg;         // ��������ȴ��¼�����Ϣ�洢λ�� 
                      // ����ʱ����Ҫ������Ϣ��������Ҫ������
  task->waitEventResult = tErrorNoError;  // ����¼��ĵȴ����

    // ������Ӿ����������Ƴ�
    tTaskSchedUnRdy(task);

  // ��������뵽�ȴ�������
  tListAddLast(&event->waitList, &task->linkNode);

  // ������������ó�ʱ����ͬʱ���뵽��ʱ������
  // ��ʱ�䵽��ʱ������ʱ������Ƹ����������ʱ�б����Ƴ���ͬʱ���¼��б����Ƴ�
  if (timeout) 
  {
    tTimeTaskWait(task, timeout);
  }

  // �˳��ٽ���
    tTaskExitCritical(status); 
}

tTask * tEventWakeUp (tEvent * event, void * msg, uint32_t result)
{
    tNode  * node;
    tTask  * task = (tTask * )0;
    
    // �����ٽ���
    uint32_t status = tTaskEnterCritical();

    // ȡ���ȴ������еĵ�һ�����
    if((node = tListRemoveFirst(&event->waitList)) != (tNode *)0)
    {                        
      // ת��Ϊ��Ӧ������ṹ                                          
        task = (tTask *)tNodeParent(node, tTask, linkNode);

        // �����յ�����Ϣ���ṹ�������Ӧ�ĵȴ���־λ
        task->waitEvent = (tEvent *)0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~TINYOS_TASK_WAIT_MASK;

        // ���������˳�ʱ�ȴ����������£��������ʱ�������Ƴ�
        if (task->delayTicks != 0)
        { 
            tTimeTaskWakeUp(task);
        }

        // ����������������
        tTaskSchedRdy(task);        
    }  

    // �˳��ٽ���
    tTaskExitCritical(status); 
   
    return task;         
}

void tEventRemoveTask (tTask * task, void * msg, uint32_t result)
{     
 	  // �����ٽ���
    uint32_t status = tTaskEnterCritical();

	 // ����������ڵĵȴ��������Ƴ�
	 // ע�⣬����û�м��waitEvent�Ƿ�Ϊ�ա���Ȼ�Ǵ��¼����Ƴ�����ô��Ϊ�Ͳ�����Ϊ��
	 tListRemove(&task->waitEvent->waitList, &task->linkNode);

  	// �����յ�����Ϣ���ṹ�������Ӧ�ĵȴ���־λ
    task->waitEvent = (tEvent *)0;
    task->eventMsg = msg;
   	task->waitEventResult = result;
	 task->state &= ~TINYOS_TASK_WAIT_MASK;

	// �˳��ٽ���
    tTaskExitCritical(status); 
}

uint32_t tEventRemoveAll (tEvent * event, void * msg, uint32_t result)
{
    tNode  * node;
    uint32_t count;
    
    // �����ٽ���
    uint32_t status = tTaskEnterCritical();

    // ��ȡ�ȴ��е���������
    count = tListCount(&event->waitList);

    // �������еȴ��е�����
    while ((node = tListRemoveFirst(&event->waitList)) != (tNode *)0)
    {                                                                   
        // ת��Ϊ��Ӧ������ṹ                                          
        tTask * task = (tTask *)tNodeParent(node, tTask, linkNode);
        
        // �����յ�����Ϣ���ṹ�������Ӧ�ĵȴ���־λ
        task->waitEvent = (tEvent *)0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~TINYOS_TASK_WAIT_MASK;

        // ���������˳�ʱ�ȴ����������£��������ʱ�������Ƴ�
        if (task->delayTicks != 0)
        { 
            tTimeTaskWakeUp(task);
        }

        // ����������������
        tTaskSchedRdy(task);        
    }  

    // �˳��ٽ���
    tTaskExitCritical(status); 

    return  count;
}

uint32_t tEventWaitCount (tEvent * event)
{  
    uint32_t count = 0;

    // �����ٽ���
    uint32_t status = tTaskEnterCritical();

    count = tListCount(&event->waitList);  

    // �˳��ٽ���
    tTaskExitCritical(status);     

    return count;
} 


