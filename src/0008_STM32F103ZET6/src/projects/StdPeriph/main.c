#include "zengjf.h"

osSemaphoreId_t sid_Thread_Semaphore; 
int count = 0;
extern char* welcome_msg;


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
    printf("%s", welcome_msg);
    printf("AplexOS # ");
    for (;;)
    {
        get_cmd_parser_char();
        led_toggle(GPIOE, GPIO_Pin_5);
    }
}

int main(void)
{    
    USART1_Config(115200);
    LED_GPIO_Config();
    EXTI_Config();
    
    jansson_pack_test();

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
