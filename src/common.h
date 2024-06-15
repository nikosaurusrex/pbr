#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define ARR_COUNT(x) ((sizeof(x) / sizeof(*x)))

static inline void
log_fatal(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "[Fatal] ");

    vfprintf(stderr, fmt, args);
    putc('\n', stderr);
    va_end(args);

    exit(1);
}

static inline void
log_dev(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stdout, "[Dev] ");

    vfprintf(stdout, fmt, args);
    putc('\n', stdout);
    va_end(args);
}
