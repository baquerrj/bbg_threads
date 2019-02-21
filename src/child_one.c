//#include <sys/syscall.h>
//#include <unistd.h>
//#include <stdlib.h>
#include "child_one.h"

static time_t my_time;
static file_t *log_file;

void child1_exit(FILE* fid)
{
   printf("Thread %d terminating... closing file handler.\n",
         (pid_t)syscall(SYS_gettid));
   fclose( fid );
   return;
}

void *child1_fn(void *arg)
{
   /* Get time that thread was spawned */
   time(&my_time);

   if( NULL != arg )
   {
      pthread_mutex_lock(&mutex);
      log_file = malloc( sizeof( file_t ) );
      log_file->name = (char*)arg;
      log_file->fid = fopen( log_file->name, "a" );

      fprintf( log_file->fid, "New thread (%d) started at %s\n",
               (pid_t)syscall(SYS_gettid), ctime(&my_time));
      pthread_mutex_unlock(&mutex);
   }
   sleep(2);
   child1_exit(log_file->fid);
   return NULL;
}
