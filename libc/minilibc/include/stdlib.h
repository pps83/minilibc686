#ifndef _STDLIB_H
#define _STDLIB_H
#include <_preincl.h>

#include <sys/types.h>

#define NULL ((void*)0)

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

__LIBC_VAR(extern char **, environ);
#ifdef __WATCOMC__  /* There is no other way with `wcc386 -za'. */
#  pragma aux environ "_mini_*"
#endif

__LIBC_FUNC(__LIBC_NORETURN void, exit, (int exit_code), __LIBC_NOATTR);   /* Flushes stdio streams first. To prevent flushing, _exit(...) instead. */
__LIBC_FUNC(__LIBC_NORETURN void, abort, (void), __LIBC_NOATTR);   /* Doesn't flush stdio streams. Both behaviors are OK according to POSIX. Limitation: it doesn't call the SIGABRT handler. */

__LIBC_FUNC(char *, getenv, (const char *name), __LIBC_NOATTR);

#define RAND_MAX 0x7fffffff
__LIBC_FUNC(int, rand, (void), __LIBC_NOATTR);
__LIBC_FUNC(void, srand, (unsigned seed), __LIBC_NOATTR);

__LIBC_FUNC(int, mkstemp, (char *__template), __LIBC_NOATTR);

/* Limitation: they don't set errno on overflow in minilibc686. */
__LIBC_FUNC(long, strtol, (const char *nptr, char **endptr, int base), __LIBC_NOATTR);
__LIBC_FUNC(unsigned long, strtoul, (const char *nptr, char **endptr, int base), __LIBC_NOATTR);
__LIBC_FUNC(__extension__ long long, strtoll, (const char *nptr, char **endptr, int base), __LIBC_NOATTR);
__LIBC_FUNC(__extension__ unsigned long long, strtoull, (const char *nptr, char **endptr, int base), __LIBC_NOATTR);

__LIBC_FUNC(int,       atoi,  (const char *nptr), __LIBC_NOATTR);
__LIBC_FUNC(long,      atol,  (const char *nptr), __LIBC_NOATTR);
__LIBC_FUNC(__extension__ long long, atoll, (const char *nptr), __LIBC_NOATTR);

/* Limitation: they don't set errno on overflow in minilibc686. */
__LIBC_FUNC(float, strtof, (const char *nptr, char **endptr), __LIBC_NOATTR);
__LIBC_FUNC(double, strtod, (const char *nptr, char **endptr), __LIBC_NOATTR);
__LIBC_FUNC(long double, strtold, (const char *nptr, char **endptr), __LIBC_NOATTR);

#ifndef CONFIG_LIBC_NO_MALLOC
#  if defined(CONFIG_MALLOC_MMAP) && defined(__MINILIBC686__)
    /* This implementation does an mmap(2) call for each allocation, and
     * it rounds up the size to 4 KiB boundary after adding 0x10. Thus it's
     * suitable for a few large allocations.
     */
#    ifdef __WATCOMC__
      void *malloc(size_t size);
      void *realloc(void *ptr, size_t size);
      void free(void *ptr);
      void *calloc(size_t nmemb, size_t size);
#      pragma aux malloc "_mini_malloc_mmap"
#      pragma aux realloc "_mini_realloc_mmap"
#      pragma aux free "_mini_free_mmap"
#      pragma aux calloc "_mini_calloc_mmap"
#    else
      void *malloc(size_t size) __asm__("mini_malloc_mmap");
      void *realloc(void *ptr, size_t size) __asm__("mini_realloc_mmap");
      void free(void *ptr) __asm__("mini_free_mmap");
      void *calloc(size_t nmemb, size_t size) __asm__("mini_calloc_mmap");
#    endif
#  else
    __LIBC_FUNC(void *, malloc, (size_t size), __LIBC_NOATTR);
    __LIBC_FUNC(void *, realloc, (void *ptr, size_t size), __LIBC_NOATTR);
    __LIBC_FUNC(void, free, (void *ptr), __LIBC_NOATTR);
    __LIBC_FUNC(void *, calloc, (size_t nmemb, size_t size), __LIBC_NOATTR);
#  endif
#endif

/* In minilibc686: short and stable, but slow: insertion sort with O(n**2)
 * worst time. It's not quicksort because the implementation of insertion
 * sort is shorter.
 */
__LIBC_FUNC(void, qsort, (void *base, size_t n, size_t size, int (*cmp)(const void*, const void*)), __LIBC_NOATTR);
#ifdef __MINILIBC686__
  /* Worst case execution time: O(n*log(n)): less than 3*n*log_2(n)
   * comparisons and swaps. (The number of swaps is usually a bit smaller than
   * the number of comparisons.) The average number of comparisons is
   * 2*n*log_2(n)-O(n). It is very fast if all values are the same (but still
   * does lots of comparisons and swaps). It is not especially faster than
   * average if the input is already ascending or descending (with unique
   * values),
   *
   * Uses a constant amount of memory in addition to the input/output array.
   *
   * Based on heapsort algorithm H from Knuth TAOCP 5.2.3. The original uses a
   * temporary variable (of `size' bytes) and copies elements between it and
   * the array. That code was changed to swaps within the original array.
   *
   * Not stable.
   */
  __LIBC_FUNC(void, qsort_fast, (void *base, size_t n, size_t size, int (*cmp)(const void*, const void*)), __LIBC_NOATTR);
  /* In-place stable sort using in-place mergesort.
   * Same signature and semantics as qsort(3).
   *
   * If you don't need a stable sort, try qsort_fast(...) instead, because
   * that does fewer comparisons. (That may do a bit more swaps though.)
   *
   * If you want to use much less stack space, or you need a shorter qsort(3)
   * implementation, try qsort_fast(...) instead. Please note that that is not
   * stable, and that may do a bit more swaps.
   *
   * The formulas below are mathematically correct, without rounding.
   *
   * Number of item swaps:
   *
   * * O(n*log(n)*log(n)).
   * * If n <= 1, then 0.
   * * If n == 2, then at most 1.
   * * If n >= 2, then less than 0.75 * n * log2(n) * log2(n).
   *
   * Number of comparisons:
   *
   * * O(n*log(n)*log(n)), but typically much less.
   * * If n <= 1, then 0.
   * * If n == 2, then at most 1.
   * * If n >= 2, then less than 0.5 * n * log2(n) * log2(n).
   * * If 2 <= n <= 2**32, then less than 1.9683 * n * log2(n).
   * * If 2 <= n <= 2**64, then less than 1.9998 * n * log2(n).
   * * If 2 <= n <= 2**128, then less than 2.0154 * n * log2(n).
   * * If 2 <= n <= 2**256, then less than 2.0232 * n * log2(n).
   * * If 2 <= n <= 2**512, then less than 2.0270 * n * log2(n).
   *
   * Uses O(log(n)) memory, mostly recursive calls to ip_merge(...). Call
   * depth is less than log(n)/log(4/3)+2.
   */
  __LIBC_FUNC(void, qsort_stable_fast, (void *base, size_t n, size_t size, int (*cmp)(const void*, const void*)), __LIBC_NOATTR);
  /* In-place stable sort using in-place mergesort.
   * Same signature and semantics as qsort(3).
   *
   * If you don't need a stable sort, try qsort_fast(...) instead, because
   * that does fewer comparisons. (That may do a bit more swaps though.)
   *
   * If you want to use much less stack space, or you need a shorter qsort(3)
   * implementation, try qsort_fast(...) instead. Please note that that is not
   * stable, and that may do a bit more swaps.
   *
   * The formulas below are mathematically correct, without rounding.
   *
   * Number of item swaps:
   *
   * * O(n*log(n)*log(n)).
   * * If n <= 1, then 0.
   * * If n == 2, then at most 1.
   * * If n >= 2, then less than 0.75 * n * log2(n) * log2(n).
   * * If the input is already sorted, then 0.
   * * If large chunks of the input is already shorted, then less.
   *
   * Number of comparisons:
   *
   * * O(n*log(n)*log(n)), but typically much less.
   * * If n <= 1, then 0.
   * * If n == 2, then at most 1.
   * * If n >= 2, then less than 0.5308 * n * log2(n) * log2(n).
   * * If 2 <= n <= 2**32, then less than 1.9844 * n * log2(n).
   * * If 2 <= n <= 2**64, then less than 2.0078 * n * log2(n).
   * * If 2 <= n <= 2**128, then less than 2.0193 * n * log2(n).
   * * If 2 <= n <= 2**256, then less than 2.0251 * n * log2(n).
   * * If 2 <= n <= 2**512, then less than 2.0279 * n * log2(n).
   * * If the input is already sorted, and n >= 1, then n-1.
   *
   * Uses O(log(n)) memory, mostly recursive calls to ip_merge(...). Call
   * depth is less than log(n)/log(4/3)+2.
   */
  __LIBC_FUNC(void, qsort_stable_fast_shortcut, (void *base, size_t n, size_t size, int (*cmp)(const void*, const void*)), __LIBC_NOATTR);
#endif

#ifdef __MINILIBC686__
  /* Returns an unaligned pointer. There is no API to free it. Suitable for
   * many small allocations. Be careful: if you use this with unaligned
   * sizes, then regular malloc(...) and realloc(...) may also return
   * unaligned pointers.
   */
  __LIBC_FUNC(void *, malloc_simple_unaligned, (size_t size), __LIBC_NOATTR);
#endif  /* __MINiLIBC686__ */

#endif  /* _STDLIB_H */
