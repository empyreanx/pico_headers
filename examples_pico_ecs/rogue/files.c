#include "files.h"

#include <stdio.h>
#include <stdlib.h>

bool file_exists(const char* path)
{
    FILE* file = fopen(path, "r+");

    if (NULL != file)
    {
        fclose(file);
        return true;
    }

    return false;
}

char* read_file(const char* path)
{
    FILE* file = fopen(path, "r");

    if (!file)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long lsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (lsize < 0)
    {
        fclose(file);
        return NULL;
    }

    size_t size = (size_t)lsize;
    char* buffer = malloc((size + 1) * sizeof(char));

    buffer[size] = '\0';

    if (size != fread(buffer, 1, size, file))
    {
        fclose(file);
        free(buffer);
        return NULL;
    }

    return buffer;
}

