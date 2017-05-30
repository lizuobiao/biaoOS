#ifndef TSEM_H
#define TSEM_H

#include "tConfig.h"
#include "tEvent.h"

// �ź�������
typedef struct _tSem 
{
	// �¼����ƿ�
	// �ýṹ������ŵ���ʼ������ʵ��tSemͬʱ��һ��tEvent��Ŀ��
	tEvent event;

	// ��ǰ�ļ���
	uint32_t count;

	// ������
	uint32_t maxCount;
}tSem;

// �ź�������Ϣ����
typedef struct _tSemInfo
{
	// ��ǰ�ź����ļ���
    uint32_t count;

    // �ź��������������
    uint32_t maxCount;

    // ��ǰ�ȴ����������
    uint32_t taskCount;
}tSemInfo;


void tSemInit (tSem * sem, uint32_t startCount, uint32_t maxCount);
uint32_t tSemWait (tSem * sem, uint32_t waitTicks);
uint32_t tSemNoWaitGet (tSem * sem);
void tSemNotify (tSem * sem);
void tSemGetInfo (tSem * sem, tSemInfo * info);
uint32_t tSemDestroy (tSem * sem);
#endif
