#!/bin/bash  

if [[ $# != 4 ]]; then
  echo "Synopsis: $0 inFile word replacement outFile"
  exit
fi

numLines=`more $1 | wc -l`
i=1

while [[ $i -le $numLines ]]; do
  for word in `head -$i $1 | tail -1`; do
    if [[ $word == $2 ]]; then
       word=$3
    fi
    echo -n "$word " >> $4
  done

  echo >> $4
  let "i = $i + 1"
done



