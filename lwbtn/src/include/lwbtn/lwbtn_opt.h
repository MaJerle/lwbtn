/**
 * \file            lwbtn_opt.h
 * \brief           LwBTN options
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
 * Version:         v0.0.1
 */
#ifndef LWBTN_OPT_HDR_H
#define LWBTN_OPT_HDR_H

/* Uncomment to ignore user options (or set macro in compiler flags) */
/* #define LWBTN_IGNORE_USER_OPTS */

/* Include application options */
#ifndef LWBTN_IGNORE_USER_OPTS
#include "lwbtn_opts.h"
#endif /* lwbtn_IGNORE_USER_OPTS */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        LWBTN_OPT Configuration
 * \brief           Default configuration setup
 * \{
 */

/**
 * \brief           Enables `1` or disables `0` periodic keep alive events.
 * 
 * Default keep alive period is set with \ref LWBTN_CFG_TIME_KEEPALIVE_PERIOD macro
 */
#ifndef LWBTN_CFG_USE_KEEPALIVE
#define LWBTN_CFG_USE_KEEPALIVE 1
#endif

/**
 * \brief           Minimum debounce time in units of milliseconds
 * 
 * This is the time input shall have stable level to detect valid *onpress* event
 */
#ifndef LWBTN_CFG_TIME_DEBOUNCE
#define LWBTN_CFG_TIME_DEBOUNCE 20
#endif

/**
 * \brief           Minimum active input time for valid click event, in milliseconds
 * 
 * Input shall be pressed at least this amount of time to even consider the potential valid click event.
 * Set the value to `0` to disable this feature
 */
#ifndef LWBTN_CFG_TIME_CLICK_MIN
#define LWBTN_CFG_TIME_CLICK_MIN 20
#endif

/**
 * \brief           Maximum active input time for valid click event, in milliseconds
 * 
 * Input shall be pressed at most this amount of time to still trigger valid click.
 * Set to `-1` to allow any time triggering click event.
 */
#ifndef LWBTN_CFG_TIME_CLICK_MAX
#define LWBTN_CFG_TIME_CLICK_MAX 300
#endif

/**
 * \brief           Maximum allowed time between last on-release and next valid on-press,
 *                  to still allow multi-click events, in milliseconds
 * 
 * This value is also used as a timeout length. It sends *onclick* event if there
 * is no further presses by the application.
 */
#ifndef LWBTN_CFG_TIME_CLICK_MULTI_MAX
#define LWBTN_CFG_TIME_CLICK_MULTI_MAX 400
#endif

/**
 * \brief           Maximum number of allowed consecutive click events,
 *                  before structure gets reset to default value
 */
#ifndef LWBTN_CFG_CLICK_MAX_CONSECUTIVE
#define LWBTN_CFG_CLICK_MAX_CONSECUTIVE 3
#endif

/**
 * \brief           Keep-alive event period, in milliseconds
 * 
 */
#ifndef LWBTN_CFG_TIME_KEEPALIVE_PERIOD
#define LWBTN_CFG_TIME_KEEPALIVE_PERIOD 100
#endif

/**
 * \brief           Enables `1` or disables `0` immediate onclick event 
 *                  after on-release event, if number of consecutive
 *                  clicks reaches max value.
 * 
 * When this mode is disabled, onclick is sent in one of 2 cases:
 * 
 * - An on-click timeout occurred
 * - Next on-press event occurred before timeout expired
 */
#ifndef LWBTN_CFG_CLICK_MAX_CONSECUTIVE_SEND_IMMEDIATELY
#define LWBTN_CFG_CLICK_MAX_CONSECUTIVE_SEND_IMMEDIATELY 1
#endif

#define LWBTN_GET_STATE_MODE_CALLBACK           0 /*!< Callback-only state mode */
#define LWBTN_GET_STATE_MODE_MANUAL             1 /*!< Manual-only state mode */
#define LWBTN_GET_STATE_MODE_CALLBACK_OR_MANUAL 2 /*!< Callback or manual state mode */

/**
 * \brief           Sets the mode how new button state is acquired.
 * 
 * Different modes are availale, set with the level number:
 * 
 * - `LWBTN_GET_STATE_MODE_CALLBACK`: State of the button is checked through *get state* callback function (default mode, legacy)
 * - `LWBTN_GET_STATE_MODE_MANUAL`: Only manual state set is enabled. Application must set the button state with API functions.
 *          Callback API is not used.
 * - `LWBTN_GET_STATE_MODE_CALLBACK_OR_MANUAL`: State of the button is checked through *get state* callback function (by default).
 *          It enables API to manually set the state with approapriate function call.
 *          Button state is checked with the callback at least until manual state API function is called.
 * 
 *      This allows multiple build configurations for various button types
 */
#ifndef LWBTN_CFG_GET_STATE_MODE
#define LWBTN_CFG_GET_STATE_MODE LWBTN_GET_STATE_MODE_CALLBACK
#endif

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWBTN_OPT_HDR_H */
