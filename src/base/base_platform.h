// inspired by https://github.com/EpicGamesExt/raddebugger/blob/master/src/base/base_core.h

#pragma once

#if defined(__clang__)
#define COMPILER_CLANG 1

#if defined(_WIN32)
#define OS_WINDOWS 1
#elif defined(__gnu_linux__) || defined(__linux__)
#define OS_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#define OS_MAC 1
#else
#error Unsupported compiler on OS.
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#define ARCH_X64 1
#elif defined(i386) || defined(__i386) || defined(__i386__)
#define ARCH_X86 1
#elif defined(__aarch64__)
#define ARCH_ARM64 1
#elif defined(__arm__)
#define ARCH_ARM32 1
#else
#error Architecture not supported.
#endif

#elif defined(_MSC_VER)
#define COMPILER_MSVC 1
#if defined(_WIN32)
#define OS_WINDOWS 1
#else
#error Unsupported compiler on OS.
#endif

#if defined(_M_AMD64)
#define ARCH_X64 1
#elif defined(_M_IX86)
#define ARCH_X86 1
#elif defined(_M_ARM64)
#define ARCH_ARM64 1
#elif defined(_M_ARM)
#define ARCH_ARM32 1
#else
#error Architecture not supported.
#endif

#elif defined(__GNUC__) || defined(__GNUG__)
#define COMPILER_GCC 1

#if defined(__gnu_linux__) || defined(__linux__)
#define OS_LINUX 1
#else
#error Unsupported compiler on OS.
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#define ARCH_X64 1
#elif defined(i386) || defined(__i386) || defined(__i386__)
#define ARCH_X86 1
#elif defined(__aarch64__)
#define ARCH_ARM64 1
#elif defined(__arm__)
#define ARCH_ARM32 1
#else
#error Architecture not supported.
#endif

#else
#error Compiler not supported.
#endif

#if defined(__cplusplus)
#define LANG_CPP 1
#else
#define LANG_C 1
#endif
