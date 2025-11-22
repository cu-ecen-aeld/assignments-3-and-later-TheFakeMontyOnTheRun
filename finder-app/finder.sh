#!/bin/sh

if [ ! "$1" ]; then
   echo "Directory not specified"
   exit 1
fi

if [ ! -d "$1" ]; then
   echo "$1 Does not exist"
   exit 1
fi

if [ ! "$2" ]; then
   echo "String not specified"
   exit 1
fi   

total="$(find "$1" | wc -l)"
matched="$(grep -rI "$2" "$1" | wc -l)"


echo "The number of files are $((total - 1)) and the number of matching lines are $matched"

