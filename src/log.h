/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * appdb - Application database via .desktop files
 *
 * Copyright (C) 2023 Nedko Arnaudov
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *********************************
 * This file contains log macros *
 *********************************/

#ifndef LOG_H__8338C154_E498_4F4F_A357_44C15EBEE750__INCLUDED
#define LOG_H__8338C154_E498_4F4F_A357_44C15EBEE750__INCLUDED

#define ANSI_BOLD_ON    "\033[1m"
#define ANSI_BOLD_OFF   "\033[22m"
#define ANSI_COLOR_RED  "\033[31m"
#define ANSI_COLOR_YELLOW "\033[33m"
#define ANSI_RESET      "\033[0m"

#include <stdio.h>

#include "config.h"

/* fallback for old gcc versions,
   http://gcc.gnu.org/onlinedocs/gcc/Function-Names.html */
#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif

#ifdef __cplusplus
extern "C"
#endif
void
appdb_log(
  unsigned int level,
  const char * file,
  unsigned int line,
  const char * func,
  const char * format,
  ...)
#if defined (__GNUC__)
  __attribute__((format(printf, 5, 6)))
#endif
  ;

#define APPDB_LOG_LEVEL_DEBUG        0
#define APPDB_LOG_LEVEL_INFO         1
#define APPDB_LOG_LEVEL_WARN         2
#define APPDB_LOG_LEVEL_ERROR        3
#define APPDB_LOG_LEVEL_ERROR_PLAIN  4

#define log_debug(fmt, args...)       appdb_log(APPDB_LOG_LEVEL_DEBUG,       __FILE__, __LINE__, __func__, fmt, ## args)
#define log_info(fmt, args...)        appdb_log(APPDB_LOG_LEVEL_INFO,        __FILE__, __LINE__, __func__, fmt, ## args)
#define log_warn(fmt, args...)        appdb_log(APPDB_LOG_LEVEL_WARN,        __FILE__, __LINE__, __func__, fmt, ## args)
#define log_error(fmt, args...)       appdb_log(APPDB_LOG_LEVEL_ERROR,       __FILE__, __LINE__, __func__, fmt, ## args)
#define log_error_plain(fmt, args...) appdb_log(APPDB_LOG_LEVEL_ERROR_PLAIN, __FILE__, __LINE__, __func__, fmt, ## args)

#endif /* #ifndef LOG_H__8338C154_E498_4F4F_A357_44C15EBEE750__INCLUDED */
