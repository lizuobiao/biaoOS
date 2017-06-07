#include "tinyOS.h"

void tTaskInit (tTask * task,void (*entry)(void *), void *param, uint32_t prio, uint32_t * stack, uint32_t size)
{
	uint32_t * stackTop;
	
	task->stackBase = stack;
    task->stackSize = size;
    memset(stack, 0, size);
	
	stackTop = stack + size / sizeof(tTaskStack);
	
	*(--stackTop) = (unsigned long)(1<<24);                // XPSR, ������Thumbģʽ���ָ���Thumb״̬����ARM״̬����
    *(--stackTop) = (unsigned long)entry;                  // �������ڵ�ַ
    *(--stackTop) = (unsigned long)0x14;                   // R14(LR), ���񲻻�ͨ��return xxx�����Լ�������δ��
    *(--stackTop) = (unsigned long)0x12;                   // R12, δ��
    *(--stackTop) = (unsigned long)0x3;                    // R3, δ��
    *(--stackTop) = (unsigned long)0x2;                    // R2, δ��
    *(--stackTop) = (unsigned long)0x1;                    // R1, δ��
    *(--stackTop) = (unsigned long)param;                  // R0 = param, �����������ں���
    *(--stackTop) = (unsigned long)0x11;                   // R11, δ��
    *(--stackTop) = (unsigned long)0x10;                   // R10, δ��
    *(--stackTop) = (unsigned long)0x9;                    // R9, δ��
    *(--stackTop) = (unsigned long)0x8;                    // R8, δ��
    *(--stackTop) = (unsigned long)0x7;                    // R7, δ��
    *(--stackTop) = (unsigned long)0x6;                    // R6, δ��
    *(--stackTop) = (unsigned long)0x5;                    // R5, δ��
    *(--stackTop) = (unsigned long)0x4;                    // R4, δ��
	
	task->slice = TINYOS_SLICE_MAX; 
	task->stack = stackTop; 
	task->delayTicks = 0;
    task->prio = prio;                                  // ������������ȼ�
	task->state = TINYOS_TASK_STATE_RDY;                // ��������Ϊ����״̬
	
	task->suspendCount = 0;                             // ��ʼ�������Ϊ0
    task->clean = (void(*)(void *))0;                   // ����������
    task->cleanParam = (void *)0;                       // ���ô��ݸ��������Ĳ���
    task->requestDeleteFlag = 0;                        // ����ɾ�����
	
	task->waitEvent = (tEvent *)0;                      // û�еȴ��¼�
    task->eventMsg = (void *)0;                         // û�еȴ��¼�
    task->waitEventResult = tErrorNoError;              // û�еȴ��¼�����

	tNodeInit(&(task->linkNode)); 
    tNodeInit(&(task->delayNode));
	
	tTaskSchedRdy(task); 
}

void tTaskSuspend (tTask * task) 
{
    // �����ٽ���
    uint32_t status = tTaskEnterCritical();

    // ��������Ѿ�������ʱ״̬���������
    if (!(task->state & TINYOS_TASK_STATE_DELAYED)) 
    {
        // ���ӹ������������������ִ�е�һ�ι������ʱ���ſ����Ƿ�
        // Ҫִ�������л�����
        if (++task->suspendCount <= 1)
        {
            // ���ù����־
            task->state |= TINYOS_TASK_STATE_SUSPEND;

            // ����ʽ�ܼ򵥣����ǽ���Ӿ����������Ƴ��������������Ͳ��ᷢ����
            // Ҳ��û���л�������������
            tTaskSchedUnRdy(task);

            // ��Ȼ���������������Լ�����ô���л�����������
            if (task == currentTask)
            {
                tTaskSched();
            }
        }
    }

    // �˳��ٽ���
    tTaskExitCritical(status); 
}

/**********************************************************************************************************
** Function name        :   tTaskWakeUp
** Descriptions         :   ���ѱ����������
** parameters           :   task        �����ѵ�����
** Returned value       :   ��
***********************************************************************************************************/
void tTaskWakeUp (tTask * task)
{
    // �����ٽ���
    uint32_t status = tTaskEnterCritical();

     // ��������Ƿ��ڹ���״̬
    if (task->state & TINYOS_TASK_STATE_SUSPEND)
    {
        // �ݼ�������������Ϊ0�ˣ�����������־��ͬʱ���ý������״̬
        if (--task->suspendCount == 0) 
        {
            // ��������־
            task->state &= ~TINYOS_TASK_STATE_SUSPEND;

            // ͬʱ������Żؾ���������
            tTaskSchedRdy(task);

            // ���ѹ����У������и������ȼ������������ִ��һ���������
            tTaskSched();
        }
    }

    // �˳��ٽ���
    tTaskExitCritical(status); 
}


void tTaskSetCleanCallFunc (tTask * task, void (*clean)(void * param), void * param) 
{
    task->clean = clean;
    task->cleanParam = param;
}

void tTaskForceDelete (tTask * task) 
{
    // �����ٽ���
    uint32_t status = tTaskEnterCritical();

     // �����������ʱ״̬�������ʱ������ɾ��
    if (task->state & TINYOS_TASK_STATE_DELAYED) 
    {
        tTimeTaskRemove(task);
    }
		// ������񲻴��ڹ���״̬����ô���Ǿ���̬���Ӿ�������ɾ��
    else if (!(task->state & TINYOS_TASK_STATE_SUSPEND))
    {
        tTaskSchedRemove(task);
    }

    // ɾ��ʱ������������������������������
    if (task->clean) 
    {
        task->clean(task->cleanParam);
    }

    // ���ɾ�������Լ�����ô��Ҫ�л�����һ����������ִ��һ���������
    if (currentTask == task) 
    {
        tTaskSched();
    }

    // �˳��ٽ���
    tTaskExitCritical(status); 
}

void tTaskRequestDelete (tTask * task)
{
   // �����ٽ���
    uint32_t status = tTaskEnterCritical();

    // �������ɾ�����
    task->requestDeleteFlag = 1;

    // �˳��ٽ���
    tTaskExitCritical(status); 
}

uint8_t tTaskIsRequestedDelete (void)
{
    uint8_t delete;

   // �����ٽ���
    uint32_t status = tTaskEnterCritical();

    // ��ȡ����ɾ�����
    delete = currentTask->requestDeleteFlag;

    // �˳��ٽ���
    tTaskExitCritical(status); 

    return delete;
}

void tTaskDeleteSelf (void)
{
    // �����ٽ���
    uint32_t status = tTaskEnterCritical();

    // �����ڵ��øú���ʱ�������Ǵ��ھ���״̬�������ܴ�����ʱ����������״̬
    // ���ԣ�ֻ��Ҫ�Ӿ����������Ƴ�����
    tTaskSchedRemove(currentTask);

    // ɾ��ʱ������������������������������
    if (currentTask->clean)
    {
        currentTask->clean(currentTask->cleanParam);
    }

    // ���������϶����л�����������ȥ����
    tTaskSched();

    // �˳��ٽ���
    tTaskExitCritical(status);
}

void tTaskGetInfo (tTask * task, tTaskInfo * info)
{
	uint32_t * stackEnd;

   // �����ٽ���
    uint32_t status = tTaskEnterCritical();

    info->delayTicks = task->delayTicks;                // ��ʱ��Ϣ
    info->prio = task->prio;                            // �������ȼ�
    info->state = task->state;                          // ����״̬
    info->slice = task->slice;                          // ʣ��ʱ��Ƭ
    info->suspendCount = task->suspendCount;            // ������Ĵ���
	
	info->stackSize = task->stackSize;

    // �����ջʹ����
    info->stackFree = 0;
    stackEnd = task->stackBase;
    while ((*stackEnd++ == 0) && (stackEnd <= task->stackBase + task->stackSize / sizeof(tTaskStack)))
    {
        info->stackFree++;
    }

    // ת�����ֽ���
    info->stackFree *= sizeof(tTaskStack);

    // �˳��ٽ���
    tTaskExitCritical(status); 
}


