/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * appdb - Application database via .desktop files
 *
 * SPDX-FileCopyrightText: Copyright © 2008-2023 Nedko Arnaudov
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *******************************************************
 * This file contains code of the application database *
 *******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include "common.h"
#include "appdb/appdb.h"
#include "log.h"
#include "catdup.h"
#include "assert.h"

static
void
appdb_free_entry(
  struct appdb_entry * entry_ptr);

#define MAP_TYPE_STRING  0
#define MAP_TYPE_BOOL    1

struct appdb_map
{
  const char * key;
  unsigned int type;
  size_t offset;
};

static struct appdb_map g_appdb_entry_map[] =
{
  {
    .key = "Name",
    .type = MAP_TYPE_STRING,
    .offset = offsetof(struct appdb_entry, name)
  },
  {
    .key = "GenericName",
    .type = MAP_TYPE_STRING,
    .offset = offsetof(struct appdb_entry, generic_name)
  },
  {
    .key = "Comment",
    .type = MAP_TYPE_STRING,
    .offset = offsetof(struct appdb_entry, comment)
  },
  {
    .key = "Icon",
    .type = MAP_TYPE_STRING,
    .offset = offsetof(struct appdb_entry, icon)
  },
  {
    .key = "Exec",
    .type = MAP_TYPE_STRING,
    .offset = offsetof(struct appdb_entry, exec)
  },
  {
    .key = "Path",
    .type = MAP_TYPE_STRING,
    .offset = offsetof(struct appdb_entry, path)
  },
  {
    .key = "Terminal",
    .type = MAP_TYPE_BOOL,
    .offset = offsetof(struct appdb_entry, terminal)
  },
  {
    .key = NULL,
  }
};

struct appdb_kv_entry
{
  const char * key;
  const char * value;
};

#define MAX_ENTRIES 1000

static
const char *
appdb_get_xdg_var(
  const char * var_name,
  const char * default_value)
{
  const char * value;

  value = getenv(var_name);

  /* Spec says that if variable is "either not set or empty", default should be used */
  if (value == NULL || strlen(value) == 0)
  {
    return default_value;
  }

  return value;
}

static
bool
appdb_suffix_match(
  const char * string,
  const char * suffix)
{
  size_t len;
  size_t len_suffix;

  len = strlen(string);
  len_suffix = strlen(suffix);

  if (len <= len_suffix)
  {
    return false;
  }

  if (memcmp(string + (len - len_suffix), suffix, len_suffix) != 0)
  {
    return false;
  }

  return true;
}

static
bool
appdb_load_file_data(
  const char * file_path,
  char ** data_ptr_ptr)
{
  FILE * file;
  long size;
  bool ret;
  char * data_ptr;

  ret = false;
  *data_ptr_ptr = NULL;

  file = fopen(file_path, "r");
  if (file == NULL)
  {
    log_error("Failed to open '%s' for reading", file_path);
    goto exit;
  }

  if (fseek(file, 0, SEEK_END) == -1)
  {
    log_error("fseek('%s') failed", file_path);
    goto exit_close;
  }

  size = ftell(file);
  if (size == -1)
  {
    log_error("ftell('%s') failed", file_path);
    goto exit_close;
  }

  data_ptr = malloc(size + 1);
  if (data_ptr == NULL)
  {
    log_error("Failed to allocate %ld bytes for data of file '%s'", size + 1, file_path);
    ret = false;
    goto exit_close;
  }

  if (fseek(file, 0, SEEK_SET) == -1)
  {
    log_error("fseek('%s') failed", file_path);
    goto exit_close;
  }

  if (fread(data_ptr, size, 1, file) != 1)
  {
    log_error("Failed to read %ld bytes of data from file '%s'", size, file_path);
    goto exit_free_data;
  }

  data_ptr[size] = 0;

  *data_ptr_ptr = data_ptr;

  ret = true;
  goto exit_close;

exit_free_data:
  free(data_ptr);

exit_close:
  fclose(file);

exit:
  return ret;
}

static
char *
appdb_strlstrip(char * string)
{
  while (*string == ' ' || *string == '\t')
  {
    string++;
  }

  return string;
}

static
void
appdb_strrstrip(char * string)
{
  char * temp;

  temp = string + strlen(string);

  while (temp > string)
  {
    temp--;

    if (*temp == ' ' || *temp == '\t')
    {
      *temp = 0;
    }
  }
}

static
bool
appdb_parse_file_data(
  char * data,
  struct appdb_kv_entry * entries_array,
  size_t max_count,
  size_t * count_ptr)
{
  char * line;
  char * next_line;
  char * value;
  bool group_found;
  size_t count;

  group_found = false;
  line = data;
  count = 0;

  do
  {
    next_line = strchr(line, '\n');
    if (next_line != NULL)
    {
      //log_info("there is next line");
      *next_line = 0;
      next_line++;
    }

    //log_info("Line '%s'", line);

    /* skip comments (and empty lines) */
    if (*line == 0 || *line == '#')
    {
      continue;
    }

    if (!group_found)
    {
      /* first real line should be begining of "Desktop Entry" group */
      if (strcmp(line, "[Desktop Entry]") != 0)
      {
        return false;
      }

      group_found = true;
      continue;
    }

    value = strchr(line, '=');
    if (value == NULL)
    {
      goto exit;
    }

    *value = 0;
    value++;

    /* strip spaces */
    appdb_strrstrip(line);
    value = appdb_strlstrip(value);

    //log_info("    Key=%s", line);
    //log_info("    Value=%s", value);

    if (count + 1 == max_count)
    {
      log_error("failed to parse desktop entry with more than %u keys", (unsigned int)max_count);
      return false;
    }

    entries_array->key = line;
    entries_array->value = value;
    entries_array++;
    count++;
  }
  while ((line = next_line) != NULL);

exit:
  *count_ptr = count;

  return group_found;
}

static
const char *
appdb_find_key(
  struct appdb_kv_entry * entries,
  size_t count,
  const char * key)
{
  size_t i;

  for (i = 0 ; i < count ; i++)
  {
    if (strcmp(entries[i].key, key) == 0)
    {
      return entries[i].value;
    }
  }

  return NULL;
}

static
bool
appdb_load_file(
  struct list_head * appdb,
  const char * file_path)
{
  char * data;
  bool ret;
  struct appdb_kv_entry entries[MAX_ENTRIES];
  size_t entries_count;
  const char * value;
  const char * name;
  const char * xlash;
  struct list_head * node_ptr;
  struct appdb_entry * entry_ptr;
  struct appdb_map * map_ptr;
  char ** str_ptr_ptr;
  bool * bool_ptr;

  //log_info("=========================");
  //log_info("Desktop entry '%s'", file_path);

  ret = true;

  if (!appdb_load_file_data(file_path, &data))
  {
    ret = false;
    goto exit;
  }

  if (data == NULL)
  {
    goto exit;
  }

  if (!appdb_parse_file_data(data, entries, MAX_ENTRIES, &entries_count))
  {
    goto exit_free_data;
  }

  //log_info("%llu entries", (unsigned long long)entries_count);

  /* check whether entry is of "Application" type */
  value = appdb_find_key(entries, entries_count, "Type");
  if (value == NULL || strcmp(value, "Application") != 0)
  {
    goto exit_free_data;
  }

  /* check whether "Name" is preset, it is required */
  name = appdb_find_key(entries, entries_count, "Name");
  if (name == NULL)
  {
    goto exit_free_data;
  }

  /* check whether entry has LIBLASH or LASHCLASS key */
  xlash = appdb_find_key(entries, entries_count, "X-LASH");
  if (xlash == NULL)
  {
    //goto exit_free_data;
  }

  /* check whether entry already exists (first found entries have priority according to XDG Base Directory Specification) */
  list_for_each(node_ptr, appdb)
  {
    entry_ptr = list_entry(node_ptr, struct appdb_entry, siblings);

    if (strcmp(entry_ptr->name, name) == 0)
    {
      goto exit_free_data;
    }
  }

  log_info("Application '%s' found", name);

  /* allocate new entry */
  entry_ptr = malloc(sizeof(struct appdb_entry));
  if (entry_ptr == NULL)
  {
    log_error("malloc() failed");
    goto fail_free_data;
  }

  memset(entry_ptr, 0, sizeof(struct appdb_entry));

  /* fill the entry */
  map_ptr = g_appdb_entry_map;
  while (map_ptr->key != NULL)
  {
    value = appdb_find_key(entries, entries_count, map_ptr->key);
    if (value == NULL)
    {
      ASSERT(strcmp(map_ptr->key, "Name") != 0); /* name is required and we already checked this */
      map_ptr++;  
      continue;
    }

    //log_info("mapping key '%s' to '%s'", map_ptr->key, value);

    if (map_ptr->type == MAP_TYPE_STRING)
    {
      str_ptr_ptr = (char **)((char *)entry_ptr + map_ptr->offset);
      *str_ptr_ptr = strdup(value);
      if (*str_ptr_ptr == NULL)
      {
        log_error("strdup() failed");
        goto fail_free_entry;
      }
    }
    else if (map_ptr->type == MAP_TYPE_BOOL)
    {
      bool_ptr = (bool *)((char *)entry_ptr + map_ptr->offset);
      if (strcmp(value, "true") == 0)
      {
        *bool_ptr = true;
      }
      else if (strcmp(value, "false") == 0)
      {
        *bool_ptr = false;
      }
      else
      {
        log_error("Ignoring %s:%s bool with wrong value '%s'", name, map_ptr->key, value);
      }
    }
    else
    {
      ASSERT_NO_PASS;
      goto fail_free_entry;
    }

    map_ptr++;
  }

  /* add entry to appdb list */
  list_add_tail(&entry_ptr->siblings, appdb);

  goto exit_free_data;

fail_free_entry:
  appdb_free_entry(entry_ptr);

fail_free_data:
  ret = false;

exit_free_data:
  free(data);

exit:
  return ret;
}

static
bool
appdb_load_dir(
  struct list_head * appdb,
  const char * base_directory)
{
  char * directory_path;
  bool ret;
  DIR * dir;
  struct dirent * dentry_ptr;
  char * file_path;

  //log_info("appdb_load_dir() called for '%s'.", base_directory);

  ret = false;

  directory_path = catdup(base_directory, "/applications/");
  if (directory_path == NULL)
  {
    log_error("catdup() failed to compose the appdb dir path");
    goto fail;
  }

  //log_info("Scanning directory '%s'", directory_path);

  dir = opendir(directory_path);
  if (dir != NULL)
  {
    while ((dentry_ptr = readdir(dir)) != NULL)
    {
      if (dentry_ptr->d_type != DT_REG)
      {
        continue;
      }

      if (!appdb_suffix_match(dentry_ptr->d_name, ".desktop"))
      {
        continue;
      }

      file_path = catdup(directory_path, dentry_ptr->d_name);
      if (file_path == NULL)
      {
        log_error("catdup() failed to compose the appdb dir file");
      }
      else
      {
        if (!appdb_load_file(appdb, file_path))
        {
          free(file_path);
          goto fail_free_path;
        }

        free(file_path);
      }
    }

    closedir(dir);
  }
  else
  {
    //log_info("failed to open directory '%s'", directory_path);
  }

  ret = true;

fail_free_path:
  free(directory_path);

fail:
  return ret;
}

bool
appdb_load_dirs(
  struct list_head * appdb,
  const char * base_directories)
{
  char * limiter;
  char * directory;
  char * directories;

  directories = strdup(base_directories);
  if (directories == NULL)
  {
    log_error("strdup() failed");
    return false;
  }

  directory = directories;

  do
  {
    limiter = strchr(directory, ':');
    if (limiter != NULL)
    {
      *limiter = 0;
    }

    if (!appdb_load_dir(appdb, directory))
    {
      free(directories);
      return false;
    }

    directory = limiter + 1;
  }
  while (limiter != NULL);

  free(directories);

  return true;
}

bool
appdb_load(
  struct list_head * appdb)
{
  const char * data_home;
  char * data_home_default;
  const char * data_dirs;
  const char * home_dir;
  bool ret;

  ret = false;

  INIT_LIST_HEAD(appdb);

  //log_info("appdb_load() called.");

  home_dir = getenv("HOME");
  if (home_dir == NULL)
  {
    log_error("HOME environment variable is not set.");
    goto fail;
  }

  data_home_default = catdup(home_dir, "/.local/share");
  if (data_home_default == NULL)
  {
    log_error("catdup failed to compose data_home_default");
    goto fail;
  }

  data_home = appdb_get_xdg_var("XDG_DATA_HOME", data_home_default);

  if (!appdb_load_dir(appdb, data_home))
  {
    goto fail_free_data_home_default;
  }

  data_dirs = appdb_get_xdg_var("XDG_DATA_DIRS", "/usr/local/share/:/usr/share/");

  if (!appdb_load_dirs(appdb, data_dirs))
  {
    goto fail_free_data_home_default;
  }

  ret = true;

fail_free_data_home_default:
  free(data_home_default);

fail:
  if (!ret)
  {
    appdb_free(appdb);
  }

  return ret;
}

static
void
appdb_free_entry(
  struct appdb_entry * entry_ptr)
{
  //log_info("appdb_free_entry() called.");

  if (entry_ptr->name != NULL)
  {
    free(entry_ptr->name);
  }

  if (entry_ptr->generic_name != NULL)
  {
    free(entry_ptr->generic_name);
  }

  if (entry_ptr->comment != NULL)
  {
    free(entry_ptr->comment);
  }

  if (entry_ptr->icon != NULL)
  {
    free(entry_ptr->icon);
  }

  if (entry_ptr->exec != NULL)
  {
    free(entry_ptr->exec);
  }

  if (entry_ptr->path != NULL)
  {
    free(entry_ptr->path);
  }

  free(entry_ptr);
}

void
appdb_free(
  struct list_head * appdb)
{
  struct list_head * node_ptr;
  struct appdb_entry * entry_ptr;

  //log_info("appdb_free() called.");

  while (!list_empty(appdb))
  {
    node_ptr = appdb->next;
    entry_ptr = list_entry(node_ptr, struct appdb_entry, siblings);

    list_del(node_ptr);

    //log_info("Destroying appdb entry '%s'", entry_ptr->name);

    appdb_free_entry(entry_ptr);
  }
}
