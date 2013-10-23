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

int
backend_init (ykneomgr_dev * d)
{
  OPGP_ERROR_STATUS status;

  OPGP_ERROR_CREATE_NO_ERROR (status);

  strcpy (d->cardContext.libraryName, "gppcscconnectionplugin");
  strcpy (d->cardContext.libraryVersion, "1");

  status = OPGP_establish_context (&d->cardContext);
  if (OPGP_ERROR_CHECK (status))
    {
      if (debug)
	printf ("establish_context failed with error 0x%08lX (%s)\n",
		status.errorCode, status.errorMessage);
      return YKNEOMGR_BACKEND_ERROR;
    }

  return YKNEOMGR_OK;
}

void
backend_done (ykneomgr_dev * dev)
{
  OPGP_ERROR_STATUS status;

  OPGP_ERROR_CREATE_NO_ERROR (status);

  if (dev->card_connected)
    {
      status = OPGP_card_disconnect (dev->cardContext, &dev->cardInfo);
      if (OPGP_ERROR_CHECK (status))
	{
	  if (debug)
	    printf ("card_disconnect() returns 0x%08lX (%s)\n",
		    status.errorCode, status.errorMessage);
	}
    }

  status = OPGP_release_context (&dev->cardContext);
  if (OPGP_ERROR_CHECK (status))
    {
      if (debug)
	printf ("release_context failed with error 0x%08lX (%s)\n",
		status.errorCode, status.errorMessage);
    }
}

int
backend_connect (ykneomgr_dev * dev, const char *name)
{
  OPGP_ERROR_STATUS status;

  OPGP_ERROR_CREATE_NO_ERROR (status);

  status = OPGP_card_connect (dev->cardContext, name, &dev->cardInfo,
			      OPGP_CARD_PROTOCOL_T0 | OPGP_CARD_PROTOCOL_T1);
  if (OPGP_ERROR_CHECK (status))
    {
      if (debug)
	printf ("card_connect() returns 0x%08lX (%s)\n",
		status.errorCode, status.errorMessage);
      return YKNEOMGR_NO_DEVICE;
    }

  dev->card_connected = 1;

  if (debug)
    {
      size_t i;
      printf ("atr length %zd\n", dev->cardInfo.ATRLength);
      printf ("atr: ");
      for (i = 0; i < dev->cardInfo.ATRLength; i++)
	printf ("%02x ", dev->cardInfo.ATR[i]);
      printf ("\natr: ");
      for (i = 0; i < dev->cardInfo.ATRLength; i++)
	printf ("%c  ", isalnum (dev->cardInfo.ATR[i])
		? dev->cardInfo.ATR[i] : '.');
      printf ("\n");
      printf ("logicalChannel %d\n", dev->cardInfo.logicalChannel);
      printf ("specVersion %d\n", dev->cardInfo.specVersion);
    }

  return YKNEOMGR_OK;
}

ykneomgr_rc
backend_apdu (ykneomgr_dev * dev,
	      const uint8_t * send,
	      size_t sendlen, uint8_t * recv, size_t * recvlen)
{
  OPGP_ERROR_STATUS status;
  PBYTE capdu = (PBYTE) send;
  DWORD recvAPDULen = *recvlen;

  OPGP_ERROR_CREATE_NO_ERROR (status);

  if (debug)
    {
      size_t i;
      printf ("--> %zd: ", sendlen);
      for (i = 0; i < sendlen; i++)
	printf ("%02x ", send[i] & 0xFF);
      printf ("\n");
    }

  status = GP211_send_APDU (dev->cardContext, dev->cardInfo,
			    NULL, capdu, sendlen, recv, &recvAPDULen);
  *recvlen = recvAPDULen;
  if (OPGP_ERROR_CHECK (status))
    {
      if (debug)
	printf ("send_APDU() returns 0x%08lX (%s)\n",
		status.errorCode, status.errorMessage);
      return YKNEOMGR_BACKEND_ERROR;
    }

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
  OPGP_ERROR_STATUS status;
  DWORD readerStrLen = *len;

  OPGP_ERROR_CREATE_NO_ERROR (status);

  status = OPGP_list_readers (dev->cardContext, devicestr, &readerStrLen);
  *len = readerStrLen;
  if (OPGP_ERROR_CHECK (status))
    {
      if (debug)
	printf ("list_readers failed with error 0x%08lX (%s)\n",
		status.errorCode, status.errorMessage);
      return YKNEOMGR_NO_DEVICE;
    }

  return YKNEOMGR_OK;
}

ykneomgr_rc
backend_authenticate (ykneomgr_dev * dev, const uint8_t * key)
{
  OPGP_ERROR_STATUS status;
  PBYTE doubledeskey = (PBYTE) key;
  BYTE recvAPDU[258];
  DWORD recvAPDULen = 258;
  uint8_t select_apdu[] =
    "\x00\xA4\x04\x00\x08\xA0\x00\x00\x00\x03\x00\x00\x00";
  int rc;

  OPGP_ERROR_CREATE_NO_ERROR (status);

  rc = backend_apdu (dev, select_apdu, sizeof select_apdu - 1,
		     recvAPDU, &recvAPDULen);
  if (rc != YKNEOMGR_OK)
    return rc;

  status = GP211_mutual_authentication (dev->cardContext, dev->cardInfo,
					NULL, doubledeskey,
					doubledeskey, NULL,
					0,
					0,
					GP211_SCP02,
					GP211_SCP02_IMPL_i55,
					GP211_SCP02_SECURITY_LEVEL_C_DEC_C_MAC,
					OPGP_DERIVATION_METHOD_NONE,
					&dev->securityInfo211);
  if (OPGP_ERROR_CHECK (status))
    {
      if (debug)
	printf ("mutual_authentication() returns 0x%08lX (%s)\n",
		status.errorCode, status.errorMessage);
      return YKNEOMGR_BACKEND_ERROR;
    }

  return YKNEOMGR_OK;
}

ykneomgr_rc
backend_applet_list (ykneomgr_dev * dev, char *appletstr, size_t * len)
{
  size_t savelen = *len;
#define NUM_APPLICATIONS 64	/* XXX how to avoid hardcoding? */
  DWORD numData = NUM_APPLICATIONS;
  GP211_APPLICATION_DATA appData[NUM_APPLICATIONS];
  GP211_EXECUTABLE_MODULES_DATA execData[NUM_APPLICATIONS];
  OPGP_ERROR_STATUS status;
  int i, j;
  char *p = appletstr;

  OPGP_ERROR_CREATE_NO_ERROR (status);

  status = GP211_get_status (dev->cardContext, dev->cardInfo,
			     &dev->securityInfo211,
			     GP211_STATUS_LOAD_FILES, appData,
			     execData, &numData);
  if (OPGP_ERROR_CHECK (status))
    {
      if (debug)
	printf ("get_status() returns 0x%08lX (%s)\n",
		status.errorCode, status.errorMessage);
      return YKNEOMGR_BACKEND_ERROR;
    }

  *len = 0;
  for (i = 0; i < (int) numData; i++)
    *len += 2 * appData[i].AIDLength + 1;

  if (!appletstr)
    return YKNEOMGR_OK;

  if (appletstr && savelen > *len)
    return YKNEOMGR_MEMORY_ERROR;

  for (i = 0; i < (int) numData; i++)
    {
      for (j = 0; j < appData[i].AIDLength; j++)
	{
	  sprintf (p, "%02x", appData[i].AID[j]);
	  p += 2;
	}
      *p = '\0';
      p += 1;
    }

  return YKNEOMGR_OK;
}

ykneomgr_rc
backend_applet_delete (ykneomgr_dev * dev, const uint8_t * aid, size_t aidlen)
{
  OPGP_AID AIDs[1];
  OPGP_ERROR_STATUS status;
  GP211_RECEIPT_DATA receipt[10];
  DWORD receiptLen = 10;

  OPGP_ERROR_CREATE_NO_ERROR (status);

  memcpy (AIDs[0].AID, aid, aidlen);
  AIDs[0].AIDLength = (BYTE) aidlen;

  status = GP211_delete_application (dev->cardContext,
				     dev->cardInfo,
				     &dev->securityInfo211,
				     AIDs, 1,
				     (GP211_RECEIPT_DATA *) receipt,
				     &receiptLen);
  if (OPGP_ERROR_CHECK (status))
    {
      if (debug)
	printf ("get_status() returns 0x%08lX (%s)\n",
		status.errorCode, status.errorMessage);
      return YKNEOMGR_BACKEND_ERROR;
    }

  return YKNEOMGR_OK;
}

#define AIDLEN 16

ykneomgr_rc
backend_applet_install (ykneomgr_dev * dev, const char *capfile)
{
  OPGP_LOAD_FILE_PARAMETERS loadFileParams;
  DWORD receiptDataAvailable = 0;
  DWORD receiptDataLen = 0;
  BYTE installParam[1] = { '\x00' };
  OPGP_ERROR_STATUS status;
  GP211_RECEIPT_DATA receipt;
  BYTE AID[AIDLEN + 1];
  DWORD AIDLen;
  BYTE instAID[AIDLEN + 1];
  DWORD instAIDLen;
  BYTE pkgAID[AIDLEN + 1];
  DWORD pkgAIDLen;
  DWORD nvCodeLimit;
  BYTE sdAID[AIDLEN + 1];
  DWORD sdAIDLen;
  OPGP_STRING file = (OPGP_STRING) capfile;

  OPGP_ERROR_CREATE_NO_ERROR (status);

  status = OPGP_read_executable_load_file_parameters (file, &loadFileParams);
  if (OPGP_ERROR_CHECK (status))
    {
      if (debug)
	printf ("read_executable_load_file_parameters() "
		"returns 0x%08lX (%s)\n", status.errorCode,
		status.errorMessage);
      return YKNEOMGR_BACKEND_ERROR;
    }

  pkgAIDLen = loadFileParams.loadFileAID.AIDLength;
  memcpy (pkgAID, loadFileParams.loadFileAID.AID, pkgAIDLen);
  AIDLen = loadFileParams.appletAIDs[0].AIDLength;
  memcpy (AID, loadFileParams.appletAIDs[0].AID, AIDLen);
  instAIDLen = loadFileParams.appletAIDs[0].AIDLength;
  memcpy (instAID, loadFileParams.appletAIDs[0].AID, instAIDLen);
  nvCodeLimit = loadFileParams.loadFileSize;
  sdAIDLen = sizeof (GP211_CARD_MANAGER_AID_ALT1);
  memcpy (sdAID, GP211_CARD_MANAGER_AID_ALT1, sdAIDLen);

  status = GP211_install_for_load (dev->cardContext, dev->cardInfo,
				   &dev->securityInfo211,
				   pkgAID, pkgAIDLen,
				   sdAID, sdAIDLen,
				   NULL, NULL, nvCodeLimit, 0, 0);
  if (OPGP_ERROR_CHECK (status))
    {
      if (debug)
	printf ("install_for_load() returns 0x%08lX (%s)\n",
		status.errorCode, status.errorMessage);
      return YKNEOMGR_BACKEND_ERROR;
    }

  status = GP211_load (dev->cardContext, dev->cardInfo, &dev->securityInfo211,
		       NULL, 0, file, NULL, &receiptDataLen, NULL);
  if (OPGP_ERROR_CHECK (status))
    {
      if (debug)
	printf ("load() returns 0x%08lX (%s)\n",
		status.errorCode, status.errorMessage);
      return YKNEOMGR_BACKEND_ERROR;
    }

  status = GP211_install_for_install_and_make_selectable
    (dev->cardContext, dev->cardInfo, &dev->securityInfo211, pkgAID,
     pkgAIDLen, AID, AIDLen,
     instAID, instAIDLen, 0, 0, 0, installParam, sizeof installParam, NULL,
     &receipt, &receiptDataAvailable);
  if (OPGP_ERROR_CHECK (status))
    {
      if (debug)
	printf ("install_for_install_and_make_selectable() "
		"returns 0x%08lX (%s)\n", status.errorCode,
		status.errorMessage);
      return YKNEOMGR_BACKEND_ERROR;
    }

  return YKNEOMGR_OK;
}
