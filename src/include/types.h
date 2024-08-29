#ifndef __TYPES_H__
#define __TYPES_H__

#define NULL                        (void*)0

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

typedef struct _uint128
{
    uint64_t u0;
    uint64_t u1;

} uint128_t, oword;

typedef struct _uint256
{
    uint64_t u0;
    uint64_t u1;
    uint64_t u2;
    uint64_t u3;

} uint256_t, yword;

typedef struct _uint512
{
    uint64_t u0;
    uint64_t u1;
    uint64_t u2;
    uint64_t u3;
    uint64_t u4;
    uint64_t u5;
    uint64_t u6;
    uint64_t u7;

} uint512_t, zword;


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


/**
 * Return offset of the member_name from struct_type base.
 * 
 * @param[in]   struct_type     Specific struct type.
 * @param[in]   member_name     The member name was defined in struct_type.
 * 
 * @retval      Offset of the member_name from struct_type base.     
*/
#define element_offset(struct_type, member_name)    ((uint64_t)(&((struct_type*)0)->member_name))

/**
 * Get base address of struct type.
 * 
 * @param[in]   struct_type     Specific struct type.
 * @param[in]   member_name     The member name was defined in struct_type.
 * @param[in]   member_ptr      The pointer that points member_name.
 * 
 * @retval      A pointer that points struct_type.
*/
#define struct_base(struct_type, member_name, member_ptr) \
    (struct_type*)((uint64_t)member_ptr - element_offset(struct_type, member_name))

#endif
