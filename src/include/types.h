#ifndef __TYPES_H__
#define __TYPES_H__

#define NULL                        (void*)0

typedef unsigned long long int      uint128_t;
typedef unsigned long int           uint64_t;
typedef unsigned int                uint32_t;
typedef unsigned short              uint16_t;
typedef unsigned char               uint8_t;

typedef long long int               int128_t;
typedef long int                    int64_t;
typedef int                         int32_t;
typedef short                       int16_t;
typedef char                        int8_t;

typedef uint8_t                     byte;
typedef uint16_t                    word;
typedef uint32_t                    dword;
typedef uint64_t                    qword;

typedef uint8_t                     boolean;
#define false                       ((boolean)(0 == 1))
#define true                        ((boolean)(0 == 0))

typedef void*                       handler_t;
typedef uint64_t                    ptr_t;
typedef int64_t                     size_t; 
typedef int                         wch_t;

typedef int64_t                     status_t;

#define ST_SUCCESS                  0
#define ST_FAILED                   1
#define ST_INVALID_PARAMETER        2
#define ST_OUT_OF_RESOURCES         3
#define ST_ERROR(status)            (status != ST_SUCCESS)

#define _in_ 
#define _out_
#define _optional_

#endif