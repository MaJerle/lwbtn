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
 * Version:         v0.0.0
 */
#ifndef LWBTN_HDR_OPT_H
#define LWBTN_HDR_OPT_H

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
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWBTN_HDR_OPT_H */
