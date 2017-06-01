#ifndef TMEMBLOCK_H
#define TMEMBLOCK_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct _tMemBlock
{
	// 事件控制块
    tEvent event;

    // 存储块的首地址
    void * memStart;

    // 每个存储块的大小
    uint32_t blockSize;

    // 总的存储块的个数
    uint32_t maxCount;

    // 存储块列表
    tList blockList;
}tMemBlock;

typedef struct _tMemBlockInfo
{
	// 当前存储块的计数
    uint32_t count;

    // 允许的最大计数
    uint32_t maxCount;

    // 每个存储块的大小
    uint32_t blockSize;

    // 当前等待的任务计数
    uint32_t taskCount;
}tMemBlockInfo;

void tMemBlockInit (tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt);
uint32_t tMemBlockWait (tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks);
uint32_t tMemBlockNoWaitGet (tMemBlock * memBlock, void ** mem);
void tMemBlockNotify (tMemBlock * memBlock, uint8_t * mem);

void tMemBlockGetInfo (tMemBlock * memBlock, tMemBlockInfo * info);
uint32_t tMemBlockDestroy (tMemBlock * memBlock);
#endif

