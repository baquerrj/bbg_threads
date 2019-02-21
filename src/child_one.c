//#include <sys/syscall.h>
//#include <unistd.h>  
//#include <stdlib.h>
#include "child_one.h"

void child1_exit(FILE* fid)
{
   printf("Thread %d terminating... closing file handler.\n",
         (pid_t)syscall(SYS_gettid));
   fclose( fid );
   return;
}

void *child1_fn(void *arg)
{
   static file_t *log_file;
   if( NULL != arg )
   {
      pthread_mutex_lock(&mutex);
      log_file = malloc( sizeof( file_t ) );
      log_file->name = (char*)arg;
      log_file->fid = fopen( log_file->name, "a" );

      fprintf( log_file->fid, "New thread started, PID %d TID %d\n",
               getpid(), (pid_t)syscall(SYS_gettid));
      pthread_mutex_unlock(&mutex);
   }
   sleep(2);
   child1_exit(log_file->fid);
   return NULL;
}
