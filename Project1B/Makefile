#NAME: Dhanush Nadella
#EMAIL: dnadella787@gmail.com	
#UID: 205150583

default:
	gcc -Wextra -Wall -lz lab1b-client.c -o lab1b-client 
	gcc -Wextra -Wall -lz lab1b-server.c -o lab1b-server 

client: 
	gcc -Wextra -Wall -lz lab1b-client.c -o lab1b-client 

server:
	gcc -Wextra -Wall -lz lab1b-server.c -o lab1b-server 

clean:
	rm -rf lab1b-client lab1b-server lab1b-205150583.tar.gz

dist:
	tar -czvf lab1b-205150583.tar.gz lab1b-client.c lab1b-server.c README Makefile 