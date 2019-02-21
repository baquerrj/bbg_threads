//#include <stdlib.h>
//#include <unistd.h>
//#include <sys/syscall.h>
#include "child_one.h"
#include "child_two.h"
#include "common.h"

static pthread_t t1;
static pthread_t t2;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main( int argc, char *argv[] )
{
   static file_t *log_file;
   printf( "Number of arguments %d\n", argc );
   if( argc > 1 )
   {
      log_file = malloc( sizeof( file_t ) );
      log_file->fid = fopen( argv[1], "a" );
      log_file->name = argv[1];
      printf( "Opened file %s\n", argv[1] );
   }

   printf("Main thread, PID %d TID %d\n",
         getpid(), (pid_t)syscall(SYS_gettid));

   /* Attempting to spwan child threads */
   pthread_create(&t1, NULL, child1_fn, log_file->name);
   pthread_create(&t2, NULL, child2_fn, log_file->name);
   pthread_join(t1, NULL);
   pthread_join(t2, NULL);
   return 0;
}
