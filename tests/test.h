/**
 * \file            test.h
 * \author          Tilen MAJERLE <tilen@majerle.eu>
 * \brief
 * \version         0.1
 * \date            2025-03-21
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef TEST_HDR_H
#define TEST_HDR_H

#include "lwbtn/lwbtn.h"

/* Test assert information */
#define TEST_ASSERT(condition)                                                                                         \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            printf("Assert failed on the line %d\r\n", __LINE__);                                                      \
            rescode = -1;                                                                                              \
        }                                                                                                              \
    } while (0)

int test_run(void);

#endif /* TEST_HDR_H */
