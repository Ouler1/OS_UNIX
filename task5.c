#include <stdio.h>
#include <syslog.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>

//Файл, где содержится конфигурация
#define CONFIG_FILE "daemon.conf"

//Дескриптор файла конфигурации
FILE* config_fd;
//Массив команд
char** list_commands;
//Массив опций ожидания
int* list_options;
//Массив PID-ов
int* list_pid;
//Размер массивов
int size;
//Количество незавершенных дочерних процессов
int count;

//Поиск последнего пробела в строке для отделения команды и опции ожидания
int get_separator(char* str_com) {
	int l = strlen(str_com);
	for(int i = l - 1; i >= 0; i--) {
		if(str_com[i] == ' ') {
			return i;
		}
	}
	return -1;
}

//Запуск команды
int start(char* command) {
	int retries = 0;
	int return_code = -1;
	int msec = 0;
	clock_t before = clock();
	while(1) {
		return_code = system(command);
		//Если завершилось хорошо, выходим
		if(return_code == 0) {
			return 0;
		}
		//Иначе ждем наступления условий выхода из команды с ошибкой
		retries = retries + 1;
		clock_t difference = clock() - before;
		msec = difference * 1000 / CLOCKS_PER_SEC;
		if(retries >= 50 && msec < 5000) {
			return -1;
		}
	}
}

//Запись PID в файл .pid
void add_pid(int index, int pid) {
	FILE* fs;
	char* filename_pid = (char*)malloc(sizeof(char) * 50);
	if(filename_pid == NULL) {
		syslog(LOG_ERR, "Allocate error");
		exit(-1);
	}
	sprintf(filename_pid, "%s%d%s", "/tmp/task5-", pid, ".pid");
	fs = fopen(filename_pid, "w");
	if(fs == NULL) {
		syslog(LOG_ERR, "Can't write file with pid");
		exit(-1);
	}
	fprintf(fs, "%d", pid);
	fclose(fs);
	free(filename_pid);
	list_pid[index] = pid;
}

//Запись команды и опции ожидания в массив
void add_com_and_opt(char* str_com) {
	int pos_space = get_separator(str_com);
	int opt;
	int len_str;
	//Проверка на ошибку с опцией ожидания
	if(strcmp(str_com + pos_space + 1, "wait\n") != 0 && strcmp(str_com + pos_space + 1, "respawn\n") != 0) {
		syslog(LOG_ERR, "Invalid option");
		return;
	}
	
	if(strcmp(str_com + pos_space + 1, "wait\n") == 0) opt = 0;
	else opt = 1;
	
	//Выделяем память в массивах под новую команду
	if(size == 0) {
		list_commands = (char**)malloc(1 * sizeof(char*));
		list_options = (int*)malloc(1 * sizeof(int));
		list_pid = (int*)malloc(1 * sizeof(int));
	}
	else {
		list_commands = (char**)realloc(list_commands, (size + 1) * sizeof(char*));
		list_options = (int*)realloc(list_options, (size + 1) * sizeof(int));
		list_pid = (int*)realloc(list_pid, (size + 1) * sizeof(int));
	}
	if(list_commands == NULL || list_options == NULL || list_pid == NULL) {
		syslog(LOG_ERR, "Allocate error");
		exit(-1);
	}
	
	//Выделяем память под команду
	char* substring = (char*)malloc(sizeof(char) * (pos_space + 1));
	if(substring == NULL) {
		syslog(LOG_ERR, "Allocate error");
		exit(-1);
	}
	
	//Кладем команду и опцию в массив
	strncpy(substring, str_com, pos_space + 1);
	substring[pos_space] = '\0';
	list_commands[size] = substring;
	list_options[size] = opt;
	size = size + 1;
}

//Чтение файла конфигурации
void read_config() {
	char* buf = malloc(4096);
	if(config_fd == NULL) {
		syslog(LOG_ERR, "Can't read config file");
		exit(-1);
	}
	rewind(config_fd);
	while(1) {
		int state = fgets(buf, 4096, config_fd);
		if(state == NULL) {
			if(feof(config_fd) != 0) {
				syslog(LOG_INFO, "End reading config file");
				break;
			}
			else {
				syslog(LOG_ERR, "Error reading config file");
				exit(-1);
			}
		}
		add_com_and_opt(buf);
	}
	free(buf);
}

//Создание дочернего процесса, который будет выполнять команду
void init_command(int index) {
	int pid = fork();
	char* command = list_commands[index];
	
	if(pid == -1) {
		syslog(LOG_ERR, "Error creating subprocess");
		exit(-1);
	}
	if(pid == 0) {
		//Потомок выполняет команду
		exit(start(command));
	}
	else {
		//Отец запоминает PID
		add_pid(index, pid);
		syslog(LOG_INFO, "Start command: '%s'; pid = %d", command, pid);
	}
}

//Запуск команд
void init_commands() {
	for(int i = 0; i < size; i++) {
		init_command(i);
	}
}

//Удаление файла дочернего процесса
void delete_pid(int index) {
	char* filename_pid = (char*)malloc(sizeof(char) * 50);
	if(filename_pid == NULL) {
		printf("Allocate error\n");
		exit(-1);
	}
	sprintf(filename_pid, "%s%d%s", "/tmp/task5-", list_pid[index], ".pid");
	remove(filename_pid);
}

//Принудительное завершение процессов при получений сигнала HUP 
void kill_all() {
	for (int i = 0; i < size; i++) {
		kill(list_pid[i], SIGKILL);
		delete_pid(i);
	}
}

//Обработчик сигнала HUP
void handler_hup(int sig_n) {
	signal(SIGHUP, handler_hup);
	syslog(LOG_INFO, "Received HUP. Reinitialization");
	kill_all();
	//Освобождение ресурсов и чтение конфигурационного файла заново.
	for(int i = 0; i < size; i++) {
		free(list_commands[i]);
	}
	free(list_commands);
	free(list_options);
	free(list_pid);
	size = 0;
	read_config();
	init_commands();
	count = size;
}

//Функция ожидания завершения дочерних процессов
void custom_wait() {
	count = size;
	int return_code;
	int pid;
	while(count) {
		pid = waitpid(-1, &return_code, 0);
		for(int i = 0; i < size; i++) {
			if(list_pid[i] == pid) {
				//Если wait, завершаем; если respawn, запускаем снова
				if(list_options[i] == 0) {
					delete_pid(i);
					if(return_code == 0) {
						syslog(LOG_INFO, "Completed command: '%s'; pid = %d", list_commands[i], pid);
					}
					else {
						syslog(LOG_ERR, "Failed command: '%s'; pid = %d", list_commands[i], pid);
					}
					count = count - 1;
				}
				else {
					delete_pid(i);
					if(return_code == 0) {
						syslog(LOG_INFO, "Completed command: '%s'; pid = %d", list_commands[i], pid);
						init_command(i);
					}
					else {
						count = count - 1;
						syslog(LOG_ERR, "Failed command: '%s'; pid = %d", list_commands[i], pid);
					}
				}
			}
		}
	}
}

int main (int argc, char* argv[]) {
	config_fd = fopen(CONFIG_FILE, "r");
	//"Скелет" демона
	int fd;
	struct rlimit flim;
	if(getppid() != 1) {
		signal(SIGTTOU, SIG_IGN);
		signal(SIGTTIN, SIG_IGN);
		signal(SIGTSTP, SIG_IGN);
		if(fork() != 0) exit(0);
		setsid();
	}
	getrlimit(RLIMIT_NOFILE, &flim);
	for(fd = 0; fd < flim.rlim_max; fd++) {
		close(fd);
	}
	chdir("/");
	openlog("T5", LOG_PID | LOG_CONS, LOG_DAEMON);
	syslog(LOG_INFO, "START DAEMON");
	openlog("t5daemon", LOG_CONS | LOG_NDELAY, LOG_LOCAL1);
	//Основные шаги: чтение конфига, запуск команд, ожидание завершения команд
	read_config();
	init_commands();
	signal(SIGHUP, handler_hup);
	custom_wait();
	syslog(LOG_INFO, "Exit");
	closelog();
	return 0;
}