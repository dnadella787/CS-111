#NAME: Dhanush Nadella
#UID: 205150583
#EMAIL: dnadella787@gmail.com


build: 
	gcc -pthread -Wall -Wextra lab2_add.c -o lab2_add 
	gcc -pthread -Wall -Wextra lab2_list.c SortedList.c SortedList.h -o lab2_list 

tests: build 
	rm -f lab2_add.csv lab2_list.csv 
	chmod +x test.sh 
	-./test.sh 2> /dev/null 

graphs: tests 
	gnuplot lab2_add.gp 2> /dev/null
	gnuplot lab2_list.gp 2> /dev/null 

dist: graphs 
	tar -czvf lab2a-205150583.tar.gz lab2_add.c SortedList.h SortedList.c lab2_list.c test.sh Makefile README *.csv *.png *.gp

clean:
	rm -f lab2_add lab2_list lab2a-205150583.tar.gz 



