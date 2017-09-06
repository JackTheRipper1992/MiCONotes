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
 * 函数名：Key_GPIO_Config
 * 描述  ：配置按键用到的I/O口
 * 输入  ：无
 * 输出  ：无
 */
void Key_GPIO_Config(void)
{ 
    GPIO_InitTypeDef GPIO_InitStructure;

    /*开启按键端口（PA0/PA13/PA15）的时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    /*初始化KEY0-->GPIOA.13,KEY1-->GPIOA.15  上拉输入*/
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_13|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*初始化 WK_UP-->GPIOA.0      下拉输入*/
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);


}

/*
 * 函数名：Key_Scan
 * 描述  ：检测是否有按键按下
 * 输入  ：无    
 * 输出  ：返回按键值
 */

unsigned char KEY_Scan(void)
{     
    unsigned char key_code;
    if(KEY1_Scan()==1)key_code=1;
    else if(KEY0_Scan()==1)key_code=2;
    else if(KEY2_Scan()==1)key_code=3;
    else key_code=0;
    return key_code;
}

unsigned char KEY1_Scan(void)
{     
    static char key_up1=0;//按键按松开标志    
    if(KEY1==0)
    {
        /*延时去抖动 */
        Delay(10000);    
        if(KEY1==0)
        {
            key_up1=1;
        }        
    }
    if(KEY1==1&&key_up1==1)
    {
        key_up1=0;
        return 1;
    }
    return 0;
}

unsigned char KEY0_Scan(void)
{     
    static char key_up0=0;//按键按松开标志    

    if(KEY0==0)
    {
        /*延时去抖动 */
        Delay(10000);    

        if(KEY0==0)
        {
            key_up0=1;
        }    
    }

    if(KEY0==1&&key_up0==1)
    {
        key_up0=0;
        return 1;
    }
    return 0;
}



unsigned char KEY2_Scan(void)
{     
    static char key_up2=0;//按键按松开标志    

    if(KEY2==1)
    {
        /*延时去抖动 */
        Delay(10000);    

        if(KEY2==1)
        {
            key_up2=1;
        }    
    }
    if(KEY2==0&&key_up2==1)
    {
        key_up2=0;
        return 1;
    }
    return 0;
}
