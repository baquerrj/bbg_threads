#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

typedef struct file_s {
   char *name;
   FILE *fid;
} file_t;

typedef struct args_s {
   void *arg1;
   void *arg2;
} args_t;

extern pthread_mutex_t mutex;

#endif /* COMMON_H */
