#include <math.h>
#include <stdio.h>
#include <stdlib.h>

extern double mini_strtod(const char *nptr, char **endptr);  /* Function under test. */

#define FP_EQ(a, b) ((a) == (b) || (__builtin_isnan(a) && __builtin_isnan(b)))

static char expect(const char *nptr) {
  char *expected_endptr;
  const double expected_value = strtod(nptr, &expected_endptr);
  const int expected_size = expected_endptr - nptr;
  char *endptr;
  const double value = mini_strtod(nptr, &endptr);
  int size = endptr - nptr;
  const double value2 = mini_strtod(nptr, NULL);
  char is_ok = (FP_EQ(value, expected_value) && size == expected_size && FP_EQ(value, expected_value));
  printf("is_ok=%d str=(%s) expected_value=%.17g expected_size=%d value=%.17g size=%d value2=%.17g\n", is_ok, nptr, expected_value, expected_size, value, size, value2);
  return is_ok;
}

int main(int argc, char **argv) {
  int exit_code = 0;
  (void)argc; (void)argv;
  if (!expect("  \t4567")) exit_code |= 1;
  if (!expect("12.25")) exit_code |= 1;
  if (!expect("inf")) exit_code |= 1;
  if (!expect("  -0.0000")) exit_code |= 1;
  if (!expect(" 000")) exit_code |= 1;
  if (!expect("\t-InfINity")) exit_code |= 1;
  if (!expect("+NaN")) exit_code |= 1;
  if (!expect("  -12.345e-67")) exit_code |= 1;
  if (!expect("  +12.345678901234567E67")) exit_code |= 1;
  if (!expect("+infhello")) exit_code |= 1;
  if (!expect("+inhello")) exit_code |= 1;
  if (!expect("+infiniTyLong")) exit_code |= 1;
  return exit_code;
}
