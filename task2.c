#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int fd = open(argv[1], O_WRONLY|O_TRUNC|O_CREAT);
	if (fd == -1) {
		char msg[100] = "FILE ERROR\n";
		printf("%s", msg);
		return 0;
	}
	while (1) {
		char buf;
		int cod = scanf("%c", &buf);
		if (cod == -1) break;
		if (buf != 0) write(fd, &buf, 1);
		else lseek(fd, 1, SEEK_CUR);
	}
	close(fd);
}