/**
 ******************************************************************************
 * @file    LocalTcpServer.c
 * @author  William Xu
 * @version V1.0.0
 * @date    05-May-2014
 * @brief   This file create a TCP listener thread, accept every TCP client
 *          connection and create thread for them.
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
#include "SppProtocol.h"
#include "SocketUtils.h"
#include "MICOAppDefine.h"

#define server_log(M, ...) custom_log("TCP SERVER", M, ##__VA_ARGS__)
#define server_log_trace() custom_log_trace("TCP SERVER")

static void localTcpClient_thread(uint32_t inFd);
static app_context_t *context;

mico_thread_t   localTcpClient_thread_handler;

void localTcpServer_thread(uint32_t inContext)
{
  // 输出log日志
  server_log_trace();
  OSStatus err = kUnknownErr;
  int j;
  context = (app_context_t *)inContext;
  struct sockaddr_in addr;
  int sockaddr_t_size;
  fd_set readfds;
  char ip_address[16];
  
  int localTcpListener_fd = -1;

  /*Establish a TCP server fd that accept the tcp clients connections*/ 
  localTcpListener_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );                    // 获取socket fd
  require_action(IsValidSocket( localTcpListener_fd ), exit, err = kNoResourcesErr );
  addr.sin_family = AF_INET;                                                            // 协议类型
  addr.sin_addr.s_addr = INADDR_ANY;                                                    // 接收任何IP地址
  addr.sin_port = htons(context->appConfig->localServerPort);                           // 设置端口号
  err = bind(localTcpListener_fd, (struct sockaddr *)&addr, sizeof(addr));              // 绑定地址设置信息数据
  require_noerr( err, exit );

  err = listen(localTcpListener_fd, 0);                                                 // 开启监听
  require_noerr( err, exit );

  // 输出一些日志信息
  server_log("Server established at port: %ld, fd: %d", context->appConfig->localServerPort, localTcpListener_fd);
  
  while(1){
    // 使用select来监视文件描述符是否有有效数据
    FD_ZERO(&readfds);
    FD_SET(localTcpListener_fd, &readfds);  
    select(localTcpListener_fd + 1, &readfds, NULL, NULL, NULL);

    /*Check tcp connection requests */
    if(FD_ISSET(localTcpListener_fd, &readfds)){
      sockaddr_t_size = sizeof(struct sockaddr_in);
      j = accept(localTcpListener_fd, (struct sockaddr *)&addr, (socklen_t *)&sockaddr_t_size); // 等待连接
	  if (IsValidFD(j)) {
        strcpy( ip_address, inet_ntoa(addr.sin_addr) );
        server_log("Client %s:%d connected, fd: %d", ip_address, addr.sin_port, j);
        if(kNoErr != mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "Local Clients", localTcpClient_thread, STACK_SIZE_LOCAL_TCP_CLIENT_THREAD, (mico_thread_arg_t)&j) ) 
          SocketClose(&j);
      }
    }
   }

exit:
    server_log("Exit: Local controller exit with err = %d", err);
    mico_rtos_delete_thread(NULL);
    return;
}

void localTcpClient_thread(uint32_t inFd)
{
  OSStatus err;
  int clientFd = *(int *)inFd;          // 通过TCP协议连接进来生成的fd
  uint8_t *inDataBuffer = NULL;
  int len;
  fd_set readfds;
  fd_set writeSet;
  struct timeval t;
  int eventFd = -1;
  mico_queue_t queue;
  socket_msg_t *msg;
  int sent_len, errno;

  // #define wlanBufferLen                       1024
  inDataBuffer = malloc(wlanBufferLen);
  require_action(inDataBuffer, exit, err = kNoMemoryErr);

  // 创建、初始化用于数据处理的消息队列，这个消息队列中的数据主要是串口传递过来的数据
  err = socket_queue_create(context, &queue);
  require_noerr( err, exit );
  eventFd = mico_create_event_fd(queue);
  if (eventFd < 0) {
    server_log("create event fd error");
    goto exit_with_queue;
  } 

  t.tv_sec = 4;
  t.tv_usec = 0;
  
  while(1){

    FD_ZERO(&readfds);
    FD_SET(clientFd, &readfds); 
    FD_SET(eventFd, &readfds);      // 这是与串口通信的消息队列生成的fd

    // 等待串口有数据发送过来了，可读
    select( Max(clientFd, eventFd) + 1, &readfds, NULL, NULL, &t);
    /* send UART data */
    if (FD_ISSET( eventFd, &readfds )) { // have data and can write
        FD_ZERO(&writeSet );
        FD_SET(clientFd, &writeSet );
        t.tv_usec = 100*1000; // max wait 100ms.
        // 等待网络可写，因为网络有时候可能处于忙状态，这样可以保证网络发送数据成功率
        select(clientFd + 1, NULL, &writeSet, NULL, &t);
        if((FD_ISSET( clientFd, &writeSet )) &&
            (kNoErr == mico_rtos_pop_from_queue( &queue, &msg, 0))) {
           sent_len = write(clientFd, msg->data, msg->len);
           if (sent_len <= 0) {
              len = sizeof(errno);
              getsockopt(clientFd, SOL_SOCKET, SO_ERROR, &errno, (socklen_t *)&len);
              socket_msg_free(msg);
              server_log("write error, fd: %d, errno %d", clientFd, errno );
              if (errno != ENOMEM) {
                  goto exit_with_queue;
              }
           } else {
                  socket_msg_free(msg);
              }
           }
        }

    /*Read data from tcp clients and process these data using HA protocol */ 
    // 前面处理了串口读的数据，这里处理网络读的数据
    if (FD_ISSET(clientFd, &readfds)) {
      len = recv(clientFd, inDataBuffer, wlanBufferLen, 0);
      require_action_quiet(len>0, exit_with_queue, err = kConnectionErr);
      sppWlanCommandProcess(inDataBuffer, &len, clientFd, context);
    }
  }

exit_with_queue:
    len = sizeof(errno);
    getsockopt(clientFd, SOL_SOCKET, SO_ERROR, &errno, (socklen_t *)&len);
    server_log("Exit: Client exit with err = %d, socket errno %d", err, errno);
    if (eventFd >= 0) {
        mico_delete_event_fd(eventFd);
    }
    socket_queue_delete(context, &queue);
exit:
    SocketClose(&clientFd);
    if(inDataBuffer) free(inDataBuffer);
    mico_rtos_delete_thread(NULL);
    return;
}

