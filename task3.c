#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

//Функция для добавления нового значения в массив
long double * add(long double * ldm, int size, char * num) {
	//Конвертирование строки в число
	long double ld = strtold(num, 0);
	//Проверка результата конвертирования, если переполнение или ошибка конвертирования,
	//то выходим и продолжаем поиск чисел
	if (ld == HUGE_VAL || ld == HUGE_VALL || ld == HUGE_VALF || ld == 0) {
		printf("Error number");
		errno = 100001;
		return ldm;
	}
	//Выделение памяти
	ldm = (long double*)realloc(ldm, (size + 1) * sizeof(long double));
	//Если выделить не получилось, то завершаем работу
	if(ldm == NULL) {
		printf("Reallocate error\n");
		errno = 100002;
		return ldm;
	}
	//Присвоение значения
	ldm[size] = ld;
	errno = 0;
	return ldm;
}

//Выводим числа в файл
int print_arr(char* name, long double A[], long long int n)
{
	long long int i;
	FILE *fpw;
	fpw = fopen(name, "wt");
	//Проверка на возможность открытия файла
	if (fpw == NULL) {
		printf("Cannot open %s for write\n", name);
		return -1;
	}
	//Печать в файл
	for (i = 0; i < n; i++)
		fprintf(fpw, "%Lf\n", A[i]);
	fclose(fpw);
	return 0;
}

//Функция сравнения
int compare(const void *a, const void *b)
{
    const long double fa = *(const long double *) a;
    const long double fb = *(const long double *) b;
    if (fa > fb) return 1;
    if (fa < fb) return -1;
    return 0;
}

int main (int argc, char * argv[]) {
	//Должно быть минимум два аргумента(откуда читаем, куда пишем)
	if(argc < 3) {
		printf("Argument error\n");
		exit(-1);
	}
	//Динамический массив
	long double * ldm = NULL;
	int size = 0;
	
	int i;
	int k;
	FILE *fp;
	int cod;
	for(k = 1; k < argc - 1; k++) {
		//Открываем на чтение
		fp = fopen(argv[k], "rt");
		//Проверка на возможность открытия на чтение, если
		//не открывается, от читаем следующий
		if (fp == NULL) {
			printf("Cannot open %s\n", argv[k]);
			continue;
		}

		//Автомат для поиска чисел в файле
		int a[7][5];
		a[0][0] = 1;
		a[0][1] = 2;
		a[0][2] = 3;
		a[0][3] = 0;
		a[0][4] = 0;
		a[1][0] = 1;
		a[1][1] = 1;
		a[1][2] = 5;
		a[1][3] = 4;
		a[1][4] = 5;
		a[2][0] = 5;
		a[2][1] = 5;
		a[2][2] = 5;
		a[2][3] = 4;
		a[2][4] = 0;
		a[3][0] = 1;
		a[3][1] = 2;
		a[3][2] = 0;
		a[3][3] = 0;
		a[3][4] = 0;
		a[4][0] = 6;
		a[4][1] = 6;
		a[4][2] = 0;
		a[4][3] = 0;
		a[4][4] = 7;
		a[6][0] = 6;
		a[6][1] = 6;
		a[6][2] = 5;
		a[6][3] = 5;
		a[6][4] = 5;
		int c_sh;
		int c_st = 0;
		int max = 257;
		int c = 0;
		long double ld;
		
		//Выделение памяти для буферов
		char* num = malloc(257);
		char* buf = malloc(1024);
		//Если выделить не получилось, выходим
		if (buf == NULL || num == NULL) {
			printf("Allocate error\n");
			fclose(fp);
			exit(-1);
		}
		
		
		int is_conculate = 1;
		while (1) {
			i = 0;
			int len;
			int fs = fscanf(fp, "%1024s%n", buf, &len);
			if(fs == EOF) break;
			while(i < len){
				//Если число слишком большое, то читаем файл до тех пор, пока
				//не встретим новое
				if(c == max - 1) {
					is_conculate = 0;
					c = 0;
					continue;
				}
				//Переходы между состояниями автомата
				if('1' <= buf[i] && buf[i] <= '9') c_sh = 0;
				else if(buf[i] == '0') c_sh = 1;
				else if(buf[i] == '-') c_sh = 2;
				else if(buf[i] == '.') c_sh = 3;
				else c_sh = 4;
				c_st = a[c_st][c_sh];
				if(c_st == 0) {
					c = 0;
					is_conculate = 1;
				}
				//Если число найдено и оно не большое, пишем в массив
				if(is_conculate == 1) {
					if(c_st == 7) {
						num[c - 1] = 0;
						ldm = add(ldm, size, num);
						//Если возникла ошибка при выделений памяти, выходим
						if(errno == 100002) {
							fclose(fp);
							exit(-1);
						}
						else if (errno == 0) {
							size = size + 1;
						}
						i = i - 1;
						c_st = 0;
						c = 0;
					}
					else if(c_st == 5) {
						num[c] = 0;
						ldm = add(ldm, size, num);
						if(errno == 100002) {
							fclose(fp);
							exit(-1);
						}
						else if (errno == 0) {
							size = size + 1;
						}
						i = i - 1;
						c_st = 0;
						c = 0;
					}
					else if(c_st != 0) {
						//Пишем цифры, минус, точку в буффер
						num[c] = buf[i];
						c = c + 1;
					}
				}
				i = i + 1;
			}
			if(len != 1024) {
				c_st = a[c_st][4];
				if(is_conculate == 1) {
					if(c_st == 0) {
						c = 0;
						is_conculate = 1;
					}	
					if(c_st == 7) {
						num[c - 1] = 0;
						ldm = add(ldm, size, num);
						//Если возникла ошибка при выделений памяти, выходим
						if(errno == 100002) {
							fclose(fp);
							exit(-1);
						}
						else if (errno == 0) {
							size = size + 1;
						}
						i = i - 1;
						c_st = 0;
						c = 0;
					}
					else if(c_st == 5) {
						num[c] = 0;
						ldm = add(ldm, size, num);
						if(errno == 100002) {
							fclose(fp);
							exit(-1);
						}
						else if (errno == 0) {
							size = size + 1;
						}
						i = i - 1;
						c_st = 0;
						c = 0;
					}
					else if(c_st != 0) {
						//Пишем цифры, минус, точку в буффер
						num[c] = buf[i];
						c = c + 1;
					}
				}
			}
		}
		//Проверка числа в конце файла
		c_st = a[c_st][4];
		if(is_conculate == 1) {
			if(c_st == 7) {
				num[c - 1] = 0;
				ldm = add(ldm, size, num);
				if(errno == 100002) {
					fclose(fp);
					exit(-1);
				}
				else if (errno == 0) {
					size = size + 1;
				}
			}
			else if(c_st == 5) {
				num[c] = 0;
				ldm = add(ldm, size, num);
				if(errno == 100002) {
					fclose(fp);
					exit(-1);
				}
				else if (errno == 0) {
					size = size + 1;
				}
			}
		}
		free(buf);
		free(num);
		fclose(fp);
	}
	//Сортировка
	qsort(ldm, size, sizeof(long double), compare);
	//Печать в файл
	print_arr(argv[argc - 1], ldm, size);
}