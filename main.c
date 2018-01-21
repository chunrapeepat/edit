#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

// bitwise and with 00011111
#define CTRL_KEY(k) ((k) & 0x1f)

// save the origin terminal flag
struct editorConfig {
  int screenrows;
  int screencols;
  struct termios orig_termios;
};

struct editorConfig E;

/*** terminal section ***/

void die(const char *s) {
  // clear the screen when exit
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}

void disable_raw_mode() {
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

void enable_raw_mode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    die("tcgetattr");
  // disable raw mode when user exit program
  atexit(disable_raw_mode);
  // turn of echoing, canonical mode, disable miscellaneons flag & signal
  // by change the forth bit to 0
  struct termios raw = E.orig_termios;
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

int get_cursor_position(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }

  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
  return -1;
}

int get_window_size(int *rows, int *cols) {
  struct winsize ws;
  // TIOCGWINS stand for terminal IOCtl get window size
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    return get_cursor_position(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

/*** output section ***/

void editor_draw_rows() {
  int y;
  for (y = 0; y < E.screenrows; y++) {
    // draw tlides ~ to the screen
    write(STDOUT_FILENO, "~\r\n", 3);
    if (y < E.screenrows - 1) {
      write(STDOUT_FILENO, "\r\n", 2);
    }
  }
}

void editor_refresh_screen() {
  // clear the screen both upper & lower (J)
  write(STDOUT_FILENO, "\x1b[2J", 4);
  // move cursor to the top left conner (H)
  write(STDOUT_FILENO, "\x1b[H", 3);

  editor_draw_rows();
  write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input section ***/

void editor_process_keypress() {
  char c = editor_read_key();
  switch (c) {
    case CTRL_KEY('q'):
      // clear the screen when exit
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
  }
}

/*** init section ***/

void init_editor() {
  // fill the rows & cols on config editor
  if (get_window_size(&E.screenrows, &E.screencols)) die("get_window_size");
}

int main()
{
  enable_raw_mode();
  init_editor();

  while (1) {
    editor_refresh_screen();
    editor_process_keypress();
  }

  return 0;
}
