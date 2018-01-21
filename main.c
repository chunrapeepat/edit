#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

// save the origin terminal flag
struct termios orig_termios;

void disable_raw_mode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  // disable raw mode when user exit program
  atexit(disable_raw_mode);
  // turn of echoing & canonical mode by change the forth bit to 0
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main()
{
  enable_raw_mode();

  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
  return 0;
}
