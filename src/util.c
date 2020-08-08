#include "util.h"

void *malloc_or_die(size_t size)
{
        void *p;

        if (!(p = malloc(size)))
                exit(EXIT_FAILURE);

        return p;
}

char *read_src_from_file(const char *path)
{
        char *src = NULL;
        long bytes;
        size_t read;
        FILE *f;

        f = fopen(path, "r");
        if (!f) {
                LOG_ERROR_F("errno %d, %s", errno, strerror(errno));
                return NULL;
        }

        if (fseek(f, 0, SEEK_END))
                goto err_free;

        if ((bytes = ftell(f)) == EOF) {
                LOG_ERROR_F("errno %d, %s", errno, strerror(errno));
                goto err_free;
        }

        if (fseek(f, 0, SEEK_SET))
                goto err_free;

        src = (char *)malloc_or_die((bytes + 1) * sizeof(char));

        if ((fread(src, sizeof(char), bytes, f)) < bytes) {
                if (feof(f))
                        LOG_ERROR("Unexpected end of file");
        
                free(src);
                src = NULL;
        } else {
                src[bytes] = '\0';
        }

err_free:
        fclose(f);

        return src;
}

void calc_work_group_dims(const size_t max, size_t *dim0, size_t *dim1)
{
	uint mod;

	*dim0 = sqrt(max);

	while(mod = max % *dim0)
		--*dim0;

	*dim1 = max / *dim0;
}

float rand_real(const float l)
{
        return (-1 + 2 * ((float)rand() / RAND_MAX)) * l;
}

float rand_positive_real(const float l)
{
        return ((float)rand() / RAND_MAX) * l;
}
