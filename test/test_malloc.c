#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef CONFIG_MALLOC_MMAP
#  undef __asm__
#  define ALIGN 0x10 
  extern void *mini_malloc(size_t size) __asm__("mini_malloc_mmap");  /* Function under test. */
  extern void *mini_realloc(void *ptr, size_t size) __asm__("mini_realloc_mmap");  /* Function under test. */
  extern void mini_free(void *ptr) __asm__("mini_free_mmap");  /* Function under test. */
#else
#  define ALIGN 4
  extern void *mini_malloc(size_t size);  /* Function under test. */
  extern void *mini_realloc(void *ptr, size_t size);  /* Function under test. */
  extern void mini_free(void *ptr);  /* Function under test. */
#endif

static const char start_msg[] = "Start of block.";
static const char end_msg[] = "End of block.";

static void fill_block(char *p, unsigned size) {
  memset(p, '*', size);
  memcpy(p, start_msg, sizeof(start_msg));
  memcpy(p + size - sizeof(end_msg), end_msg, sizeof(end_msg));
}

static int is_block_intact(const char *p, unsigned size) {
  return memcmp(p, start_msg, sizeof(start_msg)) == 0 &&
         p[sizeof(start_msg)] == '*' &&
         p[size - sizeof(end_msg) - 1] == '*' &&
         memcmp(p + size - sizeof(end_msg), end_msg, sizeof(end_msg)) == 0;
}

int main(int argc, char **argv) {
  const int size = 12345;
  char *p, *p0;
  (void)argc; (void)argv;
  if (!(p = mini_malloc(size))) return 2;
  if ((size_t)p & (ALIGN - 1)) return 3;  /* Not aligned. */
  fill_block(p, size);
  if (!is_block_intact(p, size)) return 4;
  if (!(p = mini_realloc(p, size))) return 5;
  if ((size_t)p & (ALIGN - 1)) return 6;  /* Not aligned. */
  mini_free(0);  /* No-op. */
  if (!is_block_intact(p, size)) return 7;
  if (!(p = mini_realloc(p, size + 4567))) return 8;
  if ((size_t)p & (ALIGN - 1)) return 9;  /* Not aligned. */
  if (!is_block_intact(p, size)) return 10;
  mini_free(p);
#ifndef CONFIG_MALLOC_MMAP
  p0 = p;
  if ((p = mini_malloc(size + 4567)) != p0) return 50;  /* We get back the same block, because we ask for the same size. */
  mini_free(p);
#endif
  if (!(p = mini_malloc(0))) return 11;  /* uClibc also return non-NULL here. */
  if ((size_t)p & (ALIGN - 1)) return 12;  /* Not aligned. */
  mini_free(p);
  if (!(p = mini_realloc(0, size))) return 13;
  if ((size_t)p & (ALIGN - 1)) return 14;  /* Not aligned. */
  fill_block(p, size);
  if (!is_block_intact(p, size)) return 15;
  if (mini_realloc(p, 0)) return 16;
#ifndef CONFIG_MALLOC_MMAP
  p0 = p;
  if ((p = mini_malloc(size)) != p0) return 52;  /* We get back the same block, because we ask for the same size. */
  mini_free(p);
#endif
  return 0;  /* EXIT_SUCCESS. */
}
