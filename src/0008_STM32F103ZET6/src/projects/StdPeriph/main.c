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

void vTaskI2C0(void *p)
{
    i2c_data_transmission();
}

/* USART Driver */
extern ARM_DRIVER_USART Driver_USART2;
 
void myUSART_callback(uint32_t event) { }
void vTaskUsart2(void *p)
{
    static ARM_DRIVER_USART * USARTdrv = &Driver_USART2;
    char                   cmd;
 
    /*Initialize the USART driver */
    USARTdrv->Initialize(myUSART_callback);
    /*Power up the USART peripheral */
    USARTdrv->PowerControl(ARM_POWER_FULL);
    /*Configure the USART to 115200 Bits/sec */
    USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE, 115200);
     
    /* Enable Receiver and Transmitter lines */
    USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
    USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
    char* sendstr = "\r\nPress Enter to receive a message";
    USARTdrv->Send(sendstr, strlen(sendstr));
     
    while (1)
    {
        USARTdrv->Receive(&cmd, 1);          /* Get byte from UART */
        if (cmd == 13)                       /* CR, send greeting  */
        {
          USARTdrv->Send("\r\nHello World!", 15);
        }
        cmd = 0;

    }
}


int main(void)
{    
    USART1_Config(115200);
    LED_GPIO_Config();
    EXTI_Config();

    // System Initialization
    SystemCoreClockUpdate();
    #ifdef RTE_Compiler_EventRecorder
    // Initialize and start Event Recorder
    EventRecorderInitialize(EventRecordError, 1U);
    #endif

    osKernelInitialize();                       // Initialize CMSIS-RTOS
    
    osThreadNew(vTaskLedRed, NULL, NULL);       // Create application thread
    osThreadNew(vTaskDebugPort, NULL, NULL);    // Create application thread
    osThreadNew(vTaskI2C0, NULL, NULL);         // Create application thread
    osThreadNew(vTaskUsart2, NULL, NULL);         // Create application thread
    
    sid_Thread_Semaphore = osSemaphoreNew(1, 0, NULL);
    if (!sid_Thread_Semaphore) {
        printf("get sid_Thread_Semaphore error."); // Semaphore object not created, handle failure
    }
    osThreadNew(vTaskEXTILed, NULL, NULL);    // Create application thread
    
    osKernelStart();                            // Start thread execution
}
