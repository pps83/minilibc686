#ifndef _STRING_H
#define _STRING_H
#include <_preincl.h>

#include <sys/types.h>

__LIBC_FUNC(int, strcasecmp, (const char *l, const char *r),);
__LIBC_FUNC(int, strncasecmp, (const char *l, const char *r, size_t n),);
__LIBC_FUNC(char *, strchr, (const char *s, int c),);
__LIBC_FUNC(char *, index, (const char *s, int c),);
__LIBC_FUNC(size_t, strlen, (const char *s),);
__LIBC_FUNC(char *, strtok, (char *__restrict__ s, const char *__restrict__ sep),);
__LIBC_FUNC(char *, strcpy, (char *dest, const char *src),);
__LIBC_FUNC(char *, strcat, (char *dest, const char *src),);
__LIBC_FUNC(int, strcmp, (const char *s1, const char *s2),);
__LIBC_FUNC(char *, strstr, (const char *haystack, const char *needle),);
#ifdef __MINILIBC686__
/* It is like strstr(...), but scanning haystack for needle[0] is much faster. */
__LIBC_FUNC(char *, strstr_faster, (const char *haystack, const char *needle),);
#endif  /* __MINILIBC686__ */
__LIBC_FUNC(void *, memcpy, (void *dest, const void *src, size_t n),);
__LIBC_FUNC(void *, memmove, (void *dest, const void *src, size_t n),);
__LIBC_FUNC(int, memcmp, (const void *s1, const void *s2, size_t n),);
__LIBC_FUNC(void *, memset, (void *s, int c, size_t n),);
__LIBC_FUNC(char *, memchr, (const char *s, int c, size_t n),);

#if defined(__MINILIBC686__)
  __LIBC_FUNC_RP3(void, memswap, (void *a, void *b, size_t size),);  /* Not part of standard C. */
  /*__LIBC_FUNC(void, memswap, (void *a, void *b, size_t size),);*/  /* Also present in the .a library, but the other one is recommended. It's also used by mini_qsort_fast(...). */
#endif  /* __MINILIBC686__ */

#endif  /* _STRING_H */
