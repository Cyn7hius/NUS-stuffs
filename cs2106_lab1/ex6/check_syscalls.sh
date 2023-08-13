#!/bin/bash

####################
# Lab 1 Exercise 6
# Name: Tan Wei Xiang, Calvin
# Student No: A0217529Y
# Lab Group: 12
####################

####################
# Lab 1 Exercise 6
# Name: Dillon Tan Kiat Wee
# Student No: A0218033R
# Lab Group: 13
####################

echo "Printing system call report"

# Compile file
gcc -std=c99 pid_checker.c -o ex6

# Use strace to get report
strace -c ./ex6
