/**
 * \file            lwbtn.h
 * \brief           Lightweight button manager
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
 * \brief           User argument data structure
 * 
 * This is structure user can use to define
 * GPIO port, pin and potential active state (high or low)
 */
typedef struct {
    void* port;    /*!< User defined GPIO port information */
    void* pin;     /*!< User defined GPIO pin information */
    uint8_t state; /*!< User defined GPIO state level when considered active */
} lwbtn_argdata_port_pin_state_t;

struct lwbtn_btn;
struct lwbtn;

/**
 * \brief           List of button events
 * 
 */
typedef enum {
    LWBTN_EVT_ONPRESS = 0x00,
    LWBTN_EVT_ONRELEASE,
    LWBTN_EVT_ONCLICK,
    LWBTN_EVT_KEEPALIVE,
} lwbtn_evt_t;

/**
 * \brief           Button event function callback prototype
 * \param[in]       lw: LwBTN instance
 * \param[in]       btn: Button instance from array for which event occured
 * \param[in]       evt: Event type
 */
typedef void (*lwbtn_evt_fn)(struct lwbtn* lw, struct lwbtn_btn* btn, lwbtn_evt_t evt);

/**
 * \brief           Get button/input state callback function
 * \param[in]       lw: LwBTN instance
 * \param[in]       btn: Button instance from array to read state
 * \return          `1` when button is considered `active`, `0` otherwise
 */
typedef uint8_t (*lwbtn_get_state_fn)(struct lwbtn* lw, struct lwbtn_btn* btn);

/**
 * \brief           Button/input structure
 */
typedef struct lwbtn_btn {
    uint16_t flags;       /*!< Private button flags management */
    uint8_t old_state;    /*!< Old button state - `1` means active, `0` means inactive */
    uint32_t time_change; /*!< Time in ms when button state got changed last time */

    struct {
        uint32_t last_time; /*!< Time in ms of last send keep alive event */
        uint16_t cnt;       /*!< Number of keep alive events sent after successful on-press detection.
                                    Value is reset after on-release */
    } keepalive;            /*!< Keep alive structure */

    struct {
        uint32_t last_time; /*!< Time in ms of last successfully detected (not sent!) click event */
        uint8_t cnt;        /*!< Number of consecutive clicks detected, respecting maximum timeout between clicks */
    } click;                /*!< Click event structure */

    void* arg; /*!< User defined custom argument for callback function purpose */
} lwbtn_btn_t;

/**
 * \brief           LwBTN group structure
 */
typedef struct lwbtn {
    lwbtn_btn_t* btns;               /*!< Pointer to buttons array */
    uint16_t btns_cnt;               /*!< Number of buttons in array */
    lwbtn_evt_fn evt_fn;             /*!< Pointer to event function */
    lwbtn_get_state_fn get_state_fn; /*!< Pointer to get state function */
} lwbtn_t;

uint8_t lwbtn_init_ex(lwbtn_t* lw, lwbtn_btn_t* btns, uint16_t btns_cnt, lwbtn_get_state_fn get_state_fn,
                      lwbtn_evt_fn evt_fn);
uint8_t lwbtn_process_ex(lwbtn_t* lw, uint32_t mstime);

/**
 * \brief           Initialize LwBTN library with buttons on default button group
 * \param[in]       btns: Array of buttons to process
 * \param[in]       btns_cnt: Number of buttons to process
 * \param[in]       get_state_fn: Pointer to function providing button state on demand
 * \param[in]       evt_fn: Button event function callback
 * \sa              lwbtn_init_ex
 */
#define lwbtn_init(btns, btns_cnt, get_state_fn, evt_fn) lwbtn_init_ex(NULL, btns, btns_cnt, get_state_fn, evt_fn)

/**
 * \brief           Periodically read button states and take appropriate actions
 * \param[in]       mstime: Current system time in milliseconds
 * \sa              lwbtn_process_ex
 */
#define lwbtn_process(mstime)                            lwbtn_process_ex(NULL, mstime)

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWBTN_HDR_H */
