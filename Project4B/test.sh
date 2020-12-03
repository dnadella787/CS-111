#!/bin/bash 

#NAME: Dhanush Nadella
#EMAIL: dnadella787@gmail.com
#UID: 205150583

#how to send multiple inputs to a program on CLI:
#https://unix.stackexchange.com/questions/427360/how-send-multiple-command-as-input-in-a-program

{ echo "SCALE=F"; sleep 3; echo "SCALE=C"; sleep 3; echo "PERIOD=2"; sleep 3; echo "OFF"; } | ./lab4b --log=LOGFILE --period=1 --scale=C > /dev/null 

if [ $? -ne 0 ]; then
    echo "error in program, exit code is non zero"
fi 

for command in SCALE=F SCALE=C PERIOD=2; do 
    grep "$command" LOGFILE > /dev/null 

    if [ $? -ne 0 ]; then
        echo "$command not found in LOGFILE"
    fi 
done

{ sleep 3; echo "STOP"; sleep 3; echo "START"; sleep 3; echo "LOG lineoftext"; sleep 3; echo "OFF"; } | ./lab4b --log=LOGFILE --period=1 --scale=C > /dev/null 

if [ $? -ne 0 ]; then
    echo "error in program, exit code is non zero"
fi 

for command in STOP START "LOG lineoftext" OFF; do 
    grep "$command" LOGFILE > /dev/null 

    if [ $? -ne 0 ]; then
        echo "$command not found in LOGFILE"
    fi 
done


./lab4b --bogusflag > /dev/null 
if [ $? -ne 1 ]; then 
    echo "wrong command line argument not caught"
fi 

./lab4b --scale=D > /dev/null 
if [ $? -ne 1 ]; then 
    echo "wrong command line argument not caught"
fi 

egrep '[0-9][0-9]:[0-9][0-9]:[0-9][0-9] [0-9]+\.[0-9]\>' LOGFILE > /dev/null 
if [ $? -ne 0 ]; then
	echo "incorrect format"
else
    echo "all tests passed"
fi 


rm -f LOGFILE


