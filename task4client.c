#include <stdbool.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

//Информация для сокета
int PORT = 4444;
char IP[] = "localhost";
const char* message = "\x08";

//Структура для хранения карты
struct Map
{
	bool* s;
	int w;
	int l;
};

//Печать кадра игры
void print_life(struct Map* m) {
	int ei = m->l;
	int ej = m->w;
	bool* map = m->s;
	int direct[3] = {-1, 0, 1};
	for(int i = 0; i < ei; i++) {
		for(int j = 0; j < ej; j++) {
			if(map[i * ei + j]) printf("1");
			else printf("0");
		}
		printf("\n");
	}
}

int main() 
{
    struct Map data;
	bool* map;
	int n;
	int m;
	int sock;
	//Создание сокета
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Can't create socket!\n");
        exit(1);
    }

    //Парметры сокета
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
	inet_pton(AF_INET, IP, &addr.sin_addr);

	//Подключение к серверу
    if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        printf("Can't connect!\n");
        exit(2);
    }
	//Отправка и получение кадра игры
    send(sock, message, strlen(message), 0);
    recv(sock, &n, sizeof(int), 0);
    recv(sock, &m, sizeof(int), 0);
	map = (bool*)malloc(n * m * sizeof(bool));
    recv(sock, map, m * n, 0);
	data.s = map;
	data.w = n;
	data.l = m;
	//Печать
	print_life(&data);
	//Закрытие сокета
	close(sock);
    return 0;
}