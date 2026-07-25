#include <stdio.h>
#include "lwbtn/lwbtn.h"
#include "test.h"

/* Check the conditions for the test */
#if !LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC
#error "LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC must be enabled for this test"
#endif /* !LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC */
#if !LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC
#error "LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC must be enabled for this test"
#endif /* !LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC */

/* Single button, driven manually (no get-state callback) */
static lwbtn_btn_t btn;

/* Last event captured by the event callback, checked synchronously right after each process() call */
static lwbtn_evt_t last_evt;
static uint8_t evt_received;

/* Event callback -> simply records what happened, no assertions here */
static void
prv_btn_event(struct lwbtn* lw, struct lwbtn_btn* b, lwbtn_evt_t evt) {
    (void)lw;
    (void)b;
    last_evt = evt;
    evt_received = 1;
}

/**
 * \brief           Set input state, run a single process round for that exact tick,
 *                  then immediately check whether the expected event (or no event) was received.
 *
 * \return          1 on success, 0 on failure (and prints what went wrong).
 */
static int
prv_step(uint8_t state, lwbtn_time_t mstime, uint8_t expect_event, lwbtn_evt_t expected_evt) {
    evt_received = 0;
    last_evt = (lwbtn_evt_t)-1; /* Force invalid state */

    /* Set manual state and run the button */
    lwbtn_set_btn_state(&btn, state);
    lwbtn_process(mstime);

    /* Execute and check the expected event */
    if (expect_event) {
        if (!evt_received || last_evt != expected_evt) {
            printf("FAIL at t=%u: expected event %d, got received=%u evt=%d\r\n", (unsigned)mstime, (int)expected_evt,
                   (unsigned)evt_received, (int)last_evt);
            return 0;
        }
        printf("OK   at t=%u: got expected event %d\r\n", (unsigned)mstime, (int)expected_evt);
    } else {
        if (evt_received) {
            printf("FAIL at t=%u: expected no event, got evt=%d\r\n", (unsigned)mstime, (int)last_evt);
            return 0;
        }
        printf("OK   at t=%u: no event, as expected\r\n", (unsigned)mstime);
    }
    return 1;
}

/**
 * \brief           Test function
 */
int
test_run(void) {
    int all_ok = 1;
    uint32_t mstime = 0;

    /* Init the simple instance */
    lwbtn_init_ex(NULL, &btn, 1, NULL, prv_btn_event);

    /*
     * Case 1: debounce is fully disabled.
     *
     * We expect on-press and on-release events to be fired immediately on the exact process() call
     */
    {
        printf("Case 1: debounce disabled, expect immediate on-press/on-release events\r\n-----\r\n");

        /* Disable debounce for this pin */
        lwbtn_debounce_set_press_time(&btn, 0);
        lwbtn_debounce_set_release_time(&btn, 0);

        /* Prime the "first inactive state" gate before any transition can be detected */
        all_ok &= prv_step(0, mstime++, 0, (lwbtn_evt_t)0);

        /* Press: with 0 debounce, on-press must fire on this exact call, not on some later round */
        all_ok &= prv_step(1, mstime++, 1, LWBTN_EVT_ONPRESS);

        /* Still pressed, no change -> no duplicate on-press */
        all_ok &= prv_step(1, mstime++, 0, (lwbtn_evt_t)0);

        /* Release: with 0 debounce, on-release must fire on this exact call */
        all_ok &= prv_step(0, mstime++, 1, LWBTN_EVT_ONRELEASE);

        /* Still released, no change -> no duplicate on-release */
        all_ok &= prv_step(0, mstime++, 0, (lwbtn_evt_t)0);

        /* Repeat the cycle once more to confirm it is not a one-off */
        all_ok &= prv_step(1, mstime++, 1, LWBTN_EVT_ONPRESS);
        all_ok &= prv_step(0, mstime++, 1, LWBTN_EVT_ONRELEASE);
    }

    /*
     * Case 2: debounce is enabled with 1ms tick.
     *
     * We expect on-press and on-release events to be fired on a second run after the state has already changed
     */
    {
        printf("Case 2: debounce enabled with 1ms tick, expect delayed on-press/on-release events\r\n-----\r\n");

        /* Use 1ms state changes */
        lwbtn_debounce_set_press_time(&btn, 1);
        lwbtn_debounce_set_release_time(&btn, 1);

        /* Prime the "first inactive state" gate before any transition can be detected */
        all_ok &= prv_step(0, mstime++, 0, (lwbtn_evt_t)0);

        /* Press: with 1ms debounce, on-press must fire on the second call */
        all_ok &= prv_step(1, mstime++, 0, (lwbtn_evt_t)0);
        all_ok &= prv_step(1, mstime++, 1, LWBTN_EVT_ONPRESS);

        /* Still pressed, no change -> no duplicate on-press */
        all_ok &= prv_step(1, mstime++, 0, (lwbtn_evt_t)0);

        /* Release: with 1ms debounce, on-release must fire on the second call */
        all_ok &= prv_step(0, mstime++, 0, (lwbtn_evt_t)0);
        all_ok &= prv_step(0, mstime++, 1, LWBTN_EVT_ONRELEASE);

        /* Still released, no change -> no duplicate on-release */
        all_ok &= prv_step(0, mstime++, 0, (lwbtn_evt_t)0);

        /* Repeat the cycle once more to confirm it is not a one-off */
        all_ok &= prv_step(1, mstime++, 0, (lwbtn_evt_t)0);
        all_ok &= prv_step(1, mstime++, 1, LWBTN_EVT_ONPRESS);
        all_ok &= prv_step(0, mstime++, 0, (lwbtn_evt_t)0);
        all_ok &= prv_step(0, mstime++, 1, LWBTN_EVT_ONRELEASE);
    }

    return all_ok ? 0 : -1;
}
