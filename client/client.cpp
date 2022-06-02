#pragma comment(lib, "ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS
#include <winsock.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

void SendData2Server(const char mes[], SOCKET client)
{
	char answer[256] = { 0 };
	int ret = send(client, mes, strlen(mes), 0);
	if (ret == SOCKET_ERROR)
	{
		printf("Can't send message\n");
		closesocket(client);
		return;
	}
	ret = SOCKET_ERROR;
	while (ret == SOCKET_ERROR)
	{
		//полчение ответа
		ret = recv(client, answer, sizeof(answer), 0);
		//обработка ошибок
		if (ret == 0 || ret == WSAECONNRESET)
		{
			printf("Connection closed\n");
			break;
		}
		if (ret < 0)
		{
			printf("Can't recieve message\n");
			/*closesocket(client);
			return;*/
			continue;
		}
		//вывод на экран количества полученных байт и сообщение
		printf("%s", answer);
	}
}

int main()
{
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(1, 1), &wsd) != 0)
	{
		printf("Can't connect to socket lib");
		return 1;
	}
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (client == INVALID_SOCKET)
	{
		printf("Error create socket\n");
		return 0;
	}
	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_port = htons(5510); //the same as in server
	server.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //special look-up address
	if (connect(client, (SOCKADDR*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Can't connect to server\n");
		closesocket(client);
		return 0;
	}
	char message[256] = { 0 };
	gets_s(message);
	while (strcmp("/exit", message) != 0)
	{
		SendData2Server(message, client);
		for (int i = 0; i < 256; i++) {
			message[i] = 0;
		}
		gets_s(message);
	}
	SendData2Server("/exit", client);
	closesocket(client);
	printf("Session is closed\n");
	return 0;
}