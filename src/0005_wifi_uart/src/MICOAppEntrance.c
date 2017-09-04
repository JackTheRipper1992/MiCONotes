/**
 ******************************************************************************
 * @file    MICOAppEntrance.c
 * @author  William Xu
 * @version V1.0.0
 * @date    05-May-2014
 * @brief   Mico application entrance, addd user application functons and threads.
 ******************************************************************************
 *  The MIT License
 *  Copyright (c) 2014 MXCHIP Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */

#include "mico.h"

#include "StringUtils.h"
#include "SppProtocol.h"
#include "cfunctions.h"
#include "cppfunctions.h"
#include "MICOAppDefine.h"

#define app_log(M, ...) custom_log("APP", M, ##__VA_ARGS__)
/**
 *  #define custom_log_trace(N) do {if (mico_debug_enabled==0)break;\
 *                                  mico_rtos_lock_mutex( &stdio_tx_mutex );\
 *                                  printf("[%s: [TRACE] %s] %s()\r\n", N, SHORT_FILE, __PRETTY_FUNCTION__);\
 *                                  mico_rtos_unlock_mutex( &stdio_tx_mutex );}while(0==1)
 */
#define app_log_trace() custom_log_trace("APP")

/**
 * typedef struct
 * {
 *   uint32_t  size;
 *   uint32_t  head;
 *   uint32_t  tail;
 *   uint8_t*  buffer;
 * } ring_buffer_t;
 */
volatile ring_buffer_t rx_buffer;
/**
 * MICOAppDefine.h
 *
 * #define UART_BUFFER_LENGTH                  2048
 */
volatile uint8_t rx_data[UART_BUFFER_LENGTH];

extern void localTcpServer_thread( uint32_t inContext );

extern void remoteTcpClient_thread( uint32_t inContext );

extern void uartRecv_thread( uint32_t inContext );

extern OSStatus MICOStartBonjourService( WiFi_Interface interface, app_context_t * const inContext );

/* MICO system callback: Restore default configuration provided by application */
void appRestoreDefault_callback( void * const user_config_data, uint32_t size )
{
    UNUSED_PARAMETER( size );
    application_config_t* appConfig = user_config_data;
    appConfig->configDataVer = CONFIGURATION_VERSION;
    appConfig->localServerPort = LOCAL_PORT;
    appConfig->localServerEnable = true;
    appConfig->USART_BaudRate = 115200;
    appConfig->remoteServerEnable = true;
    sprintf( appConfig->remoteServerDomain, DEAFULT_REMOTE_SERVER );
    appConfig->remoteServerPort = DEFAULT_REMOTE_SERVER_PORT;
}

/* EasyLink callback: Notify wlan configuration type */
USED void mico_system_delegate_config_success( mico_config_source_t source )
{
    app_log( "Configured by %d", source );
}

/* Config server callback: Current Device configuration sent to config client */
USED void config_server_delegate_report( json_object *app_menu, mico_Context_t *in_context )
{
    OSStatus err = kNoErr;
    application_config_t *appConfig = mico_system_context_get_user_data( in_context );

    // SPP protocol remote server connection enable
    err = config_server_create_bool_cell( app_menu, "Connect SPP Server", appConfig->remoteServerEnable, "RW" );
    require_noerr( err, exit );

    //Server address cell
    err = config_server_create_string_cell( app_menu, "SPP Server", appConfig->remoteServerDomain, "RW", NULL );
    require_noerr( err, exit );

    //Server port cell
    err = config_server_create_number_cell( app_menu, "SPP Server Port", appConfig->remoteServerPort, "RW", NULL );
    require_noerr( err, exit );

    /*UART Baurdrate cell*/
    json_object *selectArray;
    selectArray = json_object_new_array( );
    require( selectArray, exit );
    json_object_array_add( selectArray, json_object_new_int( 9600 ) );
    json_object_array_add( selectArray, json_object_new_int( 19200 ) );
    json_object_array_add( selectArray, json_object_new_int( 38400 ) );
    json_object_array_add( selectArray, json_object_new_int( 57600 ) );
    json_object_array_add( selectArray, json_object_new_int( 115200 ) );
    err = config_server_create_number_cell( app_menu, "Baurdrate", appConfig->USART_BaudRate, "RW", selectArray );
    require_noerr( err, exit );

    exit:
    return;
}

/* Config server callback: New Device configuration received from config client */
USED void config_server_delegate_recv( const char *key, json_object *value, bool *need_reboot,
                                       mico_Context_t *in_context )
{
    application_config_t *appConfig = mico_system_context_get_user_data( in_context );

    if ( !strcmp( key, "Connect SPP Server" ) )
    {
        appConfig->remoteServerEnable = json_object_get_boolean( value );
        *need_reboot = true;
    } else if ( !strcmp( key, "SPP Server" ) )
    {
        strncpy( appConfig->remoteServerDomain, json_object_get_string( value ), 64 );
        *need_reboot = true;
    } else if ( !strcmp( key, "SPP Server Port" ) )
    {
        appConfig->remoteServerPort = json_object_get_int( value );
        *need_reboot = true;
    } else if ( !strcmp( key, "Baurdrate" ) )
    {
        appConfig->USART_BaudRate = json_object_get_int( value );
        *need_reboot = true;
    }
}

int application_start( void )
{
    app_log_trace();                                // 在程序运行的初始状态输出APP字样
    OSStatus err = kNoErr;                          // 设置OS状态
    mico_uart_config_t uart_config;                 // 串口配置
    app_context_t* app_context;                     // Application全局结构体
    mico_Context_t* mico_context;                   // MiCO操作系统全局结构体

    /* Create application context */
    // calloc一个app_context_t结构体内容
    app_context = (app_context_t *) calloc( 1, sizeof(app_context_t) );
    /**
     * #if( !defined( require_action ) )
     *     #define require_action( X, LABEL, ACTION )                                                              \
     *         do                                                                                                  \
     *         {                                                                                                   \
     *             if( unlikely( !(X) ) )                                                                          \
     *             {                                                                                               \
     *                 debug_print_assert( 0, #X, NULL, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );                 \
     *                 { ACTION; }                                                                                 \
     *                 goto LABEL;                                                                                 \
     *             }                                                                                               \
     *                                                                                                             \
     *         }   while( 1==0 )
     * #endif
     *
     * 如上require_action是一个宏，这里是判断app_context是不是NULL，因为前面calloc了内存
     */
    require_action( app_context, exit, err = kNoMemoryErr );    // 这里的exit是下面的lable，不是函数

    /* Create mico system context and read application's config data from flash */

    /**
     * // Application's configuration stores in flash
     * typedef struct
     * {
     *   uint32_t          configDataVer;               // 配置数据
     *   uint32_t          localServerPort;             // 配置本地Server Socket端口号
     *
     *   // local services
     *   bool              localServerEnable;           // 使能本地Server功能
     *   bool              remoteServerEnable;          // 使能远程Server功能
     *   char              remoteServerDomain[64];      // 远程Server域名，最长64字节
     *   int               remoteServerPort;            // 远程Server Socket端口号
     *
     *   // IO settings
     *   uint32_t          USART_BaudRate;              // 串口波特率
     * } application_config_t;
     *
     * void* mico_system_context_init( uint32_t user_config_data_size )
     * {
     *   void *user_config_data = NULL;
     *
     *   // 如果sys_context非空，那么就要重新释放所有的空间，用于再次申请
     *   if( sys_context !=  NULL) {
     *     if( sys_context->user_config_data != NULL )
     *       free( sys_context->user_config_data );
     *     free( sys_context );
     *     sys_context = NULL;
     *   }
     *
     *   // 重新申请用户配置数据结构体空间
     *   if( user_config_data_size ){
     *     user_config_data = calloc( 1, user_config_data_size );
     *     require( user_config_data, exit );
     *   }
     *
     *   // 重新申请系统全局变量结构体空间
     *   sys_context = calloc( 1, sizeof(system_context_t) );
     *   require( sys_context, exit );
     *
     *   // 在系统全局变量中保存用户配置结构体指针和结构体长度
     *   sys_context->user_config_data = user_config_data;
     *   sys_context->user_config_data_size = user_config_data_size;
     *
     *   para_log( "Init context: len=%d", sizeof(system_context_t));
     *
     *   // 初始化互斥信号
     *   mico_rtos_init_mutex( &sys_context->flashContentInRam_mutex );
     *   // 该函数需要重点阅读
     *   MICOReadConfiguration( sys_context );
     *
     * exit:
     *   return &sys_context->flashContentInRam;
     * }
     *
     * void* mico_system_context_get_user_data( mico_Context_t* const in_context )
     * {
     *     if( sys_context )
     *         return sys_context->user_config_data;
     *     else
     *         return NULL;
     * }
     */
    mico_context = mico_system_context_init( sizeof(application_config_t) );
    app_context->appConfig = mico_system_context_get_user_data( mico_context );

    /* mico system initialize */
    // 站在使用者的角度，这个函数可以不看，站在分析者的角度，可以去里面看看初始化了什么东西
    err = mico_system_init( mico_context );
    require_noerr( err, exit );

    /* Initialize service discovery */
    // - Demos/applicaiton/wifi_uart/MICOBonjour.c         Zero-configuration protocol compatiable with Bonjour from Apple
    err = MICOStartBonjourService( Station, app_context );
    require_noerr( err, exit );

    /* Protocol initialize */
    /**
     * OSStatus sppProtocolInit(app_context_t * const inContext)
     * {
     *   int i;
     *
     *   spp_log_trace();
     *   (void)inContext;
     *
         // #define MAX_QUEUE_NUM                       6  // 1 remote client, 5 local server
     *   for(i=0; i < MAX_QUEUE_NUM; i++) {
     *     inContext->appStatus.socket_out_queue[i] = NULL;
     *   }
     *   // 初始化应用状态队列互斥信号
     *   mico_rtos_init_mutex(&inContext->appStatus.queue_mtx);
     *   return kNoErr;
     * }
     */
    sppProtocolInit( app_context );

    /*UART receive thread*/
    // 串口配置
    uart_config.baud_rate = app_context->appConfig->USART_BaudRate;         // 波特率
    uart_config.data_width = DATA_WIDTH_8BIT;                               // 数据长度
    uart_config.parity = NO_PARITY;                                         // 奇偶校验
    uart_config.stop_bits = STOP_BITS_1;                                    // 停止位
    uart_config.flow_control = FLOW_CONTROL_DISABLED;                       // 数据流控制
    if ( mico_context->micoSystemConfig.mcuPowerSaveEnable == true )        // 低功耗串口唤醒模式
        uart_config.flags = UART_WAKEUP_ENABLE;
    else
        uart_config.flags = UART_WAKEUP_DISABLE;

    /**
     * OSStatus ring_buffer_init( ring_buffer_t* ring_buffer, uint8_t* buffer, uint32_t size )
     * {
     *     ring_buffer->buffer     = (uint8_t*)buffer;
     *     ring_buffer->size       = size;
     *     ring_buffer->head       = 0;
     *     ring_buffer->tail       = 0;
     *     return kNoErr;
     * }
     */
    ring_buffer_init( (ring_buffer_t *) &rx_buffer, (uint8_t *) rx_data, UART_BUFFER_LENGTH );
    /**
     * #define UART_FOR_APP     MICO_UART_2
     *
     * typedef enum
     * {
     *     MICO_UART_1,
     *     MICO_UART_2,
     *     MICO_UART_MAX, // Denotes the total number of UART port aliases. Not a valid UART alias
     *     MICO_UART_NONE,
     * } mico_uart_t;
     *
     * OSStatus MicoUartInitialize( mico_uart_t uart, const mico_uart_config_t* config, ring_buffer_t* optional_rx_buffer )
     * {
     *   // 检查UART口是否正确
     *   if ( uart >= MICO_UART_NONE )
     *     return kUnsupportedErr;
     *
     * #ifndef MICO_DISABLE_STDIO
     *   // Interface is used by STDIO. Uncomment MICO_DISABLE_STDIO to overcome this
     *   if ( uart == STDIO_UART )
     *   {
     *     return kGeneralErr;
     *   }
     * #endif
     *
     *   return (OSStatus) platform_uart_init( &platform_uart_drivers[uart], &platform_uart_peripherals[uart], config, optional_rx_buffer );
     * }
     *
     * const platform_uart_t platform_uart_peripherals[] =
     * {
     *   [MICO_UART_1] =
     *   {
     *     .port                         = USART6,
     *     .pin_tx                       = &platform_gpio_pins[MICO_GPIO_8],
     *     .pin_rx                       = &platform_gpio_pins[MICO_GPIO_12],
     *     .pin_cts                      = NULL,
     *     .pin_rts                      = NULL,
     *     .tx_dma_config =
     *     {
     *       .controller                 = DMA2,
     *       .stream                     = DMA2_Stream6,
     *       .channel                    = DMA_Channel_5,
     *       .irq_vector                 = DMA2_Stream6_IRQn,
     *       .complete_flags             = DMA_HISR_TCIF6,
     *       .error_flags                = ( DMA_HISR_TEIF6 | DMA_HISR_FEIF6 ),
     *     },
     *     .rx_dma_config =
     *     {
     *       .controller                 = DMA2,
     *       .stream                     = DMA2_Stream1,
     *       .channel                    = DMA_Channel_5,
     *       .irq_vector                 = DMA2_Stream1_IRQn,
     *       .complete_flags             = DMA_LISR_TCIF1,
     *       .error_flags                = ( DMA_LISR_TEIF1 | DMA_LISR_FEIF1 | DMA_LISR_DMEIF1 ),
     *     },
     *   },
     *   [MICO_UART_2] =
     *   {
     *     .port                         = USART1,
     *     .pin_tx                       = &platform_gpio_pins[MICO_GPIO_30],
     *     .pin_rx                       = &platform_gpio_pins[MICO_GPIO_29],
     *     .pin_cts                      = &platform_gpio_pins[MICO_GPIO_35],
     *     .pin_rts                      = &platform_gpio_pins[MICO_GPIO_34],
     *     .tx_dma_config =
     *     {
     *       .controller                 = DMA2,
     *       .stream                     = DMA2_Stream7,
     *       .channel                    = DMA_Channel_4,
     *       .irq_vector                 = DMA2_Stream7_IRQn,
     *       .complete_flags             = DMA_HISR_TCIF7,
     *       .error_flags                = ( DMA_HISR_TEIF7 | DMA_HISR_FEIF7 ),
     *     },
     *     .rx_dma_config =
     *     {
     *       .controller                 = DMA2,
     *       .stream                     = DMA2_Stream2,
     *       .channel                    = DMA_Channel_4,
     *       .irq_vector                 = DMA2_Stream2_IRQn,
     *       .complete_flags             = DMA_LISR_TCIF2,
     *       .error_flags                = ( DMA_LISR_TEIF2 | DMA_LISR_FEIF2 | DMA_LISR_DMEIF2 ),
     *     },
     *   },
     * };
     *
     * typedef struct
     * {
     *     platform_uart_t*           peripheral;
     *     ring_buffer_t*             rx_buffer;
     *     mico_semaphore_t           rx_complete;
     *     mico_semaphore_t           tx_complete;
     *     mico_mutex_t               tx_mutex;
     *     mico_semaphore_t           sem_wakeup;
     *     volatile uint32_t          tx_size;
     *     volatile uint32_t          rx_size;
     *     volatile OSStatus          last_receive_result;
     *     volatile OSStatus          last_transmit_result;
     *     volatile bool              initialized;
     * } platform_uart_driver_t;
     * platform_uart_driver_t platform_uart_drivers[MICO_UART_MAX];
     *
     * 这里主要是初始化UART线程，用于接收数据
     */
    MicoUartInitialize( UART_FOR_APP, &uart_config, (ring_buffer_t *) &rx_buffer );
    err = mico_rtos_create_thread( NULL, MICO_APPLICATION_PRIORITY, "UART Recv", uartRecv_thread,
                                   STACK_SIZE_UART_RECV_THREAD, (mico_thread_arg_t)app_context );
    require_noerr_action( err, exit, app_log("ERROR: Unable to start the uart recv thread.") );

    /* Local TCP server thread */
    if ( app_context->appConfig->localServerEnable == true )
    {
        err = mico_rtos_create_thread( NULL, MICO_APPLICATION_PRIORITY, "Local Server", localTcpServer_thread,
                                       STACK_SIZE_LOCAL_TCP_SERVER_THREAD, (mico_thread_arg_t)app_context );
        require_noerr_action( err, exit, app_log("ERROR: Unable to start the local server thread.") );
    }

    /* Remote TCP client thread */
    if ( app_context->appConfig->remoteServerEnable == true )
    {
        err = mico_rtos_create_thread( NULL, MICO_APPLICATION_PRIORITY, "Remote Client", remoteTcpClient_thread,
                                       STACK_SIZE_REMOTE_TCP_CLIENT_THREAD, (mico_thread_arg_t)app_context );
        require_noerr_action( err, exit, app_log("ERROR: Unable to start the remote client thread.") );
    }

exit:
    mico_rtos_delete_thread( NULL );
    return err;
}
