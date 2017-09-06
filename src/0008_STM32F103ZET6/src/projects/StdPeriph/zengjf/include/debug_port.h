#ifndef __DEBUG_PORT_H
#define	__DEBUG_PORT_H

#include "stm32f10x.h"
#include <stdio.h>

void USART1_Config(void);
int fputc(int ch, FILE *f);


#endif /* __DEBUG_PORT_H */
