/*
Name:threads.h
Description:Thread and processes module header.
Authors:Amit Herman Raz Rajwan

*/

#ifndef _THREADS_H_
#define _THREADS_H_
#include <windows.h>

//Shortcut for ReleaseMutex
#define release ReleaseMutex

HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine,
	LPDWORD p_thread_id, LPVOID lpParam);

HANDLE CreateMutexSimple(LPCSTR s_name);

DWORD WaitThreads(int N, HANDLE thread_handles[],BOOL b_all);
DWORD WaitMutex(HANDLE h_mutex);

int Init_Threads(int N, LPTHREAD_START_ROUTINE p_start_routine, HANDLE p_thread_handles[], DWORD p_thread_ids[]);

int Close_Threads(int N, HANDLE p_thread_handles[]);
void KillThreads(DWORD code1, DWORD code2, HANDLE h_Threads[]);

#endif
