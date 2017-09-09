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

// I2C driver instance
extern ARM_DRIVER_I2C Driver_I2C1; 
static ARM_DRIVER_I2C *i2cDev = &Driver_I2C1;
 
static volatile uint32_t event = 0;
 
static void I2C_DrvEvent (uint32_t e) {
    event |= e;
}

void vTaskI2C0(void *p)
{
    uint8_t cnt = 0;
 
    /* Initialize I2C peripheral */
    i2cDev->Initialize(I2C_DrvEvent);
 
    /* Power-on SPI peripheral */
    i2cDev->PowerControl(ARM_POWER_FULL);
 
    /* Configure USART bus*/
    i2cDev->Control(ARM_I2C_OWN_ADDRESS, 0x33);
    
    printf("\r\nI2C0 thread start.\r\n\r\n");
 
    while (1) {
        /* Receive chuck */
        i2cDev->SlaveReceive(&cnt, 1);
        while ((event & ARM_I2C_EVENT_TRANSFER_DONE) == 0);
        event &= ~ARM_I2C_EVENT_TRANSFER_DONE;
        
        printf("Receive a byte data: %x\r\n", cnt);
 
        /* Transmit chunk back */
        i2cDev->SlaveTransmit(&cnt, 1);
        while ((event & ARM_I2C_EVENT_TRANSFER_DONE) == 0);
        event &= ~ARM_I2C_EVENT_TRANSFER_DONE;
        
        printf("Send a byte data: %x\r\n", cnt);
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
    
    sid_Thread_Semaphore = osSemaphoreNew(1, 0, NULL);
    if (!sid_Thread_Semaphore) {
        printf("get sid_Thread_Semaphore error."); // Semaphore object not created, handle failure
    }
    osThreadNew(vTaskEXTILed, NULL, NULL);    // Create application thread
    
    osKernelStart();                            // Start thread execution
}
