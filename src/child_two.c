//#include <sys/syscall.h>
//#include <unistd.h>
//#include <stdlib.h>
#include "child_two.h"
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <string.h>


static timer_t    timerid;
struct itimerspec trigger;

static time_t my_time;
static file_t *log;
static FILE *stat_file;

static pthread_mutex_t  tmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   tcond = PTHREAD_COND_INITIALIZER;

static void print_header(FILE* file)
{
   struct timespec time;
   clock_gettime(CLOCK_REALTIME, &time);
   fprintf( file, "\n========================================\n" );
   fprintf( file, "Thread 2 [%d]: %ld s - %ld ns\n",
            (pid_t)syscall(SYS_gettid), time.tv_sec, time.tv_nsec );
   return;
}


void child2_exit(void)
{
   time(&my_time);
   timer_delete(timerid);

   while( pthread_mutex_lock(&mutex) );
   print_header( log->fid );
   fprintf( log->fid, "Goodbye World!\n" );
   fclose( log->fid );
   fclose( stat_file );
   pthread_mutex_unlock(&mutex);

   free( log );
   pthread_exit(0);
}


static void sig_handler(int signo)
{
   if( signo == SIGUSR1 )
   {
      printf("Received SIGUSR1! Exiting...\n");
      child2_exit();
   }
   else if( signo == SIGUSR2 )
   {
      printf("Received SIGUSR2! Exiting...\n");
      child2_exit();
   }
   else if( signo == SIGCONT )
   {
      struct timespec thTimeSpec;

      clock_gettime(CLOCK_REALTIME, &thTimeSpec);
      printf("Clock_gettime: %ld s - %ld ns\n",
            thTimeSpec.tv_sec, thTimeSpec.tv_nsec);
      pthread_cond_broadcast(&tcond);
   }
   return;
}

#if 0
static void timer_handler(union sigval sv)
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
#endif


static int report_cpu(void)
{
   while( 1 )
   {
      pthread_mutex_lock(&tmutex);
      pthread_cond_wait(&tcond, &tmutex);
      pthread_mutex_unlock(&tmutex);
      char buffer[100];
      print_header( log->fid );
      fprintf( log->fid, "Reading CPU Utilization:\n" );
      while( NULL != fgets( buffer, 100, stat_file ) )
      {
         if( ferror( stat_file ) )
         {
            perror( "Error reading file!" );
            clearerr( stat_file );
            rewind( stat_file );
            return 1;
         }
         fprintf( log->fid, "%s", buffer );
      }
      rewind( stat_file );
   }
   return 0;
}

static int setup_timer(void)
{
   /* Set up timer */
   char info[] = "timer woken up!";
   struct sigevent sev;

   memset(&sev, 0, sizeof(struct sigevent));
   memset(&trigger, 0, sizeof(struct itimerspec));

   sev.sigev_notify = SIGEV_SIGNAL;
//   sev.sigev_notify_function = &timer_handler;
   sev.sigev_signo = SIGCONT;
   signal(SIGCONT, sig_handler);
   sev.sigev_value.sival_ptr = &info;

   timer_create(CLOCK_REALTIME, &sev, &timerid);

   trigger.it_value.tv_sec = 1;
//   trigger.it_interval.tv_nsec = 100 * 1000000;
   trigger.it_interval.tv_sec = 2;

   pthread_mutex_init( &tmutex, NULL );
   pthread_cond_init( &tcond, NULL );

   timer_settime(timerid, 0, &trigger, NULL);
   return 0;
}

void *child2_fn(void *arg)
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

   stat_file = fopen( "/proc/stat", "r" );
   if( stat_file == NULL )
   {
      perror( "Encountered error opening /proc/stat" );
      pthread_exit(&failure);
   }
   printf( "Opened /proc/stat for reading\n");

   signal(SIGUSR1, sig_handler);
   signal(SIGUSR2, sig_handler);

   setup_timer();
   while( !report_cpu() );
   child2_exit();
   return NULL;
}
