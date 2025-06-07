// WinSockServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
#define _CRT_SECURE_NO_WARNINGS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include<Windows.h>
#include<WinSock2.h>
#include <WS2tcpip.h>
#include<iphlpapi.h>
#include <iostream>
using namespace std;

#pragma comment(lib, "WS2_32.lib")

#define DEFAULT_PORT  "27016"
#define DEFAULT_BUFFER_LENGTH 1500
#define SZ_SORRY "Sorry, all is busy"

void main()
{
	setlocale(LC_ALL, "");
	//1) Инициализация WinSock:
	WSAData wsaData;

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		cout << "Error: WSAstartup failed: " << iResult << endl;
		return;
	}

	//2) ????Получаем IP-адреса, на которых можно запустить сокет:
	//2) Проверяем, не занят ли порт, на котором мы хотим запустить свой Сервер.
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // TCP/IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	addrinfo* result = NULL;
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		WSACleanup();
		cout << "Error: getaddrinfo failed: " << iResult << endl;
		return;
	}
	cout << hints.ai_addr << endl;

	//3) Создаем сокет, который будет пролушивать Сервер:
	SOCKET ListenSocket = 
		socket(result->ai_family, result->ai_socktype, result->ai_protocol);
 
	if (ListenSocket == INVALID_SOCKET)
	{
		cout << "Error: Socket creation failed: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//4) связываем сокет с сетевой картой, которую он будет прослушивать:
	//strcpy(result->ai_addr->sa_data, "127.0.0.1");
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Error: bind failed with code "<<WSAGetLastError() << endl;
		closesocket(ListenSocket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}
	//5) Запускаем Сокет:
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Error: Listen failed with code " << WSAGetLastError() << endl;
		closesocket(ListenSocket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}
	
	VOID WINAPI HandleClient(SOCKET ClientSocket);
	CONST INT MAX_CLIENTS = 3;
	SOCKET clients[MAX_CLIENTS] = {};
	DWORD dwThreadIDs[MAX_CLIENTS];
	HANDLE hThreads[MAX_CLIENTS] = {};

	INT i = 0; 

	while (true)
	{
		SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
		if (i < MAX_CLIENTS)
		{
			//HandleClient(ClientSocket);
			clients[i] = ClientSocket;
			hThreads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HandleClient, (LPVOID)clients[i], 0, &dwThreadIDs[i]);
			i++;
		}
		else
		{
			CHAR receive_buffer[DEFAULT_BUFFER_LENGTH] = {};
			INT iResult = recv(ClientSocket, receive_buffer, DEFAULT_BUFFER_LENGTH, 0);
			if (iResult > 0)
			{
				cout << "Bytes received: " << iResult << endl;
				cout << "Message: " << receive_buffer << endl;
				//CHAR SZ_SORRY[] = "Sorry, but all is busy";
				INT iSendResult = send(ClientSocket, SZ_SORRY, strlen(SZ_SORRY), 0);
				closesocket(ClientSocket);
			}
		}
	}

	WaitForMultipleObjects(MAX_CLIENTS, hThreads, TRUE, INFINITE);
	

	closesocket(ListenSocket);
	freeaddrinfo(result);
	WSACleanup();
}

VOID WINAPI HandleClient(SOCKET ClientSocket)
{
	INT iResult = 0;
	//6) Зацикливаем сокет на получение соединений от клиентов:
	CHAR recvbuffer[DEFAULT_BUFFER_LENGTH] = {};
	int recv_buffer_length = DEFAULT_BUFFER_LENGTH;
	do
	{
		ZeroMemory(recvbuffer, sizeof(recvbuffer));
		iResult = recv(ClientSocket, recvbuffer, recv_buffer_length, 0);
		if (iResult > 0)
		{
			cout << "Bytes received: " << iResult << endl;
			cout << "Message: " << recvbuffer << endl;
			CHAR sz_responce[] = "Hello, I am Server! Nice to meet you!";
			//INT iSendResult = send(ClientSocket, sz_responce, sizeof(sz_responce), 0);
			INT iSendResult = send(ClientSocket, recvbuffer, strlen(recvbuffer), 0);

			if (iSendResult == SOCKET_ERROR)
			{
				cout << "Errer: Send failed with code: " << WSAGetLastError() << endl;
				closesocket(ClientSocket);
				//closesocket(ListenSocket);
				//freeaddrinfo(result);
				//WSACleanup();
				//return;
			}
			cout << "Bytes sent: " << iSendResult << endl;
		}
		else if (iResult == 0)
		{
			cout << "Connection closing" << endl;
			closesocket(ClientSocket);
		}
		else
		{
			cout << "Error: recv() failed with code " << WSAGetLastError() << endl;
			closesocket(ClientSocket);
			//return;
		}
	} while (iResult > 0);
}
