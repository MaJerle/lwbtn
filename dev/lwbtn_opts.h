/**
 * \file            lwbtn_opts.h
 * \brief           lwbtn configuration file
 */

/*
 * Copyright (c) 2023 Tilen MAJERLE
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
#ifndef LWBTN_HDR_OPTS_H
#define LWBTN_HDR_OPTS_H

/* Rename this file to "lwbtn_opts.h" for your application */

/*
 * Open "include/lwbtn/lwbtn_opt.h" and
 * copy & replace here settings you want to change values
 */

/* Press config */
#define LWBTN_CFG_TIME_DEBOUNCE_PRESS 20 /* No debounce for press event */
#define LWBTN_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC                                                                          \
    1 /* Debounce for press event is statically set with macro -> no dynamic config */

/* Release config */
#define LWBTN_CFG_TIME_DEBOUNCE_RELEASE 20 /* No debounce for release event */
#define LWBTN_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC                                                                        \
    1 /* Debounce for release event is statically set with macro -> no dynamic config */

#endif /* LWBTN_HDR_OPTS_H */
