#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <iostream>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_ADDRESS "192.168.0.126"

#define SERVER_PORT 8000

#define PAUSE_AND_EXIT() system("pause"); exit(-1)


void printWSErrorAndExit(const char *msg)
{
	wchar_t *s = NULL;
	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s,
		0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);
	system("pause");
	exit(-1);
}

void client(const char *serverAddrStr, int port)
{
	
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult == SOCKET_ERROR)
	{
		
		printWSErrorAndExit("WSAStartup");
		return;
	}

	
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == INVALID_SOCKET)
	{
		printWSErrorAndExit("socket");
		return;
	}

	
	sockaddr_in toAddr;
	toAddr.sin_family = AF_INET;
	toAddr.sin_port = htons(port);
	const char *toAddrStr = serverAddrStr;
	inet_pton(AF_INET, toAddrStr, &toAddr.sin_addr);

	char buf[5];
	int bufSize = 5 * sizeof(char);
	while (true)
	{
		
		memcpy(&buf, "ping", bufSize);

		iResult = sendto(s, buf, bufSize, 0, (sockaddr*)&toAddr, sizeof(toAddr));
		if (iResult == SOCKET_ERROR)
		{
			printWSErrorAndExit("sendto");
			break;
		}

		sockaddr_in fromAddr;
		int fromSize = sizeof(fromAddr);
		iResult = recvfrom(s, buf, bufSize, 0, (sockaddr*)&fromAddr, &fromSize);
		if (iResult == SOCKET_ERROR)
		{
			printWSErrorAndExit("recvfrom");
			break;
		}

		std::cout << buf << std::endl;

		Sleep(500);
	}

	
	iResult = closesocket(s);
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("closesocket");
		return;
	}

	
	iResult = WSACleanup();
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("WSACleanup");
		return;
	}
}

int main(int argc, char **argv)
{
	client(SERVER_ADDRESS, SERVER_PORT);

	PAUSE_AND_EXIT();
}
