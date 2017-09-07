#include "zengjf.h"

void EXTI0_IRQHandler(void)
{     
    /*��ʱ����*/
    Delay(10000);    
    /*���ָ����EXTI0��·�������������*/    
    if(EXTI_GetITStatus(EXTI_Line0) != RESET)      
    {      
        /*����LED��IO��ƽ��ת*/
        GPIO_WriteBit(GPIOE, GPIO_Pin_6, (BitAction)(1-(GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_6))));
    }
    /*���EXTI0��·����λ*/
    EXTI_ClearITPendingBit(EXTI_Line0);  
}

void EXTI4_IRQHandler(void)
{   
    /*��ʱ����*/
    Delay(10000);               
    /*���ָ����EXTI13��·�������������*/    
    if(EXTI_GetITStatus(EXTI_Line4) != RESET)
    {   
        /*����LED��IO��ƽ��ת*/
        GPIO_WriteBit(GPIOE, GPIO_Pin_5, (BitAction)(1-(GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_5))));
    }
    /*���ָ����EXTI15��·�������������*/    

    /*���EXTI13��·����λ*/
    EXTI_ClearITPendingBit(EXTI_Line4); 

}

/*
 * ��������NVIC_Configuration
 * ����  ������Ƕ�������жϿ�����NVIC
 * ����  ����
 * ���  ����
 * ����  ���ڲ�����
 */
static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    /*����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);    

    /*ʹ�ܰ������ڵ��ⲿ�ж�ͨ��*/
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
    /*������ռ���ȼ�����ռ���ȼ���Ϊ2*/    
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;    
    /*���������ȼ��������ȼ���Ϊ1*/
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;        
    /*ʹ���ⲿ�ж�ͨ*/
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;    
    /*����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���*/        
    NVIC_Init(&NVIC_InitStructure); 

    /*ʹ�ܰ������ڵ��ⲿ�ж�ͨ��*/
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;            
    /*������ռ���ȼ�����ռ���ȼ���Ϊ2*/    
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;    
    /*���������ȼ��������ȼ���Ϊ2*/
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;        
    /*ʹ���ⲿ�ж�ͨ*/
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;    
    /*����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���*/    
    NVIC_Init(&NVIC_InitStructure);       

}


/*
 * ��������EXTI_Config
 * ����  ������PA0,PA13,PA15Ϊ���жϿڣ��������ж����ȼ�
 * ����  ����
 * ���  ����
 * ����  ���ⲿ����
 */
void EXTI_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    /* ��������ʱ�� */
    // ѧϰSTM32(2)-IO-AFIO(���ù���IO�͵�������)
    //   http://blog.csdn.net/k122769836/article/details/7700153
    // STM32-AFIO��ʹ��
    //   http://j1o1y.blog.sohu.com/300071648.html
    // STM32 APB2 AFIOʱ��ʲôʱ����Ҫ����
    //   http://www.rationmcu.com/stm32/1541.html
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_AFIO, ENABLE);

    /* ��ʼ�� GPIOA.4  ����Ϊ�������� */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    /* ��ʼ�� GPIOA.0      ����Ϊ��������*/
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* GPIOA.4 �ж������ã�����Ӧ��GPIO����������Ӧ�ж����� */
    // STM32֮�ⲿ�ж� EXTI
    //   http://blog.csdn.net/Davincdada/article/details/70652888
    // GPIOE.4���ж��߾�������GPIO_PINSOURCE4�ϵģ����Կ�ǰ��Ĳο��ĵ�
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource4);

    /* GPIOA.13 �жϳ�ʼ������ */
    // ����ǰ���Ѿ����ú���GPIO_PINSOURCE4����������EXTI_Line4�ͱ�ʾ����GPIO_PINSOURCE4�ϵ��ж���
    EXTI_InitStructure.EXTI_Line    = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;    
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    /* ����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ� */
    EXTI_Init(&EXTI_InitStructure);     

    /* GPIOA.0 �ж������ã�����Ӧ��GPIO����������Ӧ�ж����� */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    /* GPIOA.0 �жϳ�ʼ������ */
    EXTI_InitStructure.EXTI_Line    = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;    
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    /* ����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ��� */
    EXTI_Init(&EXTI_InitStructure);        

    /* �����жϿ�����NVIC */
    NVIC_Configuration();

}


