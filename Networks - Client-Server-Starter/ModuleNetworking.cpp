#include "Networks.h"
#include "ModuleNetworking.h"

#include <list>

static uint8 NumModulesUsingWinsock = 0;

void printWSErrorAndExit(const char* msg)
{
	wchar_t* s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)& s, 0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);
}

void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}

void ModuleNetworking::disconnect()
{
	for (SOCKET socket : sockets)
	{
		shutdown(socket, 2);
		closesocket(socket);
	}

	sockets.clear();
}

bool ModuleNetworking::sendPacket(const OutputMemoryStream& packet, SOCKET socket)
{
	int result = send(socket, packet.GetBufferPtr(), packet.GetSize(), 0);
	if (result == SOCKET_ERROR) {
		reportError("send");
		return false;
	}

	return true;
}

bool ModuleNetworking::init()
{
	if (NumModulesUsingWinsock == 0)
	{
		NumModulesUsingWinsock++;

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(version, &data) != 0)
		{
			reportError("ModuleNetworking::init() - WSAStartup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::preUpdate()
{
	if (sockets.empty()) return true;

	// NOTE(jesus): You can use this temporary buffer to store data from recv()
	const uint32 incomingDataBufferSize = Kilobytes(1);
	byte incomingDataBuffer[incomingDataBufferSize];

	// TODO(jesus): select those sockets that have a read operation available

	fd_set readSet;
	FD_ZERO(&readSet);

	for (int i = 0; i < sockets.size(); ++i)
		FD_SET(sockets[i], &readSet);

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	if (select(0, &readSet, nullptr, nullptr, &timeout) == SOCKET_ERROR)
	{
		printWSErrorAndExit("Select");

		return false;
	}

	// TODO(jesus): for those sockets selected, check wheter or not they are
	// a listen socket or a standard socket and perform the corresponding
	// operation (accept() an incoming connection or recv() incoming data,
	// respectively).
	// On accept() success, communicate the new connected socket to the
	// subclass (use the callback onSocketConnected()), and add the new
	// connected socket to the managed list of sockets.
	// On recv() success, communicate the incoming data received to the
	// subclass (use the callback onSocketReceivedData()).
	std::list<SOCKET> dis_sockets;

	for (SOCKET i : sockets) {

		if (FD_ISSET(i, &readSet)) {
			if (isListenSocket(i)) {
				sockaddr_in socket_address;
					int addrSize = sizeof(socket_address);

					SOCKET c_socket = accept(i, (sockaddr*)&socket_address, &addrSize);

					if (c_socket == INVALID_SOCKET)
					{
						reportError("Failed!");
							return false;
					}
					else {

						onSocketConnected(c_socket, socket_address);
						addSocket(c_socket);
					}
			}
			else {
				InputMemoryStream packet;
				int receiving = recv(i, packet.GetBufferPtr(), packet.GetCapacity(), 0);

				if (receiving == SOCKET_ERROR)
				{
					reportError("recv");
					dis_sockets.push_back(i);
				}
				else if (receiving == 0) {
					dis_sockets.push_back(i);
				}
				else {
					packet.SetSize(receiving);
					onSocketReceivedData(i, packet);
				}
			}
		}
	}


	// TODO(jesus): handle disconnections. Remember that a socket has been
	// disconnected from its remote end either when recv() returned 0,
	// or when it generated some errors such as ECONNRESET.
	// Communicate detected disconnections to the subclass using the callback
	// onSocketDisconnected().

	// TODO(jesus): Finally, remove all disconnected sockets from the list
	// of managed sockets.

	for (const SOCKET& j : dis_sockets) 
	{
		onSocketDisconnected(j);
		sockets.erase(std::find(sockets.begin(), sockets.end(), j));
	}

	return true;
}

bool ModuleNetworking::cleanUp()
{
	disconnect();

	NumModulesUsingWinsock--;
	if (NumModulesUsingWinsock == 0)
	{

		if (WSACleanup() != 0)
		{
			reportError("ModuleNetworking::cleanUp() - WSACleanup");
			return false;
		}
	}

	return true;
}

void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}
