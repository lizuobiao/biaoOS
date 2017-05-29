#include "tinyOS.h"
#include "ARMCM3.h"

tTask * currentTask;

tTask * nextTask;

tTask * idleTask;

//tTask * taskTable[2];

// �������ȼ��ı��λ�ýṹ
tBitmap taskPrioBitmap;

// ���������ָ�����飺�������ֻʹ����������
tList taskTable[TINYOS_PRO_COUNT];

uint8_t schedLockCount;

tList tTaskDelayedList;

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
	
    tListAddLast(&taskTable[prio], &(task->linkNode));   // �����������ȼ���
    tBitmapSet(&taskPrioBitmap, prio);                  // ������ȼ�λ���е���Ӧλ
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
    // ��������ֻ���������������񶼲�����ʱ״̬ʱ��ִ��
    // ���ԣ������ȼ���µ�ǰ�����Ƿ��ǿ�������
	
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
    // ������������delayTicks���������0�Ļ�����1��
	tNode * node;
	uint32_t status = tTaskEnterCritical();
	
    for (node = tTaskDelayedList.headNode.nextNode; node != &(tTaskDelayedList.headNode); node = node->nextNode)
    {
        tTask * task = tNodeParent(node, tTask, delayNode);
        if (--task->delayTicks == 0) 
        {
            // ���������ʱ�������Ƴ�
            tTimeTaskWakeUp(task);

            // ������ָ�������״̬
            tTaskSchedRdy(task);            
        }
    }
    
	if (--currentTask->slice == 0)
    {
        // �����ǰ�����л�����������Ļ�����ô�л�����һ������
        // �����ǽ���ǰ����Ӷ��е�ͷ���Ƴ������뵽β��
        // ��������ִ��tTaskSched()ʱ�ͻ��ͷ��ȡ���µ�����ȡ���µ�������Ϊ��ǰ��������
        if (tListCount(&taskTable[currentTask->prio]) > 0)
        {
            tListRemoveFirst(&taskTable[currentTask->prio]);
            tListAddLast(&taskTable[currentTask->prio], &(currentTask->linkNode));

            // ���ü�����
            currentTask->slice = TINYOS_SLICE_MAX;
        }
    }
	
	tTaskExitCritical(status);
    // ��������п�����������ʱ���(delayTicks = 0)������һ�ε��ȡ�
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

// ���ڿ������������ṹ�Ͷ�ջ�ռ�
tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];
int idle_count = 0;

void idleTaskEntry (void * param) {
    for (;;)
    {
		idle_count++;
        // ��������ʲô������
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
	
  // �л���nextTask�� ���������Զ���᷵��
    tTaskRunFirst();
    return 0;
}
