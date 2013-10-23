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

ykneomgr_rc
backend_init (ykneomgr_dev * d)
{
  LONG result;

  result = SCardEstablishContext (SCARD_SCOPE_USER, NULL, NULL, &d->card);
  if (result != SCARD_S_SUCCESS)
    return YKNEOMGR_BACKEND_ERROR;

  return YKNEOMGR_OK;
}

void
backend_done (ykneomgr_dev * dev)
{
  SCardReleaseContext (dev->card);
  /* XXX error code ignored */
}

ykneomgr_rc
backend_connect (ykneomgr_dev * dev, const char *name)
{
  DWORD activeProtocol;
  LONG result;

  result = SCardConnect (dev->card, name,
			 SCARD_SHARE_SHARED,
			 SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
			 &dev->cardHandle, &activeProtocol);
  if (result != SCARD_S_SUCCESS)
    return YKNEOMGR_BACKEND_ERROR;

  return YKNEOMGR_OK;
}

ykneomgr_rc
backend_apdu (ykneomgr_dev * dev,
	      const uint8_t * send,
	      size_t sendlen, uint8_t * recv, size_t * recvlen)
{
  SCARDHANDLE cardHandle = dev->cardHandle;
  DWORD recvAPDULen = *recvlen;
  LONG result;

  if (debug)
    {
      size_t i;
      printf ("--> %zd: ", sendlen);
      for (i = 0; i < sendlen; i++)
	printf ("%02x ", send[i] & 0xFF);
      printf ("\n");
    }

  result = SCardTransmit (cardHandle,
			  SCARD_PCI_T1,
			  send, sendlen, NULL, recv, &recvAPDULen);
  *recvlen = recvAPDULen;
  if (result != SCARD_S_SUCCESS)
    return YKNEOMGR_BACKEND_ERROR;

  if (debug)
    {
      size_t i;
      printf ("<-- %zd: ", *recvlen);
      for (i = 0; i < *recvlen; i++)
	printf ("%02x ", recv[i] & 0xFF);
      printf ("\n");
    }

  return YKNEOMGR_OK;
}

ykneomgr_rc
backend_list_devices (ykneomgr_dev * dev, char *devicestr, size_t * len)
{
  LONG result;
  DWORD readersSize = *len;

  result = SCardListReaders (dev->card, NULL, devicestr, &readersSize);
  *len = readersSize;
  if (result != SCARD_S_SUCCESS)
    return YKNEOMGR_BACKEND_ERROR;

  return YKNEOMGR_OK;
}

ykneomgr_rc
backend_authenticate (ykneomgr_dev * dev, const uint8_t * key)
{
  (void) dev;
  (void) key;
  return YKNEOMGR_BACKEND_ERROR;
}

ykneomgr_rc
backend_applet_list (ykneomgr_dev * dev, char *appletstr, size_t * len)
{
  (void) dev;
  (void) appletstr;
  (void) len;
  return YKNEOMGR_BACKEND_ERROR;
}

ykneomgr_rc
backend_applet_delete (ykneomgr_dev * dev, const uint8_t * aid, size_t aidlen)
{
  (void) dev;
  (void) aid;
  (void) aidlen;
  return YKNEOMGR_BACKEND_ERROR;
}

ykneomgr_rc
backend_applet_install (ykneomgr_dev * dev, const char *capfile)
{
  (void) dev;
  (void) capfile;
  return YKNEOMGR_BACKEND_ERROR;
}
