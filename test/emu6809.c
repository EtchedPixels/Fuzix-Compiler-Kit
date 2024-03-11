/*
 *	Fake 6809 mini machine to run compiler tests
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "6809.h"

int log_6809 = 0;

int main(int argc, char *argv[])
{
    int fd;

    memory = (UINT8 *)malloc(0x10000);
    if (memory == NULL) {
     fprintf(stderr, "emu6809: unable to malloc memory\n"); exit(1);
    }
    memset (memory, 0, 0x10000);

    if (argc == 4 && strcmp(argv[1], "-d") == 0) {
        argv++;
        argc--;
        log_6809 = 1;
    }
    if (argc != 3) {
        fprintf(stderr, "emu6809: test map.\n");
        exit(1);
    }
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        exit(1);
    }

    /* 0200-0xFDFF */
    if (read(fd, &memory[0x200], 0xFC00) < 1) {
        fprintf(stderr, "emu6809: bad test.\n");
        perror(argv[1]);
        exit(1);
    }
    close(fd);


  cpu_reset();
  while (1) 
    cpu_execute(100000);

  return 0;
}
