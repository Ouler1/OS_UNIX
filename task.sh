#!/bin/bash

# Выполняем, если произошел выход по исключению (CTRL-C, CTRL-Z, закрытие терминала и др.)
trap "" 20
trap "rm ./fifo1t; clear && echo -en '\e[3J'; exit 1" 1 2 3 8 9 14 15 23

# Команда очистки экрана
clear && echo -en "\e[3J"

# Подключение игроков. Первый игрок создатель FIFO, второй игрок подтверждает, что он запустил программу.
file="./fifo1t"
if [ -e "$file" ]
then
	user=2
	active=0
	echo "ready" > ./fifo1t
else
	mknod ./fifo1t p
	user=1
	active=1
	echo "Ожидание второго игрока..."
	cat ./fifo1t
fi

# Объявление начальных параметров игры (Поле, Флаг победы, Флаг пользовательской ошибки)
arraysym=( 0 0 0 0 0 0 0 0 0 )
win=0
errorgame=0

# Основной игровой цикл
while true
do
	# Очистка экрана и вывод того, кто есть кто.
	clear && echo -en "\e[3J"
	if [[ $user = 1 ]]
	then
		echo "Игрок 1. Символ X."
	else
		echo "Игрок 2. Символ O."
	fi
	
	# Вывод игрового поля
	echo " 123"
	i=0
	while [[ $i -lt 3 ]]
	do
		j=0
		s=""
		let "it = i + 1"
		s+="$it"
		while [[ $j -lt 3 ]]
		do	
			let "a = i + j * 3"
			if [[ ${arraysym[$a]} = "0" ]] ; then s+="#" ; fi
			if [[ ${arraysym[$a]} = "1" ]] ; then s+="X" ; fi
			if [[ ${arraysym[$a]} = "2" ]] ; then s+="O" ; fi
			let "j = j + 1"
		done
		echo "$s"
		let "i = i + 1"
	done

	# Проверка условий победы
	if [[ ${arraysym[0]} = 1 ]] && [[ ${arraysym[1]} = 1 ]] && [[ ${arraysym[2]} = 1 ]] ; then win=1 ; fi
	if [[ ${arraysym[3]} = 1 ]] && [[ ${arraysym[4]} = 1 ]] && [[ ${arraysym[5]} = 1 ]] ; then win=1 ; fi
	if [[ ${arraysym[6]} = 1 ]] && [[ ${arraysym[7]} = 1 ]] && [[ ${arraysym[8]} = 1 ]] ; then win=1 ; fi
	if [[ ${arraysym[0]} = 1 ]] && [[ ${arraysym[3]} = 1 ]] && [[ ${arraysym[6]} = 1 ]] ; then win=1 ; fi
	if [[ ${arraysym[1]} = 1 ]] && [[ ${arraysym[4]} = 1 ]] && [[ ${arraysym[7]} = 1 ]] ; then win=1 ; fi
	if [[ ${arraysym[2]} = 1 ]] && [[ ${arraysym[5]} = 1 ]] && [[ ${arraysym[8]} = 1 ]] ; then win=1 ; fi
	if [[ ${arraysym[0]} = 1 ]] && [[ ${arraysym[4]} = 1 ]] && [[ ${arraysym[8]} = 1 ]] ; then win=1 ; fi
	if [[ ${arraysym[2]} = 1 ]] && [[ ${arraysym[4]} = 1 ]] && [[ ${arraysym[6]} = 1 ]] ; then win=1 ; fi

	if [[ ${arraysym[0]} = 2 ]] && [[ ${arraysym[1]} = 2 ]] && [[ ${arraysym[2]} = 2 ]] ; then win=2 ; fi
	if [[ ${arraysym[3]} = 2 ]] && [[ ${arraysym[4]} = 2 ]] && [[ ${arraysym[5]} = 2 ]] ; then win=2 ; fi
	if [[ ${arraysym[6]} = 2 ]] && [[ ${arraysym[7]} = 2 ]] && [[ ${arraysym[8]} = 2 ]] ; then win=2 ; fi
	if [[ ${arraysym[0]} = 2 ]] && [[ ${arraysym[3]} = 2 ]] && [[ ${arraysym[6]} = 2 ]] ; then win=2 ; fi
	if [[ ${arraysym[1]} = 2 ]] && [[ ${arraysym[4]} = 2 ]] && [[ ${arraysym[7]} = 2 ]] ; then win=2 ; fi
	if [[ ${arraysym[2]} = 2 ]] && [[ ${arraysym[5]} = 2 ]] && [[ ${arraysym[8]} = 2 ]] ; then win=2 ; fi
	if [[ ${arraysym[0]} = 2 ]] && [[ ${arraysym[4]} = 2 ]] && [[ ${arraysym[8]} = 2 ]] ; then win=2 ; fi
	if [[ ${arraysym[2]} = 2 ]] && [[ ${arraysym[4]} = 2 ]] && [[ ${arraysym[6]} = 2 ]] ; then win=2 ; fi
	
	# Если победитель есть, то выводим информацию об этом и выходим
	if [[ $win = 1 ]]
	then
		echo "Победил игрок 1!!!"
		echo "Нажмите Enter, чтобы выйти."
		read a
		rm ./fifo1t
		clear && echo -en "\e[3J"
		exit 0
	fi
	if [[ $win = 2 ]]
	then
		echo "Победил игрок 2!!!"
		echo "Нажмите Enter, чтобы выйти."
		read a
		rm ./fifo1t
		clear && echo -en "\e[3J"
		exit 0
	fi

	# Ход игрока
	if [[ $active = "1" ]]
	then
		# Сообщение о пользовательской ошибке
		if [[ $errorgame = 1 ]] ; then echo "Ошибка. Клетка уже заполнена." ; errorgame=0 ; fi
		if [[ $errorgame = 2 ]] ; then echo "Ошибка. Некорректный выбор." ; errorgame=0 ; fi
		echo "Ваш ход: (Формат: СтолбецСтрока, Выход: CTRL+C)"
		# Ожидание ввода
		trap "echo 'exit' > ./fifo1t ; rm ./fifo1t; clear && echo -en '\e[3J'; exit 1" 1 2 3 8 9 14 15 23
		read step
		trap "rm ./fifo1t; clear && echo -en '\e[3J'; exit 1" 1 2 3 8 9 14 15 23
		# Если другой игрок вышел.
		if ! [[ -e "$file" ]]
		then
			echo "Оппонент вышел. Нажмите Enter."
			read a
			clear && echo -en "\e[3J"
			exit 0
		fi
		# Проверка ввода. Если неверный все перерисовываем.
		lenst=${#step}
		re="^[1-3]+$"
		if [[ $lenst = 2 ]] && [[ $step =~ $re ]] 
		then 
			let "index = (${step:0:1} - 1) * 3 + (${step:1:1} - 1)"
			if [[  ${arraysym[$index]} != 0 ]] 
			then
				errorgame=1
				continue 
			fi
			
			# Если ввод верный, то отправляем второму игроку
			echo $step > ./fifo1t
			# Обновляем игровое поле
			if [[ $user = "1" ]]
			then
				arraysym[$index]=1
			else
				arraysym[$index]=2
			fi
			active=0
		else
			errorgame=2
			continue
		fi
	else
		# Второй игрок ждет окончания хода первого игрока.
		echo ""
		echo "Ход оппонента... (Выход: CTRL+C)"
		step=$(cat ./fifo1t)
		
		# Если другой игрок вышел.
		if [[ $step = "exit" ]]
		then
			echo "Оппонент вышел. Нажмите Enter."
			read a
			clear && echo -en "\e[3J"
			exit 0
		fi
		# Обновляем игровое поле
		let "index = (${step:0:1} - 1) * 3 + (${step:1:1} - 1)"
		if [[ $user = "1" ]]
		then
			arraysym[$index]=2
		else
			arraysym[$index]=1
		fi
		active=1
	fi
done