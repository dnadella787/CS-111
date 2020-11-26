#NAME: Dhanush Nadella
#UID: 205150583
#EMAIL: dnadella787@gmail.com

#For "make profile," I kept getting the error:
# usr/bin/ld: cannot find -lprofiler 
#so I had to manually link it to my program using -L
#this thread was very useful (no pun intended):
#https://stackoverflow.com/questions/16710047/usr-bin-ld-cannot-find-lnameofthelibrary/34256630

default:
	gcc -pthread -Wall -Wextra lab2_list.c SortedList.c SortedList.h -o lab2_list 

tests: default 
	rm -f lab2b_list.csv 
	chmod +x test.sh 
	-./test.sh 2> /dev/null

LOCATION=/u/eng/class/classdna/111Assignments/Project2B/gperftools-2.8/lib
profile:
	rm -f raw.gperf
	gcc -g -pthread -L$(LOCATION) -lprofiler lab2_list.c SortedList.c SortedList.h -o lab2_list 
	LD_PRELOAD=$(LOCATION)/libprofiler.so.0 CPUPROFILE=raw.gperf ./lab2_list --iterations=10000 --threads=12 --sync=s
	pprof --text lab2_list raw.gperf > profile.out
	pprof --list=start_routine lab2_list raw.gperf >> profile.out 
	rm -f raw.gperf lab2_list 

graphs: tests 
	gnuplot lab2b.gp 2> /dev/null 

dist: graphs 
	tar -czvf lab2b-205150583.tar.gz SortedList.h SortedList.c lab2_list.c test.sh Makefile README lab2b_list.csv *.png lab2b.gp profile.out

clean:
	rm -f lab2b-205150583.tar.gz lab2_list 
