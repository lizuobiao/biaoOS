#include "tMemBlock.h"

#include "tinyOS.h"

void tMemBlockInit (tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt)
{
	uint8_t * memBlockStart = (uint8_t *)memStart;
	uint8_t * memBlockEnd = memBlockStart + blockSize * blockCnt;

	// ÿ���洢����Ҫ����������ָ�룬���Կռ�����Ҫ��tNode��
	// ������ˣ�ʵ���û����õĿռ䲢û����
	if (blockSize < sizeof(tNode))
	{
		return;
	}

	tEventInit(&memBlock->event, tEventTypeMemBlock);

	memBlock->memStart = memStart;
	memBlock->blockSize = blockSize;
	memBlock->maxCount = blockCnt;

	tListInit(&memBlock->blockList);
	while (memBlockStart < memBlockEnd)
	{
		tNodeInit((tNode *)memBlockStart);
		tListAddLast(&memBlock->blockList, (tNode *)memBlockStart);

		memBlockStart += blockSize;
	}
}

uint32_t tMemBlockWait (tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    // ���ȼ���Ƿ��п��еĴ洢��
    if (tListCount(&memBlock->blockList) > 0)
    {
    	// ����еĻ���ȡ��һ��
        *mem = (uint8_t *)tListRemoveFirst(&memBlock->blockList);
    	tTaskExitCritical(status);
    	return tErrorNoError;
    }
    else
    {
         // Ȼ����������¼�������
        tEventWait(&memBlock->event, currentTask, (void *)0,  tEventTypeMemBlock, waitTicks);
        tTaskExitCritical(status);

        // �����ִ��һ���¼����ȣ��Ա����л�����������
        tTaskSched();

        // ���л�����ʱ����tTask��ȡ����õ���Ϣ
        *mem = currentTask->eventMsg;

        // ȡ���ȴ����
        return currentTask->waitEventResult;
    }
}

uint32_t tMemBlockNoWaitGet (tMemBlock * memBlock, void ** mem)
{
    uint32_t status = tTaskEnterCritical();

    // ���ȼ���Ƿ��п��еĴ洢��
    if (tListCount(&memBlock->blockList) > 0)
    {
    	// ����еĻ���ȡ��һ��
        *mem = (uint8_t *)tListRemoveFirst(&memBlock->blockList);
    	tTaskExitCritical(status);
    	return tErrorNoError;
    }
    else
    {
     	// ���򣬷�����Դ������
        tTaskExitCritical(status);
    	return tErrorResourceUnavaliable;
    }
}

void tMemBlockNotify (tMemBlock * memBlock, uint8_t * mem)
{
    uint32_t status = tTaskEnterCritical();

    // ����Ƿ�������ȴ�
    if (tEventWaitCount(&memBlock->event) > 0)
    {
    	// ����еĻ�����ֱ�ӻ���λ�ڶ����ײ������ȵȴ���������
        tTask * task = tEventWakeUp(&memBlock->event, (void *)mem, tErrorNoError);

        // ��������������ȼ����ߣ���ִ�е��ȣ��л���ȥ
        if (task->prio < currentTask->prio)
        {
            tTaskSched();
    	}
    }
    else
    {
    	// ���û������ȴ��Ļ������洢����뵽������
    	tListAddLast(&memBlock->blockList, (tNode *)mem);
   	}

	tTaskExitCritical(status);
}

void tMemBlockGetInfo (tMemBlock * memBlock, tMemBlockInfo * info)
{
    uint32_t status = tTaskEnterCritical();

    // ������Ҫ����Ϣ
    info->count = tListCount(&memBlock->blockList);
    info->maxCount = memBlock->maxCount;
    info->blockSize = memBlock->blockSize;
    info->taskCount = tEventWaitCount(&memBlock->event);

    tTaskExitCritical(status);
}

uint32_t tMemBlockDestroy (tMemBlock * memBlock)
{
    uint32_t status = tTaskEnterCritical();

    // ����¼����ƿ��е�����
    uint32_t count = tEventRemoveAll(&memBlock->event, (void *)0, tErrorDel);

    tTaskExitCritical(status);

    // ��չ����п��������������ִ��һ�ε���
    if (count > 0)
    {
        tTaskSched();
    }
    return count;
}


