#include <stdio.h>
#include "lwbtn/lwbtn.h"

extern int test_run(void);

int
main(void) {
    int ret = 0;
    printf("Application running\r\n");
    ret = test_run();
    printf("Done\r\n");
    return ret;
}