#define HAVE_STRUCT_TIMESPEC
#pragma comment(lib, "ws2_32.lib")
#include <pthread.h>
#include <winsock.h>
#include <stdio.h>
#include <Windows.h>

pthread_mutex_t mutex;
pthread_mutex_t mutex_file;

void* ClientStart(void* param)
{
	SOCKET client = (SOCKET)param;
	char recieve[256], transmit[256];
	int ret;
	while (1) {
		ret = recv(client, recieve, 256, 0);
		if (!ret || ret == SOCKET_ERROR)
		{
			//pthread_mutex_lock(&mutex);
			//pthread_mutex_lock(&mutex_file);
			printf("Error getting data\n");
			//pthread_mutex_unlock(&mutex_file);
			//pthread_mutex_unlock(&mutex);
			return (void*)1;
		}
		recieve[ret] = '\0';

		//pthread_mutex_lock(&mutex);
		//pthread_mutex_lock(&mutex_file);
		printf("%s (recieved)\n", recieve);
		//pthread_mutex_unlock(&mutex_file);
		//pthread_mutex_unlock(&mutex);
		sprintf_s(transmit, "%s\n", "Your data is recieved!");
		//Sleep(2000);
		ret = send(client, transmit, sizeof(transmit), 0);
		if (ret == SOCKET_ERROR)
		{
			//pthread_mutex_lock(&mutex);
			//pthread_mutex_lock(&mutex_file);
			printf("Error sending data\n");
			//pthread_mutex_unlock(&mutex_file);
			//pthread_mutex_unlock(&mutex);
			return (void*)2;
		}
		if (strcmp("/exit", recieve) == 0) {
			break;
		}
	}
	return (void*)0;
}

int CreateServer()
{
	SOCKET server, client;
	sockaddr_in localaddr, clientaddr;
	int size;
	server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (server == INVALID_SOCKET)
	{
		printf("Error create server\n");
		return 1;
	}
	localaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(5510);//port number is for example, must be more than 1024
	if (bind(server, (sockaddr*)&localaddr, sizeof(localaddr)) == SOCKET_ERROR)
	{
		printf("Can't start server\n");
		return 2;
	}
	else
	{
		printf("Server is started\n");
	}
	listen(server, 50);//50 клиентов в очереди могут стоять
	//pthread_mutex_init(&mutex, NULL);
	//pthread_mutex_init(&mutex_file, NULL);
	while (1)
	{
		size = sizeof(clientaddr);
		client = accept(server, (sockaddr*)&clientaddr, &size);

		if (client == INVALID_SOCKET)
		{
			printf("Error accept client\n");
			continue;
		}
		else
		{
			printf("Client is accepted\n");
		}
		pthread_t mythread;
		int status = pthread_create(&mythread, NULL, ClientStart, (void*)client);
		pthread_detach(mythread);
	}
	pthread_mutex_destroy(&mutex_file);
	pthread_mutex_destroy(&mutex);
	printf("Server is stopped\n");
	closesocket(server);

	return 0;
}

int main()
{
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(1, 1), &wsd) == 0)
	{
		printf("Connected to socket lib\n");
	}
	else
	{
		return 1;
	}
	return CreateServer();
}