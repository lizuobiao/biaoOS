#include "tinyOS.h"

void tTaskInit(tTask * task, void (*entry)(void *), void *param, uint32_t prio,uint32_t * stack)
{
	
	*(--stack) = (unsigned long)(1<<24);                // XPSR, ������Thumbģʽ���ָ���Thumb״̬����ARM״̬����
    *(--stack) = (unsigned long)entry;                  // �������ڵ�ַ
    *(--stack) = (unsigned long)0x14;                   // R14(LR), ���񲻻�ͨ��return xxx�����Լ�������δ��
    *(--stack) = (unsigned long)0x12;                   // R12, δ��
    *(--stack) = (unsigned long)0x3;                    // R3, δ��
    *(--stack) = (unsigned long)0x2;                    // R2, δ��
    *(--stack) = (unsigned long)0x1;                    // R1, δ��
    *(--stack) = (unsigned long)param;                  // R0 = param, �����������ں���
    *(--stack) = (unsigned long)0x11;                   // R11, δ��
    *(--stack) = (unsigned long)0x10;                   // R10, δ��
    *(--stack) = (unsigned long)0x9;                    // R9, δ��
    *(--stack) = (unsigned long)0x8;                    // R8, δ��
    *(--stack) = (unsigned long)0x7;                    // R7, δ��
    *(--stack) = (unsigned long)0x6;                    // R6, δ��
    *(--stack) = (unsigned long)0x5;                    // R5, δ��
    *(--stack) = (unsigned long)0x4;                    // R4, δ��
	
	task->slice = TINYOS_SLICE_MAX; 
	task->stack = stack; 
	task->delayTicks = 0;
    task->prio = prio;                                  // ������������ȼ�
	task->state = TINYOS_TASK_STATE_RDY;                // ��������Ϊ����״̬

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


