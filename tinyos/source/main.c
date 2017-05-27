#include "tinyOS.h"
#include "ARMCM3.h"

tTask * currentTask;

tTask * nextTask;

tTask * idleTask;

//tTask * taskTable[2];

// �������ȼ��ı��λ�ýṹ
tBitmap taskPrioBitmap;

// ���������ָ�����飺�������ֻʹ����������
tTask * taskTable[TINYOS_PRO_COUNT];

uint8_t schedLockCount;

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
	
	task->stack = stack; 
	task->delayTicks = 0;
    task->prio = prio;                                  // ������������ȼ�

    taskTable[prio] = task;                             // �����������ȼ���
    tBitmapSet(&taskPrioBitmap, prio);                  // ������ȼ�λ���е���Ӧλ
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

void tTaskSystemTickHandler () 
{
    // ������������delayTicks���������0�Ļ�����1��
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
    // ��������п�����������ʱ���(delayTicks = 0)������һ�ε��ȡ�
    tTaskSched();
}

void tTaskDelay (uint32_t delay) {
	
	uint32_t status = tTaskEnterCritical();
    // ���úõ�ǰҪ��ʱ��ticks��
    currentTask->delayTicks = delay;
	tBitmapClear(&taskPrioBitmap, currentTask->prio);
	tTaskExitCritical(status);
    // Ȼ����������л����л�����һ�����񣬻��߿�������
    // delayTikcs����ʱ���ж����Զ���1.������0ʱ�����л������������С�
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
	tTaskInit(&tTask1,task1Entry,(void *)0x11111111,0, &task1Env[1024]);
	tTaskInit(&tTask2,task2Entry,(void *)0x22222222,1,&task2Env[1024]);
	
	taskTable[0] = &tTask1;
    taskTable[1] = &tTask2;
	
	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0,TINYOS_PRO_COUNT-1,&idleTaskEnv[1024]);
    idleTask = &tTaskIdle;
	
	nextTask = tTaskHighestReady();
	
  // �л���nextTask�� ���������Զ���᷵��
    tTaskRunFirst();
    return 0;
}
