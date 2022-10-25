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

typedef void (*lwbtn_evt_fn)(struct lwbtn* lw, struct lwbtn_btn* btn, lwbtn_evt_t evt);
typedef uint8_t (*lwbtn_get_state_fn)(struct lwbtn* lw, struct lwbtn_btn* btn);

/**
 * \brief           Button/input structure
 */
typedef struct lwbtn_btn {
    uint16_t flags;
    uint8_t old_state;
    uint32_t time_change;

    struct {
        uint32_t last_time;
        uint16_t cnt;
    } keepalive;

    struct {
        uint32_t last_time;
        uint8_t consecutive_cnt;
    } click;

    void* arg;
} lwbtn_btn_t;

/**
 * \brief           LwBTN group structure
 */
typedef struct lwbtn {
    lwbtn_btn_t* btns;
    uint16_t btns_cnt;
    lwbtn_evt_fn evt_fn;
    lwbtn_get_state_fn get_state_fn;
} lwbtn_t;

uint8_t lwbtn_init_ex(lwbtn_t* lw, lwbtn_btn_t* btns, uint16_t btns_cnt, lwbtn_get_state_fn get_state_fn,
                      lwbtn_evt_fn evt_fn);
uint8_t lwbtn_process_ex(lwbtn_t* lw, uint32_t mstime);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWBTN_HDR_H */
