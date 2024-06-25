#pragma once

#include "base.h"

C_LINKAGE_BEGIN

void *os_reserve(U64 size);
B32   os_commit(void *ptr, U64 size);
void  os_decommit(void *ptr, U64 size);
void  os_release(void *ptr, U64 size);

C_LINKAGE_END
