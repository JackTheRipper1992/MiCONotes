#include "zengjf.h"

int main()
{    
    LED_GPIO_Config(); 
	
    while(1)
	{
		LED0_ON;
		Delay(0x2fffff);
		LED0_OFF;
		Delay(0x2fffff);
	}
}
