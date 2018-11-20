#!/bin/bash

if [[ $# != 0 ]]; then
   echo "Call syntax: $0"
   exit
fi

echo Here all the regular files
for file in `ls`; do
   if [ -f $file ]; then
      echo $file
   fi
done


echo
echo

echo Here all directories
for file in `ls`; do
   if [ -d $file ]; then
      echo $file
   fi
done


