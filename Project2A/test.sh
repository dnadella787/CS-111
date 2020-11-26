#!/usr/bin/bash

#NAME: Dhanush Nadella
#UID: 205150583
#EMAIL: dnadella787@gmail.com

#research:

#bash for loops (my bash is rusty)
#https://linuxhint.com/bash_loop_list_strings/

#TESTS FOR PART 1:

#first test in part 1, iterations over 100, 1000, 10000, 100000
#basically for answering questiono 2.1.1
for thread in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do
    for iteration in 100 1000 10000 100000; do 
        ./lab2_add --threads=$thread --iterations=$iteration >> lab2_add.csv
    done
done 


#second test in part 1, (2, 4, 8, 12) threads and 
#(10, 20, 40, 80, 100, 1000, 10000, 100000) iterations
#for lab2_add-1.png
for thread in 2 4 8 12; do 
    for iteration in 10 20 40 80 100 1000 10000 100000; do
        ./lab2_add --yield --threads=$thread --iterations=$iteration >> lab2_add.csv 
    done 
done 


#third test in part 1, (2, 8) threads and 
#(100, 1000, 10000, 100000) iterations
#for lab2_add-2.png
for thread in 2 8; do
    for iteration in 100 1000 10000 100000; do 
        ./lab2_add --yield --threads=$thread --iterations=$iteration >> lab2_add.csv
        ./lab2_add --threads=$thread --iterations=$iteration >> lab2_add.csv 
    done 
done 


#fourth test in part 1, avg cost per operation vs number of iterations
#non yield for 1, 10, 100, 1000, 10000, 100000 on single thread
#for lab2_add-3.png
for iteration in 1 10 100 1000 10000 100000; do 
    ./lab2_add --iterations=$iteration >> lab2_add.csv 
done 


#fifth test in part 1, (2 4 8 12) threads and (10000/1000) iterations
#to show the mechanisms provide protection 
#for lab2_add-4.png
for thread in 2 4 8 12; do 
    ./lab2_add --yield --threads=$thread --iterations=10000 --sync=m >> lab2_add.csv 
    ./lab2_add --yield --threads=$thread --iterations=10000 --sync=c >> lab2_add.csv 
done 

for thread in 2 4 8 12; do
    ./lab2_add --yield --threads=$thread --iterations=1000 --sync=s >> lab2_add.csv 
done 


#last test in part 1, use 10000 iterations with (1 2 4 8 12) threads
#on unprotected, and all the mechanisms for synchronization
#for lab2_add-5.png
for thread in 1 2 4 8 12; do 
    ./lab2_add --threads=$thread --iterations=10000 >> lab2_add.csv 
    ./lab2_add --threads=$thread --iterations=10000 --sync=m >> lab2_add.csv 
    ./lab2_add --threads=$thread --iterations=10000 --sync=c >> lab2_add.csv 
    ./lab2_add --threads=$thread --iterations=10000 --sync=s >> lab2_add.csv
done 

#TESTS FOR PART 2:

#tests 1 thread over multiple iterations without yield or synchronization
#for lab2_list-1.png
for iteration in 10 100 1000 10000 20000; do
    ./lab2_list --threads=1 --iterations=$iteration >> lab2_list.csv 
done 


#test over multiple threads and iterations this time
#also test against yield options and multiple threads and iterations
#for lab2_list-2.png
for thread in 2 4 8 12; do
    for iteration in 1 10 100 1000; do 
        ./lab2_list --threads=$thread --iterations=$iteration >> lab2_list.csv 
    done 
done 



for thread in 2 4 8 12; do
    for iteration in 1 2 4 8 16 32; do 
        for yield in i d l id il dl idl; do 
            ./lab2_list --threads=$thread --iterations=$iteration --yield=$yield >> lab2_list.csv 
        done
    done 
done 



#test synchronnization options over multiple yields and a moderate 
#amount of threads and iterations
#for lab2_list-3.png
for yield in i d l id il dl idl; do 
    ./lab2_list --threads=12 --iterations=32 --yield=$yield --sync=m >> lab2_list.csv 
    ./lab2_list --threads=12 --iterations=32 --yield=$yield --sync=s >> lab2_list.csv 
done


#test over multiple threads and larger amount of iterations and
#both sync options
#for lab2_list-4.png
for thread in 1 2 4 8 12 16 24; do 
    ./lab2_list --threads=$thread --iteration=1000 --sync=m >>lab2_list.csv 
    ./lab2_list --threads=$thread --iteration=1000 --sync=s >> lab2_list.csv 
done