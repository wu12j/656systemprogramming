//Author: Jingchao Wu
/*
Objective : this program is used to flip over the input ".pgm" image with header "P5".
System call is used to manipulate file.
*/
#include<unistd.h>
#include<fcntl.h>

int main( int argc, char *argv[]){
	int fd;
	long int i=0, filesize=0;
	char buf;
	fd=open("m1.pgm", O_RDWR,0755);
	lseek(fd, 15, SEEK_SET);   
	while(read(fd, &buf, 1)>0)  //read file and calculate filesize
         filesize++;
	
	char buffer[filesize];     //initiate array to store file data
	lseek(fd, 15, SEEK_SET);   
	
	while(++i <= filesize) { //store data to array
		read(fd, &buffer[i-1], 1);
	
}
	lseek(fd, 15, SEEK_SET);
    i=0;
	while(++i <= filesize) {           //write data from array start from end of file
		lseek(fd, -i, SEEK_END);
		write(fd, &buffer[i-1], 1);
	
}
	
	close(fd);
}