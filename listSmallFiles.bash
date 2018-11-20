#!/bin/bash

if [[ $# != 1 ]]; then
   echo "Call syntax: $0 <size>"
   exit
fi

echo Here all the files with sizes smaller than $1

for file in `ls`; do
   if [ -f $file ]; then
      size=`cat $file | wc -c`
      if [[ $size -le $1 ]]; then
         echo $file  $size
      fi
   fi
done


