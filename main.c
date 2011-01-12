#include <stdlib.h>

#include <event_handler.h>
#include <socketserver.h>
#include <signal.h>

#include <pthread.h>
#include <macros.h>
typedef struct s_params{
	char* op;
	char* arg;
}PARAMS;


extern  sig_atomic_t do_event_loop ;


static void serverhandler(int signum)
{
    UNUSED(signum);
    do_event_loop = 0;
}



//Thread that handling events from driver
void* Event_processor(void* cnt)
{
	int error;
	PARAMS params =*((PARAMS *)cnt);
	char *op = NULL, *arg = NULL;
	op = params.op;
	arg = params.arg;
	error = event_loop(op, arg);
	pthread_exit((void*)error);

	return((void*)0);
}



int 
main(int argc, char *argv[])
{
	int error;
	char *op = NULL, *arg = NULL;
	PARAMS params = {0};
	void* result;

	pthread_t eventprocessid;
	pthread_t socketserverid;
	if (argc == 3) {
		op = argv[1];
		arg = argv[2];
		params.op = op;
		params.arg = arg;
	}
	umask(0);
	setsid();
	 if(pthread_create(&eventprocessid, NULL, (void *)Event_processor, (void *)&params) == 0){
			printf("eventprocessid thread create OK!\n");
	 }

	 if(pthread_create(&socketserverid, NULL, (void *)socketthread, (void *)&params) == 0){
			printf("socketserverid thread create OK!\n");
	 }

	 signal(SIGINT, serverhandler);

	 //once uncomment, there is segmentation failure, why ?
	  if(pthread_join(eventprocessid, &result) == 0){
			printf("thread eventprocessid = %d, result = %d\n", eventprocessid, (int)result);
	  }

	  if(pthread_join(socketserverid, &result) == 0){
				   printf("thread socketserverid = %d, result = %d\n", socketserverid, (int)result);
	  }

		return 0;
}
