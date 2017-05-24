#include "tinyOS.h"

tTask * currentTask;

tTask * nextTask;

tTask * taskTable[2];



void tTaskInit(tTask * task, void (*entry)(void *), void *param, uint32_t * stack)
{
	
	 *(--stack) = (unsigned long)(1<<24);                // XPSR, ������Thumbģʽ���ָ���Thumb״̬����ARM״̬����
    *(--stack) = (unsigned long)entry;                  // �������ڵ�ַ
    *(--stack) = (unsigned long)0x14;                   // R14(LR), ���񲻻�ͨ��return xxx�����Լ�������δ��
    *(--stack) = (unsigned long)0x12;                   // R12, δ��
    *(--stack) = (unsigned long)0x3;                    // R3, δ��
    *(--stack) = (unsigned long)0x2;                    // R2, δ��
    *(--stack) = (unsigned long)0x1;                    // R1, δ��
    *(--stack) = (unsigned long)param;                  // R0 = param, �����������ں���
    *(--stack) = (unsigned long)0x11;                   // R11, δ��
    *(--stack) = (unsigned long)0x10;                   // R10, δ��
    *(--stack) = (unsigned long)0x9;                    // R9, δ��
    *(--stack) = (unsigned long)0x8;                    // R8, δ��
    *(--stack) = (unsigned long)0x7;                    // R7, δ��
    *(--stack) = (unsigned long)0x6;                    // R6, δ��
    *(--stack) = (unsigned long)0x5;                    // R5, δ��
    *(--stack) = (unsigned long)0x4;                    // R4, δ��
	
	task->stack = stack;
}

void tTaskSched () 
{    
    // ������㷨�ܼ򵥡�
    // һ������������ѡ����һ������Ȼ���л���ȥ
    if (currentTask == taskTable[0]) 
    {
        nextTask = taskTable[1];
    }
    else 
    {
        nextTask = taskTable[0];
    }
    
    tTaskSwitch();
}


void delay(int count)
{
	while(--count);
}

int task1Flag;
void task1Entry (void * param) 
{
    for (;;) 
    {
        task1Flag = 1;
        delay(100);
        task1Flag = 0;
        delay(100);
        tTaskSched();
    }
}

int task2Flag;
void task2Entry (void * param) 
{
    for (;;) 
    {
        task2Flag = 1;
        delay(100);
        task2Flag = 0;
        delay(100);
        tTaskSched();
    }
}

int flag;

tTask tTask1;
tTask tTask2;
tTaskStack task1Env[1024];     
tTaskStack task2Env[1024];

int main()
{
	tTaskInit(&tTask1,task1Entry,(void *)0x11111111, &task1Env[1024]);
	tTaskInit(&tTask2,task2Entry,(void *)0x22222222, &task2Env[1024]);
	
	taskTable[0] = &tTask1;
    taskTable[1] = &tTask2;
	
	nextTask = taskTable[0];

  // �л���nextTask�� ���������Զ���᷵��
    tTaskRunFirst();
    return 0;
}
