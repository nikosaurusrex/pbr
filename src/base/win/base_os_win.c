#include "base/base_os.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void *
os_reserve(U64 size)
{
    return VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
}

B32
os_commit(void *ptr, U64 size)
{
    return VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != 0;
}

void
os_decommit(void *ptr, U64 size)
{
    VirtualFree(ptr, size, MEM_DECOMMIT);
}

void
os_release(void *ptr, U64 size)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}
