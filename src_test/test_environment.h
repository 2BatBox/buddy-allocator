#pragma once

#include <stdio.h>
#include <assert.h>

#ifndef TRACE_CALL
#define TRACE_CALL {printf("-> %s()\n", __FUNCTION__);}
#endif // TRACE_CALL
