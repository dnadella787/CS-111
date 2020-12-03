#NAME: Dhanush Nadella
#EMAIL: dnadella787@gmail.com
#UID: 205150583 

default:
	gcc -Wall -Wextra -lmraa -lm lab4b.c -o lab4b 

check: default 
	chmod +x test.sh 
	./test.sh 

clean:
	rm -f lab4b-205150583.tar.gz lab4b 

dist:
	tar -cvzf lab4b-205150583.tar.gz lab4b.c Makefile README test.sh 