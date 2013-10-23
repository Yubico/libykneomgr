/*
 * Copyright (C) 2013 Yubico AB
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "internal.h"

#define ERR(name, desc) { name, #name, desc }

typedef struct
{
  int rc;
  const char *name;
  const char *description;
} err_t;

static const err_t errors[] = {
  ERR (YKNEOMGR_OK, "Successful return"),
  ERR (YKNEOMGR_MEMORY_ERROR, "Memory error (e.g., out of memory)"),
  ERR (YKNEOMGR_NO_DEVICE, "No device found"),
  ERR (YKNEOMGR_TOO_MANY_DEVICES, "Too many devices found"),
  ERR (YKNEOMGR_BACKEND_ERROR, "Backend error")
};

/**
 * ykneomgr_strerror:
 * @err: error code
 *
 * Convert return code to human readable string explanation of the
 * reason for the particular error code.
 *
 * This string can be used to output a diagnostic message to the user.
 *
 * This function is one of few in the library that can be used without
 * a successful call to ykneomgr_global_init().
 *
 * Return value: Returns a pointer to a statically allocated string
 *   containing an explanation of the error code @err.
 **/
const char *
ykneomgr_strerror (int err)
{
  static const char *unknown = "Unknown " PACKAGE_NAME " error";
  const char *p;

  if (-err < 0 || -err >= (int) (sizeof (errors) / sizeof (errors[0])))
    return unknown;

  p = errors[-err].description;
  if (!p)
    p = unknown;

  return p;
}


/**
 * ykneomgr_strerror_name:
 * @err: error code
 *
 * Convert return code to human readable string representing the error
 * code symbol itself.  For example, ykneomgr_strerror_name(%YKNEOMGR_OK)
 * returns the string "YKNEOMGR_OK".
 *
 * This string can be used to output a diagnostic message to the user.
 *
 * This function is one of few in the library that can be used without
 * a successful call to ykneomgr_global_init().
 *
 * Return value: Returns a pointer to a statically allocated string
 *   containing a string version of the error code @err, or NULL if
 *   the error code is not known.
 **/
const char *
ykneomgr_strerror_name (int err)
{
  if (-err < 0 || -err >= (int) (sizeof (errors) / sizeof (errors[0])))
    return NULL;

  return errors[-err].name;
}
