#define HAVE_STRUCT_TIMESPEC
#pragma comment(lib, "ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS
#include <winsock.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

void* SendData2Server(void* param)
{
	SOCKET client = (SOCKET)param;
	char mes[256] = { 0 };
	while (1) {
		gets_s(mes);
		if (strcmp(mes, "/exit") == 0) {
			printf("Session is closed\n");
			return (void*)0;
		}
		int ret = send(client, mes, strlen(mes), 0);
		if (ret == SOCKET_ERROR)
		{
			printf("Can't send message\n");
			return (void*)1;
		}
	}
}

void* GetDataFromServer(void* param)
{
	SOCKET client = (SOCKET)param;
	char mes[256];
	int ret;
	while (1) {
		ret = recv(client, mes, 256, 0);
		if (!ret || ret == SOCKET_ERROR)
		{
			printf("Can't recieve message\n");
			return (void*)1;
		}
		mes[ret] = '\0';
		printf("%s\n", mes);
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
	//u_long iMode = 0;
	//ioctlsocket(client, FIONBIO, &iMode);
	char message[256] = { 0 };
	int ret = recv(client, message, sizeof(message), 0);
	printf("%s", message);
	for (int i = 0; i < 256; i++) {
		message[i] = 0;
	}
	gets_s(message);
	ret = send(client, message, strlen(message), 0);
	for (int i = 0; i < 256; i++) {
		message[i] = 0;
	}
	pthread_t Input;
	pthread_t Output;
	int status = pthread_create(&Input, NULL, SendData2Server, (void*)client);
	int controlling = pthread_create(&Output, NULL, GetDataFromServer, (void*)client);
	status = pthread_join(Input, NULL);
	pthread_detach(Input);
	pthread_detach(Output);
	closesocket(client);
	return 0;
}