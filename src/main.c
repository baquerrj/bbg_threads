#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

typedef struct file_s {
   char *name;
   FILE *fid;
} file_t;

static file_t *log_file;

static pthread_t t1;
static pthread_t t2;

static void *thread_child1(void *arg);
static void *thread_child2(void *arg);

static void *thread_fn(void *arg)
{
   if( NULL != arg )
   {
      file_t *my_log = (file_t*)arg;
      printf("Input arg = %s\n", my_log->name);
   }
   printf("New thread started, PID %d TID %d\n",
         getpid(), (pid_t)syscall(SYS_gettid));
   sleep(10);
   printf("New thread terminating\n");
   return NULL;
}


int main( int argc, char *argv[] )
{
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
   pthread_create(&t1, NULL, thread_fn, log_file);
   pthread_create(&t2, NULL, thread_fn, log_file);
   pthread_join(t1, NULL);
   pthread_join(t2, NULL);
   return 0;
}
