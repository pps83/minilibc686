/* by pts@fazekas.hu at Thu Mar 14 16:43:39 CET 2024
 *
 * Size-optimized C and C++ implementation of qsort(3) with in-place mergesort.
 *
 * Based on ip_merge (C, simplest) at https://stackoverflow.com/a/22839426/97248
 * Based on ip_merge in test/test_qstort_stable_analyze.c.
 * Does the same number of comparisons and swaps as test/test_qstort_stable_analyze.c.
 *
 * Compile: gcc -s -O2 -W -Wall  -ansi -pedantic test/test_qsort_stable_mini.c && ./a.out && echo OK
 * Compile: pathbin/minicc --gcc -ansi -pedantic test/test_qsort_stable_mini.c && ./a.out && echo OK
 * Compile: pathbin/minicc --wcc -ansi -pedantic test/test_qsort_stable_mini.c && ./a.out && echo OK
 * Compile: pathbin/minicc --pcc -ansi -pedantic test/test_qsort_stable_mini.c && ./a.out && echo OK
 * Compile: pathbin/minicc --tcc -ansi -pedantic test/test_qsort_stable_mini.c && ./a.out && echo OK
 * Compile: owcc -I"$WATCOM"/lh -blinux -s -Os -W -Wall -fsigned-char -o a.out -std=c89 test/test_qsort_stable_mini.c && ./a.out && echo OK
 * Compile: gcc -fsanitize=address -m64 -s -O2 -W -Wall  -ansi -pedantic test/test_qsort_stable_mini.c && ./a.out && echo OK
 */

#include <stddef.h>  /* size_t. */
#include <sys/types.h>  /* ssize_t. */

#ifndef DO_SHORTCUT_OPT
#define DO_SHORTCUT_OPT 1
#endif

#if defined(__WATCOMC__) && defined(__386__)  /* !! buggy */
#  define CMPDECL __cdecl
#else
#  define CMPDECL
#endif

/* Constant state for ip_merge and ip_mergesort. */
struct ip_cs {
  const void *base;
  size_t item_size;
  int CMPDECL (*cmp)(const void*, const void*);
};

#ifndef ASSERT
#define ASSERT(x) do { if (!(x)) ++*(char*)0; } while(0)
#endif

/* 0 comparisons, (b-a)//2 swaps.
 *
 * Precondition: a < b.
 *
 * It is quite slow, because it moves 1 byte a time (rather than 4 or 8).
 */
static void ip_reverse(const struct ip_cs *cs, size_t a, size_t b) {
  const size_t item_size = cs->item_size;
  char *cbase = (char*)cs->base;
  char *ca = cbase + item_size * a, *cb = cbase + item_size * b, *ca_end;
  char t;
  size_t cabd;
  while (0 < (ssize_t)(cabd = (cb -= item_size) - ca)) {  /* cabd cast to ssize_t can overflow here. We'll fix it in assembly. */
    /* ip_swap(cs, a, b); */  /* 0 comparisons, 1 (item) swap. */
    for (ca_end = ca + item_size; ca != ca_end; ++ca) {
      t = *ca;
      *ca = ca[cabd];
      ca[cabd] = t;
    }
  }
}

#if defined(__WATCOMC__) && defined(__386__)
  __declspec(naked) static int __watcall ip_cmp(const struct ip_cs *cs, size_t a, size_t b) { (void)cs; (void)a; (void)b; __asm {
		/* EAX: cs; EDX: a, EBX: b. */
		/* TODO(pts): Size-optimize this function. */
		push ecx
		push esi
		mov ecx, [eax+4]  /* ECX := cs->item_size. */
		mov esi, [eax+8]  /* ESI := cs->cmp. */
		mov eax, [eax]  /* EAX := cs->base. */
		imul ebx, ecx  /* EBX := b * cs->item_size. */
		add ebx, eax
		push ebx  /* arg2. */
		imul edx, ecx  /* EDX := a * cs->item_size. */
		add edx, eax
		push edx  /* arg1. */
		call esi  /* May ruin EDX and ECX. Return value in EAX. */
		pop edx  /* Clean up arg1 from stack. */
		pop edx  /* Clean up arg2 from stack. */
		pop esi
		pop ecx
		ret
  } }
#else
  static int ip_cmp(const struct ip_cs *cs, size_t a, size_t b) {
    return cs->cmp((char*)cs->base + cs->item_size * a, (char*)cs->base + cs->item_size * b);
  }
void reverse_(char *a, char *b) {
    char c;
    for (--b; a < b; a++, b--) {
      c = *a; *a = *b; *b = c;
    }
  }
#endif


/* In-place merge of [a,b) and [b,c) to [a,c) within base.
 *
 * Precondition: a <= b && b <= c.
 */
static void ip_merge(const struct ip_cs *cs, size_t a, size_t b, size_t c) {
  size_t p, q, i;
  if (a == b || b == c) return;
  if (c - a == 2) {
    if (ip_cmp(cs, b, a) < 0) {  /* 1 comparison. */
      ip_reverse(cs, a, c);  /* Same as: ip_swap(cs, a, b); */  /* 1 swap. */
    }
    return;
  }
  /* Finds first element not less (for is_lower) or greater (for !is_lower)
   * than key in sorted sequence [low,high) or end of sequence (high) if not found.
   * ceil(log2(high-low+1)) comparisons, which is == ceil(log2(min(b-a,c-b)+1)) <= ceil(log2((c-a)//2+1)).
   */
  if (b - a > c - b /* is_lower */) {
    /* key = */ p = a + ((b - a) >> 1); /* low = */ q = b; /* high = c; */ i = c - b;
    for (/* i = high - low */; i != 0; i >>= 1) {  /* low = ip_bound(cs, low, high, key, is_lower); */
      /* mid = low + (i >> 1); */
      if (ip_cmp(cs, p, q + (i >> 1)) >= 1) {
        q += (i >> 1) + 1;
        i--;
      }
    }
  } else {
    /* key = */ q = b + ((c - b) >> 1); /* low = */ p = a; /* high = b; */ i = b - a;
    for (/* i = high - low */; i != 0; i >>= 1) {  /* low = ip_bound(cs, low, high, key, is_lower); */
      /* mid = low + (i >> 1); */
      if (ip_cmp(cs, q, p + (i >> 1)) >= 0) {
        p += (i >> 1) + 1;
        i--;
      }
    }
  }
  /* ASSERT(is_lower == (p < q)); */
  /* Let t be the total number of items in [p,b) and [b,q), i.e. t == (b-p)+(q-b) == q-p.
   *
   * If is_lower, then t == (b-key)+(q-b) == q-key <= c-key == c-a-(b-a)//2 == (c-a)-(b-a)//2 == -(a+a-c-c+b-a)//2 == -(b-c+a-c)//2 == ceil((c-b+c-a)/2) <= ceil((c-a)//2+(c-a))/2 <= ceil((c-a)*3/4).
   * otherwise         t == (b-p)+(key-b) == key-p <= key-a == b+(c-b)//2-a == (b-a)+(c-b)//2 ==  (b+b-a-a+c-b)//2 ==  (b-a+c-a)//2 <= ((c-a)//2+(c-a))//2 == (c-a)*3//4.
   *
   * If is_lower, then t == (b-key)+(q-b) == q-key >= b-key == b-a-(b-a)//2 == ceil((b-a)/2) >= ceil((c-a)/4).
   * otherwise         t == (b-p)+(key-b) == key-p >= key-b == b+(c-b)//2-b == (c-b)//2 >= (c-a)//4.
   *
   * Thus we have (c-a)//4 <= t <= ceil((c-a)*3/4), no matter the value of is_lower.
   */
  if (p != b && b != q) {  /* swap adjacent sequences [p,b) and [b,q). */
    /* Let's count the total number of swaps in the 3 ip_reverse(...) calls below: (b-p)//2 + (q-b)//2 + (q-p)//2 <= (b-p+q-b+q-p)//2 == q-p == (b-p)+(q-b) == t. */
    ip_reverse(cs, p, b);
    ip_reverse(cs, b, q);
    ip_reverse(cs, p, q);
  }
  b = p + (q - b);  /* Sets b_new. */
  /* Let s be the number of items in the 1st recursive ip_merge call, i.e. b_new-a.
   * Similarly to the proof above for t, it's possible to prove that
   * (c-a)//4 <= s <= ceil((c-a)*3/4), no matter the value of is_lower.
   */
  ip_merge(cs, a, p, b);
  ip_merge(cs, b, q, c);
}

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
 * * With DO_SHORTCUT_OPT:
 *   * If the input is already sorted, then 0.
 *   * If large chunks of the input is already shorted, then less.
 *
 * Number of comparisons:
 *
 * * O(n*log(n)*log(n)), but typically much less.
 * * If n <= 1, then 0.
 * * If n == 2, then at most 1.
 * * With DO_SHORTCUT_OPT:
 *   * If n >= 2, then less than 0.5308 * n * log2(n) * log2(n).
 *   * If 2 <= n <= 2**32, then less than 1.9844 * n * log2(n).
 *   * If 2 <= n <= 2**64, then less than 2.0078 * n * log2(n).
 *   * If 2 <= n <= 2**128, then less than 2.0193 * n * log2(n).
 *   * If 2 <= n <= 2**256, then less than 2.0251 * n * log2(n).
 *   * If 2 <= n <= 2**512, then less than 2.0279 * n * log2(n).
 *   * If the input is already sorted, and n >= 1, then n-1.
 * * Without DO_SHORTCUT_OPT:
 *   * If n >= 2, then less than 0.5 * n * log2(n) * log2(n).
 *   * If 2 <= n <= 2**32, then less than 1.9683 * n * log2(n).
 *   * If 2 <= n <= 2**64, then less than 1.9998 * n * log2(n).
 *   * If 2 <= n <= 2**128, then less than 2.0154 * n * log2(n).
 *   * If 2 <= n <= 2**256, then less than 2.0232 * n * log2(n).
 *   * If 2 <= n <= 2**512, then less than 2.0270 * n * log2(n).
 *
 * Uses O(log(n)) memory, mostly recursive calls to ip_merge(...). Call
 * depth is less than log(n)/log(4/3)+2.
 */
void ip_mergesort(void *base, size_t n, size_t item_size, int CMPDECL (*cmp)(const void *, const void *)) {
  size_t a, b, d;
  struct ip_cs cs;
  cs.base = base; cs.item_size = item_size; cs.cmp = cmp;
  for (d = 1; d != 0 && d < n; d <<= 1) {  /* We check `d != 0' to detect overflow in the previous: `d <<= 1'. */
    for (a = 0; a + d < n; a = b) {
      b = a + (d << 1);
#if DO_SHORTCUT_OPT
      /* Shortcut if [a,c) is already sorted. */
      if (d > 1 && ip_cmp(&cs, a + d - 1, a + d) <= 0) continue;
#endif
      ip_merge(&cs, a, a + d,  b > n ? n : b);
    }
  }
}


/* --- Tests. */

/*
 * Copyright (C) 2005 - 2020 Rich Felker
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>

#if 0
static int CMPDECL debugcmp(const void *a, const void *b) {
  printf("DEBUGCMP (%s) (%s)\n", a, b);
  return 42;
}
#endif

static int CMPDECL scmp(const void *a, const void *b) {
	return strcmp(*(char **)a, *(char **)b);
}

static int CMPDECL icmp(const void *a, const void *b) {
	return *(int*)a - *(int*)b;
}

static int CMPDECL ricmp(const void *a, const void *b) {  /* Sorts descending. */
	return *(int*)b - *(int*)a;
}

static unsigned cmp_count;
static int CMPDECL rhucmp(const void *a, const void *b) {  /* Sorts descending, ignores low 16 bits. */
	++cmp_count;
	return (*(unsigned*)b >> 16) - (*(unsigned*)a >> 16);
}

struct three {
    unsigned char b[3];
};

#define i3(x) { { (unsigned char) ((x) >> 16), (unsigned char) ((x) >> 8), (unsigned char) ((x) >> 0) } }

static int CMPDECL tcmp(const void *av, const void *bv) {
    const struct three *a = (const struct three*)av, *b = (const struct three*)bv;
    int c;
    int i;

    for (i = 0; i < 3; i++) {
        c = (int) a->b[i] - (int) b->b[i];
        if (c)
            return c;
    }
    return 0;
}

#define FAIL(m) do {                                            \
        printf(__FILE__ ":%d: %s failed\n", __LINE__, m);       \
        err++;                                                  \
    } while(0)

int main(int argc, char **argv) {
	int i;
	int err=0;
	/* 26 items -- even */
	const char *s[] = {
		"Bob", "Alice", "John", "Ceres",
		"Helga", "Drepper", "Emeralda", "Zoran",
		"Momo", "Frank", "Pema", "Xavier",
		"Yeva", "Gedun", "Irina", "Nono",
		"Wiener", "Vincent", "Tsering", "Karnica",
		"Lulu", "Quincy", "Osama", "Riley",
		"Ursula", "Sam"
	};
	/* 23 items -- odd, prime */
	int n[] = {
		879045, 394, 99405644, 33434, 232323, 4334, 5454,
		343, 45545, 454, 324, 22, 34344, 233, 45345, 343,
		848405, 3434, 3434344, 3535, 93994, 2230404, 4334
	};

	int nx[256];
	const int nx_sizes[] = {0, 1, 2, 3, 4, 5, 6, sizeof(nx) / sizeof(nx[0])};
	int nxsi;
	unsigned expected_cmp_count;

        struct three t[] = {
                i3(879045), i3(394), i3(99405644), i3(33434), i3(232323), i3(4334), i3(5454),
                i3(343), i3(45545), i3(454), i3(324), i3(22), i3(34344), i3(233), i3(45345), i3(343),
                i3(848405), i3(3434), i3(3434344), i3(3535), i3(93994), i3(2230404), i3(4334)
        };

	(void)argc; (void)argv;

#if 0
	{
		struct ip_cs cs;
		int r;
		cs.base = "HelloWorld"; cs.item_size = 3; cs.cmp = debugcmp;
		r = ip_cmp(&cs, 1, 2);
		printf("r=%d\n", r);
	}
#endif

	ip_mergesort(s, sizeof(s)/sizeof(char *), sizeof(char *), scmp);
	for (i=0; i<(int) (sizeof(s)/sizeof(char *)-1); i++) {
		if (strcmp(s[i], s[i+1]) > 0) {
			FAIL("string sort");
			for (i=0; i<(int)(sizeof(s)/sizeof(char *)); i++)
				printf("\t%s\n", s[i]);
			break;
		}
	}

	ip_mergesort(n, sizeof(n)/sizeof(int), sizeof(int), icmp);
	for (i=0; i<(int)(sizeof(n)/sizeof(int)-1); i++) {
		if (n[i] > n[i+1]) {
			FAIL("integer sort");
			for (i=0; i<(int)(sizeof(n)/sizeof(int)); i++)
				printf("\t%d\n", n[i]);
			break;
		}
	}

	ip_mergesort(n, sizeof(n)/sizeof(int), sizeof(int), ricmp);
	for (i=0; i<(int)(sizeof(n)/sizeof(int)-1); i++) {
		if (n[i] < n[i+1]) {
			FAIL("integer sort inplace merge");
			for (i=0; i<(int)(sizeof(n)/sizeof(int)); i++)
				printf("\t%d\n", n[i]);
			break;
		}
	}

	for (nxsi = 0; nxsi + 0U < sizeof(nx_sizes) / sizeof(nx_sizes[0]); ++nxsi) {
		for (i = 0; i < nx_sizes[nxsi]; ++i) {
			nx[i] = ~(i >> 3) << 16 | (0xffff - i);
		}
		cmp_count = 0;
		ip_mergesort(nx, nx_sizes[nxsi], sizeof(nx[0]), rhucmp);
		for (i=1; i < nx_sizes[nxsi]; i++) {
			if (nx[i-1] < nx[i]) {
				FAIL("integer sort inplace merge long already_sorted");
				for (i=0; i< nx_sizes[nxsi]; i++)
					printf("\t0x%04x\n", nx[i]);
				break;
			}
		}
		expected_cmp_count = nx_sizes[nxsi] == 0 ? 0 : nx_sizes[nxsi] - 1;
#if DO_SHORTCUT_OPT
		if (cmp_count != expected_cmp_count)
#else
		if (cmp_count < expected_cmp_count)
#endif
		{
			FAIL("too many comparisons for already sorted input");
			printf("cmp_count=%u expected=%u\n", cmp_count, expected_cmp_count);
		}
	}

	for (i = 0; i + 0U < sizeof(nx)/sizeof(nx[0]); ++i) {
		nx[i] = (i % 13) << 16 | (0xffff - i);
	}
	ip_mergesort(nx, sizeof(nx)/sizeof(nx[0]), sizeof(nx[0]), rhucmp);
	for (i=0; i<(int)(sizeof(nx)/sizeof(nx[0])-1); i++) {
		if (nx[i] < nx[i+1]) {
			FAIL("integer sort inplace merge long");
			for (i=0; i<(int)(sizeof(nx)/sizeof(nx[0])); i++)
				printf("\t0x%04x\n", nx[i]);
			break;
		}
	}

        ip_mergesort(t, sizeof(t)/sizeof(t[0]), sizeof(t[0]), tcmp);
	for (i=0; i<(int)(sizeof(t)/sizeof(t[0])-1); i++) {
                if (tcmp(&t[i], &t[i+1]) > 0) {
			FAIL("three byte sort");
			for (i=0; i<(int)(sizeof(t)/sizeof(t[0])); i++)
                                printf("\t0x%02x%02x%02x\n", t[i].b[0], t[i].b[1], t[i].b[2]);
			break;
		}
	}


	return err;
}
