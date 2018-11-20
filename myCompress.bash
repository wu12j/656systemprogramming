#!/bin/bash
#Name: Jingchao Wu
#ID: 103659497

#
#Objective:This bash script that takes a list of extensions and compresses all files with these extension.
#

if [ $# == 0 ]; then
	echo "myCompress [-t] target-directory extension-list"
	exit
fi

for k in $*;do
array=(${array[@]} $k)  #store input into array
done

if [ $1 != "-t" ]&& [[ $# > 0 ]];then   #case 1
	for i in ${array[@]:0:$#};do 
		for j in `ls *.$i`;do
			bzip2 -k $j
		done
		
		echo compressed $i files 
         
    done


elif [ $1 == "-t" ]&& [[ $# > 1 ]];then  #case 2
	if [ -d $2 ];then
		#curDir=`pwd`
		cd $2
		for m in ${array[@]:2:$#-2};do 
			for n in `ls *.$m`;do
				bzip2 -k $n
				#mv $n.bz2 $curDir #move compressed file to current directory
			done
		
			echo compressed $m files 
         
        done
	fi
fi
