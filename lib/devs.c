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

#define GETU32(pt) (((uint32_t)((pt)[0] & 0xFF) << 24) ^ \
		    ((uint32_t)((pt)[1] & 0xFF) << 16) ^ \
		    ((uint32_t)((pt)[2] & 0xFF) <<  8) ^ \
		    ((uint32_t)((pt)[3] & 0xFF)))

#define GETU16(pt) (((uint32_t)((pt)[0] & 0xFF) <<  8) ^ \
		    ((uint32_t)((pt)[1] & 0xFF)))

/**
 * ykneomgr_init:
 * @dev: pointer to newly allocated device handle.
 *
 * Create a YubiKey NEO device handle.  The handle must be deallocated
 * using ykneomgr_done() when you no longer need it.
 *
 * Returns: On success, %YKNEOMGR_OK (integer 0) is returned, or
 *   another #ykneomgr_rc error code.
 */
ykneomgr_rc
ykneomgr_init (ykneomgr_dev ** dev)
{
  ykneomgr_dev *d;
  int rc;

  d = calloc (1, sizeof (*d));
  if (d == NULL)
    return YKNEOMGR_MEMORY_ERROR;

  rc = backend_init (d);
  if (rc != YKNEOMGR_OK)
    {
      free (d);
      return rc;
    }

  *dev = d;

  return YKNEOMGR_OK;
}

/**
 * ykneomgr_done:
 * @dev: device handle to deallocate, created by ykneomgr_init().
 *
 * Release all resources allocated to a YubiKey NEO device handle.
 */
void
ykneomgr_done (ykneomgr_dev * dev)
{
  backend_done (dev);
  free (dev);
}

/**
 * ykneomgr_connect:
 * @dev: a #ykneomgr_dev device handle.
 * @name: input string with device name to connect to.
 *
 * Establish connection to a named PCSC device and verify that it has
 * the YubiKey OTP applet.  The @name string should be a PCSC device
 * name; you can use ykneomgr_list_devices() to list connected
 * devices.
 *
 * Returns: On success, %YKNEOMGR_OK (integer 0) is returned, when no
 *   device could be found %YKNEOMGR_NO_DEVICE is returned, or another
 *   #ykneomgr_rc error code.
 */
ykneomgr_rc
ykneomgr_connect (ykneomgr_dev * dev, const char *name)
{
  int rc;
  uint8_t recvAPDU[258];
  size_t recvAPDULen = 258;
  uint8_t buf[] = "\x00\xA4\x04\x00\x08\xA0\x00\x00\x05\x27\x20\x01\x01";

  rc = backend_connect (dev, name);
  if (rc != YKNEOMGR_OK)
    return rc;

  rc = backend_apdu (dev, buf, sizeof buf - 1, recvAPDU, &recvAPDULen);
  if (rc != YKNEOMGR_OK)
    return rc;

  if (recvAPDULen < 12 || recvAPDU[recvAPDULen - 2] != 0x90
      || recvAPDU[recvAPDULen - 1] != 0x00)
    {
      if (debug)
	printf ("YubiKey NEO applet selection failed\n");
      return YKNEOMGR_NO_DEVICE;
    }

  dev->versionMajor = recvAPDU[0];
  dev->versionMinor = recvAPDU[1];
  dev->versionBuild = recvAPDU[2];
  dev->pgmSeq = recvAPDU[3];
  dev->touchLevel = GETU16 (&recvAPDU[4]);
  dev->mode = recvAPDU[6];
  dev->crTimeout = recvAPDU[7];
  dev->autoEjectTime = GETU16 (&recvAPDU[8]);

  if (debug)
    {
      printf ("versionMajor %d\n", dev->versionMajor);
      printf ("versionMinor %d\n", dev->versionMinor);
      printf ("versionBuild %d\n", dev->versionBuild);
      printf ("pgmSeq %d\n", dev->pgmSeq);
      printf ("touchLevel %d\n", dev->touchLevel);
      printf ("mode %02x\n", dev->mode);
      printf ("crTimeout %d\n", dev->crTimeout);
      printf ("autoEjectTime %d\n", dev->autoEjectTime);
    }

  memcpy (buf, "\x00\x01\x10\x00", 4);

  rc = backend_apdu (dev, buf, 4, recvAPDU, &recvAPDULen);
  if (rc != YKNEOMGR_OK)
    return rc;

  if (!((recvAPDULen == 2 && recvAPDU[0] == 0x90 && recvAPDU[1] == 0x00)
	|| (recvAPDULen == 6 && recvAPDU[4] == 0x90 && recvAPDU[5] == 0x00)))
    {
      if (debug)
	{
	  size_t i;
	  printf ("apdu %zd: ", recvAPDULen);
	  for (i = 0; i < recvAPDULen; i++)
	    printf ("%02x ", recvAPDU[i]);
	  printf ("\n");
	}

      return YKNEOMGR_BACKEND_ERROR;
    }

  if (recvAPDULen == 6)
    dev->serialno = GETU32 (recvAPDU);

  if (debug)
    {
      printf ("serialno %d\n", dev->serialno);
    }

  return YKNEOMGR_OK;
}

/**
 * ykneomgr_list_devices:
 * @dev: a #ykneomgr_dev device handle.
 * @devicestr: output buffer to hold string, or %NULL.
 * @len: on input length of @devicestr buffer, on output holds output length
 *
 * List devices.
 *
 * Returns: On success, %YKNEOMGR_OK (integer 0) is returned, or
 *   another #ykneomgr_rc error code.
 */
ykneomgr_rc
ykneomgr_list_devices (ykneomgr_dev * dev, char *devicestr, size_t * len)
{
  return backend_list_devices (dev, devicestr, len);
}

/**
 * ykneomgr_discover_match:
 * @dev: a #ykneomgr_dev device handle.
 * @match: substring to match card reader for, or %NULL.
 *
 * Discover and establish connection to the first found YubiKey NEO
 * that has a card reader name matching @match.  A YubiKey NEO is
 * identified by having the YubiKey OTP applet installed, i.e., a
 * connect followed by attempting to select the YubiKey OTP applet.
 * If @match is NULL, then the first YubiKey NEO device detected will
 * be used.
 *
 * Returns: On success, %YKNEOMGR_OK (integer 0) is returned, when no
 *   device could be found %YKNEOMGR_NO_DEVICE is returned, or another
 *   #ykneomgr_rc error code.
 *
 * Since: 0.1.4
 */
ykneomgr_rc
ykneomgr_discover_match (ykneomgr_dev * dev, const char *match)
{
  char *buf;
  size_t j, k, len;
  int rc;

  rc = ykneomgr_list_devices (dev, NULL, &len);
  if (rc != YKNEOMGR_OK)
    return rc;

  buf = malloc (len);
  if (buf == NULL)
    return YKNEOMGR_MEMORY_ERROR;

  rc = ykneomgr_list_devices (dev, buf, &len);
  if (rc != YKNEOMGR_OK)
    goto done;

  for (k = 0, j = 0; j < len; k++, j += strlen (buf + j) + 1)
    {
      if (buf[j] == '\0')
	break;

      if (match && strstr (buf + j, match) == 0)
	{
	  if (debug)
	    printf ("Skipping reader %zd: %s\n", k, buf + j);
	  continue;
	}

      if (debug)
	printf ("Trying reader %zd: %s\n", k, buf + j);

      rc = ykneomgr_connect (dev, buf + j);
      if (rc == YKNEOMGR_OK)
	goto done;
    }

  rc = YKNEOMGR_NO_DEVICE;

done:
  free (buf);

  return rc;
}

/**
 * ykneomgr_discover:
 * @dev: a #ykneomgr_dev device handle.
 *
 * Discover and establish connection to the first found YubiKey NEO.
 * A YubiKey NEO is identified by having the YubiKey OTP applet
 * installed, i.e., a connect followed by attempting to select the
 * YubiKey OTP applet.
 *
 * Returns: On success, %YKNEOMGR_OK (integer 0) is returned, when no
 *   device could be found %YKNEOMGR_NO_DEVICE is returned, otherwise
 *   another #ykneomgr_rc error code is returned.
 */
ykneomgr_rc
ykneomgr_discover (ykneomgr_dev * dev)
{
  return ykneomgr_discover_match (dev, NULL);
}

/**
 * ykneomgr_authenticate:
 * @dev: a #ykneomgr_dev device handle.
 * @key: Double-DES key in binary, 16 bytes
 *
 * Authenticate to the device, to prepare for privileged function
 * access.
 *
 * Returns: On success, %YKNEOMGR_OK (integer 0) is returned, or
 *   another #ykneomgr_rc error code.
 */
ykneomgr_rc
ykneomgr_authenticate (ykneomgr_dev * dev, const uint8_t * key)
{
  return backend_authenticate (dev, key);
}

/**
 * ykneomgr_modeswitch:
 * @dev: a #ykneomgr_dev device handle.
 * @mode: new mode to switch the device into
 *
 * Mode switch a YubiKey NEO.
 *
 * Returns: On success, %YKNEOMGR_OK (integer 0) is returned, or
 *   another #ykneomgr_rc error code.
 */
ykneomgr_rc
ykneomgr_modeswitch (ykneomgr_dev * dev, uint8_t mode)
{
  uint8_t buf[258];
  size_t buflen = 258;
  uint8_t select_apdu[] =
    "\x00\xA4\x04\x00\x08\xA0\x00\x00\x05\x27\x20\x01\x01";
  uint8_t mode_apdu[] = "\x00\x01\x11\x00\x01\xFF";
  int rc;

  mode_apdu[5] = mode;

  rc = backend_apdu (dev, select_apdu, sizeof select_apdu - 1, buf, &buflen);
  if (rc != YKNEOMGR_OK)
    return rc;

  buflen = 258;

  rc = backend_apdu (dev, mode_apdu, sizeof mode_apdu - 1, buf, &buflen);
  if (rc != YKNEOMGR_OK)
    return rc;

  return YKNEOMGR_OK;
}

/**
 * ykneomgr_get_version_major:
 * @dev: a #ykneomgr_dev device handle.
 *
 * Get major version of a YubiKey NEO.  Versions are in the form of
 * MAJOR.MINOR.BUILD, for example 3.0.4, in which case this function
 * would return 3.
 *
 * Returns: the YubiKey NEO major version number.
 */
uint8_t
ykneomgr_get_version_major (ykneomgr_dev * dev)
{
  return dev->versionMajor;
}

/**
 * ykneomgr_get_version_minor:
 * @dev: a #ykneomgr_dev device handle.
 *
 * Get minor version of a YubiKey NEO.  Versions are in the form of
 * MINOR.MINOR.BUILD, for example 3.0.4, in which case this function
 * would return 0.
 *
 * Returns: the YubiKey NEO minor version number.
 */
uint8_t
ykneomgr_get_version_minor (ykneomgr_dev * dev)
{
  return dev->versionMinor;
}

/**
 * ykneomgr_get_version_build:
 * @dev: a #ykneomgr_dev device handle.
 *
 * Get build version of a YubiKey NEO.  Versions are in the form of
 * BUILD.MINOR.BUILD, for example 3.0.4, in which case this function
 * would return 4.
 *
 * Returns: the YubiKey NEO build version number.
 */
uint8_t
ykneomgr_get_version_build (ykneomgr_dev * dev)
{
  return dev->versionBuild;
}

/**
 * ykneomgr_get_mode:
 * @dev: a #ykneomgr_dev device handle.
 *
 * Get mode of a YubiKey NEO.
 *
 * Returns: the YubiKey NEO device mode.
 */
uint8_t
ykneomgr_get_mode (ykneomgr_dev * dev)
{
  return dev->mode;
}

/**
 * ykneomgr_get_serialno:
 * @dev: a #ykneomgr_dev device handle.
 *
 * Get serial number of a YubiKey NEO, if visible.
 *
 * Returns: the YubiKey NEO device mode, or 0 if not visible.
 */
uint32_t
ykneomgr_get_serialno (ykneomgr_dev * dev)
{
  return dev->serialno;
}

/**
 * ykneomgr_applet_list:
 * @dev: a #ykneomgr_dev device handle.
 * @appletstr: output buffer to hold string, or %NULL.
 * @len: on input length of @appletstr buffer, on output holds output length
 *
 * List installed applets.
 *
 * Returns: On success, %YKNEOMGR_OK (integer 0) is returned, or
 *   another #ykneomgr_rc error code.
 */
ykneomgr_rc
ykneomgr_applet_list (ykneomgr_dev * dev, char *appletstr, size_t * len)
{
  /* XXX good API?  Maybe return binary AIDs instead of strings? */
  return backend_applet_list (dev, appletstr, len);
}

/**
 * ykneomgr_applet_delete:
 * @dev: a #ykneomgr_dev device handle.
 * @aid: aid to delete.
 * @aidlen: length of @aid buffer.
 *
 * Delete specified applet.
 *
 * Returns: On success, %YKNEOMGR_OK (integer 0) is returned, or
 *   another #ykneomgr_rc error code.
 */
ykneomgr_rc
ykneomgr_applet_delete (ykneomgr_dev * dev, const uint8_t * aid,
			size_t aidlen)
{
  return backend_applet_delete (dev, aid, aidlen);
}

/**
 * ykneomgr_applet_install:
 * @dev: a #ykneomgr_dev device handle.
 * @capfile: string with path filename to CAP file
 *
 * Install specified applet.
 *
 * Returns: On success, %YKNEOMGR_OK (integer 0) is returned, or
 *   another #ykneomgr_rc error code.
 */
ykneomgr_rc
ykneomgr_applet_install (ykneomgr_dev * dev, const char *capfile)
{
  return backend_applet_install (dev, capfile);
}

/**
 * ykneomgr_send_apdu:
 * @dev: a #ykneomgr_dev device handle.
 * @send: apdu to send
 * @sendlen: length of send buffer
 * @recv: response apdu
 * @recvlen: length of recv buffer
 *
 * Send an arbitrary apdu to the device.
 *
 * Returns: On success, %YKNEOMGR_OK (integer 0) is returned, or
 *   another #ykneomgr_rc error code.
 *   @recvlen will be set to the length of the data in @recv.
 */
ykneomgr_rc
ykneomgr_send_apdu (ykneomgr_dev * dev, const uint8_t * send, size_t sendlen,
		    uint8_t * recv, size_t * recvlen)
{
  return backend_apdu (dev, send, sendlen, recv, recvlen);
}
