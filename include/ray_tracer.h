#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include <CL/cl.h>
#include "common.h"
#include "host_kernel_types.h"
#include "util.h"

struct ray_tracer {
        uint w;
        uint h;
        uint ch;
        cl_context cont;
        cl_device_id dev;
        cl_command_queue cmdq;
        cl_mem wl;
        cl_mem sphs;
        cl_mem img;
        cl_program prog;
        cl_kernel kl;
        size_t witems[2];
        size_t wgrps[2];
};

int setup_ray_tracing(struct ray_tracer *rt, const uint w, const uint h, const uint ch, const uint sphc);
void transfer_world(const struct ray_tracer *rt, const struct world *w);
void transfer_payload(const struct ray_tracer *rt, const struct sphere *sphs, const uint sphc);
ubyte *trace_rays(const struct ray_tracer *rt, const uint w, const uint h, const uint ch);
void free_ray_tracing(const struct ray_tracer *rt);

cl_program create_cl_program(const cl_context *cont, const cl_device_id *dev, const char *path); 

#endif /* RAY_TRACER */
