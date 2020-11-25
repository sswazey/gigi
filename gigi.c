//gigi.c Copyright (c) 2013-2020 Scott Swazey.  All rights reserved.
//
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int gigi_prompt = 1;

int main ()
{
  FILE *tty_fp, *bas_fp;
  char line_buf[256];
  char dummy_buf[256];

  printf ("GIGI command processor 1.0\nType help for a list of commands\n\n");
  for (;;)
    {
      if (gigi_prompt != 0)
	printf ("GIGI> ");
      fgets (line_buf, sizeof (line_buf), stdin);
      line_buf[strlen (line_buf) - 1] = '\0';
      if (line_buf[0] == '?' || (strncasecmp (line_buf, "help", 4) == 0))
	{
	  printf ("DEC Gigi Host services.  Version 1.0\n");
	  printf ("Copyright (c) Scott T. Swazey 2013-2020\n\n");
	  printf ("Host commands:\n");
	  printf ("\thelp or ?    -- This help.\n");
	  printf ("\tstop         -- Exit this program.\n");
	  printf ("\tquit, exit, or BASIC -- Return to local BASIC.\n");
	  printf ("\tdir or ls    -- Directory.\n");
	  printf ("\trm           -- Delete file.\n");
	  printf ("Gigi BASIC commands:\n");
	  printf ("\told  \"filename.ext\" -- Load program into memory.\n");
	  printf ("\tsave \"filename.ext\" -- Save Gigi BASIC program to disk.\n");
	  printf ("\thost                -- Enter host mode.\n\n");
	  continue;
	}

      if (strncasecmp (line_buf, "rem host", 8) == 0)
	{
          fgets (dummy_buf, sizeof (line_buf), stdin);
	  system ("stty echo");
	  gigi_prompt = 1;
	  continue;
	}

      if (strncasecmp (line_buf, "stop", 4) == 0)
	{
	  system ("stty echo");
	  exit (0);
	}

      if ((strncasecmp (line_buf, "quit", 4) == 0)
	  || (strncasecmp (line_buf, "exit", 4) == 0)
	  || (strncasecmp (line_buf, "basic", 5) == 0))
	{
	  gigi_prompt = 0;
	  system ("stty -echo");
	  printf ("\233[?21h");	//send chr$(155)+"[?21h" -- return to local basic
	  continue;
	}

      if ((strncasecmp (line_buf, "dir", 3) == 0)
	  || (strncasecmp (line_buf, "ls", 2) == 0))
	{
	  system ("ls -CF");
	  continue;
	}

      if (strncasecmp (line_buf, "rem old ", 7) == 0)
	{
	  char *pfile, basic_line[256];
	  int nlines = 0, nbytes = 0;
	  pfile = &line_buf[8];	// filename starts after "rem old "
	  fprintf (tty_fp, "sending file %s\n", pfile);
	  if ((bas_fp = fopen (pfile, "r")) == NULL)
	    {
	      printf ("PRINT \"%s: %s\"\n", pfile, strerror (errno));
	      continue;
	    }
	  while (fgets (basic_line, sizeof (basic_line), bas_fp) != NULL)
	    {
	      nlines++;
	      nbytes += strlen (basic_line);
	      basic_line[strlen (basic_line) - 1] = '\0';
	      puts (basic_line);
	    }
	  printf ("PRINT chr$(34)+\"%s\"+chr$(34)+\" %d Lines (%d bytes) loaded.\"\n",
	     pfile, nlines, nbytes);
	  continue;
	}

      if (strncasecmp (line_buf, "rem save ", 9) == 0)
	{			// save the file and save the error until we see "rem end save")
	  FILE *save_fp;
	  int nlines = 0, nbytes = 0;
	  char *pfile = &line_buf[9], basic_line[256];
	  if ((save_fp = fopen (pfile, "wb")) == NULL)
	    {
	      printf ("PRINT \"%s: %s\"\n", pfile, strerror (errno));
	      continue;
	    }
	  while (fgets (basic_line, sizeof (basic_line), stdin) != NULL)
	    {
	      if(strlen(basic_line)<=1)
		fgets (basic_line, sizeof (basic_line), stdin);
	      if (strncasecmp (basic_line, "rem end save", 12) != 0)
		{
		  nlines++;
		  nbytes += strlen (basic_line);
//		  fwrite (basic_line, strlen (basic_line), 1, save_fp);
		  fputs(basic_line, save_fp);
		}
	      else
		{  //FIXME need to catch errors and defer to we receive "rem save end"
		  printf ("PRINT chr$(34)+\"%s\"+chr$(34)+\" %d Lines (%d bytes) saved.\"\n",
		     pfile, nlines, nbytes);
		  fclose (save_fp);
		  break;
		}
	    }
	  continue;
	}

      if ((strncasecmp (line_buf, "del ", 4) == 0)
	  || (strncasecmp (line_buf, "rm ", 3) == 0))
	{
	  system(line_buf);
	  continue;
	}

      if (gigi_prompt && (strlen(line_buf) !=0))
	printf ("Invalid command: \"%s\"  Type ? for help\n", line_buf);
    }
}
