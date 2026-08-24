#include <stdio.h>
#include "lwbtn/lwbtn.h"
#include "test.h"

/* Check the conditions for the test */
#if !LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC
#error "LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC must be enabled for this test"
#endif /* !LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC */
#if !LWBTN_CFG_USE_KEEPALIVE
#error "LWBTN_CFG_USE_KEEPALIVE must be enabled for this test"
#endif /* !LWBTN_CFG_USE_KEEPALIVE */

/* Single button, driven manually (no get-state callback) */
static lwbtn_btn_t btn;

/* Counters, updated by the event callback and checked by the test */
static uint16_t press_cnt, release_cnt, keepalive_cnt;

/* Event callback -> just counts events by type, no assertions here */
static void
prv_btn_event(struct lwbtn* lw, struct lwbtn_btn* b, lwbtn_evt_t evt) {
    (void)lw;
    (void)b;
    switch (evt) {
        case LWBTN_EVT_ONPRESS: ++press_cnt; break;
        case LWBTN_EVT_ONRELEASE: ++release_cnt; break;
        case LWBTN_EVT_KEEPALIVE: ++keepalive_cnt; break;
        default: break;
    }
}

/**
 * \brief           Test function
 */
int
test_run(void) {
    int all_ok = 1;
    lwbtn_time_t mstime = 0;

    /* Init the simple instance */
    lwbtn_init_ex(NULL, &btn, 1, NULL, prv_btn_event);

    /* Prime the "first inactive state" gate before any transition can be detected */
    lwbtn_set_btn_state(&btn, 0);
    lwbtn_process(mstime++);

    /*
     * Case 1: keep-alive period set to a normal, non-zero value.
     *
     * This is a regression check for the keep-alive loop itself: with a normal period,
     * a single process() call, far in the future, must still produce the exact number
     * of keep-alive events that elapsed in the meantime.
     */
    {
        printf("Case 1: keep-alive period = 10ms, expect exactly 5 events after 55ms\r\n-----\r\n");

        lwbtn_keepalive_set_period(&btn, 10);
        press_cnt = release_cnt = keepalive_cnt = 0;

        /* Press the button -> on-press fires immediately (debounce disabled) */
        lwbtn_set_btn_state(&btn, 1);
        lwbtn_process(mstime);
        mstime += 55;

        /* Single process call, far enough in the future to trigger multiple keep-alive events at once */
        lwbtn_process(mstime);

        if (press_cnt != 1 || keepalive_cnt != 5 || lwbtn_keepalive_get_count(&btn) != 5) {
            printf("FAIL: press_cnt=%u, keepalive_cnt=%u, get_count=%u (expected 1, 5, 5)\r\n", (unsigned)press_cnt,
                   (unsigned)keepalive_cnt, (unsigned)lwbtn_keepalive_get_count(&btn));
            all_ok = 0;
        } else {
            printf("OK: got 5 keep-alive events as expected\r\n");
        }

        /* Release the button to end this case cleanly */
        lwbtn_set_btn_state(&btn, 0);
        lwbtn_process(++mstime);
    }

    /*
     * Case 2: keep-alive period dynamically set to 0.
     *
     * Per `lwbtn_keepalive_set_period` documentation, a period of `0` disables keep-alive
     * for the button. Before the fix, the process loop never advanced its internal time
     * reference when the period was 0, so it looped forever instead of returning - this
     * case reproduces exactly that scenario (a large time jump while pressed).
     *
     * Reaching the end of this block at all (rather than hanging) is itself part of what
     * is being verified here.
     */
    {
        printf("Case 2: keep-alive period = 0, expect zero events and no hang\r\n-----\r\n");

        lwbtn_keepalive_set_period(&btn, 0);
        press_cnt = release_cnt = keepalive_cnt = 0;

        /* Press the button -> on-press fires immediately (debounce disabled) */
        lwbtn_set_btn_state(&btn, 1);
        lwbtn_process(mstime);
        mstime += 1000000UL; /* Large jump in time */

        /* If we reach this line at all, the loop above did not hang */
        lwbtn_process(mstime);

        if (press_cnt != 1 || keepalive_cnt != 0 || lwbtn_keepalive_get_count(&btn) != 0) {
            printf("FAIL: press_cnt=%u, keepalive_cnt=%u, get_count=%u (expected 1, 0, 0)\r\n", (unsigned)press_cnt,
                   (unsigned)keepalive_cnt, (unsigned)lwbtn_keepalive_get_count(&btn));
            all_ok = 0;
        } else {
            printf("OK: got zero keep-alive events, as expected\r\n");
        }

        /* Release the button and confirm on-release still fires normally with keep-alive disabled */
        lwbtn_set_btn_state(&btn, 0);
        lwbtn_process(++mstime);
        if (release_cnt != 1) {
            printf("FAIL: release_cnt=%u (expected 1)\r\n", (unsigned)release_cnt);
            all_ok = 0;
        } else {
            printf("OK: on-release still fired normally\r\n");
        }
    }

    return all_ok ? 0 : -1;
}
