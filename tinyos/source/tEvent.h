#ifndef TEVENT_H
#define TEVENT_H

#include "tConfig.h"
#include "tLib.h"
#include "tTask.h"

// Event����
typedef enum  _tEventType {   
    tEventTypeUnknown   = (0 << 16), 				// δ֪����
    tEventTypeSem   	= (1 << 16), 				// �ź�������
    tEventTypeMbox  	= (2 << 16), 				// ��������
	tEventTypeMemBlock  = (3 << 16),				// �洢������
	tEventTypeFlagGroup = (4 << 16),				// �¼���־��
	tEventTypeMutex     = (5 << 16),				// �����ź�������
 }tEventType;

// Event���ƽṹ
typedef struct _tEvent {
    tEventType type;						// Event����

    tList waitList;							// ����ȴ��б�
}tEvent;

void tEventInit (tEvent * event, tEventType type);
void tEventWait (tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout);
tTask * tEventWakeUp (tEvent * event, void * msg, uint32_t result);
void tEventWakeUpTask (tEvent * event, tTask * task, void * msg, uint32_t result);
void tEventRemoveTask (tTask * task, void * msg, uint32_t result);

uint32_t tEventRemoveAll (tEvent * event, void * msg, uint32_t result);
uint32_t tEventWaitCount (tEvent * event);
	
#endif /* TEVENT_H */



