#ifndef KERNEL_TYPES_H
#define KERNEL_TYPES_H

#include <CL/cl.h>

struct sphere {
        cl_float3 c;
        float r;
        cl_float3 clr;
};

struct world {
        cl_uint w;
        cl_uint h;
        float fov;
        float ar;
        float sc;
        cl_float3 cpos;
        cl_float4 ctrans[4];
        cl_uint sphc;
};

#endif /* KERNEL_TYPES_H */
