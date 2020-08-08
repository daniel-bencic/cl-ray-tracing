#ifndef KERNEL_TYPES_H
#define KERNEL_TYPES_H

typedef unsigned char ubyte;

struct sphere {
        float3 c;
        float r;
        float3 clr;
};

struct world {
        uint w;
        uint h;
        float fov;
        float ar;
        float sc;
        float3 cpos;
        float4 ctrans[4];
        uint sphc;
};

#endif /* KERNEL_TYPES_H */
