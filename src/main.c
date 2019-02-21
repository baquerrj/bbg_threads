#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

typedef struct file_s {
   char *name;
   FILE *fid;
} file_t;


static pthread_t t1;
static pthread_t t2;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void child_exit(FILE* fid)
{
   printf("Thread %d terminating... closing file handler.\n",
         (pid_t)syscall(SYS_gettid));
   fclose( fid );
   return;
}


static void *child1_fn(void *arg)
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
   sleep(10);
   child_exit(log_file->fid);
   return NULL;
}

static void *child2_fn(void *arg)
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
   sleep(10);
   child_exit(log_file->fid);
   return NULL;
}

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
