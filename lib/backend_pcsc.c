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

#include <zip.h>
#include "internal.h"
#include "des.h"

static const uint8_t selectApdu[] =
  { 0x00, 0xa4, 0x04, 0x00, 0x08, 0xa0, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
  0x00
};
static const uint8_t initUpdate[] = { 0x80, 0x50, 0x00, 0x00, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };	/* TODO: random challenge */
static const uint8_t listApdu[] =
  { 0x80, 0xf2, 0x40, 0x00, 0x02, 0x4f, 0x00 };
static const uint8_t sdAid[] =
  { 0xa0, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00 };

static const char *components[] =
  { "Header.cap", "Directory.cap", "Import.cap", "Applet.cap", "Class.cap",
  "Method.cap", "StaticField.cap", "ConstantPool.cap", "RefLocation.cap",
};

static int
des_encrypt_cbc (const unsigned char *in, size_t in_len, unsigned char *out,
		 size_t out_len, const unsigned char *iv,
		 unsigned char schedule[][16][6], int triple)
{
  int i;
  unsigned char tmp[8];
  for (i = 0; i < DES_BLOCK_SIZE; i++, in++, iv++)
    {
      tmp[i] = *in ^ *iv;
    }
  if (triple == 1)
    {
      three_des_crypt (tmp, out, schedule);
    }
  else
    {
      des_crypt (tmp, out, schedule[0]);
    }
  in_len -= DES_BLOCK_SIZE;
  out_len -= DES_BLOCK_SIZE;
  if (in_len > 0 && out_len >= DES_BLOCK_SIZE)
    {
      i +=
	des_encrypt_cbc (in, in_len, out + DES_BLOCK_SIZE, out_len, out,
			 schedule, triple);
    }
  return i;
}

static int
mac (ykneomgr_dev * dev, const unsigned char *in, size_t inlen,
     unsigned char *out)
{
  if (inlen > 8)
    {
      unsigned char *tmp = malloc (inlen - 8);
      des_encrypt_cbc (in, inlen - 8, tmp, inlen - 8, dev->icv, dev->macKey,
		       0);
      memcpy (dev->icv, tmp + inlen - 16, 8);	/* copy last 8 bytes */
      dev->icv[5] ^= 0x80;
      free (tmp);
    }
  des_encrypt_cbc (in + inlen - 8, 8, out, 8, dev->icv, dev->macKey, 1);
  memcpy (dev->icv, out, 8);
  return 8;
}

ykneomgr_rc
backend_init (ykneomgr_dev * d)
{
  LONG result;

  result = SCardEstablishContext (SCARD_SCOPE_USER, NULL, NULL, &d->card);
  if (result != SCARD_S_SUCCESS)
    {
      if (debug)
	printf ("SCardEstablishContext %ld\n", (long) result);
      return YKNEOMGR_BACKEND_ERROR;
    }

  d->mac = 0;
  d->encrypt = 0;
  memset (d->icv, 0, 8);

  return YKNEOMGR_OK;
}

void
backend_done (ykneomgr_dev * dev)
{
  LONG result;

  result = SCardReleaseContext (dev->card);
  if (result != SCARD_S_SUCCESS)
    {
      if (debug)
	printf ("SCardReleaseContext %ld\n", (long) result);
      /* XXX error code ignored */
    }
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
    {
      if (debug)
	printf ("SCardConnect %ld\n", (long) result);
      return YKNEOMGR_BACKEND_ERROR;
    }

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
    {
      if (debug)
	printf ("SCardTransmit %ld\n", (long) result);
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
  LONG result;
  DWORD readersSize = *len;

  result = SCardListReaders (dev->card, NULL, devicestr, &readersSize);
  *len = readersSize;
  if (result != SCARD_S_SUCCESS)
    {
      if (debug)
	printf ("SCardListReaders %ld\n", (long) result);
      return YKNEOMGR_BACKEND_ERROR;
    }

  return YKNEOMGR_OK;
}

ykneomgr_rc
backend_authenticate (ykneomgr_dev * dev, const uint8_t * key)
{
  uint8_t recv[256], send[256];
  size_t recvlen = sizeof (recv);
  unsigned char buf[16], tmp[16], iv[DES_BLOCK_SIZE], raw_key[24];
  unsigned char schedule[3][16][6];
  int i;

  if (backend_apdu (dev, selectApdu, sizeof (selectApdu), recv, &recvlen) !=
      YKNEOMGR_OK)
    {
      return YKNEOMGR_BACKEND_ERROR;
    }
  if (backend_apdu (dev, initUpdate, sizeof (initUpdate), recv, &recvlen) !=
      YKNEOMGR_OK)
    {
      return YKNEOMGR_BACKEND_ERROR;
    }

  if (recvlen != 30)
    {
      return YKNEOMGR_BACKEND_ERROR;
    }

  /* key is a 16 byte 2-key triple des key, so copy part 1 to 3 */
  memcpy (raw_key, key, 16);
  memcpy (raw_key + 16, key, 8);
  three_des_key_setup (raw_key, schedule, DES_ENCRYPT);

  memset (iv, 0, sizeof (iv));
  memset (buf, 0, sizeof (buf));
  buf[0] = 0x01;
  buf[1] = 0x82;
  buf[2] = recv[12];
  buf[3] = recv[13];
  des_encrypt_cbc (buf, sizeof (buf), raw_key, 16, iv, schedule, 1);
  memcpy (raw_key + 16, raw_key, 8);
  three_des_key_setup (raw_key, dev->encKey, DES_ENCRYPT);

  memset (iv, 0, sizeof (iv));
  memset (buf, 0, sizeof (buf));
  buf[0] = 0x01;
  buf[1] = 0x01;
  buf[2] = recv[12];
  buf[3] = recv[13];
  des_encrypt_cbc (buf, sizeof (buf), raw_key, 16, iv, schedule, 1);
  memcpy (raw_key + 16, raw_key, 8);
  three_des_key_setup (raw_key, dev->macKey, DES_ENCRYPT);

  memset (iv, 0, sizeof (iv));
  memcpy (buf, initUpdate + 5, 8);	/* our "random" challenge */
  buf[8] = recv[12];
  buf[9] = recv[13];
  memcpy (buf + 10, recv + 14, 6);	/* the card challenge */

  three_des_crypt (buf, tmp, dev->encKey);
  for (i = 0; i < 8; i++)
    tmp[i] ^= buf[i + 8];
  three_des_crypt (tmp, buf, dev->encKey);
  buf[0] ^= 0x80;
  three_des_crypt (buf, tmp, dev->encKey);

  if (memcmp (tmp, recv + 20, DES_BLOCK_SIZE) != 0)
    {
      return YKNEOMGR_BACKEND_ERROR;
    }

  buf[0] = recv[12];
  buf[1] = recv[13];
  memcpy (buf + 2, recv + 14, 6);
  memcpy (buf + 8, initUpdate + 5, 8);

  three_des_crypt (buf, tmp, dev->encKey);
  for (i = 0; i < 8; i++)
    tmp[i] ^= buf[i + 8];
  three_des_crypt (tmp, buf, dev->encKey);
  buf[0] ^= 0x80;
  three_des_crypt (buf, tmp, dev->encKey);

  memset (send, 0, sizeof (send));
  send[0] = 0x84;
  send[1] = 0x82;
  send[2] = 0x00;		/* security level */
  send[3] = 0x00;
  send[4] = 0x10;
  memcpy (send + 5, tmp, DES_BLOCK_SIZE);

  mac (dev, send, 16, send + 5 + 8);

  if (backend_apdu (dev, send, 5 + 16, recv, &recvlen) != YKNEOMGR_OK)
    {
      return YKNEOMGR_BACKEND_ERROR;
    }
  if (recvlen == 2 && recv[0] == 0x90 && recv[1] == 0x00)
    {
      return YKNEOMGR_OK;
    }
  return YKNEOMGR_BACKEND_ERROR;
}

ykneomgr_rc
backend_applet_list (ykneomgr_dev * dev, char *appletstr, size_t * len)
{
  uint8_t recv[256];
  size_t recvlen = sizeof (recv);
  size_t length = 0;
  char *p = appletstr;
  size_t real_len = 0;

  if (backend_apdu (dev, listApdu, sizeof (listApdu), recv, &recvlen) !=
      YKNEOMGR_OK)
    {
      return YKNEOMGR_BACKEND_ERROR;
    }

  while (length < recvlen - 2)
    {
      size_t i;
      size_t this_len = recv[length++];
      for (i = 0; i < this_len; i++)
	{
	  if (appletstr)
	    {
	      if (real_len + 2 > *len)
		{
		  return YKNEOMGR_BACKEND_ERROR;
		}
	      sprintf (p, "%02x", recv[length]);
	      p += 2;
	    }
	  real_len += 2;
	  length++;
	}
      if (appletstr)
	{
	  if (real_len + 1 > *len)
	    {
	      return YKNEOMGR_BACKEND_ERROR;
	    }
	  *p = '\0';
	  p++;
	}
      real_len++;
      length += 2;
    }
  *len = real_len;
  return YKNEOMGR_OK;
}

ykneomgr_rc
backend_applet_delete (ykneomgr_dev * dev, const uint8_t * aid, size_t aidlen)
{
  uint8_t recv[261];
  uint8_t send[261];
  size_t recvlen = sizeof (recv);
  uint8_t *p = send;
  size_t sendlen;

  *p++ = 0x80;
  *p++ = 0xe4;
  *p++ = 0;
  *p++ = 0x80;
  *p++ = aidlen + 2;
  *p++ = 0x4f;
  *p++ = aidlen;
  memcpy (p, aid, aidlen);
  p += aidlen;
  sendlen = p - send;

  if (backend_apdu (dev, send, sendlen, recv, &recvlen) != YKNEOMGR_OK)
    {
      return YKNEOMGR_BACKEND_ERROR;
    }
  if (recvlen == 3 && recv[1] == 0x90)
    {
      return YKNEOMGR_OK;
    }

  return YKNEOMGR_BACKEND_ERROR;
}

ykneomgr_rc
backend_applet_install (ykneomgr_dev * dev, const char *capfile)
{
  int error;
  size_t i;
  struct zip *cap = zip_open (capfile, 0, &error);
  size_t total_size = 0;
  unsigned char *buf = NULL;
  unsigned char *p;
  unsigned char *packaid = NULL;
  unsigned char *appaid = NULL;
  size_t packaidlen = 0;
  size_t appaidlen = 0;
  ykneomgr_rc ret = YKNEOMGR_BACKEND_ERROR;

  if (cap == NULL)
    {
      return YKNEOMGR_BACKEND_ERROR;
    }

  for (i = 0; i < (sizeof (components) / sizeof (char *)); i++)
    {
      struct zip_stat stat;
      if (zip_stat (cap, components[i], ZIP_FL_NODIR, &stat) == 0)
	{
	  total_size += stat.size;
	}
      else
	{
	  goto cleanup;
	}
    }

  buf = malloc (total_size + 5);
  if (buf == NULL)
    {
      goto cleanup;
    }
  p = buf;

  *p++ = 0xc4;
  if (total_size < 0x80)
    {
      *p++ = total_size;
      total_size += 2;
    }
  else if (total_size < 0xff)
    {
      *p++ = 0x81;
      *p++ = total_size;
      total_size += 3;
    }
  else if (total_size < 0xffff)
    {
      *p++ = 0x82;
      *p++ = ((total_size & 0xff00) >> 8);
      *p++ = (total_size & 0xff);
      total_size += 4;
    }
  else if (total_size < 0xffffff)
    {
      *p++ = 0x83;
      *p++ = ((total_size & 0xff0000) >> 16);
      *p++ = ((total_size & 0xff00) >> 8);
      *p++ = (total_size & 0xff);
      total_size += 5;
    }
  else
    {
      goto cleanup;
    }

  for (i = 0; i < (sizeof (components) / sizeof (char *)); i++)
    {
      struct zip_file *file = zip_fopen (cap, components[i], ZIP_FL_NODIR);
      int len = zip_fread (file, p, total_size - (p - buf));
      if (strcmp (components[i], "Header.cap") == 0)
	{
	  packaidlen = p[12];
	  packaid = p + 13;
	}
      else if (strcmp (components[i], "Applet.cap") == 0)
	{
	  if (p[3] != 1)
	    {
	      printf ("Only support for 1 applet, found %d.\n", p[3]);
	      zip_fclose (file);
	      goto cleanup;
	    }
	  appaidlen = p[4];
	  appaid = p + 5;
	}
      p += len;
      zip_fclose (file);
    }

  {
    uint8_t recv[256];
    size_t recvlen = sizeof (recv);
    uint8_t send[0xff + 5];
    uint8_t *q = send;
    size_t j;

    /* install for load */
    *q++ = 0x80;
    *q++ = 0xe6;		/* INS install */
    *q++ = 0x02;
    *q++ = 0;
    *q++ = 1 + packaidlen + 1 + sizeof (sdAid) + 1 + 1 + 1;
    *q++ = packaidlen;
    memcpy (q, packaid, packaidlen);
    q += packaidlen;
    *q++ = sizeof (sdAid);
    memcpy (q, sdAid, sizeof (sdAid));
    q += sizeof (sdAid);
    *q++ = 0;			/* hash len */
    *q++ = 0;
    *q++ = 0;			/* ? */
    if (backend_apdu (dev, send, q - send, recv, &recvlen) != YKNEOMGR_OK)
      {
	goto cleanup;
      }
    if (recvlen != 3 || recv[1] != 0x90)
      {
	goto cleanup;
      }

    /* load */
    p = buf;
    for (j = 0; j < (total_size / 0xff) + 1; j++)
      {
	size_t this_len = 0xff;
	uint8_t p2 = 0;
	recvlen = sizeof (recv);
	q = send;
	if (this_len > (total_size - (p - buf)))
	  {
	    this_len = (total_size - (p - buf));
	    p2 = 0x80;
	  }
	*q++ = 0x80;
	*q++ = 0xe8;		/* INS load */
	*q++ = p2;
	*q++ = j;
	*q++ = this_len;
	memcpy (q, p, this_len);
	if (backend_apdu (dev, send, this_len + 5, recv, &recvlen) !=
	    YKNEOMGR_OK)
	  {
	    goto cleanup;
	  }
	if (recvlen != 3 || recv[1] != 0x90)
	  {
	    goto cleanup;
	  }
	p += this_len;
      }

    /* install and make selectable */
    q = send;
    recvlen = sizeof (recv);
    *q++ = 0x80;
    *q++ = 0xe6;
    *q++ = 0x0c;
    *q++ = 0;
    *q++ = 1 + packaidlen + 1 + appaidlen + 1 + appaidlen + 1 + 1 + 3 + 1 + 1;
    *q++ = packaidlen;
    memcpy (q, packaid, packaidlen);
    q += packaidlen;
    *q++ = appaidlen;
    memcpy (q, appaid, appaidlen);
    q += appaidlen;
    *q++ = appaidlen;		/* instance aid */
    memcpy (q, appaid, appaidlen);
    q += appaidlen;
    *q++ = 1;			/* ? */
    *q++ = 0;			/* privilege */
    *q++ = 3;			/* install params len */
    *q++ = 0xc9;
    *q++ = 0x01;
    *q++ = 0;
    *q++ = 0;			/* install token len */
    if (backend_apdu (dev, send, q - send, recv, &recvlen) != YKNEOMGR_OK)
      {
	goto cleanup;
      }
    if (recvlen != 3 || recv[1] != 0x90)
      {
	goto cleanup;
      }
  }

  ret = YKNEOMGR_OK;

cleanup:
  free (buf);
  zip_close (cap);
  return ret;
}
