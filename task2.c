#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
	int fd = open(argv[1], O_WRONLY|O_TRUNC|O_CREAT);
	//Обработка ошибок с файлом
	if (fd == -1) {
		char msg[100] = "FILE ERROR\n";
		printf("%s", msg);
		return 0;
	}
	while (1) {
		char buf;
		//Считываем следующий символ из входного потока
		int cod = scanf("%c", &buf);
		//Если считывать нечего, выходим
		if (cod == -1) break;
		//Если не нулевой байт, пишем на диск, иначе пропускаем операйией lseek.
		if (buf != 0) write(fd, &buf, 1);
		else lseek(fd, 1, SEEK_CUR);
	}
	close(fd);
}