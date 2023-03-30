#include <stdio.h>
#include <string.h>
#include "lwbtn/lwbtn.h"
#include "windows.h"

extern int example_win32(void);
extern int test_win32(void);

int
main(void) {
    test_win32();
    //example_win32();
    return 0;
}
