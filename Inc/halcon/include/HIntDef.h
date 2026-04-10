/*****************************************************************************
 * HIntDef.h
 *****************************************************************************
 *
 * Project:     HALCON/libhalcon
 * Description: Definitions of HALCON data types
 *
 * (c) 1996-2023 by MVTec Software GmbH
 *                  www.mvtec.com
 *
 * Most of the types in this file are legacy types defined for (source code)
 * compatibility with older HALCON versions. New code should use standard C99
 * data types wherever applicable. One notable exception is HINT4_8 and
 * HUINT4_8, for which there are no good C99 replacements.
 *
 *****************************************************************************/

#ifndef HINTDEF_H
#define HINTDEF_H

#ifndef __midl
#  include <float.h>  /* FLT_MAX, DBL_MAX, ... */
#  include <limits.h> /* LONG_MAX, INT_MAX, ... */
#  include <stdlib.h> /* RAND_MAX */
#endif

#include "hclib/hclib_types.h" /* C99 standard integer types    */

#ifndef HC_NO_LEGACY_TYPES
typedef bool HBOOL;

typedef uint8_t HBYTE;

typedef int HINT;           /* Signed integer. HALCON assumes at least
                               4 bytes */
typedef unsigned int HUINT; /* Unsigned integer. HALCON assumes at least
                               4 bytes */
#  define HUINT_MIN 0
#endif /* #ifndef HC_NO_LEGACY_TYPES */

#ifndef _DOUBLE8
typedef double DOUBLE8;
#else
typedef _DOUBLE8 DOUBLE8;
#endif

#ifdef __vxworks
#  include <types/vxTypesBase.h>
#  ifndef DBL_MAX
#    define DBL_MAX _ARCH_DBL_MAX
#  endif
#  ifndef DBL_MIN
#    define DBL_MIN _ARCH_DBL_MIN
#  endif
#  ifndef FLT_MAX
#    define FLT_MAX _ARCH_FLT_MAX
#  endif
#  ifndef FLT_MIN
#    define FLT_MIN _ARCH_FLT_MIN
#  endif
#endif

/* float and double epsilons: These are the differences between 1 and
   the smallest floating point number of type float/double that is
   greater than 1. As we assume IEEE 754 floating point arithmetic,
   the following values are used. */
#ifndef FLT_EPSILON
#  define FLT_EPSILON 1.19209290e-07F
#endif

#ifndef DBL_EPSILON
#  define DBL_EPSILON 2.2204460492503131e-16
#endif

#ifndef RAND_MAX
#  define RAND_MAX MAXINT
#endif

#ifndef HC_NO_LEGACY_TYPES
typedef int8_t INT1; /* signed integer, 1 byte */
#  define INT1_MIN INT8_MIN
#  define INT1_MAX INT8_MAX

typedef uint8_t UINT1; /* unsigned integer, 1 byte */
#  define UINT1_MIN 0
#  define UINT1_MAX UINT8_MAX

typedef int16_t INT2; /* signed integer, 2 bytes */
#  define INT2_MIN INT16_MIN
#  define INT2_MAX INT16_MAX

typedef uint16_t UINT2; /* unsigned integer, 2 bytes */
#  define UINT2_MIN 0
#  define UINT2_MAX UINT16_MAX

typedef int32_t INT4; /* signed integer, 4 bytes */
#  define INT4_MIN INT32_MIN
#  define INT4_MAX INT32_MAX

typedef uint32_t UINT4; /* unsigned integer, 4 bytes */
#  define UINT4_MIN 0
#  define UINT4_MAX UINT32_MAX
#endif /* #ifndef HC_NO_LEGACY_TYPES */

/*
 * Defining HINT4_8 and UINT4_8 to be 64 bits on 64 bit systems and 32 bits on
 * 32 bit systems is a bit tricky. The obvious solution would be to use
 * intptr_t and uintptr_t, but this causes problems on platforms where
 * 'intptr_t' is ultimately derived from 'int'. Our C++ language interface has
 * overloaded functions with variants taking both 'int' and 'Hlong' parameters,
 * which is only allowed if 'int' and 'Hlong' are distinct types. Since typedef
 * does not create distinct types in C++, we must make sure 'Hlong' is
 * ultimately derived from a different type. The 'long' type nicely fits our
 * requirements, as it is 64 bits on all our supported platforms except for 64
 * bit Windows, and 32 bits on all 32 bit platforms (except for TI C6X DSPs
 * using COFF object format - which is obsolete - where it is 40 bits).
 */
#if defined(_WIN64)
typedef __int64 HINT4_8;
typedef __int64 INT4_8;
#  define INT4_8_FORMAT "I64"
#  define INT4_8_MIN _I64_MIN
#  define INT4_8_MAX _I64_MAX

typedef unsigned __int64 UINT4_8;
#  define UINT4_8_MIN 0UL
#  define UINT4_8_MAX _UI64_MAX
#  define UINT4_8_FORMAT INT4_8_FORMAT

#else
typedef long     HINT4_8;
typedef long     INT4_8;
#  define INT4_8_MIN LONG_MIN
#  define INT4_8_MAX LONG_MAX
#  define INT4_8_FORMAT "l"

typedef unsigned long UINT4_8;
#  define UINT4_8_MIN 0UL
#  define UINT4_8_MAX ULONG_MAX
#  define UINT4_8_FORMAT INT4_8_FORMAT
#endif

/* Hlong/Hulong is the same as INT4_8/UINT4_8 */
typedef INT4_8 Hlong;
#define HLONG_MIN INT4_8_MIN
#define HLONG_MAX INT4_8_MAX
#define HLONG_FORMAT INT4_8_FORMAT
#define LONG_FORMAT INT4_8_FORMAT /* deprecated, use HLONG_FORMAT     */

typedef UINT4_8 Hulong;
#define HULONG_MIN UINT4_8_MIN
#define HULONG_MAX UINT4_8_MAX

#ifndef HC_NO_LEGACY_TYPES
typedef int64_t HINT8; /* signed integer, 8 bytes */
#  define HINT8_MIN INT64_MIN
#  define HINT8_MAX INT64_MAX
#  if defined(_WIN32)
#    define HINT8_FORMAT "I64"
#  else
#    define HINT8_FORMAT "ll"
#  endif

typedef uint64_t HUINT8; /* unsigned integer, 8 bytes */
#  define HUINT8_MIN 0U
#  define HUINT8_MAX UINT64_MAX
#endif /* #ifndef HC_NO_LEGACY_TYPES */

#if !defined(HC_LARGE_IMAGES)
#  define HIMGCOOR_MIN INT16_MIN
#  define HIMGCOOR_MAX INT16_MAX
#  define HITEMCNT_MIN INT32_MIN
#  define HITEMCNT_MAX INT32_MAX
#  define HSUBCOOR_MIN FLT_MIN
#  define HSUBCOOR_MAX FLT_MAX
#  define HSUBCOOR_EPS FLT_EPSILON
#  define HSUBATTR_MIN FLT_MIN
#  define HSUBATTR_MAX FLT_MAX
#else
#  define HIMGCOOR_MIN INT32_MIN
#  define HIMGCOOR_MAX INT32_MAX
#  define HITEMCNT_MIN INT4_8_MIN
#  define HITEMCNT_MAX INT4_8_MAX
#  define HSUBCOOR_MIN DBL_MIN
#  define HSUBCOOR_MAX DBL_MAX
#  define HSUBCOOR_EPS DBL_EPSILON
#  define HSUBATTR_MIN DBL_MIN
#  define HSUBATTR_MAX DBL_MAX
#endif

/*
 * Define the extreme values of double that fit into the corresponding
 * integer data type such that:
 * (INTX)INTX_MINDOUBLE >= INTX_MIN and (INTX)INTX_MAXDOUBLE <= INTX_MAX.
 *
 * Notice that due to the limitation of the resolution of double,
 * INT8_MINDOUBLE and INT8_MAXDOUBLE do not match INT8_MIN and INT8_MAX.
 */
#define INT_MINDOUBLE (double)INT_MIN
#define INT_MAXDOUBLE (double)INT_MAX

#define INT2_MINDOUBLE (double)INT16_MIN /* -32768.0  */
#define INT2_MAXDOUBLE (double)INT16_MAX /*  32767.0 */

#define INT4_MINDOUBLE (double)INT32_MIN /* -2147483648.0 */
#define INT4_MAXDOUBLE (double)INT32_MAX /*  2147483647.0 */

#define INT8_MINDOUBLE (double)(INT64_MIN)        /* -9223372036854775800.0 */
#define INT8_MAXDOUBLE (double)(INT64_MAX - 1023) /*  9223372036854774800.0 */

#if INT4_8_MAX == INT32_MAX
#  define INT4_8_MINDOUBLE INT4_MINDOUBLE
#  define INT4_8_MAXDOUBLE INT4_MAXDOUBLE
#else
#  define INT4_8_MINDOUBLE INT8_MINDOUBLE
#  define INT4_8_MAXDOUBLE INT8_MAXDOUBLE
#endif

/* For legacy compatibility only. Prefer defines above. */
#define INT_4_8_MAXDOUBLE INT4_8_MAXDOUBLE

/* At least an address must fit into MACHINE_WORD */
typedef intptr_t MACHINE_WORD;

#endif /* !HINTDEF_H */
