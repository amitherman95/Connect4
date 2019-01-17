/*
*					Authors: Amit Herman		Raz Rajwan
									 313298036      208612663 
*				
*					Project:Excercise 4
*
*/



#include "client.h"
#include "server.h"
#include "stdio.h"
#include "threads.h"



int main(int argc, char *argv[]) {

	int port;

	if (st			rcmp(argv[1], "server") == 0) {
		sscanf_s(argv[3], "%ld", &port);

	MAIN_SERVER:
		if (!mainServer(argv[2], port))
			printf("Something happend, restablishing connection\n\n");
		else
			printf("Good game\n\n");

		goto MAIN_SERVER;
	}

	else if (strcmp(argv[1], "client") == 0)
	{

		char* logfile = argv[2];
		char* inputfile;
		int port =atoi( argv[3]);

		if (strcmp(argv[4], "file" )== 0)// file mode
		{

			inputfile = argv[5];
			mainClient(FILE_MODE, inputfile, logfile, port);

		}
		else if (strcmp(argv[4], "human") == 0)
		{
			mainClient(HUMAN_MODE, NULL, logfile, port);
		}
		else
			printf("problem with client aruments\n");
	}
	else
		printf("Main arguments are \"client\"  or \"server\"");

	return 0;
}