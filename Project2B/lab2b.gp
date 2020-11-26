#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#    8. avg wait time for lock (ns)
#
# output:
#    1. lab2_1.png
#    2. lab2_2.png 
#    3. lab2_3.png
#    4. lab2_4.png 
#    5. lab2_5.png 
#

set terminal png
set datafile separator ","


#lab2b_1.png 
set title "operations/sec for sychronization over different thread counts"
set xlabel "threads"
set logscale x 2
set ylabel "operations per second"
set logscale y 10 
set output "lab2b_1.png"

#grep only the non yield with synchronization options m or s for any number of threads and
#1000 iterations with only one list.
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'spin lock' with linespoints lc rgb 'green'


#lab2b_2.png
set title "mutex wait for lock and avg time per operation vs competing threads"
set xlabel "threads"
set logscale x 2 
set ylabel "time"
set logscale y 10 
set output "lab2b_2.png"


plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'average wait for lock' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'average time per operation' with linespoints lc rgb 'green'

#lab2b_3.png 
set title "protected vs unprotected runs without failure"
set xlabel "threads"
set logscale x 2 
set ylabel "iterations"
set logscale y 10 
set output "lab2b_3.png"

plot \
    "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using (2):($3) \
	with points lc rgb "red" title "unprotected", \
    "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "green" title "mutex", \
     "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "green" title "spin lock", \


#lab2b_4.png 
set title "aggregate throughput vs number of threads (mutex)"
set xlabel "threads"
set logscale x 2
set ylabel "throughput"
set logscale y 10
set output "lab2b_4.png"

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/$7) \
	with linespoints lc rgb "red" title "1 list", \
     "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/$7) \
	with linespoints lc rgb "green" title "4 lists", \
     "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/$7) \
	with linespoints lc rgb "blue" title "8 lists", \
     "< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/$7) \
	with linespoints lc rgb "yellow" title "16 lists", \
  
#lab2b_5.png 
set title "aggregate throughput vs number of threads (spin lock)"
set xlabel "threads"
set logscale x 2
set ylabel "throughput"
set logscale y 10
set output "lab2b_5.png"

plot \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/$7) \
	with linespoints lc rgb "red" title "1 list", \
     "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/$7) \
	with linespoints lc rgb "green" title "4 lists", \
     "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/$7) \
	with linespoints lc rgb "blue" title "8 lists", \
     "< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/$7) \
	with linespoints lc rgb "yellow" title "16 lists"