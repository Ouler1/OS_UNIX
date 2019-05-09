#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <linux/sched.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

//Структура для хранения карты игры
struct Map
{
	bool* s;
	int w;
	int l;
};

//Информация для сокета
int PORT = 4444;
char IP[] = "localhost";
int create_socket(struct Map* m) 
{
    int sock;
    //Создание сокета
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Can't create socket!\n");
        exit(1);
    }
	//Задание параметров сокета
    struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
	inet_pton(AF_INET, IP, &addr.sin_addr);
    int enable = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    //Привязка сокета к адресу
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("Can't bind!\n");
        exit(2);
    }
    //Количество подключений
	listen(sock, 1);
    
    int client, bytes_read;
	char buffer[1];
    while(1) {
		//Ожидание клиента
        if((client = accept(sock, NULL, NULL)) < 0) {
            printf("Bad client!\n");
            close(sock);
            exit(3);
        }
         while(1) {
			 //Передача информации по игре
            bytes_read = recv(client, buffer, 1, 0);
			if (bytes_read <= 0) break;
			bool* map = m->s;
			int l = m->l;
			int w = m->w;
			send(client, &w, sizeof(w), 0);
			send(client, &l, sizeof(l), 0);
			send(client, map, l * w, 0);
        }
        close(client);
    }
    return 0;
}

//Таймер для секундной синхронизаций
void sleep_t() {
	int msec = 0;
	clock_t before = clock();
	do {
		clock_t difference = clock() - before;
		msec = difference * 1000 / CLOCKS_PER_SEC;
	} while ( msec < 1000 );
}

//Поток для отслеживания ошибок
void* timer(int* data) {
	while(1) {
		sleep_t();
		if(data[0] == 0) {
			printf("ERROR CALC\n");
			continue;
		}
		break;
	}
}

//Расчет следующего кадра игры
void step(struct Map * m) {
	int ei = m->l;
	int ej = m->w;
	bool* map = m->s;
	
	struct Map temp;
	bool* tempm = (bool*)malloc(ei * ej * sizeof(bool));
	temp.w = ej;
	temp.l = ei;
	
	int direct[3] = {-1, 0, 1};
	for(int i = 0; i < ei; i++) {
		for(int j = 0; j < ej; j++) {
			int count = 0;
			for(int k1 = 0; k1 < 3; k1++) {
				for(int k2 = 0; k2 < 3; k2++) {
					if(k1 == 1 && k2 == 1) continue;
					int a = i + direct[k1];
					if(a == -1) a = ei - 1;
					if(a == ei) a = 0;
					int b = j + direct[k2];
					if(b == -1) b = ej - 1;
					if(b == ej) b = 0;
					if(map[a * ei + b]) {
						count = count + 1;
					}
				}
			}
			if(map[i * ei + j]) {
				if(count == 3 || count == 2) tempm[i * ei + j] = 1;
				else tempm[i * ei + j] = 0;
			}
			else {
				if (count == 3) tempm[i * ei + j] = 1;
				else tempm[i * ei + j] = 0;
			}
		}
	}
	temp.s = tempm;
	m[0] = temp;
}

//Поток для создания таймера и вычисления кадров игры
void* calculate(struct Map* data){
	int* status;
	int sts = 0;
	int * dataptr = &sts;
	void* pchild_stack = malloc(4048);
	if (pchild_stack == NULL ) {
		printf("ERROR: Unable to allocate memory.\n");
		exit(-1);
	}
	while(1) {
		dataptr[0] = 0;
		int pid = clone(timer, pchild_stack + (4048), SIGCHLD | CLONE_VM, dataptr);
		if ( pid < 0 ) {
			printf("ERROR: Unable to create the child process.\n");
			exit(-1);
		}

		step(data);
		//Используется, чтобы указать завершение выполнения вычисления кадра
		//Если не успел, то поток таймера сообщит об ошибке
		dataptr[0] = 1;
		waitpid(pid, status, 0);
	}
}
 

//Создание потока вычисления кадров
void create_calc(struct Map* a) {
	void* pchild_stack = malloc(a->l * a->w * 2 + 2048);
	if (pchild_stack == NULL ) {
		printf("ERROR: Unable to allocate memory.\n");
		exit(-1);
	}
	int pid = clone(calculate, pchild_stack + (a->l * a->w * 2 + 2048), SIGCHLD | CLONE_VM, a);
	if ( pid < 0 ) {
		printf("ERROR: Unable to create the child process.\n");
		exit(-1);
	}
}

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

int main (int argc, char * argv[]) {
	if(argc != 2) {
		printf("Argument error\n");
		exit(-1);
	}
	FILE* fp;
	//Открываем файл с начальным шаблоном
	//Формат файла:
	//----------------
	//[ширина] [длина]
	//{0, 1}^[ширина]  -   
	//...				 |
	//...				 | = [длина]
	//...				 |
	//{0, 1}^[ширина]  -
	//----------------
	//Пример:
	//3 2
	//010
	//001
	fp = fopen(argv[1], "rt");
	if (fp == NULL) {
		printf("Cannot open %s\n", argv[1]);
	}
	//Считываем начальный шаблон в память
	int n;
	int m;
	bool* a;
	int cod = fscanf(fp, "%d %d", &n, &m);
	if(cod == -1) {
		printf("Format invalid");
		fclose(fp);
		return 0;
	}
	a = (bool*)malloc(n * m * sizeof(bool));
	char* rowf = malloc(2 + n);
	char* row = malloc(n);
	char* buf = malloc(n);
	int len;
	sprintf(rowf, "%s%d%s", "%", n, "s%n");
	int i;
	int j;
	for(i = 0; i < m; i++) {
		fscanf(fp, rowf, buf, &len);
		if(len - 1 != n) {
			printf("Format invalid");
			fclose(fp);
			return 0;
		}
		for(j = 0; j < n; j++) {
			if(buf[j] == '0') a[n * i + j] = 0;
			else a[n * i + j] = 1;
		}
	}
	fclose(fp);
	
	struct Map data;
	data.s = a;
	data.w = n;
	data.l = m;
	struct Map * dataptr = &data;
	create_calc(dataptr);
	create_socket(dataptr);
}