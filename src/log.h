/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * appdb - Application database via .desktop files
 *
 * Copyright (C) 2023 Nedko Arnaudov
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 **************
 * log macros *
 **************/

#ifndef LOG_H__8338C154_E498_4F4F_A357_44C15EBEE750__INCLUDED
#define LOG_H__8338C154_E498_4F4F_A357_44C15EBEE750__INCLUDED

#include "config.h"

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

#define LOG_LEVEL_DEBUG        0
#define LOG_LEVEL_INFO         1
#define LOG_LEVEL_WARN         2
#define LOG_LEVEL_ERROR        3
#define LOG_LEVEL_ERROR_PLAIN  4

#define log_debug(fmt, args...)       \
  appdb_log(LOG_LEVEL_DEBUG,       __FILE__, __LINE__, __FUNCTION__, fmt, ## args)
#define log_info(fmt, args...)        \
  appdb_log(LOG_LEVEL_INFO,        __FILE__, __LINE__, __FUNCTION__, fmt, ## args)
#define log_warn(fmt, args...)        \
  appdb_log(LOG_LEVEL_WARN,        __FILE__, __LINE__, __FUNCTION__, fmt, ## args)
#define log_error(fmt, args...)       \
  appdb_log(LOG_LEVEL_ERROR,       __FILE__, __LINE__, __FUNCTION__, fmt, ## args)
#define log_error_plain(fmt, args...) \
  appdb_log(LOG_LEVEL_ERROR_PLAIN, __FILE__, __LINE__, __FUNCTION__, fmt, ## args)

#endif /* #ifndef LOG_H__8338C154_E498_4F4F_A357_44C15EBEE750__INCLUDED */
