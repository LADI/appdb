/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * appdb - Application database via .desktop files
 *
 * Copyright (C) 2008-2023 Nedko Arnaudov
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ***********************************************
 * This file contains code of the appdb daemon *
 ***********************************************/

#include <stdlib.h>
#include <signal.h>
#include <string.h>
//#include <sys/stat.h>

#include <cdbus/cdbus.h>

#include "common.h"

bool g_quit;
const char * g_dbus_unique_name;
//cdbus_object_path g_control_object;

static bool connect_dbus(void)
{
  int ret;

  dbus_error_init(&cdbus_g_dbus_error);

  cdbus_g_dbus_connection = dbus_bus_get(DBUS_BUS_SESSION, &cdbus_g_dbus_error);
  if (dbus_error_is_set(&cdbus_g_dbus_error))
  {
    log_error("Failed to get bus: %s", cdbus_g_dbus_error.message);
    dbus_error_free(&cdbus_g_dbus_error);
    goto fail;
  }

  g_dbus_unique_name = dbus_bus_get_unique_name(cdbus_g_dbus_connection);
  if (g_dbus_unique_name == NULL)
  {
    log_error("Failed to read unique bus name");
    goto unref_connection;
  }

  log_info("Connected to local session bus, unique name is \"%s\"", g_dbus_unique_name);

  ret = dbus_bus_request_name(
    cdbus_g_dbus_connection,
    APPDB_DBUS_SERVICE_NAME,
    DBUS_NAME_FLAG_DO_NOT_QUEUE,
    &cdbus_g_dbus_error);
  if (ret == -1)
  {
    log_error("Failed to acquire bus name: %s", cdbus_g_dbus_error.message);
    dbus_error_free(&cdbus_g_dbus_error);
    goto unref_connection;
  }

  if (ret == DBUS_REQUEST_NAME_REPLY_EXISTS)
  {
    log_error("Requested connection name already exists");
    goto unref_connection;
  }

#if 0
  g_control_object = cdbus_object_path_new(CONTROL_OBJECT_PATH, &g_lashd_interface_control, NULL, NULL);
  if (g_control_object == NULL)
  {
    goto unref_connection;
  }

  if (!cdbus_object_path_register(cdbus_g_dbus_connection, g_control_object))
  {
    goto destroy_control_object;
  }
#endif

  return true;

#if 0
destroy_control_object:
  cdbus_object_path_destroy(cdbus_g_dbus_connection, g_control_object);
#endif
unref_connection:
  dbus_connection_unref(cdbus_g_dbus_connection);

fail:
  return false;
}

static void disconnect_dbus(void)
{
  //cdbus_object_path_destroy(cdbus_g_dbus_connection, g_control_object);
  dbus_connection_unref(cdbus_g_dbus_connection);
  cdbus_call_last_error_cleanup();
}

void term_signal_handler(int signum)
{
  log_info("Caught signal %d (%s), terminating", signum, strsignal(signum));
  g_quit = true;
}

bool install_term_signal_handler(int signum, bool ignore_if_already_ignored)
{
  sig_t sigh;

  sigh = signal(signum, term_signal_handler);
  if (sigh == SIG_ERR)
  {
    log_error("signal() failed to install handler function for signal %d.", signum);
    return false;
  }

  if (sigh == SIG_IGN && ignore_if_already_ignored)
  {
    signal(SIGTERM, SIG_IGN);
  }

  return true;
}

int main(int UNUSED(argc), char ** UNUSED(argv))
{
  int ret;
  struct list_head apps_list;

  ret = EXIT_FAILURE;

  if (!appdb_load(&apps_list))
  {
    log_error("Loading of appdb failed");
    goto exit;
  }

  if (!connect_dbus())
  {
    log_error("Failed to connecto to D-Bus");
    goto free_appdb;
  }

  while (!g_quit)
  {
    dbus_connection_read_write_dispatch(cdbus_g_dbus_connection, 200);
  }

  ret = EXIT_SUCCESS;

//uninit_dbus:
  disconnect_dbus();

free_appdb:
  appdb_free(&apps_list);
exit:
  return ret;
}
