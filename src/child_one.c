//#include <sys/syscall.h>
//#include <unistd.h>
//#include <stdlib.h>
#include "child_one.h"
#include <signal.h>
#include <string.h>

static time_t my_time;
static file_t *log;

static void print_header(FILE* file)
{
   struct timespec time;
   clock_gettime(CLOCK_REALTIME, &time);
   fprintf( file, "\n========================================\n" );
   fprintf( file, "Thread 1 [%d]: %ld s - %ld ns\n",
            (pid_t)syscall(SYS_gettid), time.tv_sec, time.tv_nsec );
   return;
}


void child1_exit(void)
{
   time(&my_time);

   while( pthread_mutex_lock(&mutex) );
   print_header( log->fid );
   fprintf( log->fid, "Goodbye World!\n" );
   fclose( log->fid );
   pthread_mutex_unlock(&mutex);

   free( log );
   pthread_exit(0);
}



static void sig_handler(int signo)
{
   if( signo == SIGUSR1 )
   {
      printf("Received SIGUSR1! Exiting...\n");
      child1_exit();
   }
   else if( signo == SIGUSR2 )
   {
      printf("Received SIGUSR2! Exiting...\n");
      child1_exit();
   }
}

void *child1_fn(void *arg)
{
   /* Get time that thread was spawned */
   time(&my_time);

   static int failure = 1;
   /* Initialize thread */
   if( NULL == arg )
   {
      fprintf( stderr, "Thread requires name of log file!\n" );
      pthread_exit(&failure);
   }

   /* Take mutex to write to file */
   pthread_mutex_lock(&mutex);

   log = malloc( sizeof( file_t ) );
   if( NULL == log )
   {
      fprintf( stderr, "Ecountered error allocating memory for log file!\n");
      pthread_exit(&failure);
   }

   log->name = (char*)arg;
   log->fid = fopen( log->name, "a" );
   if( NULL == log->fid )
   {
      perror( "Ecountered error opening log file!\n" );
      pthread_exit(&failure);
   }

   print_header( log->fid );
   fprintf( log->fid, "Hello World!\n" );

   /* Release file mutex */
   pthread_mutex_unlock(&mutex);

   signal(SIGUSR1, sig_handler);
   signal(SIGUSR2, sig_handler);

   sleep(2);
   child1_exit();
   return NULL;
}
