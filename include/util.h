#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "common.h" 

void *malloc_or_die(size_t size);
char *read_src_from_file(const char *path);
void calc_work_group_dims(const size_t max, size_t *dim0, size_t *dim1);
float rand_real(const float l);
float rand_positive_real(const float l);

#endif /* UTIL_H */
