#ifndef FILES_H
#define FILES_H

#include <stdbool.h>

bool file_exists(const char* path);
char* read_file(const char* path);

#endif // FILES_H
