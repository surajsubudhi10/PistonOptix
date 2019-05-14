#pragma once

#ifndef RT_ASSERT_H
#define RT_ASSERT_H

#include "app_config.h"

#include <optix.h>

#include "rt_function.h"

#if USE_DEBUG_EXCEPTIONS

RT_FUNCTION void rtAssert(const bool condition, const unsigned int code)
{
	if (!condition)
	{
		rtThrow(RT_EXCEPTION_USER + code);
	}
}

#else 

#define rtAssert(condition, code)

#endif

#endif // RT_ASSERT_H
