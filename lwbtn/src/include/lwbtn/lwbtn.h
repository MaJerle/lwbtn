/**
 * \file            lwbtn.h
 * \brief           Lightweight button manager
 */

/*
 * Copyright (c) 2026 Tilen MAJERLE
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
 * Version:         v1.3.2
 */
#ifndef LWBTN_HDR_H
#define LWBTN_HDR_H

#include <stdint.h>
#include <string.h>
#include "lwbtn/lwbtn_opt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        LWBTN Lightweight button manager
 * \brief           Lightweight button manager
 * \{
 */

/**
 * \brief           Custom user argument data structure
 *
 * This is a simple pre-defined structure, that can be used by user
 * to define most commonly required feature in embedded systems, that being
 * GPIO port, GPIO pin and state when button is considered active.
 *
 * User can later attach this structure as argument to button structure
 */
typedef struct {
    void* port;    /*!< User defined GPIO port information */
    void* pin;     /*!< User defined GPIO pin information */
    uint8_t state; /*!< User defined GPIO state level when considered active */
} lwbtn_argdata_port_pin_state_t;

/* Forward declarations */
struct lwbtn_btn;
struct lwbtn;

/**
 * \brief           Time variable type
 */
typedef LWBTN_CFG_TIME_VARTYPE lwbtn_time_t;

/**
 * \brief           List of button events
 *
 */
typedef enum {
    LWBTN_EVT_ONPRESS = 0x00, /*!< On press event - sent when valid press is detected (after debounce if enabled) */
    LWBTN_EVT_ONRELEASE, /*!< On release event - sent when valid release event is detected (from active to inactive) */
#if LWBTN_CFG_USE_CLICK || __DOXYGEN__
    LWBTN_EVT_ONCLICK, /*!< On Click event - sent when valid sequence of on-press and on-release events occurs */
#endif                 /* LWBTN_CFG_USE_CLICK || __DOXYGEN__ */
#if LWBTN_CFG_USE_KEEPALIVE || __DOXYGEN__
    LWBTN_EVT_KEEPALIVE, /*!< Keep alive event - sent periodically when button is active */
#endif                   /* LWBTN_CFG_USE_KEEPALIVE || __DOXYGEN__ */
} lwbtn_evt_t;

/**
 * \brief           Button event function callback prototype
 * \param[in]       lwobj: LwBTN instance
 * \param[in]       btn: Button instance from array for which event occurred
 * \param[in]       evt: Event type
 */
typedef void (*lwbtn_evt_fn)(struct lwbtn* lwobj, struct lwbtn_btn* btn, lwbtn_evt_t evt);

/**
 * \brief           Get button/input state callback function
 * \param[in]       lwobj: LwBTN instance
 * \param[in]       btn: Button instance from array to read state
 * \return          `1` when button is considered `active`, `0` otherwise
 */
typedef uint8_t (*lwbtn_get_state_fn)(struct lwbtn* lwobj, struct lwbtn_btn* btn);

/**
 * \brief           Button/input structure
 */
typedef struct lwbtn_btn {
    uint16_t flags; /*!< Private button flags management */
#if LWBTN_CFG_GET_STATE_MODE != LWBTN_GET_STATE_MODE_CALLBACK || __DOXYGEN__
    uint8_t curr_state;             /*!< Current button state to be processed. It is used
                                    to keep track when application manually sets the button state */
#endif                              /* LWBTN_CFG_GET_STATE_MODE != LWBTN_GET_STATE_MODE_CALLBACK || __DOXYGEN__ */
    uint8_t last_state;             /*!< Last button state - `1` means active, `0` means inactive */
    lwbtn_time_t time_change;       /*!< Time in ms when button state got changed last time after valid debounce */
    lwbtn_time_t time_state_change; /*!< Time in ms when button state got changed last time */

#if LWBTN_CFG_USE_KEEPALIVE || __DOXYGEN__
    struct {
        lwbtn_time_t last_time; /*!< Time in ms of last send keep alive event */
        uint16_t cnt;           /*!< Number of keep alive events sent after successful on-press detection.
                                    Value is reset after on-release */
    } keepalive;                /*!< Keep alive structure */
#endif                          /* LWBTN_CFG_USE_KEEPALIVE || __DOXYGEN__ */

#if LWBTN_CFG_USE_CLICK || __DOXYGEN__
    struct {
        lwbtn_time_t last_time; /*!< Time in ms of last successfully detected (not sent!) click event */
        uint8_t cnt;            /*!< Number of consecutive clicks detected, respecting maximum timeout between clicks */
    } click;                    /*!< Click event structure */
#endif                          /* LWBTN_CFG_USE_CLICK || __DOXYGEN__ */

    void* arg; /*!< User defined custom argument for callback function purpose */

#if LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC || __DOXYGEN__
    lwbtn_time_t time_debounce; /*!< Debounce time in milliseconds */
#endif                          /* LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC || __DOXYGEN__ */
#if LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC || __DOXYGEN__
    lwbtn_time_t time_debounce_release; /*!< Debounce time in milliseconds for release event  */
#endif                                  /* LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC || __DOXYGEN__ */
#if LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC || __DOXYGEN__
    lwbtn_time_t time_keepalive_period; /*!< Time in ms for periodic keep alive event */
#endif                                  /* LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC || __DOXYGEN__ */

#if LWBTN_CFG_USE_CLICK || __DOXYGEN__
#if LWBTN_CFG_TIME_CLICK_MIN_DYNAMIC || __DOXYGEN__
    lwbtn_time_t time_click_pressed_min; /*!< Minimum pressed time for valid click event */
#endif                                   /* LWBTN_CFG_TIME_CLICK_MIN_DYNAMIC || __DOXYGEN__ */
#if LWBTN_CFG_TIME_CLICK_MAX_DYNAMIC || __DOXYGEN__
    lwbtn_time_t time_click_pressed_max; /*!< Maximum pressed time for valid click event*/
#endif                                   /* LWBTN_CFG_TIME_CLICK_MAX_DYNAMIC || __DOXYGEN__ */
#if LWBTN_CFG_TIME_CLICK_MULTI_MAX_DYNAMIC || __DOXYGEN__
    lwbtn_time_t time_click_multi_max; /*!< Maximum time between 2 clicks to be considered consecutive click */
#endif                                 /* LWBTN_CFG_TIME_CLICK_MULTI_MAX_DYNAMIC || __DOXYGEN__ */
#if LWBTN_CFG_CLICK_MAX_CONSECUTIVE_DYNAMIC || __DOXYGEN__
    uint8_t max_consecutive; /*!< Max number of consecutive clicks */
#endif                       /* LWBTN_CFG_CLICK_MAX_CONSECUTIVE_DYNAMIC || __DOXYGEN__ */
#endif                       /* LWBTN_CFG_USE_CLICK || __DOXYGEN__ */
} lwbtn_btn_t;

/**
 * \brief           LwBTN group structure
 */
typedef struct lwbtn {
    lwbtn_btn_t* btns;   /*!< Pointer to buttons array */
    uint16_t btns_cnt;   /*!< Number of buttons in array */
    lwbtn_evt_fn evt_fn; /*!< Pointer to event function */
#if LWBTN_CFG_GET_STATE_MODE != LWBTN_GET_STATE_MODE_MANUAL || __DOXYGEN__
    lwbtn_get_state_fn get_state_fn; /*!< Pointer to get state function */
#endif                               /* LWBTN_CFG_GET_STATE_MODE != LWBTN_GET_STATE_MODE_MANUAL || __DOXYGEN__ */
} lwbtn_t;

/* Core functions */
uint8_t lwbtn_init_ex(lwbtn_t* lwobj, lwbtn_btn_t* btns, uint16_t btns_cnt, lwbtn_get_state_fn get_state_fn,
                      lwbtn_evt_fn evt_fn);
uint8_t lwbtn_process_ex(lwbtn_t* lwobj, lwbtn_time_t mstime);
uint8_t lwbtn_process_btn_ex(lwbtn_t* lwobj, lwbtn_btn_t* btn, lwbtn_time_t mstime);
uint8_t lwbtn_reset(lwbtn_t* lwobj, lwbtn_btn_t* btn);
uint8_t lwbtn_is_btn_active(const lwbtn_btn_t* btn);

/* State function */
#if LWBTN_CFG_GET_STATE_MODE != LWBTN_GET_STATE_MODE_CALLBACK || __DOXYGEN__
uint8_t lwbtn_set_btn_state(lwbtn_btn_t* btn, uint8_t state);
#endif /* LWBTN_CFG_GET_STATE_MODE != LWBTN_GET_STATE_MODE_CALLBACK || __DOXYGEN__ */

/* Debounce configuration functions */
lwbtn_time_t lwbtn_debounce_get_press_time(const lwbtn_btn_t* btn);
lwbtn_time_t lwbtn_debounce_get_release_time(const lwbtn_btn_t* btn);
#if LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC || __DOXYGEN__
uint8_t lwbtn_debounce_set_press_time(lwbtn_btn_t* btn, lwbtn_time_t time);
#endif /* LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC || __DOXYGEN__ */
#if LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC || __DOXYGEN__
uint8_t lwbtn_debounce_set_release_time(lwbtn_btn_t* btn, lwbtn_time_t time);
#endif /* LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC || __DOXYGEN__ */

#if LWBTN_CFG_USE_CLICK || __DOXYGEN__

/* Click configuration functions */
lwbtn_time_t lwbtn_click_get_time_min(const lwbtn_btn_t* btn);
lwbtn_time_t lwbtn_click_get_time_max(const lwbtn_btn_t* btn);
lwbtn_time_t lwbtn_click_get_time_multi_max(const lwbtn_btn_t* btn);
uint8_t lwbtn_click_get_max_consecutive(const lwbtn_btn_t* btn);
#if LWBTN_CFG_TIME_CLICK_MIN_DYNAMIC || __DOXYGEN__
uint8_t lwbtn_click_set_time_min(lwbtn_btn_t* btn, lwbtn_time_t time);
#endif /* LWBTN_CFG_TIME_CLICK_MIN_DYNAMIC || __DOXYGEN__ */
#if LWBTN_CFG_TIME_CLICK_MAX_DYNAMIC || __DOXYGEN__
uint8_t lwbtn_click_set_time_max(lwbtn_btn_t* btn, lwbtn_time_t time);
#endif /* LWBTN_CFG_TIME_CLICK_MAX_DYNAMIC || __DOXYGEN__ */
#if LWBTN_CFG_TIME_CLICK_MULTI_MAX_DYNAMIC || __DOXYGEN__
uint8_t lwbtn_click_set_time_multi_max(lwbtn_btn_t* btn, lwbtn_time_t time);
#endif /* LWBTN_CFG_TIME_CLICK_MULTI_MAX_DYNAMIC || __DOXYGEN__ */
#if LWBTN_CFG_CLICK_MAX_CONSECUTIVE_DYNAMIC || __DOXYGEN__
uint8_t lwbtn_click_set_max_consecutive(lwbtn_btn_t* btn, uint8_t max);
#endif /* LWBTN_CFG_CLICK_MAX_CONSECUTIVE_DYNAMIC || __DOXYGEN__ */
uint8_t lwbtn_click_get_count(const lwbtn_btn_t* btn);

#endif /* LWBTN_CFG_USE_CLICK || __DOXYGEN__ */

/**
 * \brief           Initialize LwBTN library with buttons on default button group
 * \param[in]       btns: Array of buttons to process
 * \param[in]       btns_cnt: Number of buttons to process
 * \param[in]       get_state_fn: Pointer to function providing button state on demand.
 *                      Can be set to `NULL` if \ref LWBTN_CFG_GET_STATE_MODE is NOT set to \ref
 * LWBTN_GET_STATE_MODE_CALLBACK
 * \param[in]       evt_fn: Button event function callback
 * \sa              lwbtn_init_ex
 */
#define lwbtn_init(btns, btns_cnt, get_state_fn, evt_fn) lwbtn_init_ex(NULL, btns, btns_cnt, get_state_fn, evt_fn)

/**
 * \brief           Periodically read button states and take appropriate actions.
 * It processes the default buttons instance group.
 * \param[in]       mstime: Current system time in milliseconds
 * \sa              lwbtn_process_ex
 */
#define lwbtn_process(mstime)                            lwbtn_process_ex(NULL, mstime)

/**
 * \brief           Process specific button in a default LwBTN instance
 *
 * \param[in]       btn: Button instance to process
 * \param[in]       mstime: Current system time in milliseconds
 */
#define lwbtn_process_btn(btn, mstime)                   lwbtn_process_btn_ex(NULL, (btn), (mstime))

/* Keep alive functions */
#if LWBTN_CFG_USE_KEEPALIVE || __DOXYGEN__

lwbtn_time_t lwbtn_keepalive_get_period(const lwbtn_btn_t* btn);
#if LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC || __DOXYGEN__
uint8_t lwbtn_keepalive_set_period(lwbtn_btn_t* btn, lwbtn_time_t period);
#endif /* LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC || __DOXYGEN__ */
uint16_t lwbtn_keepalive_get_count(const lwbtn_btn_t* btn);

/**
 * \brief           Get number of keep alive counts for specific required time in milliseconds.
 *                  It will calculate number of keepalive ticks specific button shall make,
 *                  before requested time is reached.
 *
 * Result of the function can be used with \ref lwbtn_keepalive_get_count which returns
 * actual number of keep alive counts since last on-press event of the button.
 *
 * \note            Available only when \ref LWBTN_CFG_USE_KEEPALIVE feature is enabled
 * \note            Value is always integer aligned, with granularity of one keepalive time period
 * \note            Implemented as macro to avoid the function-call overhead for a simple derived calculation
 *
 * \param[in]       btn: Button to use for check
 * \param[in]       ms_time: Time in ms to calculate number of keep alive counts
 * \return          Number of keep alive counts
 * \sa              lwbtn_keepalive_get_count
 */
#define lwbtn_keepalive_get_count_for_time(btn, ms_time) ((ms_time) / lwbtn_keepalive_get_period(btn))

#endif /* LWBTN_CFG_USE_KEEPALIVE || __DOXYGEN__ */

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWBTN_HDR_H */
