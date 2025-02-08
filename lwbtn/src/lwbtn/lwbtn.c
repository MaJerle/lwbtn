/**
 * \file            lwbtn.c
 * \brief           Lightweight button system
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwBTN - Lightweight button manager.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v1.1.0
 */
#include <string.h>
#include "lwbtn/lwbtn.h"

#if LWBTN_CFG_GET_STATE_MODE > 2
#error "Invalid LWBTN_GET_STATE_MODE_CALLBACK configuration"
#endif

#define LWBTN_FLAG_ONPRESS_SENT ((uint16_t)0x0001) /*!< Flag indicates that on-press event has been sent */
#define LWBTN_FLAG_MANUAL_STATE                                                                                        \
    ((uint16_t)0x0002) /*!< Flag indicates that user wants to manually set button state.
                                                    Do not call "get_state" function */
#define LWBTN_FLAG_FIRST_INACTIVE_RCVD                                                                                 \
    ((uint16_t)0x0004)                      /*!< We are waiting for first inactive state before we continue further */
#define LWBTN_FLAG_RESET ((uint16_t)0x0008) /*!< Reset called on the button */

#if LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC
#define LWBTN_TIME_DEBOUNCE_PRESS_GET_MIN(btn) ((lwbtn_time_t)((btn)->time_debounce))
#else
#define LWBTN_TIME_DEBOUNCE_PRESS_GET_MIN(btn) ((lwbtn_time_t)LWBTN_CFG_TIME_DEBOUNCE_PRESS)
#endif /* LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC */

#if LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC
#define LWBTN_TIME_DEBOUNCE_RELEASE_GET_MIN(btn) ((lwbtn_time_t)((btn)->time_debounce_release))
#else
#define LWBTN_TIME_DEBOUNCE_RELEASE_GET_MIN(btn) ((lwbtn_time_t)LWBTN_CFG_TIME_DEBOUNCE_RELEASE)
#endif /* LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC */

#if LWBTN_CFG_TIME_CLICK_MIN_DYNAMIC
#define LWBTN_TIME_CLICK_GET_PRESSED_MIN(btn) ((lwbtn_time_t)((btn)->time_click_pressed_min))
#else
#define LWBTN_TIME_CLICK_GET_PRESSED_MIN(btn) ((lwbtn_time_t)LWBTN_CFG_TIME_CLICK_MIN)
#endif /* LWBTN_CFG_TIME_CLICK_MIN_DYNAMIC */
#if LWBTN_CFG_TIME_CLICK_MAX_DYNAMIC
#define LWBTN_TIME_CLICK_GET_PRESSED_MAX(btn) ((lwbtn_time_t)((btn)->time_click_pressed_max))
#else
#define LWBTN_TIME_CLICK_GET_PRESSED_MAX(btn) ((lwbtn_time_t)LWBTN_CFG_TIME_CLICK_MAX)
#endif /* LWBTN_CFG_TIME_CLICK_MAX_DYNAMIC */
#if LWBTN_CFG_TIME_CLICK_MULTI_MAX_DYNAMIC
#define LWBTN_TIME_CLICK_MAX_MULTI(btn) ((lwbtn_time_t)((btn)->time_click_multi_max))
#else
#define LWBTN_TIME_CLICK_MAX_MULTI(btn) ((lwbtn_time_t)LWBTN_CFG_TIME_CLICK_MULTI_MAX)
#endif /* LWBTN_CFG_TIME_CLICK_MULTI_MAX_DYNAMIC */
#if LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC
#define LWBTN_TIME_KEEPALIVE_PERIOD(btn) ((lwbtn_time_t)((btn)->time_keepalive_period))
#else
#define LWBTN_TIME_KEEPALIVE_PERIOD(btn) ((lwbtn_time_t)LWBTN_CFG_TIME_KEEPALIVE_PERIOD)
#endif /* LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC */
#if LWBTN_CFG_CLICK_MAX_CONSECUTIVE_DYNAMIC
#define LWBTN_CLICK_MAX_CONSECUTIVE(btn) ((btn)->max_consecutive)
#else
#define LWBTN_CLICK_MAX_CONSECUTIVE(btn) LWBTN_CFG_CLICK_MAX_CONSECUTIVE
#endif /* LWBTN_CFG_CLICK_MAX_CONSECUTIVE_DYNAMIC */

/* Get button state */
#if LWBTN_CFG_GET_STATE_MODE == LWBTN_GET_STATE_MODE_CALLBACK
#define LWBTN_BTN_GET_STATE(lwobj, btn) ((lwobj)->get_state_fn((lwobj), (btn)))
#elif LWBTN_CFG_GET_STATE_MODE == LWBTN_GET_STATE_MODE_MANUAL
#define LWBTN_BTN_GET_STATE(lwobj, btn) ((btn)->curr_state)
#elif LWBTN_CFG_GET_STATE_MODE == LWBTN_GET_STATE_MODE_CALLBACK_OR_MANUAL
#define LWBTN_BTN_GET_STATE(lwobj, btn)                                                                                \
    (((btn)->flags & LWBTN_FLAG_MANUAL_STATE)                                                                          \
         ? ((btn)->curr_state)                                                                                         \
         : (((lwobj)->get_state_fn != NULL) ? ((lwobj)->get_state_fn((lwobj), (btn))) : 0))
#endif

/* Default button group instance */
static lwbtn_t lwbtn_default;
#define LWBTN_GET_LWOBJ(in_lwobj) ((in_lwobj) != NULL ? (in_lwobj) : (&lwbtn_default))

/**
 * \brief           Process the button information and state
 * 
 * \param[in]       lwobj: LwBTN instance. Set to `NULL` to use default one
 * \param[in]       btn: Button instance to process
 * \param[in]       mstime: Current milliseconds system time
 */
static void
prv_process_btn(lwbtn_t* lwobj, lwbtn_btn_t* btn, lwbtn_time_t mstime) {
    uint8_t new_state;

    /* Get button state */
    new_state = LWBTN_BTN_GET_STATE(lwobj, btn);

    /* 
     * First state must be "inactive" before
     * any further button state is being processed.
     * 
     * This is to prevent initial detected state on hardware errors,
     * or when button is kept pressed after the system/lib reset.
     * 
     * When user uses manual state set (no callback system),
     * it is up to user to first call "set state" function and set state to inactive
     * 
     * This features is also used for "button reset"
     */
    if (!(btn->flags & LWBTN_FLAG_FIRST_INACTIVE_RCVD)) {
        if (new_state) {
            return;
        }

        /* Reset all states */
        btn->last_state = 0;
        btn->flags = LWBTN_FLAG_FIRST_INACTIVE_RCVD;
    }

#if 0
    /*
     * When the button is pressed, user can manually
     * reset the button
     */
    if ((btn->flags) & LWBTN_FLAG_RESET) {
        btn->last_state = 0;             /* Disable the state */
        btn->flags &= LWBTN_FLAG_RESET; /* Keep reset, delete others */
        if (new_state) {
            return;
        }
        btn->flags &= ~LWBTN_FLAG_RESET; /* Start over */
    }
#endif

    /* Button state has just changed */
    if (new_state != btn->last_state) {
        btn->time_state_change = mstime;
    }

    /* Button is still pressed */
    else if (new_state) {
        /* 
         * Handle debounce and send on-press event
         *
         * This is when we detect valid press
         */
        if (!(btn->flags & LWBTN_FLAG_ONPRESS_SENT)) {
            /*
             * Run if statement when:
             *
             * - Runtime mode is enabled -> user sets its own config for debounce
             * - Config debounce time for press is more than `0`
             */
#if LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC || LWBTN_CFG_TIME_DEBOUNCE_PRESS > 0
            if ((lwbtn_time_t)(mstime - btn->time_state_change) >= LWBTN_TIME_DEBOUNCE_PRESS_GET_MIN(btn))
#endif /* LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC || LWBTN_CFG_TIME_DEBOUNCE_PRESS> 0 */
            {
#if !LWBTN_CFG_CLICK_MAX_CONSECUTIVE_SEND_IMMEDIATELY
                /*
                 * Depending on the configuration,
                 * this part will send on-click event just before the next on-press release,
                 * if maximum number of consecutive clicks has been reached.
                 */
                if (btn->click.cnt > 0 && btn->click.cnt == LWBTN_CLICK_MAX_CONSECUTIVE(btn)) {
                    lwobj->evt_fn(lwobj, btn, LWBTN_EVT_ONCLICK);
                    btn->click.cnt = 0;
                }
#endif /* !LWBTN_CFG_CLICK_MAX_CONSECUTIVE_SEND_IMMEDIATELY */

                /* Start with new on-press */
                btn->flags |= LWBTN_FLAG_ONPRESS_SENT;
                lwobj->evt_fn(lwobj, btn, LWBTN_EVT_ONPRESS);
#if LWBTN_CFG_USE_KEEPALIVE
                /* Set keep alive time */
                btn->keepalive.last_time = mstime;
                btn->keepalive.cnt = 0;
#endif /* LWBTN_CFG_USE_KEEPALIVE */

                btn->time_change = mstime; /* Button state has now changed */
            }
#if LWBTN_CFG_USE_KEEPALIVE
        } else {
            /*
             * Handle keep alive, but only if on-press event has been sent
             *
             * Keep alive is sent when valid press is being detected
             */
            while ((lwbtn_time_t)(mstime - btn->keepalive.last_time) >= LWBTN_TIME_KEEPALIVE_PERIOD(btn)) {
                btn->keepalive.last_time += LWBTN_TIME_KEEPALIVE_PERIOD(btn);
                ++btn->keepalive.cnt;
                lwobj->evt_fn(lwobj, btn, LWBTN_EVT_KEEPALIVE);
            }
#endif /* LWBTN_CFG_USE_KEEPALIVE */
        }
    }

    /* Button is still released */
    else {
        /*
         * We only need to react if on-press event has even been started.
         *
         * Do nothing if that was not the case
         */
        if (btn->flags & LWBTN_FLAG_ONPRESS_SENT) {
            /*
             * Run if statement when:
             *
             * - Runtime mode is enabled -> user sets its own config for debounce
             * - Config debounce time for release is more than `0`
             */
#if LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC || LWBTN_CFG_TIME_DEBOUNCE_RELEASE > 0
            if ((mstime - btn->time_state_change) >= LWBTN_TIME_DEBOUNCE_RELEASE_GET_MIN(btn))
#endif /* LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC || LWBTN_CFG_TIME_DEBOUNCE_RELEASE > 0 */
            {
                /* Handle on-release event */
                btn->flags &= ~LWBTN_FLAG_ONPRESS_SENT;
                lwobj->evt_fn(lwobj, btn, LWBTN_EVT_ONRELEASE);

#if LWBTN_CFG_USE_CLICK
                /* Check time validity for click event */
                if ((lwbtn_time_t)(mstime - btn->time_change) >= LWBTN_TIME_CLICK_GET_PRESSED_MIN(btn)
                    && (lwbtn_time_t)(mstime - btn->time_change) <= LWBTN_TIME_CLICK_GET_PRESSED_MAX(btn)) {

                    /*
                     * Increase consecutive clicks if max not reached yet
                     * and if time between two clicks is not long enough
                     * 
                     * Otherwise we consider click as fresh one
                     */
                    if (btn->click.cnt > 0 && btn->click.cnt < LWBTN_CLICK_MAX_CONSECUTIVE(btn)
                        && (lwbtn_time_t)(mstime - btn->click.last_time) < LWBTN_TIME_CLICK_MAX_MULTI(btn)) {
                        ++btn->click.cnt;
                    } else {
                        /*
                         * Take care of any previous clicks if new one doesn't fit anymore in this context.
                         *
                         * This can only happen, if onpress started earlier than max consecutive time,
                         * while onrelease happened later than maximum consecutive time.
                         * 
                         * In this case simply report previous state before setting new click.
                         */
                        if (btn->click.cnt > 0) {
                            lwobj->evt_fn(lwobj, btn, LWBTN_EVT_ONCLICK);
                        }
                        btn->click.cnt = 1;
                    }
                    btn->click.last_time = mstime;
                } else {
#if LWBTN_CFG_CLICK_CONSECUTIVE_KEEP_AFTER_SHORT_PRESS
                    /* If last press was too short, and previous sequence of clicks was positive, send event to user */
                    if (btn->click.cnt > 0
                        && (lwbtn_time_t)(mstime - btn->time_change) < LWBTN_TIME_CLICK_GET_PRESSED_MIN(btn)) {
                        lwobj->evt_fn(lwobj, btn, LWBTN_EVT_ONCLICK);
                    }
#endif /* LWBTN_CFG_CLICK_CONSECUTIVE_KEEP_AFTER_SHORT_PRESS */
                    /*
                     * There was an on-release event, but timing
                     * for click event detection is outside allowed window.
                     * 
                     * Reset clicks counter -> not valid sequence for click event.
                     */
                    btn->click.cnt = 0;
                }

#if LWBTN_CFG_CLICK_MAX_CONSECUTIVE_SEND_IMMEDIATELY
                /* 
                 * Depending on the configuration,
                 * this part will send on-click event immediately after release event,
                 * if maximum number of consecutive clicks has been reached.
                 */
                if (btn->click.cnt > 0 && btn->click.cnt == LWBTN_CLICK_MAX_CONSECUTIVE(btn)) {
                    lwobj->evt_fn(lwobj, btn, LWBTN_EVT_ONCLICK);
                    btn->click.cnt = 0;
                }
#endif /* LWBTN_CFG_CLICK_MAX_CONSECUTIVE_SEND_IMMEDIATELY */
#endif /* LWBTN_CFG_USE_CLICK */

                btn->time_change = mstime; /* Button state has now changed */
            }
#if LWBTN_CFG_USE_CLICK
        } else {
            /* 
             * Based on te configuration, this part of the code
             * will send on-click event after certain timeout.
             * 
             * This feature is useful if user prefers multi-click feature
             * that is reported only after last click event happened,
             * including number of clicks made by user
             */
            if (btn->click.cnt > 0) {
                if ((lwbtn_time_t)(mstime - btn->click.last_time) >= LWBTN_TIME_CLICK_MAX_MULTI(btn)) {
                    lwobj->evt_fn(lwobj, btn, LWBTN_EVT_ONCLICK);
                    btn->click.cnt = 0;
                }
            }
#endif /* LWBTN_CFG_USE_CLICK */
        }
    }

    btn->last_state = new_state;
}

/**
 * \brief           Initialize button manager
 * \param[in]       lwobj: LwBTN instance. Set to `NULL` to use default one
 * \param[in]       btns: Array of buttons to process
 * \param[in]       btns_cnt: Number of buttons to process
 * \param[in]       get_state_fn: Pointer to function providing button state on demand.
 *                      May be set to `NULL` when \ref LWBTN_CFG_GET_STATE_MODE is set to manual.
 * \param[in]       evt_fn: Button event function callback
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwbtn_init_ex(lwbtn_t* lwobj, lwbtn_btn_t* btns, uint16_t btns_cnt, lwbtn_get_state_fn get_state_fn,
              lwbtn_evt_fn evt_fn) {
    lwobj = LWBTN_GET_LWOBJ(lwobj);

    if (btns == NULL || btns_cnt == 0 || evt_fn == NULL
#if LWBTN_CFG_GET_STATE_MODE == LWBTN_GET_STATE_MODE_CALLBACK
        || get_state_fn == NULL /* Parameter is a must only in callback-only mode */
#endif                          /* LWBTN_CFG_GET_STATE_MODE == LWBTN_GET_STATE_MODE_CALLBACK */
    ) {
        return 0;
    }

    LWBTN_MEMSET(lwobj, 0x00, sizeof(*lwobj));
    lwobj->btns = btns;
    lwobj->btns_cnt = btns_cnt;
    lwobj->evt_fn = evt_fn;
#if LWBTN_CFG_GET_STATE_MODE != LWBTN_GET_STATE_MODE_MANUAL
    lwobj->get_state_fn = get_state_fn;
#else
    (void)get_state_fn; /* May be unused */
#endif /* LWBTN_CFG_GET_STATE_MODE != LWBTN_GET_STATE_MODE_MANUAL */

    for (size_t i = 0; i < btns_cnt; ++i) {
#if LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC
        btns[i].time_debounce = LWBTN_CFG_TIME_DEBOUNCE_PRESS;
#endif /* LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC */
#if LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC
        btns[i].time_debounce_release = LWBTN_CFG_TIME_DEBOUNCE_RELEASE;
#endif /* LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC */
#if LWBTN_CFG_TIME_CLICK_MIN_DYNAMIC
        btns[i].time_click_pressed_min = LWBTN_CFG_TIME_CLICK_MIN;
#endif /* LWBTN_CFG_TIME_CLICK_MIN_DYNAMIC */
#if LWBTN_CFG_TIME_CLICK_MAX_DYNAMIC
        btns[i].time_click_pressed_max = LWBTN_CFG_TIME_CLICK_MAX;
#endif /* LWBTN_CFG_TIME_CLICK_MAX_DYNAMIC */
#if LWBTN_CFG_TIME_CLICK_MULTI_MAX_DYNAMIC
        btns[i].time_click_multi_max = LWBTN_CFG_TIME_CLICK_MULTI_MAX;
#endif /* LWBTN_CFG_TIME_CLICK_MULTI_MAX_DYNAMIC */
#if LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC
        btns[i].time_keepalive_period = LWBTN_CFG_TIME_KEEPALIVE_PERIOD;
#endif /* LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC */
#if LWBTN_CFG_CLICK_MAX_CONSECUTIVE_DYNAMIC
        btns[i].max_consecutive = LWBTN_CFG_CLICK_MAX_CONSECUTIVE;
#endif /* LWBTN_CFG_CLICK_MAX_CONSECUTIVE_DYNAMIC */
    }

    return 1;
}

/**
 * \brief           Button processing function, that reads the inputs and makes actions accordingly.
 * 
 * It checks state of all the buttons, linked to the specific LwBTN instance (group).
 * 
 * \param[in]       lwobj: LwBTN instance. Set to `NULL` to use default one
 * \param[in]       mstime: Current system time in milliseconds
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwbtn_process_ex(lwbtn_t* lwobj, lwbtn_time_t mstime) {
    lwobj = LWBTN_GET_LWOBJ(lwobj);

    /* Process all buttons */
    for (size_t index = 0; index < lwobj->btns_cnt; ++index) {
        prv_process_btn(lwobj, &lwobj->btns[index], mstime);
    }
    return 1;
}

/**
 * \brief           Process single button instance from the specific LwOBJ instance (group).
 * 
 * This feature can be used if application wants to process the button events only
 * when interrupt hits (as a trigger). It gives user higher autonomy to decide which 
 * and when it will call specific button processing.
 * 
 * \param[in]       lwobj: LwBTN instance. Set to `NULL` to use default one
 * \param[in]       btn: Button object. Must not be set to `NULL`
 * \param[in]       mstime: Current system time in milliseconds
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwbtn_process_btn_ex(lwbtn_t* lwobj, lwbtn_btn_t* btn, lwbtn_time_t mstime) {
    if (btn != NULL) {
        prv_process_btn(LWBTN_GET_LWOBJ(lwobj), btn, mstime);
        return 1;
    }
    return 0;
}

/**
 * \brief           Set button state to either "active" or "inactive".
 * \param[in]       btn: Button instance
 * \param[in]       state: New button state. `1` is for active (pressed), `0` is for inactive (released).
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwbtn_set_btn_state(lwbtn_btn_t* btn, uint8_t state) {
#if LWBTN_CFG_GET_STATE_MODE != LWBTN_GET_STATE_MODE_CALLBACK
    btn->curr_state = state;
    btn->flags |= LWBTN_FLAG_MANUAL_STATE;
    return 1;
#else  /* LWBTN_CFG_GET_STATE_MODE != LWBTN_GET_STATE_MODE_CALLBACK */
    (void)btn;
    (void)state;
    return 0;
#endif /* LWBTN_CFG_GET_STATE_MODE != LWBTN_GET_STATE_MODE_CALLBACK */
}

/**
 * \brief           Check if button is active.
 * Active is considered when initial debounce period has been a pass.
 * This is the period between on-press and on-release events.
 * 
 * \param[in]       btn: Button handle to check
 * \return          `1` if active, `0` otherwise
 */
uint8_t
lwbtn_is_btn_active(const lwbtn_btn_t* btn) {
    return btn != NULL && (btn->flags & LWBTN_FLAG_ONPRESS_SENT);
}

/**
 * \brief           Reset button[s] state to default
 * 
 *                  When reset is called, and if button is pressed (active),
 *                  no further events will be called up until we receive
 *                  the new valid press action.
 * 
 *                  A typical use case would be when application needs to react
 *                  on a long press (several keep alive events).
 *                  
 *                  Consider the use case where user runs the GUI and long press changes the screen.
 *                  The new screen has its own button handling method, which performs some other action (eg. "go home" action).
 *                  
 *                  Calling the reset function will prevent new screen to continue receiving the events
 *                  and forces the user to release the button and press it again.
 * 
 * \note            If button is reset during active time, there will be no further events
 *                  for this button sent to the application, up until a new valid on-press is detected
 * 
 * \param           lwobj: Object to reset buttons. Set to non-NULL to reset
 *                      all buttons in an object
 * \param           btn: Button object to reset. Optional parameter.
 *                      When non-NULL, button is reset
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwbtn_reset(lwbtn_t* lwobj, lwbtn_btn_t* btn) {
    for (size_t idx = 0; idx < (lwobj != NULL ? lwobj->btns_cnt : 0); ++idx) {
        lwobj->btns[idx].flags &= ~LWBTN_FLAG_FIRST_INACTIVE_RCVD;
    }
    if (btn != NULL) {
        btn->flags &= ~LWBTN_FLAG_FIRST_INACTIVE_RCVD;
    }
    return 1;
}
