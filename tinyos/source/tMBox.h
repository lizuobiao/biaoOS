#ifndef TMBOX_H
#define TMBOX_H

#include "tConfig.h"
#include "tEvent.h"

#define tMBOXSendNormal             0x00 
#define tMBOXSendFront              0x01

// ��������
typedef struct _tMbox
{
	// �¼����ƿ�
	// �ýṹ������ŵ���ʼ������ʵ��tSemͬʱ��һ��tEvent��Ŀ��
	tEvent event;

	// ��ǰ����Ϣ����
    uint32_t count;

    // ��ȡ��Ϣ������
    uint32_t read;

    // д��Ϣ������
    uint32_t write;

    // ����������ɵ���Ϣ����
    uint32_t maxCount;

    // ��Ϣ�洢������
    void ** msgBuffer;
}tMbox;

typedef struct _tMboxInfo {
	// ��ǰ����Ϣ����
    uint32_t count;

    // ����������ɵ���Ϣ����
    uint32_t maxCount;

    // ��ǰ�ȴ����������
    uint32_t taskCount;
}tMboxInfo;

void tMboxInit (tMbox * mbox, void ** msgBuffer, uint32_t maxCount);
uint32_t tMboxWait (tMbox * mbox, void **msg, uint32_t waitTicks);
uint32_t tMboxNoWaitGet (tMbox * mbox, void **msg);
uint32_t tMboxNotify (tMbox * mbox, void * msg, uint32_t notifyOption);
void tMboxFlush (tMbox * mbox);
uint32_t tMboxDestroy (tMbox * mbox);
#endif

