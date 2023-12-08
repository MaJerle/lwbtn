#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#if defined(WIN32)
#include "windows.h"
#endif /* defined(WIN32) */
#if defined(STM32)
#include "main.h"
#include "stm32l0xx_hal.h"
#endif /* defined(STM32) */

/**
 * \brief           Input state information
 */
typedef struct {
    uint8_t state;     /*!< Input state -> 1 = active, 0 = inactive */
    uint32_t duration; /*!< Time until this state is enabled */
    const char* text;  /*!< Text to print. If non-NULL, text is printed and the rest is ignored */
    uint8_t text_printed;
} btn_test_time_t;

/**
 * \brief           Event sequence
 */
typedef struct {
    lwbtn_evt_t evt;       /*!< Event type */
    uint8_t keepalive_cnt; /*!< Number of keep alive events while button is active */
    uint8_t conseq_clicks; /*!< Number of consecutive clicks detected */
} btn_test_evt_t;

/* Max number of ms to demonstrate */
#define MAX_TIME_MS 0xFFFF

/* Tests to run */
#define TEST1       1
#define TEST2       1
#define TEST3       1
#define TEST4       1
#define TEST5       1
#define TEST6       1
#define TEST7       1

/* List of used buttons -> test case */
static lwbtn_btn_t btns[1];
static volatile uint32_t time_current;
static volatile uint8_t is_pressed = 0, is_click = 0;

/* Set button state -> used for test purposes */
#define BTN_STATE(_state_, _duration_)                                                                                 \
    { .state = (_state_), .duration = (_duration_) }

#define BTN_PRINT(text_to_print)                                                                                       \
    { .text = (text_to_print) }

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

/*
 * Simulate click event
 */
static btn_test_time_t test_sequence[] = {
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
    BTN_PRINT("Test 1"),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MULTI_MAX + 1),
    BTN_STATE(0, 100), /* Keep low before next test */
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
    BTN_PRINT("Test 2"),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MAX),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MULTI_MAX + 1),
    BTN_STATE(0, 100), /* Keep low before next test */
#endif

#if TEST3
    /*
     * Test 3:
     *
     * Triple click with direct report
     */
    BTN_PRINT("Test 3"),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MAX),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MAX),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MAX),
    BTN_STATE(0, 100), /* Keep low before next test */
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
    BTN_PRINT("Test 4"),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    /* Hold button in release state for time that is max for 2 clicks - time that we will
        indicate in the next press state -> this is the frequency between detected events */
    BTN_STATE(0, LWBTN_CFG_TIME_CLICK_MULTI_MAX
                     /* Decrease by active time in next step */
                     - (LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN) - 1),
    /* Active time */
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MULTI_MAX + 1),
    BTN_STATE(0, 100), /* Keep low before next test */
#endif

#if TEST5
    /*
     * Test 5:
     *
     * This test shows how to handle case when 2 clicks are being executed,
     * but time between 2 release events is larger than maximum
     * allowed time for consecutive clicks.
     * 
     * In this case, 2 onclick events are sent,
     * both with consecutive clicks counter set to 1
     */
    BTN_PRINT("Test 5"),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MULTI_MAX
                     - (LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN)),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MULTI_MAX + 1),
    BTN_STATE(0, 100), /* Keep low before next test */
#endif

#if TEST6
    /*
     * Test 6:
     *
     * Make a click event, followed by the longer press.
     * Simulate "long press" w/ previous click, that has click counter set to 1
     */
    BTN_PRINT("Test 6"),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MAX),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN + 1000),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MAX),
#endif

#if TEST7
    /* This one will pass only if LWBTN_CFG_CLICK_CONSECUTIVE_KEEP_AFTER_SHORT_PRESS is enabled */
    /* Make 2 clicks, and 3rd one with short press (shorter than minimum required) */
    BTN_PRINT("Test 7"),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MAX),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE + LWBTN_CFG_TIME_CLICK_MAX),
    BTN_STATE(1, LWBTN_CFG_TIME_DEBOUNCE_PRESS + LWBTN_CFG_TIME_CLICK_MIN - 2),
    BTN_STATE(0, LWBTN_CFG_TIME_DEBOUNCE_RELEASE),
#endif
};

/* List of expected test events during different input states and their timings */
static const btn_test_evt_t test_events[] = {
#if TEST1
    /* Test 1 */
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONCLICK(1),
#endif /* TEST1 */

#if TEST2
    /* Test 2 */
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONCLICK(2),
#endif /* TEST2 */

#if TEST3
    /* Test 3 */
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONCLICK(3),
#endif /* TEST3 */

#if TEST4
    /* Test 4 */
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONCLICK(2),
#endif /* TEST4 */

#if TEST5
    /* Test 5 */
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    /* This one is to handle click for first sequence */
    BTN_EVENT_ONCLICK(1),
    /* This one is to handle click for second sequence */
    BTN_EVENT_ONCLICK(1),
#endif /* TEST5 */

#if TEST6
    /* Test 6 */
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_KEEPALIVE(1),
    BTN_EVENT_KEEPALIVE(2),
    BTN_EVENT_KEEPALIVE(3),
    BTN_EVENT_KEEPALIVE(4),
    BTN_EVENT_KEEPALIVE(5),
    BTN_EVENT_KEEPALIVE(6),
    BTN_EVENT_KEEPALIVE(7),
    BTN_EVENT_KEEPALIVE(8),
    BTN_EVENT_KEEPALIVE(9),
    BTN_EVENT_KEEPALIVE(10),
    BTN_EVENT_ONRELEASE(),
#endif /* TEST6 */

#if TEST7
    /* Test 7 */
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
    /* This one is short... */
    BTN_EVENT_ONPRESS(),
    BTN_EVENT_ONRELEASE(),
#if LWBTN_CFG_CLICK_CONSECUTIVE_KEEP_AFTER_SHORT_PRESS
    BTN_EVENT_ONCLICK(2),
#endif /* LWBTN_CFG_CLICK_CONSECUTIVE_KEEP_AFTER_SHORT_PRESS */
#endif /* TEST7 */
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
            if (i > 0 && test_sequence[i - 1].text != NULL && !test_sequence[i - 1].text_printed) {
                printf("Text: %s\r\n", test_sequence[i - 1].text);
                test_sequence[i - 1].text_printed = 1;
            }
            break;
        }
    }
    return state;
}

/* Get button state */
static uint8_t
prv_btn_get_state(struct lwbtn* lw, struct lwbtn_btn* btn) {
    uint8_t state = prv_get_state_for_time(time_current);

#if defined(STM32)
    HAL_GPIO_WritePin(BTN_GPIO_Port, BTN_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif /* defined(STM32) */

    (void)btn;
    (void)lw;
    return state;
}

/* Process button event */
static void
prv_btn_event(struct lwbtn* lw, struct lwbtn_btn* btn, lwbtn_evt_t evt) {
#if defined(WIN32)
    const char* s;
    uint32_t color, keepalive_cnt = 0, diff_time;
    static uint32_t time_prev;
    static uint32_t array_index = 0;
    const btn_test_evt_t* test_evt_data = NULL;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    /* Test errors variable */
    uint32_t test_errors = 0;
    if (array_index >= sizeof(test_events) / sizeof(test_events[0])) {
#if defined(WIN32)
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("[%7u] ERROR! Array index is out of bounds!\r\n", (unsigned)time_current);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif /* defined(WIN32) */
    } else {
        test_evt_data = &test_events[array_index];
    }

    /* Handle timing */
    diff_time = time_current - time_prev;
    time_prev = time_current;
#if LWBTN_CFG_USE_KEEPALIVE
    keepalive_cnt = btn->keepalive.cnt;
#endif

    /* Event type must match */
    test_errors += test_evt_data == NULL || test_evt_data->evt != evt;

    /* Get event string */
    if (0) {
#if LWBTN_CFG_USE_KEEPALIVE
    } else if (evt == LWBTN_EVT_KEEPALIVE) {
        s = "KEEPALIVE";
        color = FOREGROUND_RED;

        test_errors += test_evt_data == NULL || test_evt_data->keepalive_cnt != keepalive_cnt;
#endif /* LWBTN_CFG_USE_KEEPALIVE */
    } else if (evt == LWBTN_EVT_ONPRESS) {
        s = "  ONPRESS";
        color = FOREGROUND_GREEN;
        is_pressed = 1;
    } else if (evt == LWBTN_EVT_ONRELEASE) {
        s = "ONRELEASE";
        color = FOREGROUND_BLUE;
    } else if (evt == LWBTN_EVT_ONCLICK) {
        s = "  ONCLICK";
        color = FOREGROUND_RED | FOREGROUND_GREEN;
        is_click = 1;

        test_errors += test_evt_data == NULL || test_evt_data->conseq_clicks != btn->click.cnt;
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
    ++array_index; /* Go to next step in next event */
#else
    /* Control GPIO pin in STM32 */
    if (evt == LWBTN_EVT_ONPRESS) {
        HAL_GPIO_WritePin(OUT_GPIO_Port, OUT_Pin, GPIO_PIN_SET);
    } else if (evt == LWBTN_EVT_ONRELEASE) {
        HAL_GPIO_WritePin(OUT_GPIO_Port, OUT_Pin, GPIO_PIN_RESET);
    } else if (evt == LWBTN_EVT_ONCLICK) {
        is_click = 1;
        HAL_GPIO_WritePin(OUT_CLICK_GPIO_Port, OUT_CLICK_Pin, GPIO_PIN_SET);
    }
#endif /* defined(WIN32) */
    (void)lw;
}

#if defined(WIN32)
/**
 * \brief           Test function
 */
int
test_win32(void) {
    printf("Test running\r\n");

    /* Define buttons */
    lwbtn_init_ex(NULL, btns, sizeof(btns) / sizeof(btns[0]), prv_btn_get_state, prv_btn_event);

    /* Counter simulates ms tick */
    for (size_t i = 0; i < MAX_TIME_MS; ++i) {
        time_current = i; /* Set current time used in callback */
        lwbtn_process(i); /* Now run processing */
    }
    return 0;
}
#endif /* defined(WIN32) */

#if defined(STM32)

int
test_stm32(void) {
    uint32_t tick, tick_old = 0;

    /* Define buttons */
    lwbtn_init_ex(NULL, btns, sizeof(btns) / sizeof(btns[0]), prv_btn_get_state, prv_btn_event);

    /* Counter simulates ms tick */
    tick_old = 0;
    for (size_t i = 0; i < MAX_TIME_MS; ++i) {
        /* Wait for a tick to have real time data */
        while ((tick = HAL_GetTick()) == tick_old) {}
        tick_old = tick;

        /* Reset from previous click event */
        if (is_click) {
            HAL_GPIO_WritePin(OUT_CLICK_GPIO_Port, OUT_CLICK_Pin, GPIO_PIN_RESET);
            is_click = 0;
        }

        /* Set current tick time in ms */
        time_current = i; /* Set current time used in callback */
        lwbtn_process(i); /* Now run processing */
    }
    return 0;
}

#endif /* defined(STM32) */
