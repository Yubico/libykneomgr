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

#ifndef YKNEOMGR_H
#define YKNEOMGR_H

#include <stdint.h>
#include <string.h>

#include <ykneomgr/ykneomgr-version.h>
#include <ykneomgr/ykneomgr-types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Must be called successfully before using any other functions. */
  extern ykneomgr_rc ykneomgr_global_init (ykneomgr_initflags flags);
  extern void ykneomgr_global_done (void);

  extern const char *ykneomgr_strerror (int err);
  extern const char *ykneomgr_strerror_name (int err);

  extern ykneomgr_rc ykneomgr_init (ykneomgr_dev ** dev);
  extern void ykneomgr_done (ykneomgr_dev * dev);

  extern ykneomgr_rc ykneomgr_list_devices (ykneomgr_dev * dev,
					    char *devicestr, size_t * len);

  extern ykneomgr_rc ykneomgr_connect (ykneomgr_dev * dev, const char *name);
  extern ykneomgr_rc ykneomgr_discover (ykneomgr_dev * dev);

  extern uint8_t ykneomgr_get_version_major (ykneomgr_dev * dev);
  extern uint8_t ykneomgr_get_version_minor (ykneomgr_dev * dev);
  extern uint8_t ykneomgr_get_version_build (ykneomgr_dev * dev);
  extern uint8_t ykneomgr_get_mode (ykneomgr_dev * dev);
  extern uint32_t ykneomgr_get_serialno (ykneomgr_dev * dev);

  extern ykneomgr_rc ykneomgr_modeswitch (ykneomgr_dev * dev, uint8_t mode);

  extern ykneomgr_rc ykneomgr_authenticate (ykneomgr_dev * dev,
					    const uint8_t * key);
  extern ykneomgr_rc ykneomgr_applet_list (ykneomgr_dev * dev,
					   char *appletstr, size_t * len);
  extern ykneomgr_rc ykneomgr_applet_delete (ykneomgr_dev * dev,
					     const uint8_t * aid,
					     size_t aidlen);
  extern ykneomgr_rc ykneomgr_applet_install (ykneomgr_dev * dev,
					      const char *capfile);

#ifdef __cplusplus
}
#endif

#endif
