#define HAVE_STRUCT_TIMESPEC
#pragma comment(lib, "ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS
#include <pthread.h>
#include <winsock.h>
#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include <time.h>

pthread_mutex_t mutex;
pthread_mutex_t mutex_file;

typedef struct ClientsProfile {
	char nickname[256];
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
	char entered_nickname[256] = { 0 };
	char entered_password[256] = { 0 };
	int ret;
	int client_id = ClientsCount - 1;
	char recieve[256] = { 0 };
	//char transmit[256] = { 0 };
	sprintf_s(transmit, "%s", "Enter your name:\n");
	ret = send(ClientsArray[client_id].socket, transmit, sizeof(transmit), 0);
	for (int i = 0; i < 256; i++)
		transmit[i] = 0;
	//Здесь будет проверка логина в базе данных 

	FILE* fin;
	fin = fopen("passwords.txt", "r");
	char parser[256] = { 0 };
	int flag_logging = 2;//0 новый человек, 1 уже регистрировлася, 2, такой ник уже онлайн
	char real_nickname[256] = { 0 };
	char real_password[256] = { 0 };

	int nickname_used = 0;
	while (flag_logging == 2)
	{
		if (ret == SOCKET_ERROR)
		{
			printf("Error sending data\n");
			ClientsArray[client_id] = { 0 };
			//ClientsCount--;

			return (void*)1;
		}
		ret = recv(ClientsArray[client_id].socket, entered_nickname, 256, 0);
		nickname_used = 0;
		for (int i = 0; i < 50; i++)
		{
			if (strcmp(entered_nickname, ClientsArray[i].nickname) == 0)
				nickname_used = 1;
		}
		if (nickname_used == 0)
		{
			sprintf_s(transmit, "Accepted!\n");
			ret = send(ClientsArray[client_id].socket, transmit, sizeof(transmit), 0);
			flag_logging = 0;
			break;
		}
		printf("The second User{%s} tried to enter\n", entered_nickname);
		sprintf_s(transmit, "%s is already online. Enter another nickname:\n", entered_nickname);
		ret = send(ClientsArray[client_id].socket, transmit, sizeof(transmit), 0);
		for (int i = 0; i < 256; i++)
		{
			entered_nickname[i] = '\0';
			
		}
	}
	
	while (NULL != fgets(parser, 256, fin))
	{
		if (ret == SOCKET_ERROR)
		{
			printf("Error sending data\n");
			ClientsArray[client_id] = { 0 };
			//ClientsCount--;

			return (void*)1;
		}
		for (int i = 0; i < parser[i] != ' ' && parser[i] != '\n' && parser[i] != '\0'; i++)
		{
			if (parser[i] != ' ')
				real_nickname[i] = parser[i];
		}
		//printf("%s\n", parser);

		for (int i = strlen(real_nickname) + 1; parser[i] != '\0' && parser[i] != '\n'; i++)
		{
			real_password[i - (strlen(real_nickname) + 1)] = parser[i];
		}
		printf("%s {%s,%s}\n", entered_nickname, real_nickname, real_password);
		if (strcmp(real_nickname, entered_nickname) == 0)
		{
			flag_logging = 1;
			break;
		}
		for (int i = 0; i < 256; i++)
		{
			parser[i] = '\0';
			real_password[i] = '\0';
			real_nickname[i] = '\0';
		}


	}

	if (flag_logging == 0)
	{
		//printf("A new user{%s}\n", entered_nickname);
		sprintf_s(transmit, "%s", "You have not been registered. Come up with a password:\n");
		ret = send(ClientsArray[client_id].socket, transmit, sizeof(transmit), 0);

		fclose(fin);
		fin = fopen("passwords.txt", "a");

		ret = recv(ClientsArray[client_id].socket, entered_password, 256, 0);
		char new_pass[256] = { 0 };
		if (entered_password[0] != '\0')
		sprintf_s(new_pass, "%s %s\n", entered_nickname, entered_password);
		fprintf(fin, new_pass);
		fclose(fin);

		printf("A new user{%s}\n", entered_nickname);
		sprintf_s(transmit, "%s", "Password created!\n");
		ret = send(ClientsArray[client_id].socket, transmit, sizeof(transmit), 0);
	}
	else if (flag_logging == 1)
	{
		printf("User{%s}\n", entered_nickname);
		sprintf_s(transmit, "%s", "Please, enter your password:\n");
		ret = send(ClientsArray[client_id].socket, transmit, sizeof(transmit), 0);
	
		ret = recv(ClientsArray[client_id].socket, entered_password, 256, 0);
		while (strcmp(entered_password, real_password) != 0)
		{
			if (ret == SOCKET_ERROR)
			{
				printf("Error sending data\n");
				ClientsArray[client_id] = { 0 };
				//ClientsCount--;

				return (void*)1;
			}
			printf("User{%s} failed to log in\n", entered_nickname);
			printf("%s vs %s\n", real_password, entered_password);
			sprintf_s(transmit, "%s", "Wrong password. Try again!\n");
			ret = send(ClientsArray[client_id].socket, transmit, sizeof(transmit), 0);
			for (int i = 0; i < 256; i++)
				entered_password[i] = { 0 };
			ret = recv(ClientsArray[client_id].socket, entered_password, 256, 0);
		}
		sprintf_s(transmit, "%s", "You have succesfully logged in!\n");
		ret = send(ClientsArray[client_id].socket, transmit, sizeof(transmit), 0);

	}



	//вносим логин в массив клиентов
	strcpy_s(ClientsArray[client_id].nickname, entered_nickname);

	sprintf_s(transmit, "\n=====================\nWelcome to the chat!\n/all for writing in group chat\n/online for find out who is online\n=====================\n\n");
	ret = send(ClientsArray[client_id].socket, transmit, sizeof(transmit), 0);

	sprintf_s(transmit, "%s entered", ClientsArray[client_id].nickname);
	printf("%s\n", transmit);

	ClientsArray[client_id].logged_in = 1;
	
	for (int i = 0; i < 256; i++)
		transmit[i] = '\0';


	
	printf("KEK");
	while (1) 
	{

		ret = recv(ClientsArray[client_id].socket, transmit, 256, 0);
		//transmit[strlen(transmit)] = 0;
	
		if (strcmp("/exit", transmit) == 0) {
			return (void*)0;
		}
		//printf("%s online now\n", transmit);

		if (transmit[0] != 0)
		{
			if (strstr(transmit, "/all ") != 0)
			{
			    time_t s_time;
				struct tm *m_time;
				char str_t[128] = "";
				s_time = time(NULL);
				m_time = localtime (&s_time);
				strftime(str_t, 128, "%H:%M:%S", m_time);
				char mes[256] = { 0 };
				for (int i = 5; transmit[i] != 0; i++)
					mes[i - 5] = transmit[i];
				sprintf_s(transmit, "[%s] [All] <%s>: %s", str_t, ClientsArray[client_id].nickname, mes);
				printf("%s", transmit);
				for (int i = 0; i < 50; i++)
				{
					if (i != client_id && ClientsArray[i].logged_in == 1)
						ret = send(ClientsArray[i].socket, transmit, strlen(transmit), 0);
				}
			}
			else if (strstr(transmit, "/m ") != 0)
			{
				//int ptr = strstr(transmit, "/all");
				time_t s_time;
				struct tm* m_time;
				char str_t[128] = "";
				s_time = time(NULL);
				m_time = localtime(&s_time);
				strftime(str_t, 128, "%H:%M:%S", m_time);
				char mes[256] = { 0 };
				char destination[256] = { 0 };
				for (int i = 3; transmit[i] != ' ' && transmit[i] != '\n' && transmit[i] != '\0'; i++)
				{
					if (transmit[i] != ' ')
						destination[i-3] = transmit[i];
				}
				for (int i = 4 + strlen(destination); transmit[i] != '\n' && transmit[i] != '\0'; i++)
				{
					if (transmit[i] != ' ')
						mes[i - (4 + strlen(destination))] = transmit[i];
				}
				
				sprintf_s(transmit, "[%s] [Personally] <%s>: %s\n", str_t, ClientsArray[client_id].nickname, mes);
				int delivered = 0;

				printf("{%s}", mes);
				for (int i = 0; i < 50; i++)
				{
					if (strcmp(destination,ClientsArray[client_id].nickname) == 0)
					{
						delivered = 3;
						break;
					}
					if (i != client_id && ClientsArray[i].logged_in == 1)
					{
						if (mes[0] != '\0')
						{
							ret = send(ClientsArray[i].socket, transmit, strlen(transmit), 0);
							delivered = 1;
							printf("%s", transmit);
						}
						break;
					}
					
				}

				for (int i = 0; i < 256; i++) {
					transmit[i] = 0;
				}
				if (mes[0] == '\0')
					delivered = 4;
				printf("%d {%s, %s, %s,}", delivered, destination, ClientsArray[client_id].nickname, mes);
				if (delivered == 0)
				{
					sprintf_s(transmit, "This user is not online\n\0");
					ret = send(ClientsArray[client_id].socket, transmit, strlen(transmit), 0);
				}
				else if (delivered == 1)
				{
					sprintf_s(transmit, "Your message is delivered\n\0");
					ret = send(ClientsArray[client_id].socket, transmit, strlen(transmit), 0);
				}
				else if (delivered == 3)
				{
					sprintf_s(transmit, "You can't send messages to yourself!\n\0");
					ret = send(ClientsArray[client_id].socket, transmit, strlen(transmit), 0);
				}
				else if (delivered == 4)
				{
					sprintf_s(transmit, "You can't send empty messages!\n\0");
					ret = send(ClientsArray[client_id].socket, transmit, strlen(transmit), 0);
				}
			}
			else if (strstr(transmit, "/online") != 0)
			{
				char list[256] = { 0 };
				int online = 0;
				for (int i = 0; i < 50; i++)
				{
					if (ClientsArray[i].logged_in == 1)
					{
						online++;
					}
				}
				sprintf_s(list, "\n=====================\nOnline now: %d user/s\n", online);
				int index = 0;
				for (int i = 0; i < 50; i++)
				{
					if (ClientsArray[i].logged_in == 1)
					{
						char temp[256] = { 0 };
						
						sprintf_s(temp, "User %c: ", index + 49);
						strcat_s(list, temp);
						strcat_s(list, ClientsArray[i].nickname);
						strcat_s(list, "\n");
						index++;
					}
				}
				strcat_s(list, "=====================\n\n");
				ret = send(ClientsArray[client_id].socket, list, strlen(list), 0);
			}
			else
			{
				for (int i = 0; i < 256; i++) 
				{
					transmit[i] = 0;
				}
				sprintf_s(transmit, "Uknown command\n");
				ret = send(ClientsArray[client_id].socket, transmit, strlen(transmit), 0);
			}

			for (int i = 0; i < 256; i++) {
				transmit[i] = 0;
			}
			//printf("%d online now\n", ClientsCount);

		}
		if (ret == SOCKET_ERROR)
		{
			printf("Error sending data\n");
			ClientsArray[client_id] = { 0 };
			//ClientsCount--;
			
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

	while (1)
	{
		size = sizeof(clientaddr);
		client = accept(server, (sockaddr*)&clientaddr, &size);

		CLPR temp = { {0},client, {0} };
	
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