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

uint32_t tickCount;

tList tTaskDelayedList;

uint32_t idleCount;
uint32_t idleMaxCount;

static void initCpuUsageStat (void);
static void checkCpuUsage (void);
static void cpuUsageSyncWithSysTick (void);

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

void tTimeTaskRemove (tTask * task)
{
    tListRemove(&tTaskDelayedList, &(task->delayNode));
}

void tTaskSchedRemove (tTask * task)
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

void tTimeTickInit (void)
{
    tickCount = 0;
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
			
			if (task->waitEvent) 
            {
                // ��ʱ����ϢΪ�գ��ȴ����Ϊ��ʱ
                tEventRemoveTask(task, (void *)0, tErrorTimeout);
            }
			
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
	
	tickCount++;

    // ���cpuʹ����
    checkCpuUsage();
	
	tTaskExitCritical(status);
	
#if TINYOS_ENABLE_TIMER == 1	
	tTimerModuleTickNotify();
#endif
	
    // ��������п�����������ʱ���(delayTicks = 0)������һ�ε��ȡ�
    tTaskSched();
}



static float cpuUsage;                      // cpuʹ����ͳ��
static uint32_t enableCpuUsageStat;         // �Ƿ�ʹ��cpuͳ��

/**********************************************************************************************************
** Function name        :   initCpuUsageStat
** Descriptions         :   ��ʼ��cpuͳ��
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
static void initCpuUsageStat (void)
{
    idleCount = 0;
    idleMaxCount = 0;
    cpuUsage = 0;
    enableCpuUsageStat = 0;
}

/**********************************************************************************************************
** Function name        :   checkCpuUsage
** Descriptions         :   ���cpuʹ����
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
static void checkCpuUsage (void)
{
    // ����������cpuͳ��ͬ��
    if (enableCpuUsageStat == 0)
    {
        enableCpuUsageStat = 1;
        tickCount = 0;
        return;
    }

    if (tickCount == TICKS_PER_SEC)
    {
        // ͳ�����1s�ڵ�������ֵ
        idleMaxCount = idleCount;
        idleCount = 0;

        // ������ϣ������������������л�����������
        tTaskSchedEnable();
    }
    else if (tickCount % TICKS_PER_SEC == 0)
    {
        // ֮��ÿ��1sͳ��һ�Σ�ͬʱ����cpu������
        cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
        idleCount = 0;
    }
}

/**********************************************************************************************************
** Function name        :   cpuUsageSyncWithSysTick
** Descriptions         :   Ϊ���cpuʹ������ϵͳʱ�ӽ���ͬ��
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
static void cpuUsageSyncWithSysTick (void)
{
    // �ȴ���ʱ�ӽ���ͬ��
    while (enableCpuUsageStat == 0)
    {
        ;;
    }
}

/**********************************************************************************************************
** Function name        :   cpuUsageSyncWithSysTick
** Descriptions         :   Ϊ���cpuʹ������ϵͳʱ�ӽ���ͬ��
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
float tCpuUsageGet (void)
{
    float usage = 0;

    uint32_t status = tTaskEnterCritical();
    usage = cpuUsage;
    tTaskExitCritical(status);

    return usage;
}

// ���ڿ������������ṹ�Ͷ�ջ�ռ�
tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];
int idle_count = 0;

void idleTaskEntry (void * param) {
	
	tTaskSchedDisable();
	
	tInitApp();
#if TINYOS_ENABLE_TIMER == 1
	tTimerInitTask();
#endif
	tSetSysTickPeriod(TINYOS_SYSTICK_MS);
	
	cpuUsageSyncWithSysTick();
    for (;;)
    {
		uint32_t status = tTaskEnterCritical();
        idleCount++;
        tTaskExitCritical(status);
    }
}

int main()
{
	tTaskSchedInit();
	
	tTaskDelayedInit();
#if TINYOS_ENABLE_TIMER == 1	
	tTimerModuleInit();
#endif	
	tTimeTickInit();

    // ��ʼ��cpuͳ��
    initCpuUsageStat();
	
	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, TINYOS_PRO_COUNT - 1, idleTaskEnv, TINYOS_IDLETASK_STACK_SIZE);
	
	nextTask = tTaskHighestReady();
	
  // �л���nextTask�� ���������Զ���᷵��
    tTaskRunFirst();
    return 0;
}
