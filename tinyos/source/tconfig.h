#ifndef TCONFIG_H
#define TCONFIG_H

#define	TINYOS_PRO_COUNT				32						// TinyOS��������ȼ����
#define TINYOS_SLICE_MAX				10	

#define TINYOS_IDLETASK_STACK_SIZE		1024					// ��������Ķ�ջ��Ԫ��
#define TINYOS_TIMERTASK_STACK_SIZE		1024					// ��ʱ������Ķ�ջ��Ԫ��
#define TINYOS_TIMERTASK_PRIO           1                       // ��ʱ����������ȼ�

#define TINYOS_SYSTICK_MS               10                      // ʱ�ӽ��ĵ����ڣ���msΪ��λ

#define TINYOS_ENABLE_SEM               1                       // �Ƿ�ʹ���ź���
#define TINYOS_ENABLE_MUTEX             1                       // �Ƿ�ʹ�ܻ����ź���
#define TINYOS_ENABLE_FLAGGROUP         1                      // �Ƿ�ʹ���¼���־��
#define TINYOS_ENABLE_MBOX              1                       // �Ƿ�ʹ������
#define TINYOS_ENABLE_MEMBLOCK          1                       // �Ƿ�ʹ�ܴ洢��
#define TINYOS_ENABLE_TIMER             1                       // �Ƿ�ʹ�ܶ�ʱ��
//#define TINYOS_ENABLE_CPUUSAGE_STAT     0                       // �Ƿ�ʹ��CPUʹ����ͳ��
#endif /* TCONFIG_H */
