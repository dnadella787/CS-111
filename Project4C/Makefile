#NAME: Dhanush Nadella
#EMAIL: dnadella787@gmail.com
#UID: 205150583

default:
	gcc -Wall -Wextra -lmraa -lm lab4c_tcp.c -o lab4c_tcp 
	gcc -Wall -Wextra -lmraa -lm -lssl -lcrypto lab4c_tls.c -o lab4c_tls 

clean:
	rm -f lab4c_tcp lab4c_tls lab4c-205150583.tar.gz 

dist:
	tar -cvzf lab4c-205150583.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README


