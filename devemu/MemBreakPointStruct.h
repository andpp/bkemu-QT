#pragma once

#include "stdint.h"

typedef struct {
  uint16_t nRead;
  uint16_t rAddrs[8];
  uint16_t nWrite;
  uint16_t wAddrs[8];
} MemAccess_t;
