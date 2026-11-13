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

static struct {
    lwbtn_btn_t btn;

    /* Results captured from within the keep-alive callback */
    uint16_t keepalive_cnt;
    uint16_t keepalive_pressed_for_ok; /* Number of keep-alives where "pressed for" matched expectation */

    /* Results captured from within the on-press callback */
    uint8_t onpress_pressed_for_0;
    uint8_t onpress_pressed_for_1;
} context;

static void
prv_btn_event(struct lwbtn* lw, struct lwbtn_btn* b, lwbtn_evt_t evt) {
    (void)lw;
    if (evt == LWBTN_EVT_ONPRESS) {
        context.onpress_pressed_for_0 = lwbtn_is_btn_pressed_for(b, 0);
        context.onpress_pressed_for_1 = lwbtn_is_btn_pressed_for(b, 1);
    }
#if LWBTN_CFG_USE_KEEPALIVE
    if (evt == LWBTN_EVT_KEEPALIVE) {
        /* At N-th keep-alive, button has been pressed for at least N periods, but not N periods + 1 ms */
        lwbtn_time_t t = (lwbtn_time_t)(++context.keepalive_cnt) * LWBTN_CFG_TIME_KEEPALIVE_PERIOD;
        if (lwbtn_is_btn_pressed_for(b, t) && !lwbtn_is_btn_pressed_for(b, t + 1)) {
            ++context.keepalive_pressed_for_ok;
        }
    }
#endif /* LWBTN_CFG_USE_KEEPALIVE */
}

/**
 * \brief           Test function
 *
 * Tests for `lwbtn_is_btn_pressed_for()`:
 *
 * - Returns `0` for NULL button and when button is not active
 * - Pressed time is measured from on-press until the last process call
 * - Valid within event callbacks (time of the current process call)
 * - Returns `0` after release and after reset, even if button is still physically held
 */
int
test_run(void) {
    int rescode = 0;
    lwbtn_time_t mstime = 1000;
    lwbtn_time_t press_time;

    memset(&context, 0x00, sizeof(context));
    lwbtn_init_ex(NULL, &context.btn, 1, NULL, prv_btn_event);
    lwbtn_set_btn_state(&context.btn, 0);
    lwbtn_process(mstime++);

    /* NULL and inactive button */
    TEST_ASSERT(!lwbtn_is_btn_pressed_for(NULL, 0));
    TEST_ASSERT(!lwbtn_is_btn_pressed_for(&context.btn, 0));

    {
        /* Press -> on-press fires immediately, pressed time is 0 */
        press_time = mstime;
        lwbtn_set_btn_state(&context.btn, 1);
        lwbtn_process(mstime);

        printf("In on-press: pressed for 0: %u (expected 1), pressed for 1: %u (expected 0)\r\n",
               (unsigned)context.onpress_pressed_for_0, (unsigned)context.onpress_pressed_for_1);
        TEST_ASSERT(context.onpress_pressed_for_0 && !context.onpress_pressed_for_1);
        TEST_ASSERT(lwbtn_is_btn_pressed_for(&context.btn, 0));
        TEST_ASSERT(!lwbtn_is_btn_pressed_for(&context.btn, 1));

        /* Hold, processing every ms. Value must follow the last process time */
        for (mstime = press_time + 1; mstime <= press_time + 2000; ++mstime) {
            lwbtn_process(mstime);
            lwbtn_time_t held = (lwbtn_time_t)(mstime - press_time);
            if (!lwbtn_is_btn_pressed_for(&context.btn, held) || lwbtn_is_btn_pressed_for(&context.btn, held + 1)) {
                printf("Pressed time mismatch at %u ms of holding\r\n", (unsigned)held);
                TEST_ASSERT(0);
                break;
            }
        }

        /* Value does not move without process call */
        TEST_ASSERT(lwbtn_is_btn_pressed_for(&context.btn, 2000));
        TEST_ASSERT(!lwbtn_is_btn_pressed_for(&context.btn, 2001));

#if LWBTN_CFG_USE_KEEPALIVE
        printf("Keep-alive events: %u, with correct pressed time: %u\r\n", (unsigned)context.keepalive_cnt,
               (unsigned)context.keepalive_pressed_for_ok);
        TEST_ASSERT(context.keepalive_cnt > 0 && context.keepalive_cnt == context.keepalive_pressed_for_ok);
#endif /* LWBTN_CFG_USE_KEEPALIVE */

        /* Release -> no longer pressed */
        lwbtn_set_btn_state(&context.btn, 0);
        lwbtn_process(mstime++);
        printf("After release, pressed for 0: %u (expected 0)\r\n",
               (unsigned)lwbtn_is_btn_pressed_for(&context.btn, 0));
        TEST_ASSERT(!lwbtn_is_btn_pressed_for(&context.btn, 0));
    }

    {
        /* New press -> time starts again from the new on-press */
        mstime += 500;
        press_time = mstime;
        lwbtn_set_btn_state(&context.btn, 1);
        lwbtn_process(mstime);
        mstime += 300;
        lwbtn_process(mstime);
        TEST_ASSERT(lwbtn_is_btn_pressed_for(&context.btn, 300));
        TEST_ASSERT(!lwbtn_is_btn_pressed_for(&context.btn, 301));

        /* Range check, pressed time is 300 ms - both bounds are inclusive */
        TEST_ASSERT(lwbtn_is_btn_pressed_for_range(&context.btn, 300, 300));
        TEST_ASSERT(lwbtn_is_btn_pressed_for_range(&context.btn, 0, 300));
        TEST_ASSERT(lwbtn_is_btn_pressed_for_range(&context.btn, 300, 1000));
        TEST_ASSERT(lwbtn_is_btn_pressed_for_range(&context.btn, 200, 400));
        TEST_ASSERT(!lwbtn_is_btn_pressed_for_range(&context.btn, 301, 1000)); /* Below min */
        TEST_ASSERT(!lwbtn_is_btn_pressed_for_range(&context.btn, 0, 299));    /* Above max */
        TEST_ASSERT(!lwbtn_is_btn_pressed_for_range(&context.btn, 400, 200));  /* min > max */
        TEST_ASSERT(!lwbtn_is_btn_pressed_for_range(NULL, 0, (lwbtn_time_t)-1));
        printf("Range [200, 400] at 300 ms: %u (expected 1), range [0, 299]: %u (expected 0)\r\n",
               (unsigned)lwbtn_is_btn_pressed_for_range(&context.btn, 200, 400),
               (unsigned)lwbtn_is_btn_pressed_for_range(&context.btn, 0, 299));

        /* Reset while held -> no longer pressed, even after further processing */
        lwbtn_reset(NULL, &context.btn);
        TEST_ASSERT(!lwbtn_is_btn_pressed_for(&context.btn, 0));
        mstime += 1000;
        lwbtn_process(mstime);
        printf("After reset while held, pressed for 0: %u (expected 0)\r\n",
               (unsigned)lwbtn_is_btn_pressed_for(&context.btn, 0));
        TEST_ASSERT(!lwbtn_is_btn_pressed_for(&context.btn, 0));
    }

    return rescode;
}
