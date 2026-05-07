#ifndef MISS_COUNTER_H
#define MISS_COUNTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  MISS_3C_COMPULSORY = 0,
  MISS_3C_CAPACITY   = 1,
  MISS_3C_CONFLICT   = 2,
  MISS_3C_COUNT      = 3
} Miss3CType;

void miss_3c_init(unsigned proc_id, unsigned total_fa_lines);

int miss_3c_on_access(unsigned proc_id, uint64_t line_addr, int is_miss);

#ifdef __cplusplus
}
#endif

#endif
