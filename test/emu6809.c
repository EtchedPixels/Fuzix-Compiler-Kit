/*
    main.c   	-	main program for 6809 simulator
    Copyright (C) 2001  Arto Salmi
                        Joze Fabcic

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "6809.h"

char *exename;

static void usage(void) {
  printf("Usage: %s <options> executable\n", exename);
  printf("Options are:\n");
  printf("-m	- start in the monitor\n");

  if (memory != NULL)
    free(memory);
  exit(1);
}


int main(int argc, char *argv[]) {
  int fd, opt;
  int start_in_monitor = 0;

  exename = argv[0];

  if (argc == 1)
    usage();

  // Get the options
  while ((opt = getopt(argc, argv, "m")) != -1) {
    switch (opt) {
    case 'm':
      start_in_monitor = 1;
      break;
    }
  }

  memory = (UINT8 *) malloc(0x10000);
  if (memory == NULL)
    usage();
  memset(memory, 0, 0x10000);

  cpu_quit = 1;

  fd = open(argv[optind], O_RDONLY);
  if (fd == -1) {
    perror(argv[1]);
    exit(1);
  }

  /* 0200-0xFDFF */
  if (read(fd, &memory[0x200], 0xFC00) < 1) {
    fprintf(stderr, "emu6809: bad executable file\n");
    perror(argv[1]);
    exit(1);
  }
  close(fd);

  monitor_init(start_in_monitor);
  cpu_reset();
  do {
    cpu_execute(60);
  } while (cpu_quit != 0);
  free(memory);
  return 0;
}
