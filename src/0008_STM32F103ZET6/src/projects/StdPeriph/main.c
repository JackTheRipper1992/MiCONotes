#include "zengjf.h"

char str[512] = {0};
osSemaphoreId_t sid_Thread_Semaphore; 
int count = 0;
    
void vTaskLedRed(void *p)
{
    for (;;)
    {
        led_toggle(GPIOE, GPIO_Pin_6);
        Delay(0x1FFFFF);
    }
}

void vTaskEXTILed(void *p)
{
    for (;;)
    {
        osSemaphoreAcquire (sid_Thread_Semaphore, osWaitForever);
        led_toggle(GPIOE, GPIO_Pin_5);
        printf("EXTI Count Value: %d\r\n", count++);
    }
}

void vTaskDebugPort(void *p)
{

    for (;;)
    {
        scanf("%s", str);
        memset(str, 0, strlen(str));
        led_toggle(GPIOE, GPIO_Pin_5);
    }
}

int main(void)
{    
    USART1_Config(115200);
    LED_GPIO_Config();
    EXTI_Config();
    
    jansson_pack_test();

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
    
    osThreadNew(vTaskLedRed, NULL, NULL);       // Create application thread
    osThreadNew(vTaskDebugPort, NULL, NULL);    // Create application thread
    
    sid_Thread_Semaphore = osSemaphoreNew(1, 0, NULL);
    if (!sid_Thread_Semaphore) {
        printf("get sid_Thread_Semaphore error."); // Semaphore object not created, handle failure
    }
    osThreadNew(vTaskEXTILed, NULL, NULL);    // Create application thread
    
    osKernelStart();                            // Start thread execution
}
