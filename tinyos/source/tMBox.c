#include "tMBox.h"
#include "tinyOS.h"

void tMboxInit (tMbox * mbox, void ** msgBuffer, uint32_t maxCount) 
{
	tEventInit(&mbox->event, tEventTypeMbox);

	mbox->msgBuffer = msgBuffer;
	mbox->maxCount = maxCount;
	mbox->read = 0;
	mbox->write = 0;
	mbox->count = 0;
}

uint32_t tMboxWait (tMbox * mbox, void **msg, uint32_t waitTicks) 
{
    uint32_t status = tTaskEnterCritical();

    // ���ȼ����Ϣ�����Ƿ����0
    if (mbox->count > 0)
    {
    	// �������0�Ļ���ȡ��һ��
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];

        // ͬʱ��ȡ����ǰ�ƣ���������߽������
        if (mbox->read >= mbox->maxCount) 
        {
        	mbox->read = 0;
        }
    	tTaskExitCritical(status); 
    	return tErrorNoError;
    }
    else                                                                
    { 
         // Ȼ����������¼�������
        tEventWait(&mbox->event, currentTask, (void *)0,  tEventTypeMbox, waitTicks);
        tTaskExitCritical(status);
        
        // �����ִ��һ���¼����ȣ��Ա����л�����������
        tTaskSched();

        // ���л�����ʱ����tTask��ȡ����õ���Ϣ
        *msg = currentTask->eventMsg;

        // ȡ���ȴ����
        return currentTask->waitEventResult;
    }
}

uint32_t tMboxNoWaitGet (tMbox * mbox, void **msg)
{
    uint32_t status = tTaskEnterCritical();

    // ���ȼ����Ϣ�����Ƿ����0
    if (mbox->count > 0)
    {
    	// �������0�Ļ���ȡ��һ��
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];

        // ͬʱ��ȡ����ǰ�ƣ���������߽������
        if (mbox->read >= mbox->maxCount) 
        {
        	mbox->read = 0;
        }
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

uint32_t tMboxNotify (tMbox * mbox, void * msg, uint32_t notifyOption)
{
    uint32_t status = tTaskEnterCritical();        
    
    // ����Ƿ�������ȴ�
    if (tEventWaitCount(&mbox->event) > 0)
    {
    	// ����еĻ�����ֱ�ӻ���λ�ڶ����ײ������ȵȴ���������
        tTask * task = tEventWakeUp(&mbox->event, (void *)msg, tErrorNoError );
        
        // ��������������ȼ����ߣ���ִ�е��ȣ��л���ȥ
        if (task->prio < currentTask->prio)
        {
            tTaskSched(); 
    	}
    }
    else
    {
    	// ���û������ȴ��Ļ�������Ϣ���뵽��������
    	if (mbox->count >= mbox->maxCount) 
    	{
		    tTaskExitCritical(status);
		    return tErrorResourceFull;
    	}

		// ����ѡ����Ϣ���뵽ͷ���������������ȡ����Ϣ��ʱ�����Ȼ�ȡ����Ϣ
		if (notifyOption & tMBOXSendFront)
		{
			if (mbox->read <= 0) 
			{
				mbox->read = mbox->maxCount - 1;
			} 
			else 
			{
				--mbox->read;
			}
			mbox->msgBuffer[mbox->read] = msg;
		}
		else 
		{
			mbox->msgBuffer[mbox->write++] = msg;
			if (mbox->write >= mbox->maxCount)
			{
				mbox->write = 0;
			}
		}

		// ������Ϣ����
		mbox->count++;
   	}

	tTaskExitCritical(status);
	return tErrorNoError;
}

void tMboxFlush (tMbox * mbox)
{
    uint32_t status = tTaskEnterCritical();        
    
    // ���������������ȴ���˵�������Ѿ��ǿյ��ˣ�����Ҫ�����
    if (tEventWaitCount(&mbox->event) == 0) 
    {
        mbox->read = 0;
        mbox->write = 0;
        mbox->count = 0;
    }

    tTaskExitCritical(status);
}

uint32_t tMboxDestroy (tMbox * mbox)
{       
    uint32_t status = tTaskEnterCritical(); 

    // ����¼����ƿ��е�����
    uint32_t count = tEventRemoveAll(&mbox->event, (void *)0, tErrorDel);  
    
    tTaskExitCritical(status);
    
    // ��չ����п��������������ִ��һ�ε���
    if (count > 0) 
    {
        tTaskSched();
    } 
    return count;  
}

void tMboxGetInfo (tMbox * mbox, tMboxInfo * info)
{
    uint32_t status = tTaskEnterCritical();

    // ������Ҫ����Ϣ
    info->count = mbox->count;
    info->maxCount = mbox->maxCount;
    info->taskCount = tEventWaitCount(&mbox->event);

    tTaskExitCritical(status);
}


