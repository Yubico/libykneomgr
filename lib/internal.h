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

#ifndef INTERNAL_H
#define INTERNAL_H

#include "config.h"

#include <ykneomgr/ykneomgr.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#if BACKEND_GLOBALPLATFORM
#include <globalplatform/globalplatform.h>
#endif
#if BACKEND_PCSC
#if defined HAVE_PCSC_WINSCARD_H
#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>
#else
#include <winscard.h>
#endif
#include "des.h"
#endif

extern int debug;

struct ykneomgr_dev
{
#if BACKEND_GLOBALPLATFORM
  OPGP_CARD_CONTEXT cardContext;
  OPGP_CARD_INFO cardInfo;
  GP211_SECURITY_INFO securityInfo211;
#endif
#if BACKEND_PCSC || BACKEND_WINSCARD
  SCARDCONTEXT card;
  SCARDHANDLE cardHandle;
  gl_des_ctx macDesKey;
  gl_3des_ctx mac3DesKey;
  gl_3des_ctx enc3DesKey;
  unsigned char icv[8];
  int mac;
  int encrypt;
#endif
  int card_connected;
  uint8_t versionMajor, versionMinor, versionBuild, pgmSeq, mode, crTimeout;
  uint16_t touchLevel, autoEjectTime;
  uint32_t serialno;
};

ykneomgr_rc backend_init (ykneomgr_dev * dev);
void backend_done (ykneomgr_dev * dev);
ykneomgr_rc backend_list_devices (ykneomgr_dev * dev, char *devicestr,
				  size_t * len);
ykneomgr_rc backend_connect (ykneomgr_dev * dev, const char *name);
ykneomgr_rc backend_apdu (ykneomgr_dev * dev, const uint8_t * send,
			  size_t sendlen, uint8_t * recv, size_t * recvlen);
ykneomgr_rc backend_authenticate (ykneomgr_dev * dev, const uint8_t * key);
ykneomgr_rc backend_applet_list (ykneomgr_dev * dev,
				 char *appletstr, size_t * len);
ykneomgr_rc backend_applet_delete (ykneomgr_dev * dev, const uint8_t * aid,
				   size_t aidlen);
ykneomgr_rc backend_applet_install (ykneomgr_dev * dev, const char *capfile);

#endif
