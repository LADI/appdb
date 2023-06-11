/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * appdb - Application database via .desktop files
 *
 * Copyright (C) 2009-2023 Nedko Arnaudov
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 **************************************************************
 * This file contains implementation of the catdup() function *
 **************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include "catdup.h"
#include <assert.h>
#include "assert.h"

char * catdup(const char * s1, const char * s2)
{
  char * buffer;
  size_t s1_len, s2_len;

  ASSERT(s1 != NULL && s2 != NULL);

  s1_len = strlen(s1);
  s2_len = strlen(s2);

  buffer = malloc(s1_len + s2_len + 1);
  if (buffer == NULL)
  {
    log_error("malloc() failed.");
    return NULL;
  }

  memcpy(buffer, s1, s1_len);
  memcpy(buffer + s1_len, s2, s2_len);
  buffer[s1_len + s2_len] = 0;

  return buffer;
}

char * catdup3(const char * s1, const char * s2, const char * s3)
{
  char * buffer;
  size_t s1_len, s2_len, s3_len;

  ASSERT(s1 != NULL && s2 != NULL && s3 != NULL);

  s1_len = strlen(s1);
  s2_len = strlen(s2);
  s3_len = strlen(s3);

  buffer = malloc(s1_len + s2_len + s3_len + 1);
  if (buffer == NULL)
  {
    log_error("malloc() failed.");
    return NULL;
  }

  memcpy(buffer, s1, s1_len);
  memcpy(buffer + s1_len, s2, s2_len);
  memcpy(buffer + s1_len + s2_len, s3, s3_len);
  buffer[s1_len + s2_len + s3_len] = 0;

  return buffer;
}

char * catdup4(const char * s1, const char * s2, const char * s3, const char * s4)
{
  char * buffer;
  size_t s1_len, s2_len, s3_len, s4_len;

  ASSERT(s1 != NULL && s2 != NULL && s3 != NULL && s4 != NULL);

  s1_len = strlen(s1);
  s2_len = strlen(s2);
  s3_len = strlen(s3);
  s4_len = strlen(s4);

  buffer = malloc(s1_len + s2_len + s3_len + s4_len + 1);
  if (buffer == NULL)
  {
    log_error("malloc() failed.");
    return NULL;
  }

  memcpy(buffer, s1, s1_len);
  memcpy(buffer + s1_len, s2, s2_len);
  memcpy(buffer + s1_len + s2_len, s3, s3_len);
  memcpy(buffer + s1_len + s2_len + s3_len, s4, s4_len);
  buffer[s1_len + s2_len + s3_len + s4_len] = 0;

  return buffer;
}

char * catdupv(const char * s1, const char * s2, ...)
{
  va_list ap;
  const char * str;
  size_t len;
  char * buffer;
  char * p;

  len = strlen(s1) + strlen(s2);
  va_start(ap, s2);
  while ((str = va_arg(ap, const char *)) != NULL)
  {
    len += strlen(str);
  }
  va_end(ap);
  len++;

  buffer = malloc(len);
  if (buffer == NULL)
  {
    log_error("malloc(%zu) failed.", len);
    return NULL;
  }

  len = strlen(s1);
  memcpy(buffer, s1, len);
  p = buffer + len;

  len = strlen(s2);
  memcpy(p, s2, len);
  p += len;

  va_start(ap, s2);
  while ((str = va_arg(ap, const char *)) != NULL)
  {
    len = strlen(str);
    memcpy(p, str, len);
    p += len;
  }
  va_end(ap);

  *p = 0;

  return buffer;
}

char * catdup_array(const char ** array, const char * delimiter)
{
  size_t len;
  size_t i;
  size_t delimiter_length;
  char * buffer;
  char * p;

  delimiter_length = delimiter != NULL ? strlen(delimiter) : 0;

  len = 0;
  for (i = 0; array[i] != NULL; i++)
  {
    len += strlen(array[i]);
    len += delimiter_length;
  }
  len++;

  buffer = malloc(len);
  if (buffer == NULL)
  {
    log_error("malloc(%zu) failed.", len);
    return NULL;
  }

  p = buffer;
  for (i = 0; array[i] != NULL; i++)
  {
    len = strlen(array[i]);
    memcpy(p, array[i], len);
    p += len;
    if (delimiter_length > 0)
    {
      memcpy(p, delimiter, delimiter_length);
      p += delimiter_length;
    }
  }

  *p = 0;

  return buffer;
}
