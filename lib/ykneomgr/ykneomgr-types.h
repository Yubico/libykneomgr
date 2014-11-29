/*
 * Copyright (C) 2013-2014 Yubico AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef YKNEOMGR_TYPES_H
#define YKNEOMGR_TYPES_H

/**
 * ykneomgr_rc:
 * @YKNEOMGR_OK: Success.
 * @YKNEOMGR_MEMORY_ERROR: Memory error.
 * @YKNEOMGR_NO_DEVICE: No device found.
 * @YKNEOMGR_TOO_MANY_DEVICES: Too many devices found.
 * @YKNEOMGR_BACKEND_ERROR: Input/Output error.
 * @YKNEOMGR_INCORRECT_KEY: Key error.
 *
 * Error codes.
 */
typedef enum
{
  YKNEOMGR_OK = 0,
  YKNEOMGR_MEMORY_ERROR = -1,
  YKNEOMGR_NO_DEVICE = -2,
  YKNEOMGR_TOO_MANY_DEVICES = -3,
  YKNEOMGR_BACKEND_ERROR = -4,
  YKNEOMGR_INCORRECT_KEY = -5,
} ykneomgr_rc;

/**
 * ykneomgr_initflags:
 * @YKNEOMGR_DEBUG: Print debug messages.
 *
 * Flags passed to ykneomgr_global_init().
 */
typedef enum
{
  YKNEOMGR_DEBUG = 1
} ykneomgr_initflags;


typedef struct ykneomgr_dev ykneomgr_dev;

#endif
