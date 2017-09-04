# 时钟设置

* SystemInit函数被startup_stm32f10x_hd.s调用：
  ```
  ; Reset handler
  Reset_Handler   PROC
                  EXPORT  Reset_Handler             [WEAK]
                  IMPORT  __main
                  IMPORT  SystemInit
                  LDR     R0, =SystemInit         ; 被调用
                  BLX     R0               
                  LDR     R0, =__main
                  BX      R0
                  ENDP
  ```
* SystemInit调用SetSysClock函数：
  ```
  void SystemInit (void)
  {
    /* Reset the RCC clock configuration to the default reset state(for debug purpose) */
    /* Set HSION bit */
    RCC->CR |= (uint32_t)0x00000001;
  
    /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
  #ifndef STM32F10X_CL
    RCC->CFGR &= (uint32_t)0xF8FF0000;
  #else
    RCC->CFGR &= (uint32_t)0xF0FF0000;
  #endif /* STM32F10X_CL */   
    
    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= (uint32_t)0xFEF6FFFF;
  
    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;
  
    /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
    RCC->CFGR &= (uint32_t)0xFF80FFFF;
  
  #ifdef STM32F10X_CL
    /* Reset PLL2ON and PLL3ON bits */
    RCC->CR &= (uint32_t)0xEBFFFFFF;
  
    /* Disable all interrupts and clear pending bits  */
    RCC->CIR = 0x00FF0000;
  
    /* Reset CFGR2 register */
    RCC->CFGR2 = 0x00000000;
  #elif defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || (defined STM32F10X_HD_VL)
    /* Disable all interrupts and clear pending bits  */
    RCC->CIR = 0x009F0000;
  
    /* Reset CFGR2 register */
    RCC->CFGR2 = 0x00000000;      
  #else
    /* Disable all interrupts and clear pending bits  */
    RCC->CIR = 0x009F0000;
  #endif /* STM32F10X_CL */
      
  #if defined (STM32F10X_HD) || (defined STM32F10X_XL) || (defined STM32F10X_HD_VL)
    #ifdef DATA_IN_ExtSRAM
      SystemInit_ExtMemCtl(); 
    #endif /* DATA_IN_ExtSRAM */
  #endif 
  
    /* Configure the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers */
    /* Configure the Flash Latency cycles and enable prefetch buffer */
    SetSysClock();
  
  #ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
  #else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
  #endif 
  }
  ```
* 设置系统时钟：
  ```
  static void SetSysClock(void)
  {
  #ifdef SYSCLK_FREQ_HSE
    SetSysClockToHSE();
  #elif defined SYSCLK_FREQ_24MHz
    SetSysClockTo24();
  #elif defined SYSCLK_FREQ_36MHz
    SetSysClockTo36();
  #elif defined SYSCLK_FREQ_48MHz
    SetSysClockTo48();
  #elif defined SYSCLK_FREQ_56MHz
    SetSysClockTo56();  
  #elif defined SYSCLK_FREQ_72MHz
    SetSysClockTo72();
  #endif
   
   /* If none of the define above is enabled, the HSI is used as System clock
      source (default after reset) */ 
  }
  ```
* 系统时钟设置宏：
  ```
  #if defined (STM32F10X_LD_VL) || (defined STM32F10X_MD_VL) || (defined STM32F10X_HD_VL)
  /* #define SYSCLK_FREQ_HSE    HSE_VALUE */
   #define SYSCLK_FREQ_24MHz  24000000
  #else
  /* #define SYSCLK_FREQ_HSE    HSE_VALUE */
  /* #define SYSCLK_FREQ_24MHz  24000000 */ 
  /* #define SYSCLK_FREQ_36MHz  36000000 */
  /* #define SYSCLK_FREQ_48MHz  48000000 */
  /* #define SYSCLK_FREQ_56MHz  56000000 */
  #define SYSCLK_FREQ_72MHz  72000000                           // <--------------------
  #endif
  ```

