#ifndef LINEAR_ALGEBRA_H
#define LINEAR_ALGEBRA_H

#include <math.h>
#include <CL/cl.h>

void add_vec(cl_float3 *v0, const cl_float3 *v1);
void sub_vec(cl_float3 *v0, const cl_float3 *v1);
void mult_vec(cl_float3 *v, const double s);
void normalize(cl_float3 *v);
cl_float dot(const cl_float3 *v0, const cl_float3 *v1);
void cross(const cl_float3 *v0, const cl_float3 *v1, cl_float3 *r);
void identity44(cl_float4 *mat);
void look_at(const cl_float3 *pos, const cl_float3 *tgt, cl_float3 up, cl_float4 *mat);

double rad(const double deg);

#endif /* LINEAR_ALGEBRA */
