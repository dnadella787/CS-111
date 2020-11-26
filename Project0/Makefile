#NAME: Dhanush Nadella
#EMAIL: dnadella787@gmail.com	
#ID: 205150583

default: lab0.c 
	gcc -Wall -Wextra lab0.c -o lab0 

check: lab0 
	./smoketest.sh

clean:
	rm -rf lab0 bob.txt john.txt input.txt output.txt lab0-205150583.tar.gz

dist:
	tar -czvf lab0-205150583.tar.gz lab0.c backtrace.png breakpoint.png Makefile smoketest.sh README

