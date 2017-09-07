#include <stdio.h>
#include <string.h>

extern int stdout_init(void);
extern int stdin_init(void);
extern int stderr_init(void);

#define array(x) sizeof(x)/sizeof(x[0])

char str[512] = {0};

int main()
{
    stdout_init();
    stdin_init();
    stderr_init();
    
    printf("\r\n Hardware Auto Detect System.");
    printf("\r\n Version: 0.0.1");
    printf("\r\n           ---- Designed By zengjf \r\n");
    
    while(1) 
    {
        scanf("%s", str);
        printf("str[%d] = %s\r\n", array(str), str);
        memset(str, 0, array(str));
    }
}
