#define HAVE_STRUCT_TIMESPEC
#pragma comment(lib, "ws2_32.lib")
#include <pthread.h>
#include <winsock.h>
#include <stdio.h>
#include <Windows.h>
#include <string.h>

pthread_mutex_t mutex;
pthread_mutex_t mutex_file;

typedef struct ClientsProfile {
	char name[100];
	SOCKET socket;
	int logged_in;
}CLPR;

int ClientsCount = 0;

void* TakingInput(void* param)
{
	SOCKET client = (SOCKET)param;
	char recieve[256];
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
	}
	return (void*)0;
}
void* ClientControl(void* param)
{
	CLPR* ClientsArray = (CLPR*)param;
	char transmit[256] = { 0 };
	int ret;
	int clientcount = ClientsCount - 1;
	char recieve[256] = { 0 };
	//char transmit[256] = { 0 };
	sprintf_s(transmit, "%s", "Enter your name:\n");
	ret = send(ClientsArray[clientcount].socket, transmit, sizeof(transmit), 0);
	for (int i = 0; i < 256; i++)
		transmit[i] = 0;
	//Здесь будет проверка в базе данных имени

	strcpy_s(ClientsArray[clientcount].name, recieve);

	ret = recv(ClientsArray[clientcount].socket, recieve, 256, 0);
	sprintf_s(transmit, "%s has entered to the chat", recieve);
	printf("%s\n", transmit);
	for (int i = 0; i < 256; i++)
		transmit[i] = '\0';

	sprintf_s(transmit, "Welcome to the chat!\n");
	ret = send(ClientsArray[clientcount].socket, transmit, sizeof(transmit), 0);
	ClientsArray[clientcount].logged_in = 1;
	for (int i = 0; i < 256; i++)
		transmit[i] = '\0';

	/*while (ClientsArray[clientcount].name[0]!=0) {
		clientcount++;
	}*/
	while (1) {

		ret = recv(ClientsArray[clientcount].socket, transmit, 256, 0);
		printf("%s", transmit);
		//transmit[strlen(transmit)] = 0;
		/*
		if (strcmp("/exit", transmit) == 0) {
			return (void*)0;
		}*/
		//printf("%s online now\n", transmit);


		if (transmit[0] != 0) {

			for (int i = 0; i < ClientsCount; i++)
			{
				if (i != clientcount && ClientsArray[i].logged_in == 1)
					ret = send(ClientsArray[i].socket, transmit, strlen(transmit), 0);
			}
			for (int i = 0; i < 256; i++) {
				transmit[i] = 0;
			}
			//printf("%d online now\n", ClientsCount);

		}
		if (ret == SOCKET_ERROR)
		{
			//pthread_mutex_lock(&mutex);
			//pthread_mutex_lock(&mutex_file);
			printf("Error sending data\n");
			//pthread_mutex_unlock(&mutex_file);
			//pthread_mutex_unlock(&mutex);
			return (void*)1;
		}

	}
}
int CreateServer()
{
	CLPR ClientsArray[50] = { 0 };

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

		CLPR temp = { {0},client, {0} };
		
		//sprintf_s(transmit, "%s", "Ok,\n");
		//ret = send(client, transmit, strlen(transmit), 0);
		//printf("IP address is: %s\n", inet_ntoa(clientaddr.sin_addr));
		//strcpy_s(temp.name, recieve);
		ClientsArray[ClientsCount++] = temp;
		if (client == INVALID_SOCKET)
		{
			printf("Error accept client\n");
			continue;
		}
		else
		{
			printf("Client is accepted\n");
		}
		//pthread_t Input;
		//u_long iMode = 0;
		//ioctlsocket(client, FIONBIO, &iMode);
		//int status = pthread_create(&Input, NULL, TakingInput, (void*)client);
		//pthread_detach(Input);
		pthread_t Control;
		int status = pthread_create(&Control, NULL, ClientControl, (void*)ClientsArray);
		pthread_detach(Control);
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