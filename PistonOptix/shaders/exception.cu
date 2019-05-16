#include "app_config.h"

#include <optix.h>


rtBuffer<float4, 2> sysOutputBuffer; // RGBA32F

rtDeclareVariable(uint2, theLaunchIndex, rtLaunchIndex, );

RT_PROGRAM void exception()
{
#if USE_DEBUG_EXCEPTIONS
	const unsigned int code = rtGetExceptionCode();
	if (RT_EXCEPTION_USER <= code)
	{
		rtPrintf("User exception %d at (%d, %d)\n", code - RT_EXCEPTION_USER, theLaunchIndex.x, theLaunchIndex.y);
	}
	else
	{
		rtPrintf("Exception code 0x%X at (%d, %d)\n", code, theLaunchIndex.x, theLaunchIndex.y);
	}

	sysOutputBuffer[theLaunchIndex] = make_float4(1000000.0f, 0.0f, 1000000.0f, 1.0f);  // RGBA32F super magenta
#endif
}
