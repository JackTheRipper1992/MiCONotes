#include "zengjf.h"

int main(void)
{	
    USART1_Config(115200);
    LED_GPIO_Config();

    printf("\r\n Hardware Auto Detect System.");
    printf("\r\n Version: 0.0.1");
    printf("\r\n           ---- Designed By zengjf \r\n");
    
    char str[512] = {0};

	while(1)
	{
        scanf("%s", str);
        memset(str, 0, strlen(str));
    }
}
