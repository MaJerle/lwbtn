#include <stdio.h>
#include "lwbtn/lwbtn.h"
#include "test.h"

/* Check the conditions for the test */
#if LWBTN_CFG_TIME_DEBOUNCE_PRESS > 0
#error "LWBTN_CFG_TIME_DEBOUNCE_PRESS must be 0 for this test"
#endif /* LWBTN_CFG_TIME_DEBOUNCE_PRESS > 0 */
#if LWBTN_CFG_TIME_DEBOUNCE_RELEASE > 0
#error "LWBTN_CFG_TIME_DEBOUNCE_RELEASE must be 0 for this test"
#endif /* LWBTN_CFG_TIME_DEBOUNCE_RELEASE > 0 */
#if !LWBTN_CFG_USE_CLICK
#error "LWBTN_CFG_USE_CLICK must be enabled for this test"
#endif /* !LWBTN_CFG_USE_CLICK */

/* Single button, driven manually (no get-state callback) */
static lwbtn_btn_t btn;

/* Counts how many ONCLICK events actually reach the application */
static uint16_t click_cnt;

/* Event callback -> just counts click events, no assertions here */
static void
prv_btn_event(struct lwbtn* lw, struct lwbtn_btn* b, lwbtn_evt_t evt) {
    (void)lw;
    (void)b;
    if (evt == LWBTN_EVT_ONCLICK) {
        ++click_cnt;
    }
}

/**
 * \brief           Test function
 *
 * Regression test for `lwbtn_reset()`: a click that is still "pending"
 * (on-release already seen, but the multi-click timeout has not expired yet)
 * must not be reported to the application once the button has been reset.
 */
int
test_run(void) {
    int rescode = 0;
    lwbtn_time_t mstime = 0;

    lwbtn_init_ex(NULL, &btn, 1, NULL, prv_btn_event);

    /* Start with the default state */
    lwbtn_set_btn_state(&btn, 0);
    lwbtn_process(mstime++);

    /* Press and release within valid click timing -> click is now pending (cnt == 1) */
    lwbtn_set_btn_state(&btn, 1);
    lwbtn_process(mstime++);
    mstime += LWBTN_CFG_TIME_CLICK_MIN;
    lwbtn_set_btn_state(&btn, 0);
    lwbtn_process(mstime++);

    printf("Pending click count before reset: %u (expected 1)\r\n", lwbtn_click_get_count(&btn));
    TEST_ASSERT(lwbtn_click_get_count(&btn) == 1);

    /* Reset the button right in the middle of the pending click sequence */
    lwbtn_reset(NULL, &btn);

    printf("Pending click count right after reset: %u (expected 0)\r\n", lwbtn_click_get_count(&btn));
    TEST_ASSERT(lwbtn_click_get_count(&btn) == 0);

    /* Advance well past the multi-click timeout, button left idle -> no ONCLICK must ever fire */
    for (lwbtn_time_t i = 0; i < (lwbtn_time_t)LWBTN_CFG_TIME_CLICK_MULTI_MAX + 10; ++i) {
        lwbtn_process(mstime++);
    }

    printf("ONCLICK events fired after reset: %u (expected 0)\r\n", click_cnt);
    TEST_ASSERT(click_cnt == 0);

    return rescode;
}
