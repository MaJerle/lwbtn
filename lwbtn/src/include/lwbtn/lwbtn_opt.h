/**
 * \file            lwbtn_opt.h
 * \brief           LwBTN options
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
 * \brief           Memory set function
 * 
 * \note            Function footprint is the same as \ref memset
 */
#ifndef LWBTN_MEMSET
#define LWBTN_MEMSET(dst, val, len) memset((dst), (val), (len))
#endif

/**
 * \brief           Memory copy function
 * 
 * \note            Function footprint is the same as \ref memcpy
 */
#ifndef LWBTN_MEMCPY
#define LWBTN_MEMCPY(dst, src, len) memcpy((dst), (src), (len))
#endif

/**
 * \brief           Enables `1` or disables `0` periodic keep alive events.
 * 
 * Keep alive events are periodically sent to the application,
 * when input is kept in active state.s
 * 
 * Default keep alive period is set with \ref LWBTN_CFG_TIME_KEEPALIVE_PERIOD macro
 */
#ifndef LWBTN_CFG_USE_KEEPALIVE
#define LWBTN_CFG_USE_KEEPALIVE 1
#endif

/**
 * \brief           Minimum debounce time for press event in units of milliseconds
 * 
 * This is the time when the input shall have stable active level to detect valid *onpress* event.
 * 
 * When value is set to `> 0`, input must be in active state for at least
 * minimum milliseconds time, before valid *onpress* event is detected.
 * 
 * \note            If value is set to `0`, debounce is not used and *press* event will be triggered
 *                  immediately when input states goes to *inactive* state.
 *  
 *                  To be safe not using this feature, external logic must ensure stable transition at input level.
 * 
 * \sa              LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC
 */
#ifndef LWBTN_CFG_TIME_DEBOUNCE_PRESS
#define LWBTN_CFG_TIME_DEBOUNCE_PRESS 20
#endif

/**
 * \brief           Enables `1` or disables `0` dynamic settable time debounce
 * 
 * When enabled, additional field is added to button structure to allow
 * each button setting its very own debounce time for press event.
 * 
 * If not used, \ref LWBTN_CFG_TIME_DEBOUNCE_PRESS is used as default
 * debouncing configuration
 */
#ifndef LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC
#define LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC 0
#endif

/**
 * \brief           Minimum debounce time for release event in units of milliseconds
 * 
 * This is the time when the input shall have minimum stable released level to detect valid *onrelease* event.
 * 
 * This setting can be useful if application wants to protect against
 * unwanted glitches on the line when input is considered "active".
 * 
 * When value is set to `> 0`, input must be in inactive low for at least
 * minimum milliseconds time, before valid *onrelease* event is detected
 * 
 * \note            If value is set to `0`, debounce is not used and *release* event will be triggered
 *                  immediately when input states goes to *inactive* state
 * 
 * \sa              LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC
 */
#ifndef LWBTN_CFG_TIME_DEBOUNCE_RELEASE
#define LWBTN_CFG_TIME_DEBOUNCE_RELEASE 0
#endif

/**
 * \brief           Enables `1` or disables `0` dynamic settable time debounce for release event
 * 
 * When enabled, additional field is added to button structure to allow
 * each button setting its very own debounce time for release event.
 * 
 * If not used, \ref LWBTN_CFG_TIME_DEBOUNCE_RELEASE is used as default
 * debouncing configuration
 */
#ifndef LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC
#define LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC 0
#endif

/**
 * \brief           Minimum active input time for valid click event, in milliseconds
 * 
 * Input shall be in active state (after debounce) at least this amount of time to even consider the potential valid click event.
 * Set the value to `0` to disable this feature
 * 
 * \sa              LWBTN_CFG_TIME_CLICK_MIN_DYNAMIC
 */
#ifndef LWBTN_CFG_TIME_CLICK_MIN
#define LWBTN_CFG_TIME_CLICK_MIN 20
#endif

/**
 * \brief           Enables `1` or disables `0` dynamic settable min time for click
 * 
 * When enabled, additional field is added to button structure,
 * allowing application to manually set min pressed time for click for each button instance.
 * Default value is set to \ref LWBTN_CFG_TIME_CLICK_MIN
 */
#ifndef LWBTN_CFG_TIME_CLICK_MIN_DYNAMIC
#define LWBTN_CFG_TIME_CLICK_MIN_DYNAMIC 0
#endif

/**
 * \brief           Maximum active input time for valid click event, in milliseconds
 * 
 * Input shall be pressed at most this amount of time to still trigger valid click.
 * Set to `-1` to allow any time triggering click event.
 * 
 * When input is active for more than the configured time, click even is not detected and is ignored.
 * 
 * \sa              LWBTN_CFG_TIME_CLICK_MAX_DYNAMIC
 */
#ifndef LWBTN_CFG_TIME_CLICK_MAX
#define LWBTN_CFG_TIME_CLICK_MAX 300
#endif

/**
 * \brief           Enables `1` or disables `0` dynamic settable max time for click
 * 
 * When enabled, additional field is added to button structure,
 * allowing application to manually set max pressed time for click for each button instance.
 * Default value is set to \ref LWBTN_CFG_TIME_CLICK_MAX
 */
#ifndef LWBTN_CFG_TIME_CLICK_MAX_DYNAMIC
#define LWBTN_CFG_TIME_CLICK_MAX_DYNAMIC 0
#endif

/**
 * \brief           Maximum allowed time between last on-release and next valid on-press,
 *                  to still allow multi-click events, in milliseconds
 * 
 * This value is also used as a timeout length to send the *onclick* event to application from
 * previously detected valid click events.
 * 
 * If application relies on multi consecutive clicks, this is the max time to allow user
 * to trigger potential new click, or structure will get reset (before sent to user if any clicks have been detected so far)
 * 
 * \sa              LWBTN_CFG_TIME_CLICK_MULTI_MAX_DYNAMIC
 */
#ifndef LWBTN_CFG_TIME_CLICK_MULTI_MAX
#define LWBTN_CFG_TIME_CLICK_MULTI_MAX 400
#endif

/**
 * \brief           Enables `1` or disables `0` dynamic settable max time for multi click
 * 
 * When enabled, additional field is added to button structure,
 * allowing application to manually set max time for multi click for each button instance.
 * Default value is set to \ref LWBTN_CFG_TIME_CLICK_MULTI_MAX
 */
#ifndef LWBTN_CFG_TIME_CLICK_MULTI_MAX_DYNAMIC
#define LWBTN_CFG_TIME_CLICK_MULTI_MAX_DYNAMIC 0
#endif

/**
 * \brief           Maximum number of allowed consecutive click events,
 *                  before structure gets reset to default value.
 * 
 * \note            When consecutive value is reached, application will get notification of clicks.
 *                  This can be executed immediately after last click has been detected,
 *                  or after standard timeout (unless next on-press has already been detected,
 *                  then it is send to application just before valid next press event).
 *                  Configuration can be changed with \ref LWBTN_CFG_CLICK_MAX_CONSECUTIVE_SEND_IMMEDIATELY macro
 * 
 * \sa              LWBTN_CFG_CLICK_MAX_CONSECUTIVE_DYNAMIC, LWBTN_CFG_CLICK_MAX_CONSECUTIVE_SEND_IMMEDIATELY
 */
#ifndef LWBTN_CFG_CLICK_MAX_CONSECUTIVE
#define LWBTN_CFG_CLICK_MAX_CONSECUTIVE 3
#endif

/**
 * \brief           Enables `1` or disables `0` dynamic settable max consecutive clicks
 * 
 * When enabled, additional field is added to button structure,
 * allowing application to manually set max consecutive clicks for each button instance.
 * Default value is set to \ref LWBTN_CFG_CLICK_MAX_CONSECUTIVE
 */
#ifndef LWBTN_CFG_CLICK_MAX_CONSECUTIVE_DYNAMIC
#define LWBTN_CFG_CLICK_MAX_CONSECUTIVE_DYNAMIC 0
#endif

/**
 * \brief           Keep-alive event period, in milliseconds
 * 
 * When input is active, keep alive events will be sent through this period of time.
 * First keep alive will be sent \ref LWBTN_CFG_TIME_KEEPALIVE_PERIOD ms after input being considered active.
 * 
 * \sa              LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC
 */
#ifndef LWBTN_CFG_TIME_KEEPALIVE_PERIOD
#define LWBTN_CFG_TIME_KEEPALIVE_PERIOD 100
#endif

/**
 * \brief           Enables `1` or disables `0` dynamic settable keep alive period
 * 
 * When enabled, additional field is added to button structure,
 * allowing application to manually set keep alive period for each button instance.
 * Default value is set to \ref LWBTN_CFG_TIME_KEEPALIVE_PERIOD
 */
#ifndef LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC
#define LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC 0
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

/**
 * \brief           Get button state options
 * \name            LWBTN_CFG_GET_STATE_MODE_GROUP
 * \anchor          LWBTN_CFG_GET_STATE_MODE_GROUP
 * \{
 * 
 * Configuration option for \ref LWBTN_CFG_GET_STATE_MODE configuration
 */

#define LWBTN_GET_STATE_MODE_CALLBACK           0 /*!< Callback-only state mode */
#define LWBTN_GET_STATE_MODE_MANUAL             1 /*!< Manual-only state mode */
#define LWBTN_GET_STATE_MODE_CALLBACK_OR_MANUAL 2 /*!< Callback or manual state mode */

/**
 * \}
 */

/**
 * \brief           Sets the mode how new button state is acquired.
 * 
 * Different modes are availale, set with the level number:
 * 
 * - `LWBTN_GET_STATE_MODE_CALLBACK`: State of the button is checked through *get state* callback function
 * - `LWBTN_GET_STATE_MODE_MANUAL`: Only manual state set is enabled. Application must set the button state with API functions.
 *          Callback API is not used.
 * - `LWBTN_GET_STATE_MODE_CALLBACK_OR_MANUAL`: State of the button is checked through *get state* callback function (by default).
 *          It enables API to manually set the state with approapriate function call.
 *          Button state is checked with the callback at least until manual state API function is called.
 * 
 *          This allows multiple build configurations for various button types
 */
#ifndef LWBTN_CFG_GET_STATE_MODE
#define LWBTN_CFG_GET_STATE_MODE LWBTN_GET_STATE_MODE_CALLBACK
#endif

/**
 * \brief           Enables `1` or disables `0` keeping the consecutive click event group if last
 *                  sequence of *onpress* and *onrelease* was too short.
 * 
 */
#ifndef LWBTN_CFG_CLICK_CONSECUTIVE_KEEP_AFTER_SHORT_PRESS
#define LWBTN_CFG_CLICK_CONSECUTIVE_KEEP_AFTER_SHORT_PRESS 0
#endif

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWBTN_OPT_HDR_H */
