#!/bin/bash

####################
# Lab 1 Exercise 7
# Name: Tan Wei Xiang, Calvin
# Student No: A0217529Y
# Lab Group: 12
####################

####################
# Lab 1 Exercise 7
# Name: Dillon Tan Kiat Wee
# Student No: A0218033R
# Lab Group: 13
####################

####################
# Strings that you may need for prompt (not in order)
####################
# Enter $N numbers:
# Enter NEW prefix (only alphabets):
# Enter prefix (only alphabets):
# INVALID
# Number of files to create:
# Please enter a valid number [0-9]:
# Please enter a valid prefix [a-z A-Z]:

####################
# Strings that you may need for prompt (not in order)
####################

#################### Steps #################### 


# Fill in the code to request user for the prefix
echo "Enter prefix (only alphabets):"

while true
    do
        read prefix

    # Check the validity of the prefix #
    if [[ "${prefix}" =~ [^a-zA-Z] ]];
        then
            echo "INVALID"
            echo "Please enter a valid prefix [a-z A-Z]:"
        else
            break
    fi
done

# Enter numbers and create files #
echo "Number of files to create:"
while true
    do 
        read N
    if ! [[ $N =~ ^[0-9]+$ ]];
        then
            echo "INVALID"
            echo "Please enter a valid number [0-9]:"
        else
            break
    fi
done
    

# Fill in the code to request user for the new prefix
echo "Enter $N numbers:"
for ((i = 0; i <N; ))
    do
        read number

    if ! [[ $number =~ ^[0-9]+$ ]];

        then
            echo "INVALID"
            echo "Please enter a valid number [0-9]:"

        else
            underscore="_"
            dottxt=".txt"
            filename="$prefix$underscore$number$dottxt"
            touch $filename
            ((i++)) #only increament if valid

    fi
done

echo ""
echo "Files Created"
ls *.txt

echo ""

# Renaming to new prefix #
echo "Enter NEW prefix (only alphabets):"

while true
    do
        read new_prefix
    if [[ "${new_prefix}" =~ [^a-zA-Z] ]]
        then
            echo "INVALID"
            echo "Please enter a valid prefix [a-z A-Z]:"
        else
            break
    fi
done

for file in *.txt
    do
        mv "$file" "$new_prefix${file//$prefix/}"
done

echo ""
echo "Files Renamed"
ls *.txt
echo ""
