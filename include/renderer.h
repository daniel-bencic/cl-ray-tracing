#ifndef RENDERER_H
#define RENDERER_H

#include <math.h>
#include <time.h>
#include <GL/glew.h>
#include <CL/cl.h>
#include "common.h"
#include "host_kernel_types.h"
#include "linear_algebra.h"
#include "util.h"

enum cam_move {
        front,
        back,
        left,
        right
};

struct camera {
        double spd;
        double sns;
        cl_float fov;
        cl_float3 pos;
        cl_float3 dir;
        cl_float3 up;
        double pitch;
        double yaw;
};

struct renderer {
        uint w;
        uint h;
        uint ch;
        uint sphc;
        struct sphere *sphs;
        struct camera cam;
        double dt;
        double lx;
        double ly;
        size_t numidxs;
        GLuint prog;
        GLuint vao;
        GLuint vbo;
        GLuint ebo;
        GLuint tex;
};

void set_viewport(struct renderer *r, const uint w, const uint h);

int setup_rendering(struct renderer *r, const uint sphc);
void move_camera(struct renderer *r, const enum cam_move dir);
void look_camera(struct renderer *r, const double x, const double y);
void render(const struct renderer *r, const ubyte *img);
void generate_spheres(struct sphere *sphs, const uint c);
void get_world(const struct renderer *r, struct world *w);
void free_rendering(const struct renderer *r);

GLuint create_shader(const GLenum type, const char *path);
GLuint create_gl_program(const GLuint vs, const GLuint fs);
GLuint create_buffer(const GLenum type, const GLsizeiptr size, const GLvoid *data, const GLenum use);
GLuint create_texture();

void add_vertex_attr(const GLuint idx, const GLint size, const GLenum type, const GLboolean norm,
                        const GLsizei stride, const GLvoid *off);
void set_uniform_ui1(const GLuint prog, const GLchar* name, const GLuint val);

#endif /* RENDERER_H */
