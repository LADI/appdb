/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * appdb - Application database via .desktop files
 *
 * Copyright (C) 2009-2023 Nedko Arnaudov
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ************************************
 * This file contains assert macros *
 ************************************/

#ifndef ASSERT_H__F236CB4F_D812_4636_958C_C82FD3781EC8__INCLUDED
#define ASSERT_H__F236CB4F_D812_4636_958C_C82FD3781EC8__INCLUDED

#include "log.h"

#include <assert.h>


#define ASSERT(expr)                                                  \
  do                                                                  \
  {                                                                   \
    if (!(expr))                                                      \
    {                                                                 \
      log_error("ASSERT(" #expr ") failed. function %s in %s:%4u\n",  \
                __FUNCTION__,                                         \
                __FILE__,                                             \
                __LINE__);                                            \
      assert(false);                                                  \
    }                                                                 \
  }                                                                   \
  while(false)

#define ASSERT_NO_PASS                                                \
  do                                                                  \
  {                                                                   \
    log_error("Code execution taboo point. function %s in %s:%4u\n",  \
              __FUNCTION__,                                           \
              __FILE__,                                               \
              __LINE__);                                              \
    assert(false);                                                    \
  }                                                                   \
  while(false)

#endif /* #ifndef ASSERT_H__F236CB4F_D812_4636_958C_C82FD3781EC8__INCLUDED */
