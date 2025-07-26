#ifndef DEFINES_H
#define DEFINES_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                  COMPILER, OS AND ARCHITECTURE DETECTION                     *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#if defined(__clang__)
# define CC_CLANG

# if defined(_WIN32)
#   define OS_WIN
# elif defined(__gnu_linux__)
#   define OS_LINUX
# elif defined(__APPLE__) && defined(__MACH__)
#   define OS_MAC
# else
#   error missing OS detection
# endif

# if defined(__amd64__)
#   define ARCH_X64
# elif defined(__i386__)
#   define ARCH_X86
# elif defined(__aarch64__)
#   define ARCH_ARM
# elif defined(__arm__)
#   define ARCH_ARM
# else
#   error missing ARCH detection
# endif

#elif defined(__GNUC__)
# define CC_GCC

# if defined(_WIN32)
#   define OS_WIN
# elif defined(__gnu_linux__)
#   define OS_LINUX
# elif defined(__APPLE__) && defined(__MACH__)
#   define OS_MAC
# else
#   error missing OS detection
# endif

# if defined(__amd64__)
#   define ARCH_X64
# elif defined(__i386__)
#   define ARCH_X86
# elif defined(__aarch64__)
#   define ARCH_ARM
# elif defined(__arm__)
#   define ARCH_ARM
# else
#   error missing ARCH detection
# endif

#elif defined(_MSC_VER)
# define CC_MSVC

# if defined(_WIN32)
#   define OS_WIN
# else
#   error missing OS detection
# endif

# if defined(_M_AMD64)
#   define ARCH_X64
# elif defined(_M_IX86)
#   define ARCH_X86
# elif defined(_M_ARM)
#   define ARCH_ARM
# else
#   error missing ARCH detection
# endif

#else
# error missing CC detection
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                 BASIC TYPES                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdint.h>

/* Integer Types */
typedef int8_t    S8;
typedef uint8_t   U8;
typedef int16_t   S16;
typedef uint16_t  U16;
typedef int32_t   S32;
typedef uint32_t  U32;
typedef int64_t   S64;
typedef uint64_t  U64;

/* Boolean Types */
typedef U8  B8;
typedef U16 B16;
typedef U32 B32;
typedef U64 B64;

/* Pointer Size Types */
typedef ptrdiff_t SZ;
typedef size_t    UZ;
typedef intptr_t  SP;
typedef uintptr_t UP;

/* Floating Point Types */
typedef float   F32;
typedef double  F64;

/* Function Pointer Type */
typedef void VoidFunc(void);
typedef void (*VoidFuncPtr)(void);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                 BASIC LIMITS                                 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define MIN_S8  INT8_MIN
#define MIN_S16 INT16_MIN
#define MIN_S32 INT32_MIN
#define MIN_S64 INT64_MIN
#define MIN_SZ  PTRDIFF_MIN
#define MIN_SP  INTPTR_MIN

#define MAX_S8  INT8_MAX
#define MAX_S16 INT16_MAX
#define MAX_S32 INT32_MAX
#define MAX_S64 INT64_MAX
#define MAX_SZ  PTRDIFF_MAX
#define MAX_SP  INTPTR_MAX

#define MAX_U8  UINT8_MAX
#define MAX_U16 UINT16_MAX
#define MAX_U32 UINT32_MAX
#define MAX_U64 UINT64_MAX
#define MAX_UZ  SIZE_MAX
#define MAX_UP  UINTPTR_MAX

#include <float.h>

#define EPSILON_F32 FLT_EPSILON
#define EPSILON_F64 DBL_EPSILON

#define MIN_F32 FLT_MIN
#define MIN_F64 DBL_MIN

#define MAX_F32 FLT_MAX
#define MAX_F64 DBL_MAX

#define INFINITY_F32  (1 / 0.0f)
#define INFINITY_F64  (1 / 0.0)

#define NEGATIVE_INFINITY_F32 (-INFINITY_F32)
#define NEGATIVE_INFINITY_F64 (-INFINITY_F64)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                 BOOL & NULL                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdbool.h>

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define null 0

#ifndef __cplusplus

#define nullptr ((void*)0)

#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                HELPER MACROS                                 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#if defined(CC_MSVC) || defined(CC_CLANG)
#  define dll_export __declspec(dllexport)
#  define dll_import __declspec(dllimport)
#elif defined (CC_GCC)
#  define dll_export __attribute__((dllexport))
#  define dll_import __attribute__((dllimport))
#else
#  error dll_export not defined for this compiler
#endif

#define Statement(S) do { S } while(0)

#ifndef AssertBreak
#define AssertBreak() (*(int*)0 = 0)
#endif

#ifdef ENABLE_ASSERT
#define Assert(c) Statement( if (!(c)) { AssertBreak(); } )
#else
#define Assert(c)
#endif

#define Stringify_(S) #S
#define Stringify(S) Stringify_(S)
#define Glue_(A, B) A##B
#define Glue(A, B) Glue_(A, B)

#define ArrayLength(a) (sizeof(a) / sizeof(*a))

#define IntFromPtr(p) ((UP)((char*)(p) - (char*)0))
#define PtrFromInt(n) ((void*)((char*)0 + (n)))

#define Member(T, m) (((T*)0)->m)
#define MemberOffset(T, m) IntFromPtr(&Member(T, m))
#define MemberSize(T, m) sizeof(Member(T, m))

#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Clamp(a, x, b) (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x)))
#define Abs(x)  Max((x), -(x))

#ifdef __cplusplus
#define c_linkage_begin extern "C" {
#define c_linkage_end   }
#define c_linkage       extern "C"
#else
#define c_linkage_begin
#define c_linkage_end
#define c_linkage
#endif

#include <string.h>

#define MemoryZero(p, z) (memset((p), 0, (z)))
#define MemoryZeroStruct(p) MemoryZero((p), sizeof(*(p)))
#define MemoryZeroArray(a) MemoryZero((a), sizeof(a))
#define MemoryZeroTyped(p, c) MemoryZero((p), sizeof(*(p)) * (c))

#define MemoryCopy(d, s, z) (memmove((d), (s), (z)))
#define MemoryCopyStruct(d, s) MemoryCopy((d),(s),Min(sizeof(*(d),sizeof(*(s)))))
#define MemoryCopyArray(d, s) MemoryCopy((d),(s),Min(sizeof(d),sizeof(s)))
#define MemoryCopyTyped(d, s, c) MemoryCopy((d),(s),Min(sizeof(*(d),sizeof(*(s))))*(c))

#define KiB(n) ((U64)(n) << 10)
#define MiB(n) ((U64)(n) << 20)
#define GiB(n) ((U64)(n) << 30)
#define TiB(n) ((U64)(n) << 40)

#endif
