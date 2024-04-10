extern void printchar(int);
extern void printint(int);
void cprintf(char *fmt, ...);

#include <unistd.h>

int main() {
  char *p, *q;
  int x = 2;
  p = 0x62a; cprintf("%x %x\n", p, p+2);

  q = p + x;

  if (p + x > p) cprintf("Y\n");
  else           cprintf("N\n");

  if (p < p + x) cprintf("Y\n");
  else           cprintf("N\n");

  if (q > p) cprintf("Y\n");
  else       cprintf("N\n");
  return (0);
}
