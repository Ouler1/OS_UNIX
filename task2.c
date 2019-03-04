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
	char* buf = malloc(1024*4);
	while (1) {
		int len;
		//Считываем следующий блок символов из входного потока
		int cod = scanf("%1024s%n", buf, &len);
		//Если считывать нечего, выходим
		if (cod == -1) break;
		int nullsearch = 1;
		int i = 0;
		int c = 0;
		while(i < len) {
			//Если отслеживаем ненулевые байты и встретили нулевой, то записываем на диск прочитанную часть и начинаем отслеживать нулевую последовательность
			//Если отслеживали нулевую последовательность и встретили ненулевой байт, сдвигаемся оперцией lseek
			if(nullsearch == 1 && buf[c] == 0) {
				write(fd, buf, c);
				buf = &buf[c];
				nullsearch = 0;
				c = 0;
			}
			else if (nullsearch == 0 && buf[c] != 0) {
				lseek(fd, c, SEEK_CUR);
				buf = &buf[c];
				nullsearch = 1;
				c = 0;
			}
			i = i + 1;
			c = c + 1;
		}
		//Если вышли за предел блока, выталкиваем в диск прочитанную часть.
		if(nullsearch == 1) write(fd, buf, c);
		else lseek(fd, c, SEEK_CUR);
	}
	free(buf);
	close(fd);
}