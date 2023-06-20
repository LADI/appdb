/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * appdb - Application database via .desktop files
 *
 * Copyright (C) 2023 Nedko Arnaudov
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ***************************
 * internal reusable stuff *
 ***************************/

#ifndef COMMON_H__BD287362_4EAE_4DB0_BAB4_5EE0FEED86E4__INCLUDED
#define COMMON_H__BD287362_4EAE_4DB0_BAB4_5EE0FEED86E4__INCLUDED

#include <stdbool.h>

#include "log.h"
#include "appdb/klist.h"
#include "appdb/appdb.h"

#define UNUSED(x) UNUSED_ ## x __attribute__((unused))

#define APPDB_DBUS_SERVICE_NAME "org.ladish.appdb"

#endif /* #ifndef COMMON_H__BD287362_4EAE_4DB0_BAB4_5EE0FEED86E4__INCLUDED */
