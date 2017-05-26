/*************************************** Copyright (c)******************************************************
** File name            :   switch.c
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS�����л�����CPU��صĺ�����
**
**--------------------------------------------------------------------------------------------------------
** Created by           :   01���� lishutong
** Created date         :   2016-06-01
** Version              :   1.0
** Descriptions         :   The original version
**
**--------------------------------------------------------------------------------------------------------
** Copyright            :   ��Ȩ���У���ֹ������ҵ��;
** Website              :   http://course.ztsoftware.win
** Study Video          :   �������ص�����ѧϰ��Ƶ��� http://study.163.com/u/01course
**********************************************************************************************************/
#include "tinyOS.h"
#include "ARMCM3.h"

#define NVIC_INT_CTRL       0xE000ED04      // �жϿ��Ƽ�״̬�Ĵ���
#define NVIC_PENDSVSET      0x10000000      // ��������жϵ�ֵ
#define NVIC_SYSPRI2        0xE000ED22      // ϵͳ���ȼ��Ĵ���
#define NVIC_PENDSV_PRI     0x000000FF      // �������ȼ�

#define MEM32(addr)         *(volatile unsigned long *)(addr)
#define MEM8(addr)          *(volatile unsigned char *)(addr)

uint32_t tTaskEnterCritical (void) 
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();        // CPSID I
    return primask;
}

void tTaskExitCritical (uint32_t status) {
    __set_PRIMASK(status);
}
// ����Ĵ����У��õ���C�ļ�Ƕ��ARM���
// �����﷨Ϊ:__asm ����ֵ ������(��������) {....}�� ��������÷���Keil�������ֲᣬ�˴�������ע��

/**********************************************************************************************************
** Function name        :   PendSV_Handler
** Descriptions         :   PendSV�쳣������������Щ����֣������������������������á�ʵ���ϣ�ֻҪ���ֺ���ͷ����
**                          void PendSV_Handler (), ��PendSV����ʱ���ú����ᱻ�Զ�����
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
__asm void PendSV_Handler ()
{   
    IMPORT  currentTask               // ʹ��import����C�ļ���������ȫ�ֱ���
    IMPORT  nextTask                  // ��������C���ļ���ʹ��extern int variable
    
    MRS     R0, PSP                   // ��ȡ��ǰ����Ķ�ջָ��
    CBZ     R0, PendSVHandler_nosave  // if ������tTaskSwitch������(��ʱ��PSP�϶�������0�ˣ�0�Ļ��ض���tTaskRunFirst)����
                                      // ������Ļ��������ȿ�tTaskRunFirst��tTaskSwitch��ʵ��
    STMDB   R0!, {R4-R11}             //     ��ô��������Ҫ�����쳣�Զ�����ļĴ�������������Ĵ����Զ���������{R4, R11}
                                      //     ����ĵ�ַ�ǵ�ǰ�����PSP��ջ�У������������ı����˱�Ҫ��CPU�Ĵ���,�����´λָ�
    LDR     R1, =currentTask          //     ����ú󣬽����Ķ�ջ��λ�ã����浽currentTask->stack��    
    LDR     R1, [R1]                  //     ����stack���ڽṹ��stack���Ŀ�ʼλ�ô�����ȻcurrentTask��stack���ڴ��е���ʼ
    STR     R0, [R1]                  //     ��ַ��һ���ģ���ô���������κ�����

PendSVHandler_nosave                  // ������tTaskSwitch��tTaskSwitch�����ģ����Ҫ����һ��Ҫ���е�����Ķ�ջ�лָ�
                                      // CPU�Ĵ�����Ȼ���л���������������
    LDR     R0, =currentTask          // ���ˣ�׼���л���
    LDR     R1, =nextTask             
    LDR     R2, [R1]  
    STR     R2, [R0]                  // �Ƚ�currentTask����ΪnextTask��Ҳ������һ�������˵�ǰ����
 
    LDR     R0, [R2]                  // Ȼ�󣬴�currentTask�м���stack��������֪�����ĸ�λ��ȡ��CPU�Ĵ����ָ�����
    LDMIA   R0!, {R4-R11}             // �ָ�{R4, R11}��Ϊʲôֻ�ָ�����ô�㣬��Ϊ�������˳�PendSVʱ��Ӳ���Զ��ָ�

    MSR     PSP, R0                   // ��󣬻ָ������Ķ�ջָ�뵽PSP  
    ORR     LR, LR, #0x04             // ����·��ر�ǣ�ָ�����˳�LRʱ���л���PSP��ջ��(PendSVʹ�õ���MSP) 
    BX      LR                        // ��󷵻أ���ʱ����ͻ�Ӷ�ջ��ȡ��LRֵ���ָ����ϴ����е�λ��
}  

/**********************************************************************************************************
** Function name        :   tTaskRunFirst
** Descriptions         :   ������tinyOSʱ�����øú��������л�����һ����������
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void tTaskRunFirst()
{
    __set_PSP(0);

    MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;   // ��NVIC_SYSPRI2дNVIC_PENDSV_PRI��������Ϊ������ȼ�
    
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;    // ��NVIC_INT_CTRLдNVIC_PENDSVSET������PendSV

}
    
void tTaskSwitch() 
{

    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;  // ��NVIC_INT_CTRLдNVIC_PENDSVSET������PendSV
}
