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

#include "internal.h"

int debug;

/**
 * ykneomgr_global_init:
 * @flags: initialization flags, ORed #ykneomgr_initflags.
 *
 * Initialize the library.  This function is not guaranteed to be
 * thread safe and must be invoked on application startup.
 *
 * Returns: On success %YKNEOMGR_OK (integer 0) is returned, and on errors
 * an #ykneomgr_rc error code.
 */
ykneomgr_rc
ykneomgr_global_init (ykneomgr_initflags flags)
{
  if (flags & YKNEOMGR_DEBUG)
    {
#if BACKEND_GLOBALPLATFORM
      OPGP_enable_trace_mode (1, stdout);
#endif
      debug = 1;
    }

  return YKNEOMGR_OK;
}

/**
 * ykneomgr_global_done:
 *
 * Release all resources from the library.  Call this function when no
 * further use of the library is needed.
 */
void
ykneomgr_global_done (void)
{
  debug = 0;
}
