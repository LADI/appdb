/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * appdb - Application database via .desktop files
 *
 * Copyright (C) 2008-2023 Nedko Arnaudov
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *****************************************************************
 * This file contains interface to the application database code *
 *****************************************************************/

#ifndef APPDB_H__4839D031_68EF_43F5_BDE2_2317C6B956A9__INCLUDED
#define APPDB_H__4839D031_68EF_43F5_BDE2_2317C6B956A9__INCLUDED

#include "klist.h"

/* all strings except name can be not present (NULL) */
/* all strings are utf-8 */
struct lash_appdb_entry
{
  struct list_head siblings;
  char * name;    /* Specific name of the application, for example "Ingen" */
  char * generic_name;  /* Generic name of the application, for example "Audio Editor" */
  char * comment;   /* Tooltip for the entry, for example "Record and edit audio files" */
  char * icon;    /* Icon */
  char * exec;    /* Program to execute, possibly with arguments. */
  char * path;    /* The working directory to run the program in. */
  bool terminal;    /* Wheter to run application in terminal */
};

/* parses .desktop entries in suitable XDG directories and returns list of lash_appdb_entry structs in appdb parameter */
/* returns success status */
bool
lash_appdb_load(
  struct list_head * appdb);

/* free list of lash_appdb_entry structs, as returned by lash_appdb_load() */
void
lash_appdb_free(
  struct list_head * appdb);

#endif /* #ifndef APPDB_H__4839D031_68EF_43F5_BDE2_2317C6B956A9__INCLUDED */
