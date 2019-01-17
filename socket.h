#ifndef _SOCKET_H
#define _SOCKET_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;

int Init_WinSocket(WSADATA * lp_wsa_data);
int SocketGetLastError();
SOCKET createSocket();

TransferResult_t SendString(const char *Str, SOCKET sd);
TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd);
TransferResult_t ReceiveBuffer(char* OutputBuffer, int RemainingBytesToReceive, SOCKET sd);
TransferResult_t SendBuffer(const char* Buffer, int BytesToSend, SOCKET sd);

#endif