//#include <sys/syscall.h>
//#include <unistd.h>
//#include <stdlib.h>
#include "child_one.h"
#include <signal.h>
#include <string.h>
#include <limits.h>

#define UPPER_CASE(c) (c &= ~(1<<5))

static int ascii_map[CHAR_MAX];

static file_t *log;

static void print_header(FILE* file)
{
   struct timespec time;
   clock_gettime(CLOCK_REALTIME, &time);
   fprintf( file, "\n=====================================================\n" );
   fprintf( file, "Thread 1 [%d]: %ld.%ld secs\n",
            (pid_t)syscall(SYS_gettid), time.tv_sec, time.tv_nsec );
   return;
}


void child1_exit(int exit_status)
{
   struct timespec time;
   clock_gettime(CLOCK_REALTIME, &time);

   while( pthread_mutex_lock(&mutex) );
   print_header( log->fid );

   switch( exit_status )
   {
      case SIGUSR1:
         fprintf( stdout, "Caught SIGUSR1 Signal! Exiting...\n");
         fprintf( log->fid, "Caught SIGUSR1 Signal! Exiting...\n");
         break;
      case SIGUSR2:
         fprintf( stdout, "Caught SIGUSR2 Signal! Exiting...\n");
         fprintf( log->fid, "Caught SIGUSR2 Signal! Exiting...\n");
         break;
      default:
         break;
   }
   fprintf( stdout, "Goodbye World! End Time: %ld.%ld secs\n",
            time.tv_sec, time.tv_nsec );
   fprintf( log->fid, "Goodbye World! End Time: %ld.%ld secs\n",
            time.tv_sec, time.tv_nsec );
   fclose( log->fid );
   pthread_mutex_unlock(&mutex);

   free( log );
   pthread_exit(EXIT_SUCCESS);
}

static void sig_handler(int signo)
{
   if( signo == SIGUSR1 )
   {
      printf("Received SIGUSR1! Exiting...\n");
      child1_exit(signo);
   }
   else if( signo == SIGUSR2 )
   {
      printf("Received SIGUSR2! Exiting...\n");
      child1_exit(signo);
   }
}


int print_result(void)
{
   /* Take mutex to write to file */
   pthread_mutex_lock(&mutex);

   print_header( log->fid );
   fprintf( log->fid, "Tallies from text file:\n" );
   char i = 'A';

   while( i < 'Z' )
   {
      if( ( 0 != ascii_map[ (int)i ] ) && ( 100 > ascii_map[ (int)i ] ) )
      {
         fprintf( log->fid, "%c : %u\n",
                  (char)i, ascii_map[(int)i] );
      }
      i++;
   }
   pthread_mutex_unlock(&mutex);
   return 0;
}


int process_arg(char* arg)
{
   FILE *fid = fopen( arg, "r" );
   if( NULL == fid )
   {
      perror( "Encountered error opening text file" );
      return 1;
   }

   char c;
   while( !feof( fid ) )
   {
      c = fgetc( fid );
      if( ferror( fid ) )
      {
         perror( "Encountered error reading text file" );
         return 1;
      }
      if( 0 < c )
      {
         UPPER_CASE( c );
         ascii_map[ (int)c ]++;
      }
   }

   print_result();
   return 0;
}


void *child1_fn(void *arg)
{
   /* Get time that thread was spawned */
   struct timespec time;
   clock_gettime(CLOCK_REALTIME, &time);

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

   args_t *args = arg;
   log->name = (char *)args->arg1;
   log->fid = fopen( log->name, "a+" );
   if( NULL == log->fid )
   {
      perror( "Ecountered error opening log file!\n" );
      pthread_exit(&failure);
   }

   print_header( log->fid );
   fprintf( log->fid, "Hello World! Start Time: %ld.%ld secs\n",
            time.tv_sec, time.tv_nsec );

   /* Release file mutex */
   pthread_mutex_unlock(&mutex);

   signal(SIGUSR1, sig_handler);
   signal(SIGUSR2, sig_handler);

   if( NULL != args->arg2 )
   {
      if( process_arg( args->arg2 ) )
      {
         fprintf( stderr, "Enouncered error processing text file!\n" );
         child1_exit(0);
      }
   }
   sleep(2);
   child1_exit(0);
   return NULL;
}
