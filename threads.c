/*
Name:threads.c
Description:Thread and processes module implementation.
Authors:Amit Herman Raz Rajwan

*/


#include "threads.h"
#include <stdio.h>



HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine,
	LPDWORD p_thread_id, LPVOID lpParam)
{
	/* Should check for NULL pointers. Skipped for the sake of simplicity. */

	return CreateThread(
		NULL,            /*  default security attributes */
		0,               /*  use default stack size */
		p_start_routine, /*  thread function */
		lpParam,            /*  argument to thread function */
		0,               /*  use default creation flags */
		p_thread_id);  /*  returns the thread identifier */
}

HANDLE CreateMutexSimple(LPCSTR str_mutex_name)
{
	return CreateMutex(NULL,0, str_mutex_name);
}

DWORD WaitThreads(int N, HANDLE thread_handles[],BOOL all){
	DWORD wait_code;
	wait_code = WaitForMultipleObjects(N, thread_handles, all, INFINITE);
	return wait_code;
}

DWORD WaitMutex(HANDLE h_mutex)
{
	DWORD wait_code;
	wait_code = WaitForSingleObject(h_mutex, INFINITE);
	return wait_code;
}


int Init_Threads(int N, LPTHREAD_START_ROUTINE p_start_routine,
	HANDLE p_thread_handles[], DWORD p_thread_ids[]) {

	int i;

	for (i = 0; i < N; i++) {
		p_thread_handles[i] = CreateThreadSimple(p_start_routine, &(p_thread_ids[i]), NULL);

		if (p_thread_handles[i] == NULL) {
			printf("Error when creating thread: %d\n", GetLastError());
			return 0;
		}
	}

	return 1;
}

int Close_Threads(int N, HANDLE p_thread_handles[]) {

	int ret_val;
	int i;

	for (i = 0; i < N; i++)
	{
		ret_val = CloseHandle(p_thread_handles[i]);
		if (FALSE == ret_val)
		{
			printf("Error when closing thread: %d\n", GetLastError());
			return 0;
		}
	}

	return 1;
}

void KillThreads(DWORD code1, DWORD code2, HANDLE h_Threads[]) {
	if (code1 == STILL_ACTIVE)
		TerminateThread(h_Threads[0], 0);
	if(code2== STILL_ACTIVE)
		TerminateThread(h_Threads[1], 0);

}