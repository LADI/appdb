/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * appdb - Application database via .desktop files
 *
 * Copyright (C) 2008-2023 Nedko Arnaudov
 * Copyright (C) 2008 Marc-Olivier Barre
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *****************************************************************
 * This file contains code that implements logging functionality *
 *****************************************************************/

#include "config.h"

#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "log.h"
#include "catdup.h"
//#include "dirhelpers.h"

#define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#define ANSI_BOLD_ON    "\033[1m"
#define ANSI_BOLD_OFF   "\033[22m"
#define ANSI_COLOR_RED  "\033[31m"
#define ANSI_COLOR_YELLOW "\033[33m"
#define ANSI_RESET      "\033[0m"

#define LOG_OUTPUT_STDOUT

#define DEFAULT_XDG_LOG "/.log"
#define APPDB_XDG_SUBDIR "/" BASE_NAME
#define APPDB_XDG_LOG "/" BASE_NAME ".log"

#if !defined(LOG_OUTPUT_STDOUT)
static ino_t g_log_file_ino;
static FILE * g_logfile;
static char * g_log_filename;

static bool appdb_log_open(void)
{
    struct stat st;
    int ret;
    int retry;

    if (g_logfile != NULL)
    {
        ret = stat(g_log_filename, &st);
        if (ret != 0 || g_log_file_ino != st.st_ino)
        {
            fclose(g_logfile);
        }
        else
        {
            return true;
        }
    }

    for (retry = 0; retry < 10; retry++)
    {
        g_logfile = fopen(g_log_filename, "a");
        if (g_logfile == NULL)
        {
            fprintf(stderr, "Cannot open appdbd log file \"%s\": %d (%s)\n", g_log_filename, errno, strerror(errno));
            return false;
        }

        ret = stat(g_log_filename, &st);
        if (ret == 0)
        {
            g_log_file_ino = st.st_ino;
            return true;
        }

        fclose(g_logfile);
        g_logfile = NULL;
    }

    fprintf(stderr, "Cannot stat just opened appdbd log file \"%s\": %d (%s). %d retries\n", g_log_filename, errno, strerror(errno), retry);
    return false;
}

void appdb_log_init() __attribute__ ((constructor));
void appdb_log_init()
{
  char * appdb_log_dir;
  const char * home_dir;
  char * xdg_log_home;

  home_dir = getenv("HOME");
  if (home_dir == NULL)
  {
    log_error("Environment variable HOME not set");
    goto exit;
  }

  xdg_log_home = catdup(home_dir, DEFAULT_XDG_LOG);
  if (xdg_log_home == NULL)
  {
    log_error("catdup failed for '%s' and '%s'", home_dir, DEFAULT_XDG_LOG);
    goto exit;
  }

  appdb_log_dir = catdup(xdg_log_home, APPDB_XDG_SUBDIR);
  if (appdb_log_dir == NULL)
  {
    log_error("catdup failed for '%s' and '%s'", home_dir, APPDB_XDG_SUBDIR);
    goto free_log_home;
  }

  if (!ensure_dir_exist(xdg_log_home, 0700))
  {
    goto free_log_dir;
  }

  if (!ensure_dir_exist(appdb_log_dir, 0700))
  {
    goto free_log_dir;
  }

  g_log_filename = catdup(appdb_log_dir, APPDB_XDG_LOG);
  if (g_log_filename == NULL)
  {
    log_error("Out of memory");
    goto free_log_dir;
  }

  appdb_log_open();
  //cdbus_log_setup(appdb_log);

free_log_dir:
  free(appdb_log_dir);

free_log_home:
  free(xdg_log_home);

exit:
  return;
}

void appdb_log_uninit()  __attribute__ ((destructor));
void appdb_log_uninit()
{
  if (g_logfile != NULL)
  {
    fclose(g_logfile);
  }

  free(g_log_filename);
}
#else
void appdb_log_init() __attribute__ ((constructor));
void appdb_log_init()
{
  //cdbus_log_setup(appdb_log);
}
#endif  /* #if !defined(LOG_OUTPUT_STDOUT) */

#if 0
# define log_debug(fmt, args...) appdb_log(LOG_LEVEL_DEBUG, "%s:%d:%s: " fmt "\n", __FILE__, __LINE__, __func__, ## args)
# define log_info(fmt, args...) appdb_log(LOG_LEVEL_INFO, fmt "\n", ## args)
# define log_warn(fmt, args...) appdb_log(LOG_LEVEL_WARN, ANSI_COLOR_YELLOW "WARNING: " ANSI_RESET "%s: " fmt "\n", __func__, ## args)
# define log_error(fmt, args...) appdb_log(LOG_LEVEL_ERROR, ANSI_COLOR_RED "ERROR: " ANSI_RESET "%s: " fmt "\n", __func__, ## args)
# define log_error_plain(fmt, args...) appdb_log(LOG_LEVEL_ERROR_PLAIN, ANSI_COLOR_RED "ERROR: " ANSI_RESET fmt "\n", ## args)
#endif

static
bool
appdb_log_enabled(
  unsigned int level,
  const char * UNUSED(file),
  unsigned int UNUSED(line),
  const char * UNUSED(func))
{
  return level != LOG_LEVEL_DEBUG;
}

void
appdb_log(
  unsigned int level,
  const char * file,
  unsigned int line,
  const char * func,
  const char * format,
  ...)
{
  va_list ap;
  FILE * stream;
#if !defined(LOG_OUTPUT_STDOUT)
  time_t timestamp;
  char timestamp_str[26];
#endif
  const char * color;

  if (!appdb_log_enabled(level, file, line, func))
  {
    return;
  }

#if !defined(LOG_OUTPUT_STDOUT)
  if (g_logfile != NULL && appdb_log_open())
  {
    stream = g_logfile;
  }
  else
#endif
  {
    switch (level)
    {
    case LOG_LEVEL_DEBUG:
    case LOG_LEVEL_INFO:
      stream = stdout;
      break;
    case LOG_LEVEL_WARN:
    case LOG_LEVEL_ERROR:
    case LOG_LEVEL_ERROR_PLAIN:
    default:
      stream = stderr;
    }
  }

#if !defined(LOG_OUTPUT_STDOUT)
  time(&timestamp);
  ctime_r(&timestamp, timestamp_str);
  timestamp_str[24] = 0;

  fprintf(stream, "%s: ", timestamp_str);
#endif

  color = NULL;
  switch (level)
  {
  case LOG_LEVEL_DEBUG:
    fprintf(stream, "%s:%d:%s ", file, line, func);
    break;
  case LOG_LEVEL_WARN:
    color = ANSI_COLOR_YELLOW;
    break;
  case LOG_LEVEL_ERROR:
  case LOG_LEVEL_ERROR_PLAIN:
    color = ANSI_COLOR_RED;
    break;
  }

  if (color != NULL)
  {
    fputs(color, stream);
  }

  va_start(ap, format);
  vfprintf(stream, format, ap);
  va_end(ap);

  if (color != NULL)
  {
    fputs(ANSI_RESET, stream);
  }

  fputs("\n", stream);

  fflush(stream);
}
