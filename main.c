#include <stdlib.h>

#include <event_handler.h>

int 
main(int argc, char *argv[])
{
	int error;
	char *op = NULL, *arg = NULL;

	if (argc == 3) {
		op = argv[1];
		arg = argv[2];
	}
	umask(0);
	setsid();
	error = event_loop(op, arg);

	exit(error);
}
