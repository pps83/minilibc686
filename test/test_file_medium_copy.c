/*
 * demo_file_medium_copy.c: a demo program which copies file argv[1] to argv[2] using stdio functions.
 * by pts@fazekas.hu at Mon May 22 16:23:06 CEST 2023
 */

#include <stdio.h>

#define NULL ((void*)0)

int main(int argc, char **argv) {
  FILE *fin, *fout;
  char buf[123];  /* Make it a small odd number for testing interactions of various buffer sizes. */
  int got, got2, i;
  char mode;
  off_t ofs, ofs_delta;
  (void)argc;
  /* We need exactly 2 arguments: argv[1] and argv[2]. */
  if (!argv[0] || !argv[1] || !argv[2]) return 1;
  if (argv[3] && (mode = argv[3][0], mode == 'r' || mode == 'c' || mode == 'd' || mode == 'a' || mode == 's' || mode == 'p' || mode == 'q' || mode == 'f') && argv[3][1] == '\0') {
  } else if (!argv[3]) {
    mode = 'r';  /* Default. */
  } else {
    return 2;
  }
  if (mode == 'f') {
    if ((fin = fopen("/dev/null", "wb")) == NULL) return 21;
    if ((fin = freopen(argv[1], "rb", fin)) == NULL) return 22;
    got = fileno(fin);
    if (got < 3 || got > 0xff) return 23;
    if ((fout = fopen("/dev/null", "rb")) == NULL) return 24;
    if (fclose(fout)) return 25;
    if ((fout = freopen(argv[2], "wb", fout)) == NULL) return 26;
  } else {
    if ((fin = fopen(argv[1], "rb")) == NULL) return 2;
    got = fileno(fin);
    if (got < 3 || got > 0xff) return 16;
    if ((fout = fopen(argv[2], "wb")) == NULL) return 3;
  }
  got2 = fileno(fout);
  if (got2 < 3 || got2 > 0xff) return 17;
  if (got == got2) return 18;
  if (fgetc(fout) != EOF) return 19;  /* fout is only opened for reading. */
  if (fputc('*', fin) != EOF) return 20;  /* fin is only opened for writing. */
  if (mode == 'c') {  /* Just to check that it works. */
    fflush(fin);
    fflush(fout);
  }
  ofs = got = 0;
  goto check_ofs;
  for (;;) {
    if (mode != 'c' && mode != 'd') {
      if ((got = fread(buf, 1, sizeof(buf), fin)) == 0) break;
    } else {  /* mode == 'c' || mode == 'd'. */
      got = mode == 'c' ? fgetc(fin) : getc(fin);
      if (got == EOF) break;
      if (got == 'L') {
        if (ungetc('x', fin) != 'x') return 61;
        if ((mode == 'c' ? fgetc(fin) : getc(fin)) != 'x') return 62;
      }
      buf[0] = got;
      got = 1;
    }
    if ((ssize_t)got < 0) return 4;  /* fread(...) never neturns negative, such as EOF. */
    if (mode == 'p') {
      for (i = 0; i < got; ++i) {
        if (fputc(buf[i], fout) != buf[i]) return 51;
      }
    } else if (mode == 'q') {
      for (i = 0; i < got; ++i) {
        if (putc(buf[i], fout) != buf[i]) return 52;
      }
    } else {
      if ((got2 = fwrite(buf, 1, got, fout)) == 0) return 53;
      if ((ssize_t)got < 0) return 6;  /* fread(...) never neturns negative, such as EOF. */
      if (got2 != got) return 7;  /* fwrite(...) must go as far as possible. */
    }
    ofs += got;
   check_ofs:
    if (ftell(fin) != ofs) return 8;
    if (ftell(fout) != ofs) return 9;
    ofs_delta = (ofs % 13) + (ofs % 17);
    if (mode == 's' && ofs_delta < got) {
      if (fseek(fin, -ofs_delta, SEEK_CUR) != 0) return 12;
      if (fseek(fout, -ofs_delta, SEEK_CUR) != 0) return 13;
      ofs -= ofs_delta;
      if (ftell(fin) != ofs) return 14;
      if (ftell(fout) != ofs) return 15;
    }
  }
  if (ftell(fin) != ofs) return 10;
  if (ftell(fin) != ofs) return 11;
  if (mode != 'a' && mode != 'f') {  /* Let autoflush at exit(3) time take care of writing unflushed data to fout. */
    if (fclose(fout)) return 4;
    if (fclose(fin)) return 54;
    if (getc(fin) != EOF) return 55;
    if (getc(fout) != EOF) return 56;
    if (putc('*', fin) != EOF) return 57;
    if (putc('*', fout) != EOF) return 58;
  }
  return 0;
}
