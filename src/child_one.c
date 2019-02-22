//#include <sys/syscall.h>
//#include <unistd.h>
//#include <stdlib.h>
#include "child_one.h"

static time_t my_time;
static file_t *log_file;

void child1_exit(void)
{
   time(&my_time);
   while( pthread_mutex_lock(&mutex) );
   fprintf( log_file->fid, "%sTID [%d]: Goodbye World!\n",
            ctime(&my_time), (pid_t)syscall(SYS_gettid));
   fclose( log_file->fid );
   pthread_mutex_unlock(&mutex);
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

      fprintf( log_file->fid, "%sTID [%d]: Hello World!\n",
               ctime(&my_time), (pid_t)syscall(SYS_gettid));
      pthread_mutex_unlock(&mutex);
   }
   sleep(2);
   child1_exit();
   return NULL;
}
