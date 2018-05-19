#ifndef MODE_TO_STRING_H
#define MODE_TO_STRING_H

#include <sys/types.h>
#include <sys/stat.h>

void mode_to_string(mode_t mode, char str[11]);

#endif /* MODE_TO_STRING */
