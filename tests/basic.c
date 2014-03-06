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

#include <ykneomgr/ykneomgr.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (void)
{
  int rc;

  if (strcmp (YKNEOMGR_VERSION_STRING, ykneomgr_check_version (NULL)) != 0)
    {
      printf ("version mismatch %s != %s\n", YKNEOMGR_VERSION_STRING,
	      ykneomgr_check_version (NULL));
      return EXIT_FAILURE;
    }

  if (ykneomgr_check_version (YKNEOMGR_VERSION_STRING) == NULL)
    {
      printf ("version NULL?\n");
      return EXIT_FAILURE;
    }

  if (ykneomgr_check_version ("99.99.99") != NULL)
    {
      printf ("version not NULL?\n");
      return EXIT_FAILURE;
    }

  printf ("ykneomgr version: header %s library %s\n",
	  YKNEOMGR_VERSION_STRING, ykneomgr_check_version (NULL));

  rc = ykneomgr_global_init (0);
  if (rc != YKNEOMGR_OK)
    {
      printf ("ykneomgr_global_init rc %d\n", rc);
      return EXIT_FAILURE;
    }

  if (ykneomgr_strerror (YKNEOMGR_OK) == NULL)
    {
      printf ("ykneomgr_strerror NULL\n");
      return EXIT_FAILURE;
    }

  {
    const char *s;
    s = ykneomgr_strerror_name (YKNEOMGR_OK);
    if (s == NULL || strcmp (s, "YKNEOMGR_OK") != 0)
      {
	printf ("ykneomgr_strerror_name %s\n", s);
	return EXIT_FAILURE;
      }
  }

  ykneomgr_global_done ();

  return EXIT_SUCCESS;
}
