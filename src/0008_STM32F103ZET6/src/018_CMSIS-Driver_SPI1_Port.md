# CMSIS Driver UART2 Port

## 参考资料

* [CMSIS-Driver SPI Interface](https://www.keil.com/pack/doc/CMSIS/Driver/html/group__spi__interface__gr.html)

## 硬件连接

SPI_MISO连接SPI_MOSI进行Loop循环测试。

## 裸板示例

* Source Code
  ```C
  //SPIx 读写一个字节
  //TxData:要写入的字节
  //返回值:读取到的字节
  u8 SPI2_ReadWriteByte(u8 TxData)
  {        
      u8 retry=0;                     
      while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)        //检查指定的SPI标志位设置与否:发送缓存空标志位
      {
          retry++;
          if(retry>200)return 0;
      }              
      SPI_I2S_SendData(SPI2, TxData);                                       //通过外设SPIx发送一个数据
      retry=0;
  
      while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)       //检查指定的SPI标志位设置与否:接受缓存非空标志位
      {
          retry++;
          if(retry>200)return 0;
      }                                  
      return SPI_I2S_ReceiveData(SPI2);                                     //返回通过SPIx最近接收的数据
  }
  
  void SPI2_Init(void)
  {
       GPIO_InitTypeDef GPIO_InitStructure;
        SPI_InitTypeDef  SPI_InitStructure;
  
      RCC_APB2PeriphClockCmd(    RCC_APB2Periph_GPIOB, ENABLE );            //PORTB时钟使能 
      RCC_APB1PeriphClockCmd(    RCC_APB1Periph_SPI2,  ENABLE );            //SPI2时钟使能
   
      GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
      GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;                      // PB13/14/15复用推挽输出 
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_Init(GPIOB, &GPIO_InitStructure);                                //初始化GPIOB
  
       // GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);          //PB13/14/15上拉
  
      SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;    //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
      SPI_InitStructure.SPI_Mode      = SPI_Mode_Master;                    //设置SPI工作模式:设置为主SPI
      SPI_InitStructure.SPI_DataSize  = SPI_DataSize_8b;                    //设置SPI的数据大小:SPI发送接收8位帧结构
      SPI_InitStructure.SPI_CPOL      = SPI_CPOL_High;                      //串行同步时钟的空闲状态为高电平
      SPI_InitStructure.SPI_CPHA      = SPI_CPHA_2Edge;                     //串行同步时钟的第二个跳变沿（上升或下降）数据被采样
      SPI_InitStructure.SPI_NSS       = SPI_NSS_Soft;                       //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
      SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;  //定义波特率预分频的值:波特率预分频值为256
      SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;           //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
      SPI_InitStructure.SPI_CRCPolynomial     = 7;                          //CRC值计算的多项式
      SPI_Init(SPI2, &SPI_InitStructure);                                   //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
   
      SPI_Cmd(SPI2, ENABLE);                                                //使能SPI外设
  }
  ```
* Test Code：
  ```
  char ch = 0;
  
  SPI2_Init();
  
  while(1) {
      ch = SPI2_ReadWriteByte('a');
      printf("%c.\r\n", ch);
      Delay(0x2ffff);
  }
  ```

## SPI CMSIS Driver测试

* Source Code：
  ```C
  /* SPI Driver */
  extern ARM_DRIVER_SPI Driver_SPI1;
  osSemaphoreId_t spi_Thread_Semaphore; 
   
  void mySPI_callback(uint32_t event)
  {
      switch (event)
      {
      case ARM_SPI_EVENT_TRANSFER_COMPLETE:
          /* Success: Wakeup Thread */
          osSemaphoreRelease (spi_Thread_Semaphore);
          break;
      case ARM_SPI_EVENT_DATA_LOST:
          /*  Occurs in slave mode when data is requested/sent by master
              but send/receive/transfer operation has not been started
              and indicates that data is lost. Occurs also in master mode
              when driver cannot transfer data fast enough. */
          __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
          break;
      case ARM_SPI_EVENT_MODE_FAULT:
          /*  Occurs in master mode when Slave Select is deactivated and
              indicates Master Mode Fault. */
          __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
          break;
      }
  }
   
  /* Test data buffers */
  uint8_t testdata_out[8] = { 0, 1, 2, 3, 4, 5, 6, 7 }; 
  uint8_t testdata_in [8];
   
  void mySPI_Thread(void* arg)
  {
      ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
   
      /* Initialize the SPI driver */
      SPIdrv->Initialize(mySPI_callback);
      /* Power up the SPI peripheral */
      SPIdrv->PowerControl(ARM_POWER_FULL);
      /* Configure the SPI to Master, 8-bit mode @10000 kBits/sec */
      SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_SW | ARM_SPI_DATA_BITS(8), 10000000);
   
      /* SS line = INACTIVE = HIGH */
      SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
   
      /* thread loop */
      while (1)
      {
          /* SS line = ACTIVE = LOW */
          SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
          /* Transmit some data */
          SPIdrv->Transfer(testdata_out, testdata_in, sizeof(testdata_out));
          /* Wait for completion */
          osSemaphoreAcquire (spi_Thread_Semaphore, osWaitForever);
          /* SS line = INACTIVE = HIGH */
          SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
          
          int i = 0;
          for (i = 0; i < sizeof(testdata_in) / sizeof(testdata_in[0]); i++) {
              printf("%d\n\r", testdata_in[i]);
              testdata_out[i] += 1;
          }

          osDelay(1000); 
      }
  }
  ```
* 线程创建：
  ```C
  spi_Thread_Semaphore = osSemaphoreNew(1, 0, NULL);
  if (!spi_Thread_Semaphore) {
      printf("get spi_Thread_Semaphore error.\r\n");  // Semaphore object not created, handle failure
  }
  osThreadNew(mySPI_Thread, NULL, NULL);              // Create application thread
  ```
