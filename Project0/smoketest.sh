#!/bin/bash

#NAME: Dhanush Nadella
#EMAIL: dnadella787@gmal.com
#ID: 205150583

#1A
./lab0 --bob 2> /dev/null

if [ $? == 1 ]; then
    echo "Case 1A passed: alone invalid argument caught"
else 
    echo "Case 1A failed: alone invalid argument not caught"
fi


#1B
./lab0 --bob --output=john.txt 2> /dev/null 

if [ $? == 1 ]; then
    echo "Case 1B passed: invalid argument amongst correct ones caught"
else   
    echo "Case 1B failed: invalid argument amongst correct ones not caught"
fi

#2A
./lab0 --input=bob.txt 2> /dev/null

if [ $? == 2 ]; then
    echo "Case 2A passed: alone invalid input file caught"
else   
    echo "Case 2A failed: alone invalid input file not caught"
fi

#2B
./lab0 --output=john.txt --input=bob --catch 2> /dev/null 

if [ $? == 2 ]; then
    echo "Case 2B passed: invalid input file amongst correct flags caught"
else   
    echo "Case 2B failed: invalid input file amongst correct not caught"
fi

#3A
touch bob.txt
chmod -w bob.txt 

./lab0 --output=bob.txt 2> /dev/null 

if [ $? == 3 ]; then
    echo "Case 3A passed: alone invalid output file caught"
else   
    echo "Case 3A failed: alone invalid output file not caught"
fi

#3B
./lab0 --input=john.txt --output=bob.txt 2> /dev/null 

if [ $? == 3 ]; then
    echo "Case 3A passed: invalid output file amongst correct flags caught"
else   
    echo "Case 3A failed: invalid output file amongst correct flags not caught"
fi

#4A
./lab0 --segfault --catch 2> /dev/null

if [ $? == 4 ]; then
    echo "Case 4A passed: segmentation fault caught"
else
    echo "Case 4A failed: segmentation fault not caught"
fi

#4B
./lab0 --catch --segfault 2> /dev/null 

if [ $? == 4 ]; then 
    echo "Case 4B passed: segmentation fault caught"
else
    echo "Case 4B failed: segmentation fault not caught"
fi 

#5A
touch input.txt
touch output.txt 
echo "Correct test case for Project 0" > input.txt 

./lab0 --input=input.txt --output=output.txt 

if [ $? == 0 ]; then
    echo "Case 5A passed: proper return code for no errors"
else
    echo "Case 5A failed: improper return code for no errors"
fi 

#5B

cmp -s input.txt output.txt 

if [ $? == 0 ]; then
    echo "Case 5B passed: input and output files are the same"
else
    echo "Case 5B failed: input and output files are not the same"
fi 

#6A
./lab0 --input=charlie.txt --output=bob.txt 2> /dev/null

if [ $? == 2 ]; then
    echo "Case 6A passed: input error code returned when two errors present"
else
    echo "Case 6A failed: input error code not returned when two errors present"
fi

#6B
./lab0 --output=bob.txt --input=charlie.txt 2> /dev/null 

if [ $? == 3 ]; then
    echo "Case 6B passed: output error code returned when two errors present"
else
    echo "Case 6B failed: output error code not returned when two errors present"
fi

