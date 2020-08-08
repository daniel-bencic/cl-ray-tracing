#include "renderer.h"

void set_viewport(struct renderer *r, const uint w, const uint h)
{
        CALL_GL(glViewport(0, 0, w, h));

        r->w = w;
        r->h = h;

        LOG_INFO_F("Viewport set to %d:%d", w, h);
}

int setup_rendering(struct renderer *r, const uint sphc)
{
        GLint vs, fs, prog, vao, vbo, ebo, tex;
        float verts[] = {
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f
        };
        uint idxs[] = {
                0, 1, 2,
                0, 2, 3
        };
        
        r->sphs = malloc_or_die(sizeof(struct sphere) * sphc);
        generate_spheres(r->sphs, sphc);
        r->sphc = sphc;

        r->cam.spd = 3.0;
        r->cam.sns = 0.05;
        r->cam.fov = 20.0f; 

        r->cam.pos.x = 0.0f;
        r->cam.pos.y = 0.0f;
        r->cam.pos.z = 0.0f;
        
        r->cam.dir.x = 0.0f;
        r->cam.dir.y = 0.0f;
        r->cam.dir.z = -1.0f;

        r->cam.up.x = 0.0f;
        r->cam.up.y = 1.0f;
        r->cam.up.z = 0.0f;

        r->lx = r->w / 2;
        r->ly = r->h / 2;
        r->cam.pitch = 0.0;
        r->cam.yaw = -90.0;
              
        
        vs = create_shader(GL_VERTEX_SHADER, ROOT_DIR"/shader/vertex_shader.glsl");
        if (!vs) {
                LOG_ACTION_FAILURE("VERTEX SHADER", "CREATE");
                return -EFAIL;
        } else {
                LOG_ACTION_SUCCESS("VERTEX SHADER", "CREATE");
        }

        fs = create_shader(GL_FRAGMENT_SHADER, ROOT_DIR"/shader/fragment_shader.glsl");
        if (!fs) {
                LOG_ACTION_FAILURE("FRAGMENT SHADER", "CREATE");
                return -EFAIL;
        }
        else {
                LOG_ACTION_SUCCESS("FRAGMENT SHADER", "CREATE");
        }

        prog = create_gl_program(vs, fs);
        if (!prog) {
                LOG_ACTION_FAILURE("GL PROGRAM", "CREATE");
                return -EFAIL;
        }
        else {
                LOG_ACTION_SUCCESS("GL PROGRAM", "CREATE");
        }

        CALL_GL(glDeleteShader(vs));
        CALL_GL(glDeleteShader(fs));

        CALL_GL(glGenVertexArrays(1, &vao));
        CALL_GL(glBindVertexArray(vao));

        vbo = create_buffer(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        add_vertex_attr(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);
        add_vertex_attr(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(sizeof(float) * 3));
        CALL_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

        ebo = create_buffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);

        CALL_GL(glBindVertexArray(0));

        tex = create_texture();
        
        CALL_GL(glUseProgram(prog));
        set_uniform_ui1(prog, "tex", 0);
        CALL_GL(glUseProgram(0));

        r->numidxs = sizeof(idxs) / sizeof(uint);
        r->prog = prog;
        r->vao = vao;
        r->vbo = vbo;
        r->ebo = ebo;
        r->tex = tex;

        return 0;
}

void move_camera(struct renderer *r, const enum cam_move dir)
{
        double sp;
        cl_float3 d, ri;

        sp = r->cam.spd * r->dt;
        
        d = r->cam.dir;

        cross(&r->cam.dir, &r->cam.up, &ri);
        normalize(&ri);

        switch (dir) {
                case front:
                        mult_vec(&d, sp);
                        add_vec(&r->cam.pos, &d);
                        break;

                case left:
                        mult_vec(&ri, sp);
                        sub_vec(&r->cam.pos, &ri);
                        break;

                case right:
                        mult_vec(&ri, sp);
                        add_vec(&r->cam.pos, &ri);
                        break;

                case back:
                        mult_vec(&d, sp);
                        sub_vec(&r->cam.pos, &d);
                        break;
        }
}

void look_camera(struct renderer *r, const double x, const double y)
{
        double xoff, yoff;

        xoff = x - r->lx;
        yoff = r->ly - y;

        r->cam.pitch += yoff * r->cam.sns;
        r->cam.pitch = r->cam.pitch > 89.9 ? 89.9 : r->cam.pitch;
        r->cam.pitch = r->cam.pitch < -89.9 ? -89.9 : r->cam.pitch;

        r->cam.yaw += xoff * r->cam.sns;
        r->cam.dir.x = cos(rad(r->cam.yaw)) * cos(rad(r->cam.pitch));
        r->cam.dir.y = sin(rad(r->cam.pitch));
        r->cam.dir.z = sin(rad(r->cam.yaw)) * cos(rad(r->cam.pitch));

        r->lx = x;
        r->ly = y;
}

void render(const struct renderer *r, const ubyte *img)
{
        CALL_GL(glClearColor(0.2f, 0.2f, 0.2f, 1.0f));
        CALL_GL(glClear(GL_COLOR_BUFFER_BIT));

        CALL_GL(glBindTexture(GL_TEXTURE_2D, r->tex));
        CALL_GL(glUseProgram(r->prog));
        CALL_GL(glBindVertexArray(r->vao));

        CALL_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, r->w, r->h, 0, GL_RGB, GL_UNSIGNED_BYTE, img));
        CALL_GL(glDrawElements(GL_TRIANGLES, r->numidxs, GL_UNSIGNED_INT, NULL));

        CALL_GL(glBindVertexArray(0));
        CALL_GL(glUseProgram(0));
        CALL_GL(glBindTexture(GL_TEXTURE_2D, 0));
}

void generate_spheres(struct sphere *sphs, const uint c)
{
        srand(time(NULL));

        for (size_t i = 0; i < c; ++i) {
                float posx, posy, posz, r; 
                uint clrx, clry, clrz;
                struct sphere s;

                posx = rand_real(15.0f);
                posy = rand_real(15.0f);
                posz = -rand_positive_real(20.0f);
                clrx = rand_positive_real(255.0f);
                clry = rand_positive_real(255.0f);
                clrz = rand_positive_real(255.0f);
                r = rand_positive_real(2.0f);

                s.c.x = posx;
                s.c.y = posy;
                s.c.z = posz;
                s.r = r;
                s.clr.x = clrx;
                s.clr.y = clry;
                s.clr.z = clrz;

                sphs[i] = s;
        }
}

void get_world(const struct renderer *r, struct world *w)
{
        cl_float3 up, tgt;

        w->w = r->w;
        w->h = r->h;
        w->fov = r->cam.fov;
        w->ar = r->w / (float)r->h;
        w->sc = tan(rad(w->fov) * 0.5) * 2.0;
        w->cpos.x = 0.0f;
        w->cpos.y = 0.0f;
        w->cpos.z = 0.0f;

        up.x = 0.0f;
        up.y = 1.0f;
        up.z = 0.0f;
        
        tgt = r->cam.pos;
        add_vec(&tgt, &r->cam.dir);
        look_at(&r->cam.pos, &tgt, up, w->ctrans);

        w->sphc = r->sphc;
}

void free_rendering(const struct renderer *r)
{
        uint buffs[] = { r->vbo, r->ebo };

        free(r->sphs);

        CALL_GL(glDeleteProgram(r->prog));
        CALL_GL(glDeleteVertexArrays(1, &r->vao));
        CALL_GL(glDeleteBuffers(2, buffs));
        CALL_GL(glDeleteTextures(1, &r->tex));
}

GLuint create_shader(const GLenum type, const char *path)
{
        char *src;
        GLint id, comp;

        CALL_GL(id = glCreateShader(type));
        
        LOG_DEBUG_F("Reading shader source code from file %s", path);
        
        if (!(src = read_src_from_file(path))) {
                LOG_ERROR("Failed to read shader source from file");
                return -EFAIL;
        }

        LOG_DEBUG_F("Shader source ->\n%s", src);

        CALL_GL(glShaderSource(id, 1, (const char **)&src, NULL));
        CALL_GL(glCompileShader(id));

        free(src);

        CALL_GL(glGetShaderiv(id, GL_COMPILE_STATUS, &comp));
        if (!comp) {
                GLint loglen;
                GLchar *log;

                CALL_GL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &loglen));

                log = (GLchar *)malloc_or_die(loglen * sizeof(GLchar));
                CALL_GL(glGetShaderInfoLog(id, loglen * sizeof(GLchar), NULL, log));

                LOG_ERROR_F("Failed to compile shader -> %s", log);

                free(log);

                return 0;
        }

        return id;
}

GLuint create_gl_program(const GLuint vs, const GLuint fs)
{
        GLint id, link;

        CALL_GL(id = glCreateProgram());
        
        CALL_GL(glAttachShader(id, vs));
        CALL_GL(glAttachShader(id, fs));

        CALL_GL(glLinkProgram(id));

        CALL_GL(glGetProgramiv(id, GL_LINK_STATUS, &link));
        if (!link) {
                GLint loglen;
                GLchar *log;

                CALL_GL(glGetProgramiv(id, GL_INFO_LOG_LENGTH, &loglen));

                log = (GLchar *)malloc_or_die(loglen * sizeof(GLchar));
                CALL_GL(glGetProgramInfoLog(id, loglen * sizeof(GLchar), NULL, log));

                LOG_ERROR_F("Failed to link gl program -> %s", log);

                free(log);

                return 0;
        }

        return id;
}

GLuint create_buffer(const GLenum type, const GLsizeiptr size, const GLvoid *data, const GLenum use)
{
        GLint id;

        CALL_GL(glGenBuffers(1, &id));
        CALL_GL(glBindBuffer(type, id));
        CALL_GL(glBufferData(type, size, data, use));
}

GLuint create_texture()
{
        GLint id;

        CALL_GL(glGenTextures(1, &id));
        CALL_GL(glActiveTexture(GL_TEXTURE0));
        CALL_GL(glBindTexture(GL_TEXTURE_2D, id));
        CALL_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
        CALL_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
        CALL_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CALL_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        return id;
}


void add_vertex_attr(const GLuint idx, const GLint size, const GLenum type, const GLboolean norm,
                        const GLsizei stride, const GLvoid *off)
{
        CALL_GL(glVertexAttribPointer(idx, size, type, norm, stride, off));
        CALL_GL(glEnableVertexAttribArray(idx));
}

void set_uniform_ui1(const GLuint prog, const GLchar* name, const GLuint val)
{
        GLint loc;

        CALL_GL(loc = glGetUniformLocation(prog, name));
        CALL_GL(glUniform1i(loc, val));
}
