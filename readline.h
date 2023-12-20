#ifndef readline_h
#define readline_h

#include "header.h"

ssize_t readline(int fd, void *vptr, size_t maxlen);
static ssize_t my_read(int fd, char *ptr);
ssize_t readlinebuf(void **vptrptr);
ssize_t Readline(int fd, void *ptr, size_t maxlen);

#endif