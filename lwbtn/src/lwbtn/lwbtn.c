/**
 * \file            lwbtn.c
 * \brief           Lightweight event system
 */

/*
 * Copyright (c) 2022 Tilen MAJERLE
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
 * Version:         v0.0.0
 */
#include <string.h>
#include "lwbtn/lwbtn.h"

#define LWBTN_FLAG_ONPRESS_SENT               ((uint16_t)0x0001)

#define LWBTN_TIME_DEBOUNCE_GET_MIN(btn)      20  /*!< Minimum debounce time to stabilize input */
#define LWBTN_TIME_CLICK_GET_PRESSED_MIN(btn) 20  /*!< Minimum pressed time to consider it as a potential click event */
#define LWBTN_TIME_CLICK_GET_PRESSED_MAX(btn) 300 /*!< Maximal pressed time to consider "click" event */
#define LWBTN_TIME_CLICK_MAX_MULTI(btn)       500 /*!< Maximum allowed time for user to handle consecutive click events */
#define LWBTN_TIME_KEEPALIVE_PERIOD(btn)      100 /*!< Keep alive time period in milliseconds */
#define LWBTN_TIME_CLICK_SEND_TIMEOUT(btn)    300 /*!< Minimum time in ms to wait to send click event in timeout */
#define LWBTN_CLICK_MAX_CONSECUTIVE(btn)      3   /*!< Max consecutive clicks allowed */

/* Default button group instance */
static lwbtn_t lwbtn_default;
#define LWBTN_GET_LW(in_lw) ((in_lw) != NULL ? (in_lw) : (&lwbtn_default))

/**
 * \brief           Initialize button manager
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwbtn_init_ex(lwbtn_t* lw, lwbtn_btn_t* btns, uint16_t btns_cnt, lwbtn_get_state_fn get_state_fn, lwbtn_evt_fn evt_fn) {
    lw = LWBTN_GET_LW(lw);

    memset(lw, 0x00, sizeof(*lw));
    lw->btns = btns;
    lw->btns_cnt = btns_cnt;
    lw->evt_fn = evt_fn;
    lw->get_state_fn = get_state_fn;

    return 1;
}

/**
 * \brief           Button processing function,
 * that reads the inputs and makes actions accordingly
 * \param[in]       mstime: Current time in milliseconds
 */
uint8_t
lwbtn_process_ex(lwbtn_t* lw, uint32_t mstime) {
    lwbtn_btn_t* b;
    uint8_t new_state;

    lw = LWBTN_GET_LW(lw);

    /* Process all buttons */
    for (size_t i = 0; i < lw->btns_cnt; ++i) {
        b = &lw->btns[i];

        new_state = lw->get_state_fn(lw, b); /* Get button state */

        /*
         * Button state has changed
         */
        if (new_state != b->old_state) {
            /*
             * Button just became inactive
             *
             * - Handle on-release event
             * - Handle on-click event
             */
            if (!new_state) {
                /*
                 * We only need to react if on-press event has even been started.
                 *
                 * Do nothing if that was not the case
                 */
                if (b->flags & LWBTN_FLAG_ONPRESS_SENT) {
                    /* Handle on-release event */
                    b->flags &= ~LWBTN_FLAG_ONPRESS_SENT;
                    lw->evt_fn(lw, b, LWBTN_EVT_ONRELEASE);

                    /* Check time validity for click event */
                    if ((mstime - b->time_change) >= LWBTN_TIME_CLICK_GET_PRESSED_MIN(b)
                        && (mstime - b->time_change) <= LWBTN_TIME_CLICK_GET_PRESSED_MAX(b)) {

                        /*
                         * Increase consecutive clicks if max not reached yet
                         * and if time between two clicks is not too long
                         */
                        if (b->click.cnt > 0 && b->click.cnt < LWBTN_CLICK_MAX_CONSECUTIVE(b)
                            && (mstime - b->click.last_time) < LWBTN_TIME_CLICK_MAX_MULTI(b)) {
                            ++b->click.cnt;
                        } else {
                            /* Start over - set as first click */
                            b->click.cnt = 1;
                        }
                        b->click.last_time = mstime;
                    } else {
                        /* TODO: Shall we check previous clicks and send to user (??) */

                        /*
                         * Button released with invalid click event
                         * 
                         * TODO: Consider sending all data immediately from previous clicks?
                         * 
                         * Clear then counter
                         */
                        b->click.cnt = 0;
                    }
                }
            }

            /*
             * Button just pressed
             */
            else {
                /* Do nothing - things are handled after debounce period */
            }
            b->time_change = mstime;
            b->keepalive.last_time = mstime;
            b->keepalive.cnt = 0;
        }

        /*
         * Button is still pressed
         */
        else if (new_state) {
            /* 
             * Handle debounce and send on-press event
             *
             * This is when we detect valid press
             */
            if (!(b->flags & LWBTN_FLAG_ONPRESS_SENT)) {
                /* Check minimum stable time */
                if ((mstime - b->time_change) >= LWBTN_TIME_DEBOUNCE_GET_MIN(b)) {
                    /*
                     * Immediately send click event if number of 
                     * previous consecutive clicks reached maximum level.
                     * 
                     * Handle this on on-press state
                     */
                    if (b->click.cnt > 0 && b->click.cnt == LWBTN_CLICK_MAX_CONSECUTIVE(b)) {
                        lw->evt_fn(lw, b, LWBTN_EVT_ONCLICK);
                        b->click.cnt = 0;
                    }

                    /* Now start with new on-press */
                    b->flags |= LWBTN_FLAG_ONPRESS_SENT;
                    lw->evt_fn(lw, b, LWBTN_EVT_ONPRESS);

                    /* Set keep alive time */
                    b->keepalive.last_time = mstime;
                }
            }
            /*
             * Handle keep alive, but only if on-press event has been sent
             *
             * Keep alive is sent when valid press is being detected
             */
            else {
                if ((mstime - b->keepalive.last_time) >= LWBTN_TIME_KEEPALIVE_PERIOD(b)) {
                    b->keepalive.last_time += LWBTN_TIME_KEEPALIVE_PERIOD(b);
                    ++b->keepalive.cnt;
                    lw->evt_fn(lw, b, LWBTN_EVT_KEEPALIVE);
                }
            }
        }

        /*
         * Button is still released
         */
        else {
            /*
             * This is the part to process "timeout" after release event
             *
             * - Send click event with delay - to send it once for multi-click setup
             *      This is sent after last valid click has been detected for OnRelease event
             * - Reset consecutive clicks counter if time after last click is longer than maximum one
             */
            if (b->click.cnt > 0) {
                if ((mstime - b->click.last_time) >= LWBTN_TIME_CLICK_SEND_TIMEOUT(b)) {
                    lw->evt_fn(lw, b, LWBTN_EVT_ONCLICK);
                    b->click.cnt = 0;
                }
            }
        }
        b->old_state = new_state;
    }
    return 1;
}
