// inspired by https://github.com/EpicGamesExt/raddebugger/blob/master/src/base/base_core.h

#pragma once

#include <stdint.h>

#include "base_platform.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef float    f32;
typedef double   f64;
typedef int8_t   b8;
typedef int32_t  b32;

#if COMPILER_MSVC || (COMPILER_CLANG && OS_WINDOWS)
#pragma section(".rdata$", read)
#define read_only __declspec(allocate(".rdata$"))
#elif (COMPILER_CLANG && OS_LINUX)
#define read_only __attribute__((section(".rodata")))
#else
#define read_only const
#endif

#if LANG_CPP
#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END   }
#define C_LINKAGE       extern "C"
#else
#define C_LINKAGE_BEGIN
#define C_LINKAGE_END
#define C_LINKAGE
#endif

#define KB(n) (((U64)(n)) << 10)
#define MB(n) (((U64)(n)) << 20)
#define GB(n) (((U64)(n)) << 30)
#define TB(n) (((U64)(n)) << 40)

#define Min(A, B)      (((A) < (B)) ? (A) : (B))
#define Max(A, B)      (((A) > (B)) ? (A) : (B))
#define ClampTop(A, X) Min(A, X)
#define ClampBot(X, B) Max(X, B)
#define Clamp(A, X, B) (((X) < (A)) ? (A) : ((X) > (B)) ? (B) : (X))

#define MemoryCopy(dst, src, size) memmove((dst), (src), (size))
#define MemorySet(dst, byte, size) memset((dst), (byte), (size))
#define MemoryCompare(a, b, size)  memcmp((a), (b), (size))
#define MemoryZero(s, z)           memset((s), 0, (z))

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#if COMPILER_MSVC
#define DebugBreak() __debugbreak()
#elif COMPILER_CLANG || COMPILER_GCC
#define DebugBreak() __builtin_trap()
#else
#error Unsupported Compiler.
#endif

#if BUILD_DEBUG
#define Assert(x)                                                                                                                          \
    do {                                                                                                                                   \
        if (!(x)) {                                                                                                                        \
            DebugBreak();                                                                                                                  \
        }                                                                                                                                  \
    } while (0)
#else
#define Assert(x) ((void)0)
#endif

#define NotImplemented Assert("Not Implemented!")

#define AlignPow2(x, b) (((x) + (b)-1) & (~((b)-1)))
#define IsPow2(x)       ((x) != 0 && ((x) & ((x)-1)) == 0)

static f32 pi32 = 3.1415926535897f;

static u64 max_U64 = 0xffffffffffffffffull;
static u32 max_u32 = 0xffffffff;
static u16 max_U16 = 0xffff;
static u8  max_U8  = 0xff;

static s64 max_S64 = (s64)0x7fffffffffffffffull;
static s32 max_S32 = (s32)0x7fffffff;
static s16 max_S16 = (s16)0x7fff;
static s8  max_S8  = (s8)0x7f;

static s64 min_S64 = (s64)0xffffffffffffffffull;
static s32 min_S32 = (s32)0xffffffff;
static s16 min_S16 = (s16)0xffff;
static s8  min_S8  = (s8)0xff;

static const u32 bitmask1  = 0x00000001;
static const u32 bitmask2  = 0x00000003;
static const u32 bitmask3  = 0x00000007;
static const u32 bitmask4  = 0x0000000f;
static const u32 bitmask5  = 0x0000001f;
static const u32 bitmask6  = 0x0000003f;
static const u32 bitmask7  = 0x0000007f;
static const u32 bitmask8  = 0x000000ff;
static const u32 bitmask9  = 0x000001ff;
static const u32 bitmask10 = 0x000003ff;
static const u32 bitmask11 = 0x000007ff;
static const u32 bitmask12 = 0x00000fff;
static const u32 bitmask13 = 0x00001fff;
static const u32 bitmask14 = 0x00003fff;
static const u32 bitmask15 = 0x00007fff;
static const u32 bitmask16 = 0x0000ffff;
static const u32 bitmask17 = 0x0001ffff;
static const u32 bitmask18 = 0x0003ffff;
static const u32 bitmask19 = 0x0007ffff;
static const u32 bitmask20 = 0x000fffff;
static const u32 bitmask21 = 0x001fffff;
static const u32 bitmask22 = 0x003fffff;
static const u32 bitmask23 = 0x007fffff;
static const u32 bitmask24 = 0x00ffffff;
static const u32 bitmask25 = 0x01ffffff;
static const u32 bitmask26 = 0x03ffffff;
static const u32 bitmask27 = 0x07ffffff;
static const u32 bitmask28 = 0x0fffffff;
static const u32 bitmask29 = 0x1fffffff;
static const u32 bitmask30 = 0x3fffffff;
static const u32 bitmask31 = 0x7fffffff;
static const u32 bitmask32 = 0xffffffff;

static const u32 bit1  = (1 << 0);
static const u32 bit2  = (1 << 1);
static const u32 bit3  = (1 << 2);
static const u32 bit4  = (1 << 3);
static const u32 bit5  = (1 << 4);
static const u32 bit6  = (1 << 5);
static const u32 bit7  = (1 << 6);
static const u32 bit8  = (1 << 7);
static const u32 bit9  = (1 << 8);
static const u32 bit10 = (1 << 9);
static const u32 bit11 = (1 << 10);
static const u32 bit12 = (1 << 11);
static const u32 bit13 = (1 << 12);
static const u32 bit14 = (1 << 13);
static const u32 bit15 = (1 << 14);
static const u32 bit16 = (1 << 15);
static const u32 bit17 = (1 << 16);
static const u32 bit18 = (1 << 17);
static const u32 bit19 = (1 << 18);
static const u32 bit20 = (1 << 19);
static const u32 bit21 = (1 << 20);
static const u32 bit22 = (1 << 21);
static const u32 bit23 = (1 << 22);
static const u32 bit24 = (1 << 23);
static const u32 bit25 = (1 << 24);
static const u32 bit26 = (1 << 25);
static const u32 bit27 = (1 << 26);
static const u32 bit28 = (1 << 27);
static const u32 bit29 = (1 << 28);
static const u32 bit30 = (1 << 29);
static const u32 bit31 = (1 << 30);
static const u32 bit32 = (1 << 31);

C_LINKAGE_BEGIN

void log_fatal(const char *fmt, ...);
void log_dev(const char *fmt, ...);

C_LINKAGE_END