#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "common.h"
#include "renderer.h"
#include "ray_tracer.h"

void on_glfw_error(int code, const char *desc);
void on_opencl_error(const char *errinfo, const void *privinfo, size_t cb, void *userdata);
int init_glfw_window(GLFWwindow **win, const uint w, const uint h);
int init_opengl_context(GLFWwindow *win);
cl_device_id query_opencl_device();
void process_key_input(GLFWwindow *win, struct renderer *r);
void process_mouse_input(GLFWwindow *win, struct renderer *r);

int main(int argc, char** argv)
{
        uint frs = 0;
        const uint w = 800;
        const uint h = 800;
        const uint sphc = 50;
        double lframe, fps;
        struct renderer rdr;
        struct ray_tracer rtcr;
        GLFWwindow* win;
        cl_int status;
        cl_device_id cldev;
        cl_context clcont;
        
        if (init_glfw_window(&win, w, h)) {
                LOG_ACTION_FAILURE("GLFW WINDOW", "INIT");
                return EXIT_FAILURE;
        } else {
                LOG_ACTION_SUCCESS("GLFW WINDOW", "INIT");

                glfwSetInputMode(win, GLFW_STICKY_KEYS, GLFW_TRUE);
                glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        if (init_opengl_context(win)) {
                LOG_ACTION_FAILURE("OPENGL CONTEXT", "INIT");
                return EXIT_FAILURE;
        } else {
                LOG_ACTION_SUCCESS("OPENGL CONTEXT", "INIT");

                set_viewport(&rdr, w, h);
                rdr.ch = 3;
        }

        if (!(cldev = query_opencl_device())) {
                LOG_ACTION_FAILURE("OPENCL CONTEXT", "INIT");
                return EXIT_FAILURE;
        } else {
                LOG_ACTION_SUCCESS("OPENCL CONTEXT", "INIT");
        
                rtcr.dev = cldev;
                CALL_CL_OUT(rtcr.cont = clCreateContext(NULL, 1, &cldev, on_opencl_error, NULL, &status), status);
        }

        if (setup_rendering(&rdr, sphc)) {
                LOG_ACTION_FAILURE("RENDERING", "SETUP");
                return EXIT_FAILURE;
        } else {
                LOG_ACTION_SUCCESS("RENDERING", "SETUP");
        }

        if (setup_ray_tracing(&rtcr, rdr.w, rdr.h, rdr.ch, sphc)) {
                LOG_ACTION_FAILURE("RAY TRACING", "SETUP");
                return EXIT_FAILURE;
        } else {
                LOG_ACTION_SUCCESS("RAY TRACING", "SETUP");
        }

        transfer_payload(&rtcr, rdr.sphs, rdr.sphc);

        fps = lframe = glfwGetTime();
        
        while (!glfwWindowShouldClose(win)) {
                double cframe, dt;
                ubyte *img;
                struct world wl;
        
                cframe = glfwGetTime();
                dt = cframe -lframe;
                lframe = cframe;
                rdr.dt = dt;

                if (cframe - fps >= 1.0) {
                        LOG_FPS((cframe - fps) * 1000 / frs, frs);
                        fps = cframe;
                        frs = 0;
                }

                get_world(&rdr, &wl);
                transfer_world(&rtcr, &wl);
                
                img = trace_rays(&rtcr, rdr.w, rdr.h, rdr.ch);
                
                render(&rdr, img);

                free(img);

                process_key_input(win, &rdr);
                process_mouse_input(win, &rdr);

                glfwSwapBuffers(win);
                glfwPollEvents();

                ++frs;
        }

	printf("\n");

        free_ray_tracing(&rtcr);
        free_rendering(&rdr);

        glfwTerminate();

        return EXIT_SUCCESS;
}

void on_glfw_error(int code, const char *desc)
{
        LOG_ERROR_F("GLFW error code %d -> %s", code, desc);
}

void on_opencl_error(const char *errinfo, const void *privinfo, size_t cb, void *userdata)
{
        LOG_ERROR_F("OpenCL error callback -> %s", errinfo);
}

int init_glfw_window(GLFWwindow **win, const uint w, const uint h)
{
        if (!glfwInit()) {
                LOG_ERROR("Failed to initialize GLFW");
                return -EFAIL;
        } 

        glfwSetErrorCallback(on_glfw_error);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        *win = glfwCreateWindow(w, h, PROGRAM_NAME, NULL, NULL);
        if (!*win) {
                LOG_ERROR("Failed to create GLFW window");
                return -EFAIL;
        }

        return 0;
}

int init_opengl_context(GLFWwindow *win)
{
        glfwMakeContextCurrent(win);

        if (glewInit()) {
                LOG_ERROR("Failed to initialize GLEW");
                return -EFAIL;
        }

        if (!GLEW_VERSION_3_3) {
                LOG_ERROR("OpenGL 3.3 is not supported");
                return -EFAIL;
        } else {
                LOG_INFO("OpenGL 3.3 is supported");
        }

        return 0;
}

cl_device_id query_opencl_device(cl_context *cont)
{
        uint devfound = FALSE;
        cl_uint status, numplats;
        cl_platform_id *plats;
        cl_device_id dev;

        CALL_CL_RET(clGetPlatformIDs(0, NULL, &numplats));
        if (numplats == 0) {
                LOG_ERROR("No OpenCL implementation found");
                return NULL;
        }

        plats = (cl_platform_id *)malloc_or_die(sizeof(cl_platform_id) * numplats);
        CALL_CL_RET(clGetPlatformIDs(numplats, plats, NULL));

        for (size_t i = 0; i < numplats; ++i) {
                cl_uint numdevs;
                
                CALL_CL_RET(clGetDeviceIDs(plats[i], CL_DEVICE_TYPE_GPU, 0, NULL, &numdevs));
                if (numdevs > 0) {
                        CALL_CL_RET(clGetDeviceIDs(plats[i], CL_DEVICE_TYPE_GPU, 1, &dev, NULL));
                        devfound = TRUE;
                        break;
                }
        }

        free(plats);

        if (!devfound) {
                LOG_ERROR("No OpenCL GPU device found");
                return NULL;
        }

        return dev;
}

void process_key_input(GLFWwindow *win, struct renderer *r)
{
        if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(win, GL_TRUE);

        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
                move_camera(r, front);

        if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
                move_camera(r, left);

        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
                move_camera(r, back);

        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
                move_camera(r, right);
}

void process_mouse_input(GLFWwindow *win, struct renderer *r)
{
        double x, y;

        glfwGetCursorPos(win, &x, &y);

        look_camera(r, x, y);
}
