#include "host_kernel_types.h"

float4 transform_vec(__constant float4 *mat, const float4 *vec);
float3 cast_ray(const float3 *orig, const float3 *dir, __constant struct sphere *sphs, const uint sphc);
bool intersect(const float3 *orig, const float3 *dir, __constant struct sphere *sph, float *t);
bool solve_quadratic(const float a, const float b, const float c, float *x0, float *x1);

__kernel
void trace_ray(__constant struct world *wl, __constant struct sphere *sphs, __global ubyte *img)
{
        int x, y;
        float4 cpos, pix;
        float3 tcpos, tpix, dir, pixclr;
        
        x = get_global_id(0);
        y = get_global_id(1);

        cpos = (float4)(0.0f, 0.0f, 0.0f, 1.0f);
        tcpos = transform_vec(wl->ctrans, &cpos).xyz;

        pix.x = (2 * (x + 0.5f) / wl->w - 1) * wl->ar * wl->sc;
        pix.y = (1 - 2 * (y + 0.5f) / wl->h) * wl->sc;
        pix.z = -1.0f;
        pix.w = 1.0f;

        tpix = transform_vec(wl->ctrans, &pix).xyz;

        dir = normalize(tpix - tcpos);

        pixclr = cast_ray(&tcpos, &dir, sphs, wl->sphc);

        img[(x + (wl->h - y - 1) * wl->w) * 3] = pixclr.x;
        img[(x + (wl->h - y - 1) * wl->w) * 3 + 1] = pixclr.y;
        img[(x + (wl->h - y - 1) * wl->w) * 3 + 2] = pixclr.z;
}

float4 transform_vec(__constant float4 *mat, const float4 *vec)
{
        float4 res;

        res.x = dot(mat[0], *vec);
        res.y = dot(mat[1], *vec);
        res.z = dot(mat[2], *vec);
        res.w = dot(mat[3], *vec);

        return res;
}

float3 cast_ray(const float3 *orig, const float3 *dir, __constant struct sphere *sphs, const uint sphc)
{
        float t, tmin = FLT_MAX;
        bool hit = false;
        size_t hitidx;
        float3 clr;

        for (size_t i = 0; i < sphc; ++i) {
                if (intersect(orig, dir, &sphs[i], &t) && t < tmin) {
                     hit = true;
                     hitidx = i;
                     tmin = t;
                }
        }

        if (hit) {
                float sh;
                float3 phit, nhit;

                phit = *orig + *dir * tmin;
                nhit = normalize(phit - sphs[hitidx].c);
                sh = fmax(0.0f, dot(nhit, -*dir));

                clr.x = sphs[hitidx].clr.x * sh;
                clr.y = sphs[hitidx].clr.y * sh;
                clr.z = sphs[hitidx].clr.z * sh;
        } else {
                clr.x = 51;
                clr.y = 51;
                clr.z = 51;
        }

        return clr;
}

bool intersect(const float3 *orig, const float3 *dir, __constant struct sphere *sph, float *t)
{
        float a, b, c, t0, t1;
        float3 oc;

        oc = (*orig) - sph->c;
        a = dot(*dir, *dir);
        b = 2 * dot(*dir, oc);
        c = dot(oc, oc) - sph->r * sph->r;
       
        if (!solve_quadratic(a, b, c, &t0, &t1))
                return false;

        if (t0 < 0) {
                if (t1 < 0)
                        return false;

                *t = t1;
        }

        *t = t0;

        return true;
}

bool solve_quadratic(const float a, const float b, const float c, float *x0, float *x1)
{
        float dcr;

        dcr = b * b - 4 * a * c;
        if (dcr < 0) {
                return false;
        } else if (dcr == 0) {
                *x0 = *x1 = - 0.5f * b / a;
        } else {
                float q;

                q = b > 0 ? -0.5f * (b + sqrt(dcr)) : -0.5f * (b - sqrt(dcr));
                *x0 = q / a;
                *x1 = c / q;
        }

        if (*x0 > *x1) {
                float tmp;

                tmp = *x0;
                *x0 = *x1;
                *x1 = tmp;
        }

        return true;
}
