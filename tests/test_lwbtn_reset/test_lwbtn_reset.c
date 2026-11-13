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

    /* Per-event counters */
    uint16_t keepalive_cnt;
    uint16_t onpress_cnt;

    /* When set, `lwbtn_reset` is called from within the callback of `reset_evt` event */
    uint8_t reset_in_cb;
    lwbtn_evt_t reset_evt;
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
    if (evt == LWBTN_EVT_KEEPALIVE) {
        ++context.keepalive_cnt;
    }
    if (evt == LWBTN_EVT_ONPRESS) {
        ++context.onpress_cnt;
    }
    if (context.reset_in_cb && evt == context.reset_evt) {
        context.reset_in_cb = 0; /* One-shot */
        lwbtn_reset(lw, b);
    }
}

/* Single click: press, hold for `hold` ms, release. Process is called only at press and release */
static lwbtn_time_t
prv_click(lwbtn_time_t mstime, lwbtn_time_t hold) {
    lwbtn_set_btn_state(&context.btn, 1);
    lwbtn_process(mstime);
    mstime += hold;
    lwbtn_set_btn_state(&context.btn, 0);
    lwbtn_process(mstime);
    return mstime + 1;
}

/* Keep the button released and process every ms, to let any pending (delayed) click fire */
static lwbtn_time_t
prv_idle(lwbtn_time_t mstime, lwbtn_time_t duration) {
    lwbtn_set_btn_state(&context.btn, 0);
    for (lwbtn_time_t i = 0; i < duration; ++i) {
        lwbtn_process(mstime++);
    }
    return mstime;
}

/* Init the default instance with single button and set it to the default (released) state */
static lwbtn_time_t
prv_init_single(lwbtn_time_t mstime) {
    memset(&context, 0x00, sizeof(context));
    lwbtn_init_ex(NULL, &context.btn, 1, NULL, prv_btn_event);
    lwbtn_set_btn_state(&context.btn, 0);
    lwbtn_process(mstime++);
    return mstime;
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
 * - Reset called from within any event callback stops further events for that button,
 *   including keep-alive, on-press and (delayed) on-click events.
 * - Reset scope: single button, given instance, or default instance.
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

#if LWBTN_CFG_USE_KEEPALIVE
    {
        /*
         * Test reset called from within keep-alive callback.
         *
         * Process is called late (several keep-alive periods elapsed at once),
         * which would normally fire multiple keep-alive events in a single process call.
         * After reset in the first callback, no further events must be sent.
         */
        memset(&context, 0x00, sizeof(context));
        lwbtn_init_ex(NULL, &context.btn, 1, NULL, prv_btn_event);

        lwbtn_set_btn_state(&context.btn, 0);
        lwbtn_process(mstime++);
        lwbtn_set_btn_state(&context.btn, 1);
        lwbtn_process(mstime++);
        TEST_ASSERT(lwbtn_is_btn_active(&context.btn));

        context.event_cnt = 0;
        context.keepalive_cnt = 0;
        context.reset_in_cb = 1;
        context.reset_evt = LWBTN_EVT_KEEPALIVE;
        mstime += 5 * LWBTN_CFG_TIME_KEEPALIVE_PERIOD;
        lwbtn_process(mstime++);

        printf("Keep-alive events with reset in callback: %u (expected 1), events: %u (expected 1)\r\n",
               context.keepalive_cnt, context.event_cnt);
        TEST_ASSERT(context.keepalive_cnt == 1 && context.event_cnt == 1);
        TEST_ASSERT(!lwbtn_is_btn_active(&context.btn));

        /* Keep holding -> nothing more must come, then release -> no onrelease */
        mstime += 5 * LWBTN_CFG_TIME_KEEPALIVE_PERIOD;
        lwbtn_process(mstime++);
        lwbtn_set_btn_state(&context.btn, 0);
        lwbtn_process(mstime++);
        printf("Events after reset in keep-alive: %u (expected 1)\r\n", context.event_cnt);
        TEST_ASSERT(context.event_cnt == 1);
    }
#endif /* LWBTN_CFG_USE_KEEPALIVE */

    {
        /*
         * Test reset called from within on-release callback.
         *
         * Release is a valid click, but click sequence must not start
         * as button has been reset -> no (delayed) click event must be sent.
         */
        mstime = prv_init_single(mstime);
        context.reset_in_cb = 1;
        context.reset_evt = LWBTN_EVT_ONRELEASE;
        mstime = prv_click(mstime, LWBTN_CFG_TIME_CLICK_MIN + 10);
        mstime = prv_idle(mstime, LWBTN_CFG_TIME_CLICK_MULTI_MAX + 10);

        printf("Reset in on-release: click events: %u (expected 0), events: %u (expected 2)\r\n", context.click_cnt,
               context.event_cnt);
        TEST_ASSERT(context.click_cnt == 0 && context.event_cnt == 2);
    }

    {
        /*
         * Test reset called from within on-click callback, which reports previous click sequence on release.
         *
         * First click is pending. Second press starts within multi-click window, but its release comes
         * after the window expired -> previous sequence is reported at release time.
         * Reset in that callback must prevent new sequence from starting -> no second (delayed) click.
         */
        mstime = prv_init_single(mstime);
        mstime = prv_click(mstime, LWBTN_CFG_TIME_CLICK_MIN + 10);
        context.reset_in_cb = 1;
        context.reset_evt = LWBTN_EVT_ONCLICK;
        mstime += LWBTN_CFG_TIME_CLICK_MULTI_MAX / 2;
        mstime = prv_click(mstime, LWBTN_CFG_TIME_CLICK_MAX - 10);
        printf("Reset in on-click on release: click events at release: %u (expected 1)\r\n", context.click_cnt);
        TEST_ASSERT(context.click_cnt == 1);

        mstime = prv_idle(mstime, LWBTN_CFG_TIME_CLICK_MULTI_MAX + 10);
        printf("Reset in on-click on release: click events after timeout: %u (expected 1)\r\n", context.click_cnt);
        TEST_ASSERT(context.click_cnt == 1);
    }

#if !LWBTN_CFG_CLICK_MAX_CONSECUTIVE_SEND_IMMEDIATELY
    {
        /*
         * Test reset called from within on-click callback, sent just before the new on-press
         * when maximum number of consecutive clicks has been reached.
         *
         * On-press of the new press must not be sent after reset, neither its on-release.
         */
        mstime = prv_init_single(mstime);
        for (size_t i = 0; i < LWBTN_CFG_CLICK_MAX_CONSECUTIVE; ++i) {
            mstime = prv_click(mstime, LWBTN_CFG_TIME_CLICK_MIN + 10);
        }
        TEST_ASSERT(lwbtn_click_get_count(&context.btn) == LWBTN_CFG_CLICK_MAX_CONSECUTIVE);

        context.event_cnt = 0;
        context.onpress_cnt = 0;
        context.reset_in_cb = 1;
        context.reset_evt = LWBTN_EVT_ONCLICK;
        mstime = prv_click(mstime, LWBTN_CFG_TIME_CLICK_MIN + 10);
        mstime = prv_idle(mstime, LWBTN_CFG_TIME_CLICK_MULTI_MAX + 10);

        printf("Reset in on-click before on-press: on-press events: %u (expected 0), events: %u (expected 1)\r\n",
               context.onpress_cnt, context.event_cnt);
        TEST_ASSERT(context.onpress_cnt == 0 && context.event_cnt == 1 && context.click_cnt == 1);
    }
#endif /* !LWBTN_CFG_CLICK_MAX_CONSECUTIVE_SEND_IMMEDIATELY */

    {
        /* Test reset scopes: single button, custom instance, default instance */
        static lwbtn_t custom_obj;
        static lwbtn_btn_t custom_btns[2];
        static lwbtn_btn_t default_btns[2];

        memset(&context, 0x00, sizeof(context));
        lwbtn_init_ex(&custom_obj, custom_btns, 2, NULL, prv_btn_event);
        lwbtn_init_ex(NULL, default_btns, 2, NULL, prv_btn_event);

#define PRESS_ALL()                                                                                                    \
    do {                                                                                                               \
        for (size_t i = 0; i < 2; ++i) {                                                                               \
            lwbtn_set_btn_state(&custom_btns[i], 0);                                                                   \
            lwbtn_set_btn_state(&default_btns[i], 0);                                                                  \
        }                                                                                                              \
        lwbtn_process_ex(&custom_obj, mstime);                                                                         \
        lwbtn_process_ex(NULL, mstime++);                                                                              \
        for (size_t i = 0; i < 2; ++i) {                                                                               \
            lwbtn_set_btn_state(&custom_btns[i], 1);                                                                   \
            lwbtn_set_btn_state(&default_btns[i], 1);                                                                  \
        }                                                                                                              \
        lwbtn_process_ex(&custom_obj, mstime);                                                                         \
        lwbtn_process_ex(NULL, mstime++);                                                                              \
    } while (0)
#define ACTIVE_MASK()                                                                                                  \
    ((lwbtn_is_btn_active(&custom_btns[0]) << 0) | (lwbtn_is_btn_active(&custom_btns[1]) << 1)                         \
     | (lwbtn_is_btn_active(&default_btns[0]) << 2) | (lwbtn_is_btn_active(&default_btns[1]) << 3))

        /* Button given -> only that button, even if lwobj points elsewhere */
        PRESS_ALL();
        TEST_ASSERT(ACTIVE_MASK() == 0x0F);
        lwbtn_reset(NULL, &custom_btns[1]);
        printf("Active mask after single reset: 0x%02X (expected 0x0D)\r\n", (unsigned)ACTIVE_MASK());
        TEST_ASSERT(ACTIVE_MASK() == 0x0D);

        /* Instance given, no button -> all buttons of that instance */
        PRESS_ALL();
        TEST_ASSERT(ACTIVE_MASK() == 0x0F);
        lwbtn_reset(&custom_obj, NULL);
        printf("Active mask after instance reset: 0x%02X (expected 0x0C)\r\n", (unsigned)ACTIVE_MASK());
        TEST_ASSERT(ACTIVE_MASK() == 0x0C);

        /* Both NULL -> all buttons of the default instance */
        PRESS_ALL();
        TEST_ASSERT(ACTIVE_MASK() == 0x0F);
        lwbtn_reset(NULL, NULL);
        printf("Active mask after default reset: 0x%02X (expected 0x03)\r\n", (unsigned)ACTIVE_MASK());
        TEST_ASSERT(ACTIVE_MASK() == 0x03);
#undef PRESS_ALL
#undef ACTIVE_MASK
    }

    return rescode;
}
