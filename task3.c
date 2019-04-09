#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

//Структура для массива ссылок на строки
struct array_links {
	char** links;
	long int size;
};

//Выводим числа в файл
int print_arr(FILE* file, char** ls, long int size)
{
	long int i;
	//Печать в файл
	for (i = 0; i < size; i++)
		fprintf(file, "%s\n", ls[i]);
	return 0;
}

//Сравнение строк как чисел(собственно сравнение)
int compare_len_str(char* a, char* b) {
    if (strlen(a) < strlen(b))
        return -1;
    if (strlen(a) > strlen(b))
        return 1;
    return strcmp(a, b);
}

//Функция сравнения чисел как строк(получение строк по ссылкам,
//Разрешение сравнения между числами разных знаков)
int compare(const void* a, const void* b) {
    char* aec = *(char**)a;
    char* bec = *(char**)b;
    if (aec[0] == bec[0] && aec[0] == '-')
        return -compare_len_str(aec, bec);
    else if (aec[0] == '-') return -1;
    else if (bec[0] == '-') return 1;
    return compare_len_str(aec, bec);
}

//Создание автомата
int* create_KA() {
	int* a = (int *)malloc(sizeof(int) * 16);
	a[0] = 1;
	a[1] = 2;
	a[2] = 3;
	a[3] = 0;
	a[4] = 1;
	a[5] = 1;
	a[6] = 4;
	a[7] = 4;
	a[8] = 4;
	a[9] = 4;
	a[10] = 4;
	a[11] = 4;
	a[12] = 1;
	a[13] = 0;
	a[14] = 0;
	a[15] = 0;
	return a;
}

//Чтение файла в память
char * read_file(FILE* file) {
	size_t len;
	long int file_size;
	char* buf = NULL;
	//Определяем размер файла
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);
	//Выделяем память под прочтение файла
	buf = (char *)malloc(sizeof(char) * (file_size + 1));
	if(buf == NULL) {
		return buf;
	}
	len = fread(buf, sizeof(char), (size_t)file_size, file);
	buf[file_size] = '\0';
	//Если файл был прочитан не весь, то произошла ошибка
	if(len != file_size) {
		free(buf);
		buf = NULL;
	}
	return buf;
}

//Добывление строки в массив
char** add(char** ls, long int size, char* content, long int pos_con, long int len_num) {
	//Выделяем память для массива
	if(size == 0) {
		ls = (char**)malloc(1 * sizeof(char*));
	}
	else {
		ls = (char**)realloc(ls, (size + 1) * sizeof(char*));
	}
	//Если выделить не получилось, выдаем ошибку
	if(ls == NULL) {
		printf("Reallocate error\n");
		return NULL;
	}
	//Выделяем память для цифры
	char* substring = (char*)malloc(sizeof(char) * (len_num + 1));
	//Если выделить не получилось
	if(substring == NULL) {
		printf("Allocate error\n");
		return NULL;
	}
	//Кладем ссылку на строку в массив
	strncpy(substring, content + pos_con - len_num, len_num);
	substring[len_num] = '\0';
	ls[size] = substring;
	return ls;
}

//Вычисление нового состояния автомата
int shift_KA(int* ka, int c_st, char sym) {
	int c_sh;
	if('1' <= sym && sym <= '9') c_sh = 0;
	else if(sym == '0') c_sh = 1;
	else if(sym == '-') c_sh = 2;
	else c_sh = 3;
	return ka[c_st * 4 + c_sh];
}

int main (int argc, char * argv[]) {
	//Должно быть минимум два аргумента(откуда читаем, куда пишем)
	if(argc < 3) {
		printf("Argument error\n");
		exit(-1);
	}
	FILE *fpw;
	fpw = fopen(argv[argc - 1], "wt");
	//Проверка на возможность открытия файла
	if (fpw == NULL) {
		printf("Cannot open %s for write\n", argv[argc - 1]);
		return -1;
	}
	int k;
	FILE *fp;
	char* buf;
	
	char** ls;
	long int size = 0;
	
	//Автомат для поиска чисел в файле
	int* a = create_KA();
	for(k = 1; k < argc - 1; k++) {
		//Открываем на чтение
		fp = fopen(argv[k], "rt");
		//Проверка на возможность открытия на чтение, если
		//не открывается, то читаем следующий
		if (fp == NULL) {
			printf("Cannot open %s\n", argv[k]);
			continue;
		}
		buf = read_file(fp);
		//Если при прочтении произошла ошибка, то закрываем файл и
		//читаем следующий
		if(buf == NULL) {
			printf("Cannot read %s\n", argv[k]);
			fclose(fp);
			continue;
		}
		int c_st = 0;
		long int i = 0;
		long int c = 0;
		int cod;
		char* num;
		int while_end = 0;
		while(!while_end) {
			if(buf[i] == '\0') {
				while_end = 1;
			}
			//Переходы между состояниями автомата
			c_st = shift_KA(a, c_st, buf[i]);
			if(c_st == 4) {
				ls = add(ls, size, buf, i, c);
				//Если произошла ошибка при выделении памяти, то выходим
				if(ls == NULL) {
					fclose(fp);
					fclose(fpw);
					exit(-1);
				}
				size = size + 1;
				c = 0;
				i = i - 1;
			}
			else if(c_st == 0) {
				c = 0;
			}
			else {
				c = c + 1;
			}
			i = i + 1;
		}
		fclose(fp);
	}
	qsort(ls, size, sizeof(char *), compare);
	print_arr(fpw, ls, size);
	fclose(fpw);
}