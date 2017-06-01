#ifndef TMEMBLOCK_H
#define TMEMBLOCK_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct _tMemBlock
{
	// �¼����ƿ�
    tEvent event;

    // �洢����׵�ַ
    void * memStart;

    // ÿ���洢��Ĵ�С
    uint32_t blockSize;

    // �ܵĴ洢��ĸ���
    uint32_t maxCount;

    // �洢���б�
    tList blockList;
}tMemBlock;

typedef struct _tMemBlockInfo
{
	// ��ǰ�洢��ļ���
    uint32_t count;

    // �����������
    uint32_t maxCount;

    // ÿ���洢��Ĵ�С
    uint32_t blockSize;

    // ��ǰ�ȴ����������
    uint32_t taskCount;
}tMemBlockInfo;

void tMemBlockInit (tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt);
uint32_t tMemBlockWait (tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks);
uint32_t tMemBlockNoWaitGet (tMemBlock * memBlock, void ** mem);
void tMemBlockNotify (tMemBlock * memBlock, uint8_t * mem);

void tMemBlockGetInfo (tMemBlock * memBlock, tMemBlockInfo * info);
uint32_t tMemBlockDestroy (tMemBlock * memBlock);
#endif

