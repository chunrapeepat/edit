#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

// bitwise and with 00011111
#define CTRL_KEY(k) ((k) & 0x1f)

// save the origin terminal flag
struct termios orig_termios;

/*** terminal section ***/

void die(const char *s) {
  perror(s);
  exit(1);
}

void disable_raw_mode() {
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enable_raw_mode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    die("tcgetattr");
  // disable raw mode when user exit program
  atexit(disable_raw_mode);
  // turn of echoing, canonical mode, disable miscellaneons flag & signal
  // by change the forth bit to 0
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_iflag &= ~(ICRNL | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

char editor_read_key() {
  int nread;
  char c;
  // wait for keypress and return char
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

/*** input section ***/

void editor_process_keypress() {
  char c = editor_read_key();
  switch (c) {
    case CTRL_KEY('q'):
      exit(0);
      break;
  }
}

/*** init sectin ***/

int main()
{
  enable_raw_mode();

  while (1) {
    editor_process_keypress();
  }

  return 0;
}
