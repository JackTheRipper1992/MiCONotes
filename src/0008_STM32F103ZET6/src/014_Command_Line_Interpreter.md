# Command Line Interpreter For STM32

## 参考资料

* [MicroCLI: A command-line interface (CLI) for the STM32F0-Discovery](https://github.com/christianjann/microcli-stm32f0-discovery)
* [MicroCLI: Befehlsinterpreter auf einem STM32F0-Discovery + WLAN + Handy-App](https://www.jann.cc/2012/08/13/microcli_befehlsinterpreter_auf_einem_stm32f0_discovery.html)
* [libemb](https://github.com/wendlers/libemb)

## 获取需要解析的字符串

* 需求：
  * 获取串口输入的字符串；
  * 能够支持backspace/delete按键进行删除输入错误的字符；
  * string的buffer最大512字节；
* Example Code
  ```
  #include "zengjf.h"
  
  // command
  #define CMD_BUF_SIZE 512
  char cmd_buf[CMD_BUF_SIZE] = {0};
  int cmd_buf_count = 0;
  
  void get_cmd_parser_char(void) 
  {
      char ch = getchar();
      
      if (ch == 127 || ch == 8) {
          --cmd_buf_count;
          
          if (cmd_buf_count < 0)
              cmd_buf_count = 0;
  
          if (cmd_buf_count > CMD_BUF_SIZE - 1)
              cmd_buf_count = CMD_BUF_SIZE - 1;
          
          cmd_buf[cmd_buf_count] = 0;
          printf("\033[1K\rAplexOS # %s", cmd_buf);
  
          return;
      } 
      else if (ch == '\r')
      {
          if (strlen(cmd_buf) == 0) 
              printf("AplexOS # ");
          else 
              printf("%s\r\nAplexOS # ", cmd_buf);
          
          memset(cmd_buf, 0, strlen(cmd_buf));
          cmd_buf_count = 0;
          
          return;
      }
      
      cmd_buf[cmd_buf_count++] = ch;
  }
  ```
* 输出显示  
  ![../img/Cmd_get_string_from_console.png](../img/Cmd_get_string_from_console.png)

## 命令解析

* 定义数据结构：
  ```
  #ifndef __CMD_PARSER_H
  #define	__CMD_PARSER_H
  
  #include "zengjf.h"
  
  #define countof(a)   (sizeof(a) / sizeof(*(a)))
  
  /**
   * Definition of a single element
   */
  typedef struct
  {
      /**
       * Name of the element
       */
      const char     *cmd;
      
      /**
       * Name of the element
       */
      const char     *desc;
  
      /**
       * Value of the elment
       */
      int (*func)(char *args);
  
  } element_t;
  
  /**
   * All elements of the list
   */
  typedef struct
  {
      /**
       * Number of elements
       */
      short   count;
  
      /**
       * The elements
       */
      element_t   cmds[];
  } shell_cmds;
  
  extern char *welcome_msg;
  extern shell_cmds microcli_shell_cmds;
  
  int shell_process(char *cmd_line);
  
  void get_cmd_parser_char(void);
  
  #endif /* __CMD_PARSER_H */
  ```
* 命令解析、函数调用：
  ```
  char* welcome_msg =
      "\r\n\r\nHardware Auto Detect System v0.0.1 (" __DATE__ ")\r\n"
      "\r\n               ---- Designed By AplexOS Team \r\n\r\n";
  
  int shell_cmd_list(char *args);
  
  shell_cmds microcli_shell_cmds =
  {
      .count = 1,
      .cmds  = {
          {
              .cmd     = "list",
              .desc    = "List available",
              .func    = shell_cmd_list,
          },
      },
  };
  
  int shell_cmd_list(char *args) 
  {
      printf("%s", welcome_msg);
      return 0;
  }
  
  int shell_process(char *cmd_line) 
  {
      int i = 0;
      for (i = 0; i < microcli_shell_cmds.count; i++) 
      {
          if (strncmp(microcli_shell_cmds.cmds[i].cmd, cmd_line, strlen(microcli_shell_cmds.cmds[i].cmd)) == 0) 
          {
              if (cmd_line[strlen(microcli_shell_cmds.cmds[i].cmd)] == ' ') 
              {
                  microcli_shell_cmds.cmds[i].func(&cmd_line[strlen(microcli_shell_cmds.cmds[i].cmd) + 1]);
              } else if(strlen(cmd_line) == strlen(microcli_shell_cmds.cmds[i].cmd)) {
                  microcli_shell_cmds.cmds[i].func(NULL);
              }
              
              return 0;
          }
      }
      
      printf("unsupport command.");
      return -1;
  }
  ```
* 测试输出信息：
  ![../img/Cmd_Parser_With_Args.png](../img/Cmd_Parser_With_Args.png)
