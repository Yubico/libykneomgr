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

#include "config.h"

#include <ykneomgr/ykneomgr.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

#include "cmdline.h"

static int
doit (struct gengetopt_args_info *args_info, uint8_t mode,
      uint8_t * deleteaid, size_t deleteaidlen)
{
  const uint8_t key[] = "\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49"
    "\x4a\x4b\x4c\x4d\x4e\x4f";
  ykneomgr_rc rc;
  ykneomgr_dev *dev;

  rc = ykneomgr_init (&dev);
  if (rc != YKNEOMGR_OK)
    {
      printf ("error: ykneomgr_init (%d): %s\n", rc, ykneomgr_strerror (rc));
      return EXIT_FAILURE;
    }

  if (!args_info->list_readers_flag)
    {
      rc = ykneomgr_discover (dev);
      if (rc != YKNEOMGR_OK)
	{
	  printf ("error: ykneomgr_discover (%d): %s\n",
		  rc, ykneomgr_strerror (rc));
	  return EXIT_FAILURE;
	}
    }

  if (args_info->applet_delete_given
      || args_info->applet_list_given || args_info->applet_install_given)
    {
      rc = ykneomgr_authenticate (dev, key);
      if (rc != YKNEOMGR_OK)
	{
	  printf ("error: ykneomgr_authenticate (%d): %s\n",
		  rc, ykneomgr_strerror (rc));
	  return EXIT_FAILURE;
	}
    }

  if (args_info->get_mode_flag)
    printf ("%02x\n", ykneomgr_get_mode (dev));
  else if (args_info->get_version_flag)
    printf ("%d.%d.%d\n", ykneomgr_get_version_major (dev),
	    ykneomgr_get_version_minor (dev),
	    ykneomgr_get_version_build (dev));
  else if (args_info->get_serialno_flag)
    printf ("%d\n", ykneomgr_get_serialno (dev));
  else if (args_info->list_readers_flag)
    {
      size_t len, j, k;
      char *str;

      rc = ykneomgr_list_devices (dev, NULL, &len);
      if (rc != YKNEOMGR_OK)
	{
	  printf ("error: ykneomgr_list_devices (%d): %s\n",
		  rc, ykneomgr_strerror (rc));
	  return EXIT_FAILURE;
	}

      str = malloc (len);
      if (str == NULL)
	{
	  perror ("malloc");
	  return EXIT_FAILURE;
	}

      rc = ykneomgr_list_devices (dev, str, &len);
      if (rc != YKNEOMGR_OK)
	{
	  printf ("error: ykneomgr_list_devices (%d): %s\n",
		  rc, ykneomgr_strerror (rc));
	  return EXIT_FAILURE;
	}

      for (j = 0, k = 0; j < len;)
	{
	  if (str[j] == '\0')
	    break;

	  printf ("%zd: %s\n", k, str + j);

	  k++;
	  j += strlen (str + j) + 1;
	}

      free (str);
    }
  else if (args_info->set_mode_given)
    {
      rc = ykneomgr_modeswitch (dev, mode);
      if (rc != YKNEOMGR_OK)
	{
	  printf ("error: ykneomgr_modeswitch (%d): %s\n",
		  rc, ykneomgr_strerror (rc));
	  return EXIT_FAILURE;
	}
    }
  else if (args_info->applet_delete_given)
    {
      rc = ykneomgr_applet_delete (dev, deleteaid, deleteaidlen);
      if (rc != YKNEOMGR_OK)
	{
	  printf ("error: ykneomgr_applet_delete (%d): %s\n",
		  rc, ykneomgr_strerror (rc));
	  return EXIT_FAILURE;
	}
    }
  else if (args_info->applet_install_given)
    {
      rc = ykneomgr_applet_install (dev, args_info->applet_install_arg);
      if (rc != YKNEOMGR_OK)
	{
	  printf ("error: ykneomgr_applet_install (%d): %s\n",
		  rc, ykneomgr_strerror (rc));
	  return EXIT_FAILURE;
	}
    }
  else if (args_info->applet_list_flag)
    {
      size_t len, j, k;
      char *str;

      rc = ykneomgr_applet_list (dev, NULL, &len);
      if (rc != YKNEOMGR_OK)
	{
	  printf ("error: ykneomgr_applet_list (%d): %s\n",
		  rc, ykneomgr_strerror (rc));
	  return EXIT_FAILURE;
	}

      str = malloc (len);
      if (str == NULL)
	{
	  perror ("malloc");
	  return EXIT_FAILURE;
	}

      rc = ykneomgr_applet_list (dev, str, &len);
      if (rc != YKNEOMGR_OK)
	{
	  printf ("error: ykneomgr_applet_list (%d): %s\n",
		  rc, ykneomgr_strerror (rc));
	  return EXIT_FAILURE;
	}

      for (j = 0, k = 0; j < len;)
	{
	  if (str[j] == '\0')
	    break;

	  printf ("%zd: %s\n", k, str + j);

	  k++;
	  j += strlen (str + j) + 1;
	}

      free (str);
    }

  ykneomgr_done (dev);

  return EXIT_SUCCESS;
}

int
main (int argc, char *argv[])
{
  int exit_code = EXIT_FAILURE;
  struct gengetopt_args_info args_info;
  ykneomgr_rc rc;
  uint8_t mode = 0;
#define AIDMAXLEN 16
  uint8_t deleteaid[AIDMAXLEN] = { '\x00' };
  size_t deleteaidlen = 0;

  if (cmdline_parser (argc, argv, &args_info) != 0)
    exit (EXIT_FAILURE);

  if (args_info.help_given
      || (!args_info.list_readers_given
	  && !args_info.applet_list_given
	  && !args_info.applet_delete_given
	  && !args_info.applet_install_given
	  && !args_info.get_mode_given
	  && !args_info.get_version_given
	  && !args_info.get_serialno_given && !args_info.set_mode_given))
    {
      cmdline_parser_print_help ();
      printf ("\nReport bugs at <" PACKAGE_BUGREPORT ">.\n");
      exit (EXIT_SUCCESS);
    }

  if (args_info.list_readers_given + args_info.applet_list_given
      + args_info.applet_delete_given + args_info.applet_install_given
      + args_info.get_mode_given + args_info.get_version_given
      + args_info.get_serialno_given + args_info.set_mode_given > 1)
    {
      fprintf (stderr, "%s: too many parameters\n", argv[0]);
      exit (EXIT_FAILURE);
    }

  if (args_info.set_mode_given)
    {
      char *endptr;
      long m = strtol (args_info.set_mode_arg, &endptr, 16);

      if (*endptr != '\0'
	  || !(m == 0x00 || m == 0x01 || m == 0x02 || m == 0x81 || m == 0x82))
	{
	  fprintf (stderr, "%s: invalid mode: %s\n", argv[0],
		   args_info.set_mode_arg);
	  exit (EXIT_FAILURE);
	}

      mode = m;
    }

  if (args_info.applet_delete_given)
    {
      size_t deleteaidhexlen = strlen (args_info.applet_delete_arg);

      if (deleteaidhexlen > 16 || deleteaidhexlen % 2 != 0)
	{
	  fprintf (stderr, "%s: applet AID syntax error: %s\n", argv[0],
		   args_info.applet_delete_arg);
	  exit (EXIT_FAILURE);
	}

      for (deleteaidlen = 0;
	   args_info.applet_delete_arg[2 * deleteaidlen]; deleteaidlen++)
	{
	  int p;

	  if (sscanf (&args_info.applet_delete_arg[2 * deleteaidlen],
		      "%02x", &p) != 1)
	    {
	      fprintf (stderr, "%s: cannot hex deocde AID: %s\n", argv[0],
		       args_info.applet_delete_arg);
	      exit (EXIT_FAILURE);
	    }

	  deleteaid[deleteaidlen] = p;
	}
    }

  rc = ykneomgr_global_init (args_info.debug_flag ? YKNEOMGR_DEBUG : 0);
  if (rc != YKNEOMGR_OK)
    {
      printf ("error: ykneomgr_global_init (%d): %s\n",
	      rc, ykneomgr_strerror (rc));
      exit (EXIT_FAILURE);
    }

  exit_code = doit (&args_info, mode, deleteaid, deleteaidlen);

  ykneomgr_global_done ();

  exit (exit_code);
}
