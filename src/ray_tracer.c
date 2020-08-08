#include "ray_tracer.h"

int setup_ray_tracing(struct ray_tracer *rt, const uint w, const uint h, const uint ch, const uint sphc)
{
        char *src, *path;
	size_t klwgrps, dim0, dim1;
        cl_int status;
        cl_command_queue cmdq;
        cl_mem wl, sphs, img;
        cl_program prog;
        cl_kernel kl;

        CALL_CL_OUT(cmdq = clCreateCommandQueue(rt->cont, rt->dev, 0, &status), status);

        CALL_CL_OUT(wl = clCreateBuffer(rt->cont, CL_MEM_READ_ONLY, sizeof(struct world), NULL, &status), status);
        CALL_CL_OUT(sphs = clCreateBuffer(rt->cont, CL_MEM_READ_ONLY, sizeof(struct sphere) * sphc, NULL, &status), status);
        CALL_CL_OUT(img = clCreateBuffer(rt->cont, CL_MEM_WRITE_ONLY, w * h * ch, NULL, &status), status);

        if (!(prog = create_cl_program(&rt->cont, &rt->dev, ROOT_DIR"/kernel/ray_tracing.cl"))) {
                LOG_ACTION_FAILURE("CL PROGRAM", "CREATE");
                return -EFAIL;
        } else {
                LOG_ACTION_SUCCESS("CL PROGRAM", "CREATE");
        }

        CALL_CL_OUT(kl = clCreateKernel(prog, "trace_ray", &status), status);
        CALL_CL_RET(clSetKernelArg(kl, 0, sizeof(cl_mem), &wl));
        CALL_CL_RET(clSetKernelArg(kl, 1, sizeof(cl_mem), &sphs));
        CALL_CL_RET(clSetKernelArg(kl, 2, sizeof(cl_mem), &img));

	CALL_CL_RET(clGetKernelWorkGroupInfo(kl, rt->dev, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &klwgrps, NULL));
	calc_work_group_dims(klwgrps, &dim1, &dim0);

	LOG_DEBUG_F("Work group size -> %zu * %zu", dim0, dim1);
	
	// TODO add support for all resolutions
	if (w % dim0 || h % dim1) {
		LOG_ERROR_F("Unsupported resolution -> choose width evenly divided by %zu and height evenly divided by %zu", dim0, dim1);
		return -EFAIL;
	}

        rt->w = w;
        rt->h = h;
        rt->ch = ch;
        rt->cmdq = cmdq;
        rt->wl = wl;
        rt->img = img;
        rt->sphs = sphs;
        rt->prog = prog;
        rt->kl = kl;
        rt->witems[0] = w;
        rt->witems[1] = h;
        rt->wgrps[0] = dim0;
        rt->wgrps[1] = dim1;

        return 0;
}

void transfer_world(const struct ray_tracer *rt, const struct world *w)
{
        CALL_CL_RET(clEnqueueWriteBuffer(rt->cmdq, rt->wl, CL_TRUE, 0, sizeof(struct world), w, 0, NULL, NULL));
}

void transfer_payload(const struct ray_tracer *rt, const struct sphere *sphs, const uint sphc)
{
        CALL_CL_RET(clEnqueueWriteBuffer(rt->cmdq, rt->sphs, CL_TRUE, 0, sizeof(struct sphere) * sphc, sphs, 0, NULL, NULL));
}

ubyte *trace_rays(const struct ray_tracer *rt, const uint w, const uint h, const uint ch)
{
        size_t dsize;

        dsize = w * h * ch;
        ubyte *trays = (ubyte *)malloc_or_die(dsize);

        CALL_CL_RET(clEnqueueNDRangeKernel(rt->cmdq, rt->kl, 2, NULL, rt->witems, rt->wgrps, 0, NULL, NULL));
        CALL_CL_RET(clEnqueueReadBuffer(rt->cmdq, rt->img, CL_TRUE, 0, dsize, trays, 0, NULL, NULL));

        return trays;
}

void free_ray_tracing(const struct ray_tracer *rt)
{
        CALL_CL_RET(clReleaseKernel(rt->kl));
        CALL_CL_RET(clReleaseProgram(rt->prog));
        CALL_CL_RET(clReleaseCommandQueue(rt->cmdq));
        CALL_CL_RET(clReleaseMemObject(rt->wl));
        CALL_CL_RET(clReleaseMemObject(rt->sphs));
        CALL_CL_RET(clReleaseMemObject(rt->img));
        CALL_CL_RET(clReleaseContext(rt->cont));
}

cl_program create_cl_program(const cl_context *cont, const cl_device_id *dev, const char *path)
{
        char* src;
        cl_int status;
        cl_program prog;
        
        LOG_DEBUG_F("Reading kernel source code from file %s", path);

        if (!(src = read_src_from_file(path))) {
                LOG_ERROR("Failed to read kernel source from file");
                return NULL;
        }

        LOG_DEBUG_F("Kernel source ->\n%s", src);

        CALL_CL_OUT(prog = clCreateProgramWithSource(*cont, 1, (const char **)&src, NULL, &status), status);

        free(src);
        
        const char *cflags = "-I "ROOT_DIR"/kernel/";
        if (status = clBuildProgram(prog, 1, dev, cflags, NULL, NULL)) {
                size_t sinfo;
                char *info;

                CALL_CL_RET(clGetProgramBuildInfo(prog, *dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &sinfo));
                info = (char *)malloc_or_die(sinfo);
                CALL_CL_RET(clGetProgramBuildInfo(prog, *dev, CL_PROGRAM_BUILD_LOG, sinfo, info, &sinfo));

                LOG_ERROR_F("Failed to build cl program -> %s", info);

                free(info);

                return NULL;
        }

        return prog;
}
