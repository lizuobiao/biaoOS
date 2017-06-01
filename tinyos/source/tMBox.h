#ifndef TMBOX_H
#define TMBOX_H

#include "tConfig.h"
#include "tEvent.h"

#define tMBOXSendNormal             0x00 
#define tMBOXSendFront              0x01

// 邮箱类型
typedef struct _tMbox
{
	// 事件控制块
	// 该结构被特意放到起始处，以实现tSem同时是一个tEvent的目的
	tEvent event;

	// 当前的消息数量
    uint32_t count;

    // 读取消息的索引
    uint32_t read;

    // 写消息的索引
    uint32_t write;

    // 最大允许容纳的消息数量
    uint32_t maxCount;

    // 消息存储缓冲区
    void ** msgBuffer;
}tMbox;

typedef struct _tMboxInfo {
	// 当前的消息数量
    uint32_t count;

    // 最大允许容纳的消息数量
    uint32_t maxCount;

    // 当前等待的任务计数
    uint32_t taskCount;
}tMboxInfo;

void tMboxInit (tMbox * mbox, void ** msgBuffer, uint32_t maxCount);
uint32_t tMboxWait (tMbox * mbox, void **msg, uint32_t waitTicks);
uint32_t tMboxNoWaitGet (tMbox * mbox, void **msg);
uint32_t tMboxNotify (tMbox * mbox, void * msg, uint32_t notifyOption);
void tMboxFlush (tMbox * mbox);
uint32_t tMboxDestroy (tMbox * mbox);
#endif

