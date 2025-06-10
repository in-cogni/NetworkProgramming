#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include<Windows.h>
#include<iphlpapi.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iostream>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27016"
#define DEFAULT_BUFFER_LENGTH 1500
#define SZ_SORRY "Sorry, all is busy"

void main()
{
	setlocale(LC_ALL, "");

	//1) Инициализируем WinSock:
	WSAData wsaData;
	INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	addrinfo hints;//содержит критерии создания сокета
	ZeroMemory(&hints, sizeof(hints));//обнуляет память структуры hints
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;//потоковый сокет (TCP)
	hints.ai_protocol = IPPROTO_TCP;

	//2) Выполняем разрешение имен: 
	addrinfo* result;
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		cout << "getaddrinfo() failed with code: " << iResult << endl;
		WSACleanup();
		return;
	}

	//3) Создаем сокет для подключения к серверу:
	SOCKET connect_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (connect_socket == INVALID_SOCKET)
	{
		cout << "Socket creation failed with code: " << WSAGetLastError;
		WSACleanup();
		return;
	}

	//4) Подключаемся к серверу:
	iResult = connect(connect_socket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Unable to connect to Server" << endl;
		closesocket(connect_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//5) Получение и отправка данных:
	//int recvbuflen = DEFAULT_BUFFER_LENGTH;
	CHAR send_buffer[DEFAULT_BUFFER_LENGTH] = "Hello, Server, I am Client";
	CHAR recvbuffer[DEFAULT_BUFFER_LENGTH]{};

	do
	{
		iResult = send(connect_socket, send_buffer, strlen(send_buffer), 0);
		if (iResult == SOCKET_ERROR)
		{
			cout << "Send data failed with " << WSAGetLastError() << endl;
			closesocket(connect_socket);
			freeaddrinfo(result);
			WSACleanup();
			return;
		}
		cout << iResult << " Bytes sent" << endl;

		//6) Receive data:

		iResult = recv(connect_socket, recvbuffer, DEFAULT_BUFFER_LENGTH, 0);
		if (iResult > 0)cout << "Bytes received: " << iResult << ", Message: " << recvbuffer << endl;
		else if (iResult == 0)cout << "Connection closed " << endl;
		else cout << "Receive failed with code: " << WSAGetLastError() << endl;
		////////////////////////////////////////////////////////////////////
		if (strcmp(recvbuffer, SZ_SORRY) == 0) break;
		////////////////////////////////////////////////////////////////////
		ZeroMemory(send_buffer, sizeof(send_buffer));
		ZeroMemory(recvbuffer, sizeof(recvbuffer));
		cout << "Введите сообщение: ";
		SetConsoleCP(1251);
		cin.getline(send_buffer, DEFAULT_BUFFER_LENGTH);
		SetConsoleCP(866);
		//for (int i = 0; send_buffer[i]; i++)send_buffer[i] = tolower(send_buffer[i]);
	} while (iResult > 0 && strcmp(send_buffer, "exit"));

	//7) Disconnect:
	iResult = shutdown(connect_socket, SD_SEND);
	closesocket(connect_socket);
	freeaddrinfo(result);
	WSACleanup();
}

