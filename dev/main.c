#include <stdio.h>
#include <string.h>
#include "lwbtn/lwbtn.h"
#include "windows.h"

static LARGE_INTEGER freq, sys_start_time;
static uint32_t get_tick(void);

/* User defined settings */
const int keys[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

/* List of buttons to process */
static lwbtn_btn_t btns[] = {{.arg = (void*)&keys[0]}, {.arg = (void*)&keys[1]}, {.arg = (void*)&keys[2]},
                             {.arg = (void*)&keys[3]}, {.arg = (void*)&keys[4]}, {.arg = (void*)&keys[5]},
                             {.arg = (void*)&keys[6]}, {.arg = (void*)&keys[7]}, {.arg = (void*)&keys[8]},
                             {.arg = (void*)&keys[9]}};

uint8_t
prv_btn_get_state(struct lwbtn* lw, struct lwbtn_btn* btn) {
    return GetAsyncKeyState(*(int*)btn->arg) < 0;
}

void
prv_btn_event(struct lwbtn* lw, struct lwbtn_btn* btn, lwbtn_evt_t evt) {
    (void)lw;
    printf("[%7u] State !!!. CH: %c, evt: ", (unsigned)get_tick(), *(int*)btn->arg);
    switch (evt) {
        case LWBTN_EVT_KEEPALIVE: {
            printf("KEEPALIVE");
            break;
        }
        case LWBTN_EVT_ONPRESS: {
            printf("ONPRESS");
            break;
        }
        case LWBTN_EVT_ONRELEASE: {
            printf("ONRELEASE");
            break;
        }
        case LWBTN_EVT_ONCLICK: {
            printf("ONCLICK");
            break;
        }
        default: {
            break;
        }
    }
    printf("\r\n");
}

int
main(void) {
    printf("Application running\r\n");
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&sys_start_time);

    lwbtn_init_ex(NULL, btns, sizeof(btns) / sizeof(btns[0]), prv_btn_get_state, prv_btn_event);

    while (1) {
        lwbtn_process_ex(NULL, get_tick());
        Sleep(10);
    }
}

static uint32_t
get_tick(void) {
    LONGLONG ret;
    LARGE_INTEGER now;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&now);
    ret = now.QuadPart - sys_start_time.QuadPart;
    return (uint32_t)((ret * 1000) / freq.QuadPart);
}
