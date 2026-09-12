#include <stdio.h>
#include <string.h>
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

static struct {
    /* Single button, driven manually (no get-state callback) */
    lwbtn_btn_t btn;

    /* Counts how many ONCLICK events actually reach the application */
    uint16_t click_cnt;

    /* Last event received, and how many events were received since last cleared to 0 */
    lwbtn_evt_t last_evt;
    uint16_t event_cnt;
} context;

/* Event callback -> records what happened, no assertions here */
static void
prv_btn_event(struct lwbtn* lw, struct lwbtn_btn* b, lwbtn_evt_t evt) {
    (void)lw;
    (void)b;
    context.last_evt = evt;
    ++context.event_cnt;
    if (evt == LWBTN_EVT_ONCLICK) {
        ++context.click_cnt;
    }
}

/**
 * \brief           Test function
 *
 * Regression tests for `lwbtn_reset()`:
 *
 * - If the button is reset while still pressed, the pending onrelease for that
 *   press must never be reported, and the very next event must be a fresh onpress.
 * - A click that is still "pending" (onrelease already seen, but the multi-click
 *   timeout has not expired yet) must not be reported once the button is reset.
 */
int
test_run(void) {
    int rescode = 0;
    lwbtn_time_t mstime = 0;

    {
        /*
         * Test no onrelease if reset is called while button is still pressed.
         *
         * The very next event the application sees afterwards must be a fresh
         * onpress (from a brand new press), never the onrelease of the press
         * that was active when reset() was called.
         */
        memset(&context, 0x00, sizeof(context));
        lwbtn_init_ex(NULL, &context.btn, 1, NULL, prv_btn_event);

        /* Start with the default state */
        lwbtn_set_btn_state(&context.btn, 0);
        lwbtn_process(mstime++);

        /* Press -> onpress fires immediately (debounce disabled) */
        context.event_cnt = 0;
        lwbtn_set_btn_state(&context.btn, 1);
        lwbtn_process(mstime++);

        printf("Event after press: %d (expected ONPRESS=%d), event count: %u (expected 1)\r\n", (int)context.last_evt,
               (int)LWBTN_EVT_ONPRESS, context.event_cnt);
        TEST_ASSERT(context.event_cnt == 1 && context.last_evt == LWBTN_EVT_ONPRESS);
        TEST_ASSERT(lwbtn_is_btn_active(&context.btn));

        /* Reset the button while it is still physically held down */
        lwbtn_reset(NULL, &context.btn);

        printf("Button active right after reset: %u (expected 0)\r\n", (unsigned)lwbtn_is_btn_active(&context.btn));
        TEST_ASSERT(!lwbtn_is_btn_active(&context.btn));

        /* Release the button -> no onrelease must be reported for the press that was reset */
        context.event_cnt = 0;
        lwbtn_set_btn_state(&context.btn, 0);
        lwbtn_process(mstime++);

        printf("Events fired on release after reset: %u (expected 0)\r\n", context.event_cnt);
        TEST_ASSERT(context.event_cnt == 0);

        /* Fresh press -> a normal onpress must fire again, and it must be the only event */
        context.event_cnt = 0;
        lwbtn_set_btn_state(&context.btn, 1);
        lwbtn_process(mstime++);

        printf("Event after fresh press: %d (expected ONPRESS=%d), event count: %u (expected 1)\r\n",
               (int)context.last_evt, (int)LWBTN_EVT_ONPRESS, context.event_cnt);
        TEST_ASSERT(context.event_cnt == 1 && context.last_evt == LWBTN_EVT_ONPRESS);

        /* Release to leave the button idle for the next block */
        lwbtn_set_btn_state(&context.btn, 0);
        lwbtn_process(mstime++);
    }

    {
        /* Test no onclick if reset called before onclick event would fire (after the timeout) */
        memset(&context, 0x00, sizeof(context));
        lwbtn_init_ex(NULL, &context.btn, 1, NULL, prv_btn_event);

        /* Start with the default state */
        lwbtn_set_btn_state(&context.btn, 0);
        lwbtn_process(mstime++);

        /* Press and release within valid click timing -> click is now pending (cnt == 1) */
        lwbtn_set_btn_state(&context.btn, 1);
        lwbtn_process(mstime++);
        mstime += LWBTN_CFG_TIME_CLICK_MIN;
        lwbtn_set_btn_state(&context.btn, 0);
        lwbtn_process(mstime++);

        printf("Pending click count before reset: %u (expected 1)\r\n", lwbtn_click_get_count(&context.btn));
        TEST_ASSERT(lwbtn_click_get_count(&context.btn) == 1);

        /* Reset the button right in the middle of the pending click sequence */
        lwbtn_reset(NULL, &context.btn);

        printf("Pending click count right after reset: %u (expected 0)\r\n", lwbtn_click_get_count(&context.btn));
        TEST_ASSERT(lwbtn_click_get_count(&context.btn) == 0);

        /* Advance well past the multi-click timeout, button left idle -> no ONCLICK must ever fire */
        for (lwbtn_time_t i = 0; i < (lwbtn_time_t)LWBTN_CFG_TIME_CLICK_MULTI_MAX + 10; ++i) {
            lwbtn_process(mstime++);
        }

        printf("ONCLICK events fired after reset: %u (expected 0)\r\n", context.click_cnt);
        TEST_ASSERT(context.click_cnt == 0);
    }

    return rescode;
}
