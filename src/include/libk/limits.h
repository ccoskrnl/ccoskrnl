#ifndef __LIBK_LIMITS_H__
#define __LIBK_LIMITS_H__

#include "../types.h"

#define UINT8_MAX               ((uint8_t)(-1))
#define INT8_MAX                (UINT8_MAX >> 1)
#define CHAR_MAX                (INT8_MAX)

#define UINT_MAX                ((uint32_t)(-1))
#define ULONG_MAX               ((uint64_t)(-1))

#define INT_MAX                 (UINT_MAX >> 1)
#define INT_MIN                 ((int32_t)(1 << 31))

#define LONG_MAX                (ULONG_MAX >> 1)
#define LONG_MIN                ((int64_t)(1L << 63))

#define INT64_MAX               (LONG_MAX)
#define INT64_MIN               (LONG_MIN)
#define UINT64_MAX              (ULONG_MAX)

#define UINT64_MIN              0

#endif