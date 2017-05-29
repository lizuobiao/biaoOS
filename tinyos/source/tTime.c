#include "tinyOS.h"

void tTaskDelay (uint32_t delay) 
{
    // �����ٽ������Ա�������������������л��ڼ䣬������Ϊ�����жϵ���currentTask��nextTask���ܸ���
    uint32_t status = tTaskEnterCritical();
 
    // ������ʱֵ��������ʱ����
    tTimeTaskWait(currentTask, delay);
 
    // ������Ӿ��������Ƴ�
    tTaskSchedUnRdy(currentTask);

    // Ȼ����������л����л�����һ�����񣬻��߿�������
    // delayTikcs����ʱ���ж����Զ���1.������0ʱ�����л������������С�
    tTaskSched();

    // �˳��ٽ���
    tTaskExitCritical(status); 
}
