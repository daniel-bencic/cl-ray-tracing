#include "linear_algebra.h"

void add_vec(cl_float3 *v0, const cl_float3 *v1)
{
        v0->x += v1->x;
        v0->y += v1->y;
        v0->z += v1->z;
}

void sub_vec(cl_float3 *v0, const cl_float3 *v1)
{
        v0->x -= v1->x;
        v0->y -= v1->y;
        v0->z -= v1->z;
}

void mult_vec(cl_float3 *v, const double s)
{
        v->x *= s;
        v->y *= s;
        v->z *= s;
}

void normalize(cl_float3 *v)
{
        cl_float l;

        l = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);

        v->x /= l;
        v->y /= l;
        v->z /= l;
}

cl_float dot(const cl_float3 *v0, const cl_float3 *v1)
{
        return v0->x * v1->x + v0->y * v1->y + v0->z * v1->z;
}

void cross(const cl_float3 *v0, const cl_float3 *v1, cl_float3 *r)
{
        r->x = v0->y * v1->z - v0->z * v1->y;
        r->y = v0->z * v1->x - v0->x * v1->z;
        r->z = v0->x * v1->y - v0->y * v1->x;
}

void identity44(cl_float4 *mat)
{
        for (size_t i = 0; i < 4; ++i) {
                for (size_t j = 0; j < 4; ++j) {
                        if (i == j)
                                mat[i].s[j] = 1.0f;
                        else
                                mat[i].s[j] = 0.0f;
                }
                        
        }
}

void look_at(const cl_float3 *pos, const cl_float3 *tgt, cl_float3 tmp, cl_float4 *mat)
{
        cl_float3 front, right, up;

        front.x = pos->x - tgt->x;
        front.y = pos->y - tgt->y;
        front.z = pos->z - tgt->z;
        normalize(&front);
        
        normalize(&tmp);
        cross(&tmp, &front, &right);

        cross(&front, &right, &up);

        mat[0].x = right.x;
        mat[0].y = up.x;
        mat[0].z = front.x;
        mat[0].w = pos->x;
        mat[1].x = right.y;
        mat[1].y = up.y;
        mat[1].z = front.y;
        mat[1].w = pos->y;
        mat[2].x = right.z;
        mat[2].y = up.z;
        mat[2].z = front.z;
        mat[2].w = pos->z;
        mat[3].x = 0.0f;
        mat[3].y = 0.0f;
        mat[3].z = 0.0f;
        mat[3].w = 1.0f;
}

double rad(const double deg)
{
        return deg * M_PI / 180.0;
}
