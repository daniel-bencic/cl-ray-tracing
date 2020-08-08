#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <GL/glew.h>

#define RESET           "\033[0m"
#define BOLD_CYAN       "\033[1m\033[36m"
#define BOLD_WHITE      "\033[1m\033[37m"
#define BOLD_YELLOW     "\033[1m\033[33m"
#define BOLD_RED        "\033[1m\033[31m"
#define BOLD_GREEN      "\033[1m\033[32m"

#define LOG(header, msg, ...)           printf(header msg "\n", __VA_ARGS__)
#define LOG_E(header, msg, ...)         fprintf(stderr, header msg "\n", __VA_ARGS__)

#ifndef NDEBUG
        #define LOG_DEBUG(msg)          LOG("%s[DEBUG:%s:%d]%s: ", msg, BOLD_CYAN, __FILE__, __LINE__, RESET)
        #define LOG_DEBUG_F(msg, ...)   LOG("%s[DEBUG:%s:%d]%s: ", msg, BOLD_CYAN, __FILE__, __LINE__, RESET, __VA_ARGS__)
#else
        #define LOG_DEBUG(msg)          ((void)0)
        #define LOG_DEBUG_F(msg, ...)   ((void)0)
#endif

#define LOG_INFO(msg)                   LOG("%s[INFO]%s: ", msg, BOLD_WHITE, RESET)
#define LOG_INFO_F(msg, ...)            LOG("%s[INFO]%s: ", msg, BOLD_WHITE, RESET, __VA_ARGS__)
#define LOG_ERROR(msg)                  LOG_E("%s[ERROR:%s:%d]%s: ", msg, BOLD_RED, __FILE__, __LINE__, RESET)
#define LOG_ERROR_F(msg, ...)           LOG_E("%s[ERROR:%s:%d]%s: ", msg, BOLD_RED, __FILE__, __LINE__, RESET, __VA_ARGS__)
#define LOG_ACTION_SUCCESS(mod, act)    LOG("%s[%s-%s]: ", "%s%s%s", BOLD_WHITE, mod, act, BOLD_GREEN, "ok.", RESET)
#define LOG_ACTION_FAILURE(mod, act)    LOG_E("%s[%s-%s]: ", "%s%s%s", BOLD_WHITE, mod, act, BOLD_RED, "failed.", RESET)
#define LOG_FPS(msf, fps)               printf("\r%s[INFO]%s: ms/frame: %f | fps: %d", BOLD_WHITE, RESET, msf, fps); fflush(stdout);

#define CALL_GL(func)                   while(glGetError());\
                                        func;\
                                        {\
                                                GLenum err;\
                                                while (err = glGetError())\
                                                        LOG_ERROR_F("OpenGL error code %d", err);\
                                        }

#define CALL_CL_RET(func)               {\
                                                cl_int status;\
                                                if (status = func)\
                                                        LOG_ERROR_F("OpenCL error code %d", status);\
                                        }

#define CALL_CL_OUT(func, status)       func;\
                                        if (status)\
                                                LOG_ERROR_F("OpenCL error code %d", status);

#endif /* LOG_H */
