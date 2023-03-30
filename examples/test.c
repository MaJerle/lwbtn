#include "lwbtn/lwbtn.h"
#include "windows.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * \brief           Input state information
 */
typedef struct {
    uint8_t state;     /*!< Input state -> 1 = active, 0 = inactive */
    uint32_t duration; /*!< Time until this state is enabled */
} btn_test_time_t;

/**
 * \brief           Event sequence
 */
typedef struct {
    lwbtn_evt_t evt;       /*!< Event type */
    uint8_t keepalive_cnt; /*!< Number of keep alive events while button is active */
    uint8_t conseq_clicks; /*!< Number of consecutive clicks detected */
} btn_test_evt_t;

/* List of used buttons -> test case */
static lwbtn_btn_t btns[1];
static volatile uint32_t time_current;

/* Set button state -> used for test purposes */
#define BTN_STATE(_state_, _duration_)                                                                                 \
    { .state = (_state_), .duration = (_duration_) }

/* On-Press event */
#define BTN_EVENT_ONPRESS()                                                                                            \
    { .evt = LWBTN_EVT_ONPRESS }
/* On-Release event */
#define BTN_EVENT_ONRELEASE()                                                                                          \
    { .evt = LWBTN_EVT_ONRELEASE }
/* On-Click event */
#define BTN_EVENT_ONCLICK(_conseq_clicks_)                                                                             \
    { .evt = LWBTN_EVT_ONCLICK, .conseq_clicks = (_conseq_clicks_) }
/* On-Click event */
#define BTN_EVENT_KEEPALIVE(_keepalive_cnt_)                                                                           \
    { .evt = LWBTN_EVT_KEEPALIVE, .keepalive_cnt = (_keepalive_cnt_) }

#define TEST1 0
#define TEST2 0
#define TEST3 0
#define TEST4 1

/*
 * Simulate click event
 */
static const btn_test_time_t test_sequence[] = {
#if TEST1
    /* 
     * Test 1:
     *
     * Step 1:
     * Go to active state and stay there for a period
     * of minimum debounce time and minimum
     * time input must be active to later detect click event
     * 
     * Step 2:
     * Go low and stay inactive until time to report click has elapsed.
     * Include debounce timing for release event.
     * 
     * Add +1 to the end, to force click event,
     * and not to go to "consecutive clicks" if any further tests are added in this sequence
     */
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MULTI_MAX + 1),
#endif

#if TEST2
    /*
     * Test 2:
     *
     * Repeat above steps, this time w/o +1 at the end.
     *
     * Simulate "2" consecutive clicks and report final "click" event at the end of the sequence,
     * with "2" consecutive clicks in the report info
     */
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MAX),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MULTI_MAX + 1),
#endif

#if TEST3
    /*
     * Test 3:
     *
     * Triple click with direct report
     */
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MAX),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MAX),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MAX),
#endif

#if TEST4
    /*
     * Test 4:
     *
     * Repeat above steps, this time w/o +1 at the end.
     *
     * Simulate "2" consecutive clicks and report final "click" event at the end of the sequence,
     * with "2" consecutive clicks in the report info
     */
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MULTI_MAX - 1),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MULTI_MAX + 1),
#endif
};

/* List of expected test events during different input states and their timings */
static const btn_test_evt_t test_events[] = {
#if TEST1
    /* Test 1 */
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONCLICK(1),
#endif

#if TEST2
    /* Test 2 */
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONCLICK(2),
#endif

#if TEST3
    /* Test 3 */
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONCLICK(3),
#endif

#if TEST4
    /* Test 4 */
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONCLICK(2),
#endif
};

/* Get button state for given current time */
static uint8_t
prv_get_state_for_time(uint32_t time) {
    uint8_t state = 0;
    uint32_t duration = 0;

    for (size_t i = 0; i < sizeof(test_sequence) / sizeof(test_sequence[0]); ++i) {
        duration += test_sequence[i].duration; /* Advance time */
        if (time <= duration) {
            state = test_sequence[i].state;
            break;
        }
    }
    return state;
}

/* Get button state */
static uint8_t
prv_btn_get_state(struct lwbtn* lw, struct lwbtn_btn* btn) {
    (void)btn;
    (void)lw;
    return prv_get_state_for_time(time_current);
}

/* Process button event */
static void
prv_btn_event(struct lwbtn* lw, struct lwbtn_btn* btn, lwbtn_evt_t evt) {
    const char* s;
    uint32_t color, keepalive_cnt = 0, diff_time;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    static uint32_t time_prev;
    static uint32_t array_index = 0;
    const btn_test_evt_t* test_evt_data;

    /* Test errors variable */
    uint32_t test_errors = 0;
    if (array_index >= sizeof(test_events) / sizeof(test_events[0])) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("[%7u] ERROR! Array index is out of bounds!\r\n", (unsigned)time_current);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return;
    }
    test_evt_data = &test_events[array_index];

    /* Handle timing */
    diff_time = time_current - time_prev;
    time_prev = time_current;
#if LWBTN_CFG_USE_KEEPALIVE
    keepalive_cnt = btn->keepalive.cnt;
#endif

    /* Get event string */
    if (0) {
#if LWBTN_CFG_USE_KEEPALIVE
    } else if (evt == LWBTN_EVT_KEEPALIVE) {
        s = "KEEPALIVE";
        color = FOREGROUND_RED;

        test_errors += test_evt_data->keepalive_cnt != keepalive_cnt;
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

        test_errors += test_evt_data->conseq_clicks != btn->click.cnt;
    } else {
        s = "  UNKNOWN";
        color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
    SetConsoleTextAttribute(hConsole, color);
    printf("[%7u][%6u] evt: %s, keep-alive cnt: %3u, click cnt: %3u\r\n", (unsigned)time_current, (unsigned)diff_time,
           s, (unsigned)keepalive_cnt, (unsigned)btn->click.cnt);
    if (test_errors > 0) {
        printf("TEST FAILED...\r\n");
    }
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    (void)lw;

    ++array_index; /* Go to next step in next event */
}

/**
 * \brief           Test function
 */
int
test_win32(void) {
    printf("Test running\r\n");

    /* Define buttons */
    lwbtn_init_ex(NULL, btns, sizeof(btns) / sizeof(btns[0]), prv_btn_get_state, prv_btn_event);

    /* Counter simulates ms tick */
    for (size_t i = 0; i < 3000; ++i) {
        time_current = i; /* Set current time used in callback */
        lwbtn_process(i); /* Now run processing */
    }
    return 0;
}
