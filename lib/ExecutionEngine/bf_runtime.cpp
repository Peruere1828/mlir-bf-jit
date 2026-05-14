#include "Bf/ExecutionEngine/bf_runtime.h"

#include <stdio.h>

void bf_putchar(int8_t c) {
  printf("%c", c);
  fflush(stdout);
}

int8_t bf_getchar() {
  int32_t val;
  scanf("%d", &val);
  return (int8_t)val;
}
