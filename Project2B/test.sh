#!/usr/bin/bash 


#these two are for lab2_1.png 
for thread in 1 2 4 8 12 16 24; do
    ./lab2_list --iterations=1000 --thread=$thread --sync=m >> lab2b_list.csv 
done

for thread in 1 2 4 8 12 16 24; do 
    ./lab2_list --iterations=1000 --thread=$thread --sync=s >> lab2b_list.csv 
done 


#for lab2_2.png, the data was already there for this test so I removed it
#otherwise, the graphing script would produce multiple lines for a single
#type since there was more than one value for a specific thread amount
#for thread in 1 2 4 8 16 24; do 
#    ./lab2_list --iterations=1000 --threads=$thread --sync=m >> lab2b_list.csv 
#done 

#for lab2b_3.png
for iteration in 1 2 4 8 16; do 
    for thread in 1 4 8 12 16; do 
        ./lab2_list --iterations=$iteration --lists=4 --threads=$thread --yield=id >> lab2b_list.csv 
    done
done

for iteration in 10 20 40 80; do 
    for thread in 1 4 8 12 16; do 
        ./lab2_list --iterations=$iteration --lists=4 --threads=$thread --yield=id --sync=m >> lab2b_list.csv
        ./lab2_list --iterations=$iteration --lists=4 --threads=$thread --yield=id --sync=s >> lab2b_list.csv
    done
done

#for lab2b_4.png and lab2b_5.png
#I removed --list=1 because it was a repeat of the test in lab2_1.png basically
#and was causing repeat lines to show for lab2_1.png
for list in 4 8 16; do 
    for thread in 1 2 4 8 12; do 
        ./lab2_list --iterations=1000 --lists=$list --threads=$thread --sync=m >> lab2b_list.csv
        ./lab2_list --iterations=1000 --lists=$list --threads=$thread --sync=s >> lab2b_list.csv
    done 
done 