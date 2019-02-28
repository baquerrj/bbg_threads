//#include <stdlib.h>
//#include <unistd.h>
//#include <sys/syscall.h>
#include "child_one.h"
#include "child_two.h"
#include "common.h"

static pthread_t t1;
static pthread_t t2;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void print_header(FILE *file)
{
   struct timespec time;
   clock_gettime(CLOCK_REALTIME, &time);
   fprintf( file, "=====================================================\n" );
   fprintf( file, "Master Thread [%d]: %ld s - %ld ns\n",
            (pid_t)syscall(SYS_gettid), time.tv_sec, time.tv_nsec );
   return;
}
int main( int argc, char *argv[] )
{
   static time_t master_time;
   static file_t *log;
   args_t *args;
   printf( "Number of arguments %d\n", argc );
   if( argc > 1 )
   {
      log = malloc( sizeof( file_t ) );
      log->fid = fopen( argv[1], "a" );
      log->name = argv[1];
      printf( "Opened file %s\n", argv[1] );
      args = malloc(sizeof(args_t));
      args->arg1 = log->name;
      if( argv[2] )
      {
         args->arg2 = argv[2];
      }
   }
   else
   {
      fprintf( stderr, "Name of log file required!\n" );
      return 1;
   }

   time(&master_time);
   print_header( log->fid );
   fprintf( log->fid, "Starting Threads! Start Time: %s\n",
            ctime(&master_time));
   print_header( stdout );
   fprintf( stdout, "Starting Threads! Start Time: %s\n",
            ctime(&master_time));

   /* Attempting to spawn child threads */
   pthread_create(&t1, NULL, child1_fn, (void *)args);
   pthread_create(&t2, NULL, child2_fn, (void *)args);
   pthread_join(t1, NULL);
   pthread_join(t2, NULL);

   time(&master_time);

   print_header( log->fid );
   fprintf( log->fid, "All threads exited! Main thread exiting... " );
   fprintf( log->fid, "End Time: %s",
            ctime(&master_time));
   fclose( log->fid );

   print_header( stdout );
   fprintf( stdout, "All threads exited! Main thread exiting... " );
   fprintf( stdout, "End Time: %s",
            ctime(&master_time));
   free( log );
   free( args );
   return 0;
}
