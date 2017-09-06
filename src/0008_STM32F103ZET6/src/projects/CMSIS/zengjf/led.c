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
 * 函数名：LED_GPIO_Config
 * 描述  ：配置LED用到的I/O口
 * 输入  ：无
 * 输出  ：无
 */
void LED_GPIO_Config(void)
{		
	/*定义一个GPIO_InitTypeDef类型的结构体*/
	 GPIO_InitTypeDef GPIO_InitStructure;

	/*开启GPIOC的外设时钟*/
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

	/*选择要控制的GPIOC引脚*/															   
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	

	/*设置引脚模式为通用推挽输出*/
  	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

	/*设置引脚速率为50MHz */   
  	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*调用库函数，初始化GPIOC*/
     GPIO_Init(GPIOE, &GPIO_InitStructure);
	 
    /*PA.8 输出高*/
     GPIO_SetBits(GPIOE,GPIO_Pin_6);  
    
    /*
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE); 
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6 ;            
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;  
	
    GPIO_Init(GPIOE,&GPIO_InitStructure); 
    */
}
