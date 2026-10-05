/*
 * Compressonator's three x86 SIMD kernels, as the portable scalar kernel. Compiled instead of cmp_core's
 * CMP_Core_SSE / CMP_Core_AVX / CMP_Core_AVX512 on every Linux and macOS build by core/CMakeLists.txt
 * (whose comment says why all of Unix and not only ARM); not part of AGRemapCore.
 *
 * Those three are x86 through and through -- <immintrin.h>, __m256, _mm256_* -- and cmp_core builds
 * them unconditionally with -march=nehalem / haswell / knl, which an ARM compiler rejects outright.
 * They cannot simply be left out either: common_def.h UNDEFINES the ASPM_GPU that cmp_core's CMake
 * sets for Unix, so bc1_encode_kernel.cpp's CPU dispatch (bc1ToggleSIMD, in bc1_cmp.h) is live and
 * names all three -- the link fails without them (checked against the vendored sources).
 *
 * So each is defined here as the portable one: bc1_cmp.h's own scalar _cpu_bc1ComputeBestEndpoints,
 * the function the dispatch falls back to when no extension is available. That header's functions are
 * 'static', so this file gets its own copy of the scalar one to call, and the result is the scalar
 * result whichever of the three the dispatch picks. That matters on macOS, where cmp_math's
 * GetCPUID is an empty function and the flags it reports are read from uninitialised memory.
 *
 * The block below mirrors the top of cmp_core/shaders/bc1_encode_kernel.cpp, so bc1_cmp.h is seen
 * with the same configuration there as here.
 */

#include "common_def.h"

#ifndef ASPM_OPENCL
#define USE_NEW_SINGLE_HEADER_INTERFACES
#ifdef USE_NEW_SINGLE_HEADER_INTERFACES
#define USE_CMP
#endif
#endif

#include "bc1_encode_kernel.h"

CGU_FLOAT sse_bc1ComputeBestEndpoints(CGU_FLOAT* endpointsOut, CGU_FLOAT* endpointsIn, CGU_FLOAT* prj, CGU_FLOAT* prjError,
                                      CGU_FLOAT* preMRep, int numColours, int numPoints) {
    return _cpu_bc1ComputeBestEndpoints(endpointsOut, endpointsIn, prj, prjError, preMRep, numColours, numPoints);
}

CGU_FLOAT avx_bc1ComputeBestEndpoints(CGU_FLOAT* endpointsOut, CGU_FLOAT* endpointsIn, CGU_FLOAT* prj, CGU_FLOAT* prjError,
                                      CGU_FLOAT* preMRep, int numColours, int numPoints) {
    return _cpu_bc1ComputeBestEndpoints(endpointsOut, endpointsIn, prj, prjError, preMRep, numColours, numPoints);
}

CGU_FLOAT avx512_bc1ComputeBestEndpoints(CGU_FLOAT* endpointsOut, CGU_FLOAT* endpointsIn, CGU_FLOAT* prj, CGU_FLOAT* prjError,
                                         CGU_FLOAT* preMRep, int numColours, int numPoints) {
    return _cpu_bc1ComputeBestEndpoints(endpointsOut, endpointsIn, prj, prjError, preMRep, numColours, numPoints);
}
