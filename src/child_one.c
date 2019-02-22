//#include <sys/syscall.h>
//#include <unistd.h>
//#include <stdlib.h>
#include "child_one.h"
#include <signal.h>
#include <string.h>

static time_t my_time;
static file_t *log_file;

void child1_exit(void)
{
   time(&my_time);
   while( pthread_mutex_lock(&mutex) );
   fprintf( log_file->fid, "%sTID Child 1 [%d]: Goodbye World!\n",
            ctime(&my_time), (pid_t)syscall(SYS_gettid));
   fclose( log_file->fid );
   pthread_mutex_unlock(&mutex);
   pthread_exit(0);
}

static void sig_handler(int signo)
{
   if( signo == SIGUSR1 )
   {
      printf("Received SIGUSR1! TID [%d] Exiting...\n",
            (pid_t)syscall(SYS_gettid));
      child1_exit();
   }
   else if( signo == SIGUSR2 )
   {
      printf("Received SIGUSR2! TID [%d] Exiting...\n",
            (pid_t)syscall(SYS_gettid));
      child1_exit();
   }
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

      fprintf( stdout, "%sTID Child 1 [%d]: Hello World!\n",
               ctime(&my_time), (pid_t)syscall(SYS_gettid));
      fprintf( log_file->fid, "%sTID Child 1 [%d]: Hello World!\n",
               ctime(&my_time), (pid_t)syscall(SYS_gettid));

      signal(SIGUSR1, sig_handler);
      signal(SIGUSR2, sig_handler);
      pthread_mutex_unlock(&mutex);
   }
   sleep(2);
   child1_exit();
   return NULL;
}
