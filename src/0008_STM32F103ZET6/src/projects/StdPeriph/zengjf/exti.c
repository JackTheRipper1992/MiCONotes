/******************** 尚学科技 **************************
 * 实验平台：开拓者STM32开发板
 * 库版本  ：ST3.5.0
 * 作者    ：尚学科技团队 
 * 淘宝    ：http://shop102218275.taobao.com/
 * 本程序只供学习使用，未经作者许可，不得用于其它任何用途
 *版权所有，盗版必究。
 **********************************************************************************/

#include "zengjf.h"


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
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
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

    /*开启外设时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);

    /*初始化 GPIOA.13,GPIOA.15  设置为上拉输入*/
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_13|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* 初始化 GPIOA.0      设置为下拉输入*/
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*GPIOA.13 中断线配置*/
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource13);

    /*GPIOA.13 中断初始化配置*/
    EXTI_InitStructure.EXTI_Line=EXTI_Line13;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;    
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    /*根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存*/
    EXTI_Init(&EXTI_InitStructure);     

    /*GPIOA.15      中断线配置*/
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource15);

    /*GPIOA.15      中断初始化配置*/
    EXTI_InitStructure.EXTI_Line=EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;    
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    /*根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器*/
    EXTI_Init(&EXTI_InitStructure);          

    /*GPIOA.0 中断线配置*/
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);

    /*GPIOA.0 中断初始化配置*/
    EXTI_InitStructure.EXTI_Line=EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;    
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    /*根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器*/
    EXTI_Init(&EXTI_InitStructure);        

    /*配置中断控制器NVIC*/
    NVIC_Configuration();

}


