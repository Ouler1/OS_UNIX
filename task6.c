#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Получение имени файла .lck
char* get_lck (char* filename) {
	char* extension = ".lck";
	char* lck = (char*)malloc(sizeof(char) * (strlen(filename) + strlen(extension) + 1));
	if(lck == NULL) {
		printf("Allocate error\n");
		exit(-1);
	}
	strcat(lck, filename);
	strcat(lck, extension);
	return lck;
}

//Запись в файл .lck информации о блокировке
void write_perm(char* lock_file) {
	FILE* lock = fopen(lock_file, "w");
    fprintf(lock, "%d\n", getpid());
    fprintf(lock, "%s\n", "w");
    fclose(lock);
}

//Получение права записи
void acquire(char* lck) {
	FILE* lock = fopen(lck, "r");
    while (lock != NULL) {
        fclose(lock);
        sleep(1);
		lock = fopen(lck, "r");
    }
	write_perm(lck);
}

//Отпускаем блокировку
void release(char* lck) {
	remove(lck);
}

//Строка запуска: ./programm namefile [write|read]
int main (int argc, char* argv[]) {
	//Проверка аргументов
	if (argc != 3) {
		printf("Argument error (need 2)\n");
		exit(-1);
	}
	if (strcmp(argv[2], "read") != 0 && strcmp(argv[2], "write") != 0) {
		printf("Argument №2 should be 'read' or 'write'\n");
		exit(-1);
	}
	//Получение имени файла блокировки
	char* file_lck = get_lck(argv[1]);
	//Выделение памяти для команды ОС
	char* command = (char*)malloc(sizeof(char) * (strlen(argv[1]) + 14));
	if (command == NULL) {
		printf("Allocate error\n");
		exit(-1);
	}
	//Если открываем файл на чтение, то не блокируем
	if (strcmp(argv[2], "read") == 0) {
		strcat(command, "cat ");
		strcat(command, argv[1]);
		system(command);
	}
	else {
		//Если на запись, то блокируем, пока команда nano не будет выполнена.
		acquire(file_lck);
		strcat(command, "nano ");
		strcat(command, argv[1]);
		system(command);
		release(file_lck);
	}
	return 0;
}