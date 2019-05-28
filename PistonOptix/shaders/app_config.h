 // This header with defines is included in all shaders
 // to be able to switch different code paths at a central location.
 // Changing any setting here will rebuild the whole solution.

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// From the OptiX Header:
// Unless compatibility with SM_10 is needed, new code should use this define and rely on the new templated version of rtCallableProgram.
#define RT_USE_TEMPLATED_RTCALLABLEPROGRAM 1

// DAR Prevent that division by very small floating point values results in huge values, for example dividing by pdf.
#define DENOMINATOR_EPSILON 1.0e-6f

// 0 == Disable all OptiX exceptions, rtPrintfs and rtAssert functionality. (Benchmark only in this mode!)
// 1 == Enable  all OptiX exceptions, rtPrintfs and rtAssert functionality. (Really only for debugging, big performance hit!)
#define USE_DEBUG_EXCEPTIONS 0

#define USE_SHADER_TONEMAP 0


#endif // APP_CONFIG_H
