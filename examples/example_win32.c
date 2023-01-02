#include "lwbtn/lwbtn.h"
#include "windows.h"
#include <stdio.h>
#include <stdlib.h>

static LARGE_INTEGER freq, sys_start_time;
static uint32_t get_tick(void);

/* User defined settings */
const int keys[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
uint32_t last_time_keys[sizeof(keys) / sizeof(keys[0])] = {0};

/* List of buttons to process with assigned custom arguments for callback functions */
static lwbtn_btn_t btns[] = {
    {.arg = (void*)&keys[0]}, {.arg = (void*)&keys[1]}, {.arg = (void*)&keys[2]}, {.arg = (void*)&keys[3]},
    {.arg = (void*)&keys[4]}, {.arg = (void*)&keys[5]}, {.arg = (void*)&keys[6]}, {.arg = (void*)&keys[7]},
    {.arg = (void*)&keys[8]}, {.arg = (void*)&keys[9]},
};

/**
 * \brief           Get input state callback 
 * \param           lw: LwBTN instance
 * \param           btn: Button instance
 * \return          `1` if button active, `0` otherwise
 */
uint8_t
prv_btn_get_state(struct lwbtn* lw, struct lwbtn_btn* btn) {
    (void)lw;

    /*
     * Function will return negative number if button is pressed,
     * or zero if button is releases
     */
    return GetAsyncKeyState(*(int*)btn->arg) < 0;
}

/**
 * \brief           Button event
 * 
 * \param           lw: LwBTN instance
 * \param           btn: Button instance
 * \param           evt: Button event
 */
void
prv_btn_event(struct lwbtn* lw, struct lwbtn_btn* btn, lwbtn_evt_t evt) {
    const char* s;
    uint32_t color, keepalive_cnt = 0;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    uint32_t* diff_time_ptr = &last_time_keys[(*(int*)btn->arg) - '0'];
    uint32_t diff_time = get_tick() - *diff_time_ptr;

    /* This is for purpose of test and timing validation */
    if (diff_time > 2000) {
        diff_time = 0;
    }
    *diff_time_ptr = get_tick(); /* Set current date as last one */

    /* Get event string */
    if (0) {
#if LWBTN_CFG_USE_KEEPALIVE
    } else if (evt == LWBTN_EVT_KEEPALIVE) {
        s = "KEEPALIVE";
        color = FOREGROUND_RED;
#endif /* LWBTN_CFG_USE_KEEPALIVE */
    } else if (evt == LWBTN_EVT_ONPRESS) {
        s = "  ONPRESS";
        color = FOREGROUND_GREEN;
    } else if (evt == LWBTN_EVT_ONRELEASE) {
        s = "ONRELEASE";
        color = FOREGROUND_BLUE;
    } else if (evt == LWBTN_EVT_ONCLICK) {
        s = "  ONCLICK";
        color = FOREGROUND_RED | FOREGROUND_GREEN;
    } else {
        s = "  UNKNOWN";
        color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
#if LWBTN_CFG_USE_KEEPALIVE
    keepalive_cnt = btn->keepalive.cnt;
#endif
    SetConsoleTextAttribute(hConsole, color);
    printf("[%7u][%6u] CH: %c, evt: %s, keep-alive cnt: %3u, click cnt: %3u\r\n", (unsigned)get_tick(),
           (unsigned)diff_time, *(int*)btn->arg, s, (unsigned)keepalive_cnt, (unsigned)btn->click.cnt);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    (void)lw;
}

/**
 * \brief           Example function
 */
int
example_win32(void) {
    uint32_t time_last;
    printf("Application running\r\n");
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&sys_start_time);

    /* Define buttons */
    lwbtn_init_ex(NULL, btns, sizeof(btns) / sizeof(btns[0]), prv_btn_get_state, prv_btn_event);

    time_last = get_tick();
    while (1) {
        /* Process forever */
        lwbtn_process_ex(NULL, get_tick());

        /* Manually read button state */
#if LWBTN_CFG_GET_STATE_MODE == LWBTN_GET_STATE_MODE_MANUAL
        for (size_t i = 0; i < sizeof(btns) / sizeof(btns[0]); ++i) {
            lwbtn_set_btn_state(&btns[i], prv_btn_get_state(NULL, &btns[i]));
        }
#endif /* LWBTN_CFG_GET_STATE_MODE == LWBTN_GET_STATE_MODE_MANUAL */

        /* Check if specific button is active and do some action */
        if (lwbtn_is_btn_active(&btns[0])) {
            if ((get_tick() - time_last) > 200) {
                time_last = get_tick();
                printf("Button is active\r\n");
            }
        }

        /* Artificial sleep to offload win process */
        Sleep(5);
    }
    return 0;
}

/**
 * \brief           Get current tick in ms from start of program
 * \return          uint32_t: Tick in ms
 */
static uint32_t
get_tick(void) {
    LONGLONG ret;
    LARGE_INTEGER now;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&now);
    ret = now.QuadPart - sys_start_time.QuadPart;
    return (uint32_t)((ret * 1000) / freq.QuadPart);
}
