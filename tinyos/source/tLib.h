/*************************************** Copyright (c)******************************************************
** File name            :   tLib.h
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS所用的通用数据结构库文件。
**
**--------------------------------------------------------------------------------------------------------
** Created by           :   01课堂 lishutong
** Created date         :   2016-06-01
** Version              :   1.0
** Descriptions         :   The original version
**
**--------------------------------------------------------------------------------------------------------
** Copyright            :   版权所有，禁止用于商业用途
** Website              :   http://course.ztsoftware.win
** Study Video          :   与代码相关的配套学习视频请见 http://study.163.com/u/01course
**********************************************************************************************************/
#ifndef TLIB_H
#define TLIB_H

// 标准头文件，里面包含了常用的类型定义，如uint32_t
#include <stdint.h>

// 位图类型
typedef struct 
{
	uint32_t bitmap;
}tBitmap;

/**********************************************************************************************************
** Function name        :   tBitmapInit
** Descriptions         :   初始化bitmap将所有的位全清0
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tBitmapInit (tBitmap * bitmap);

/**********************************************************************************************************
** Function name        :   tBitmapPosCount
** Descriptions         :   返回最大支持的位置数量
** parameters           :   无
** Returned value       :   最大支持的位置数量
***********************************************************************************************************/
uint32_t tBitmapPosCount (void);

/**********************************************************************************************************
** Function name        :   tBitmapSet
** Descriptions         :   设置bitmap中的某个位
** parameters           :   pos 需要设置的位
** Returned value       :   无
***********************************************************************************************************/
void tBitmapSet (tBitmap * bitmap, uint32_t pos);

/**********************************************************************************************************
** Function name        :   tBitmapClear
** Descriptions         :   清除bitmap中的某个位
** parameters           :   pos 需要清除的位
** Returned value       :   无
***********************************************************************************************************/
void tBitmapClear (tBitmap * bitmap, uint32_t pos);

/**********************************************************************************************************
** Function name        :   tBitmapGetFirstSet
** Descriptions         :   从位图中第0位开始查找，找到第1个被设置的位置序号
** parameters           :   无
** Returned value       :   第1个被设置的位序号
***********************************************************************************************************/
uint32_t tBitmapGetFirstSet (tBitmap * bitmap);


// tinyOS链表的结点类型
typedef struct _tNode
{
	// 该结点的前一个结点
    struct _tNode * preNode;

    // 该结点的后一个结点
    struct _tNode * nextNode;
}tNode;

void tNodeInit (tNode * node);

// tinyOS链表类型
typedef struct _tList
{   
	// 该链表的头结点
    tNode headNode;

    // 该链表中所有结点数量
    uint32_t nodeCount;
}tList;

#define tNodeParent(node, parent, name) (parent *)((uint32_t)node - (uint32_t)&((parent *)0)->name);
void tListInit (tList * list);
uint32_t tListCount (tList * list);
tNode * tListFirst (tList * list);
tNode * tListLast (tList * list);
tNode * tListPre (tList * list, tNode * node);
tNode * tListNext (tList * list, tNode * node);
void tListRemoveAll (tList * list);
void tListAddFirst (tList * list, tNode * node);
void tListAddLast (tList * list, tNode * node);
tNode * tListRemoveFirst (tList * list);
void tListInsertAfter (tList * list, tNode * nodeAfter, tNode * nodeToInsert);
void tListRemove (tList * list, tNode * node);

#endif /* TLIB_H */
