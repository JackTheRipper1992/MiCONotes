#include "zengjf.h"
#include "cmsis_os2.h"

char str[512] = {0};
    
void vTaskLedRed(void *p)
{
    for (;;)
    {
        led_toggle(GPIOE, GPIO_Pin_6);
        Delay(0x1FFFFF);
    }
}

void vTaskDebugPort(void *p)
{

    for (;;)
    {
        scanf("%s", str);
        memset(str, 0, strlen(str));
    }
}

int main(void)
{    
    USART1_Config(115200);
    LED_GPIO_Config();

    printf("\r\n Hardware Auto Detect System.");
    printf("\r\n Version: 0.0.1");
    printf("\r\n           ---- Designed By zengjf \r\n");
    
    // System Initialization
    SystemCoreClockUpdate();
    #ifdef RTE_Compiler_EventRecorder
    // Initialize and start Event Recorder
    EventRecorderInitialize(EventRecordError, 1U);
    #endif

    osKernelInitialize();                       // Initialize CMSIS-RTOS
    osThreadNew(vTaskLedRed, NULL, NULL);       // Create application main thread
    osThreadNew(vTaskDebugPort, NULL, NULL);    // Create application main thread
    osKernelStart();                            // Start thread execution
}
