//#include <sys/syscall.h>
//#include <unistd.h>
//#include <stdlib.h>
#include "child_two.h"
#include <errno.h>
#include "time.h"
#include <signal.h>
#include <string.h>
static timer_t    timerid;
struct itimerspec trigger;

static time_t my_time;
static file_t *log_file;
static FILE *stat_file;

static pthread_mutex_t  tmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   tcond = PTHREAD_COND_INITIALIZER;

void child2_exit(void)
{
   time(&my_time);
   timer_delete(timerid);
   while( pthread_mutex_lock(&mutex) );
   fprintf( log_file->fid, "%sTID [%d]: Goodbye World!\n",
            ctime(&my_time), (pid_t)syscall(SYS_gettid));
   fclose( log_file->fid );
   pthread_mutex_unlock(&mutex);
   return;
}


void timer_handler(union sigval sv)
{
   char *s = sv.sival_ptr;
   struct timespec thTimeSpec;

   clock_gettime(CLOCK_REALTIME, &thTimeSpec);
   printf("Clock_gettime: %ld s - %ld ns\n",
         thTimeSpec.tv_sec, thTimeSpec.tv_nsec);

   puts(s);
   pthread_cond_broadcast(&tcond);
   return;
}


void *child2_fn(void *arg)
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

   /* Set up timer */
   char info[] = "timer woken up!";
   struct sigevent sev;

   memset(&sev, 0, sizeof(struct sigevent));
   memset(&trigger, 0, sizeof(struct itimerspec));

   sev.sigev_notify = SIGEV_THREAD;
   sev.sigev_notify_function = &timer_handler;
   sev.sigev_value.sival_ptr = &info;

   timer_create(CLOCK_REALTIME, &sev, &timerid);

   trigger.it_value.tv_sec = 1;
   trigger.it_interval.tv_nsec = 100 * 1000000;

   pthread_mutex_init( &tmutex, NULL );
   pthread_cond_init( &tcond, NULL );

   timer_settime(timerid, 0, &trigger, NULL);

   /* Setting up timer */
   stat_file = fopen( "/proc/stat", "r" );
   if( stat_file == NULL )
   {
      perror( "error opening /proc/stat" );
      child2_exit();
   }
   printf( "Opened /proc/stat for reading\n");

   while ( 1 )
   {
      pthread_mutex_lock(&tmutex);
      pthread_cond_wait(&tcond, &tmutex);
      pthread_mutex_unlock(&tmutex);
      char buffer[100];
      while( NULL != fgets( buffer, 100, stat_file ) )
      {
         if( ferror( stat_file ) )
         {
            perror( "Error reading file" );
            clearerr( stat_file );
            rewind( stat_file );
            break;
         }
         fprintf( log_file->fid, "%s", buffer );
      }
      rewind( stat_file );
   }
   child2_exit();
   return NULL;
}
