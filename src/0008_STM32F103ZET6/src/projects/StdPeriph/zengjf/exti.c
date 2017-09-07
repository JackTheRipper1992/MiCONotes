#include "zengjf.h"

void EXTI0_IRQHandler(void)
{     
    /*延时消抖*/
    Delay(10000);    
    /*检查指定的EXTI0线路触发请求发生与否*/    
    if(EXTI_GetITStatus(EXTI_Line0) != RESET)      
    {      
        /*控制LED的IO电平翻转*/
        GPIO_WriteBit(GPIOE, GPIO_Pin_6, (BitAction)(1-(GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_6))));
    }
    /*清除EXTI0线路挂起位*/
    EXTI_ClearITPendingBit(EXTI_Line0);  
}

void EXTI4_IRQHandler(void)
{   
    /*延时消抖*/
    Delay(10000);               
    /*检查指定的EXTI13线路触发请求发生与否*/    
    if(EXTI_GetITStatus(EXTI_Line4) != RESET)
    {   
        /*控制LED的IO电平翻转*/
        GPIO_WriteBit(GPIOE, GPIO_Pin_5, (BitAction)(1-(GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_5))));
    }
    /*检查指定的EXTI15线路触发请求发生与否*/    

    /*清除EXTI13线路挂起位*/
    EXTI_ClearITPendingBit(EXTI_Line4); 

}

/*
 * 函数名：NVIC_Configuration
 * 描述  ：配置嵌套向量中断控制器NVIC
 * 输入  ：无
 * 输出  ：无
 * 调用  ：内部调用
 */
static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    /*设置NVIC中断分组2:2位抢占优先级，2位响应优先级*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);    

    /*使能按键所在的外部中断通道*/
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
    /*设置抢占优先级，抢占优先级设为2*/    
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;    
    /*设置子优先级，子优先级设为1*/
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;        
    /*使能外部中断通*/
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;    
    /*根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器*/        
    NVIC_Init(&NVIC_InitStructure); 

    /*使能按键所在的外部中断通道*/
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;            
    /*设置抢占优先级，抢占优先级设为2*/    
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;    
    /*设置子优先级，子优先级设为2*/
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;        
    /*使能外部中断通*/
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;    
    /*根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器*/    
    NVIC_Init(&NVIC_InitStructure);       

}


/*
 * 函数名：EXTI_Config
 * 描述  ：配置PA0,PA13,PA15为线中断口，并设置中断优先级
 * 输入  ：无
 * 输出  ：无
 * 调用  ：外部调用
 */
void EXTI_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    /* 开启外设时钟 */
    // 学习STM32(2)-IO-AFIO(复用功能IO和调试配置)
    //   http://blog.csdn.net/k122769836/article/details/7700153
    // STM32-AFIO的使用
    //   http://j1o1y.blog.sohu.com/300071648.html
    // STM32 APB2 AFIO时钟什么时候需要开启
    //   http://www.rationmcu.com/stm32/1541.html
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_AFIO, ENABLE);

    /* 初始化 GPIOA.4  设置为上拉输入 */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    /* 初始化 GPIOA.0      设置为下拉输入*/
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* GPIOA.4 中断线配置，将相应的GPIO引脚连到相应中断线上 */
    // STM32之外部中断 EXTI
    //   http://blog.csdn.net/Davincdada/article/details/70652888
    // GPIOE.4的中断线就是连在GPIO_PINSOURCE4上的，可以看前面的参考文档
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource4);

    /* GPIOA.13 中断初始化配置 */
    // 由于前面已经设置好了GPIO_PINSOURCE4，所以这里EXTI_Line4就表示了是GPIO_PINSOURCE4上的中断了
    EXTI_InitStructure.EXTI_Line    = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;    
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    /* 根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存 */
    EXTI_Init(&EXTI_InitStructure);     

    /* GPIOA.0 中断线配置，将相应的GPIO引脚连到相应中断线上 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    /* GPIOA.0 中断初始化配置 */
    EXTI_InitStructure.EXTI_Line    = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;    
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    /* 根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器 */
    EXTI_Init(&EXTI_InitStructure);        

    /* 配置中断控制器NVIC */
    NVIC_Configuration();

}


