//Author:Jingchao Wu
//Date:6/17/2018
#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>

 


void parent(int *fd , char *command1, char *command2){
	 close(fd[1]); // close writing 
	 dup2(fd[0],0); // redirect the stdin
	 close(fd[0]); // close reading 
 
	 execlp(command1,command1,command2, NULL); // execute command
}

//one argument on the left of pipe command 
void child1(int *fd, char *command1, char *command2){
	close(fd[0]); // close reading 
	dup2(fd[1],1); // redirect stdout 
	close(fd[1]); // close writing 
	  	 
	execlp(command1,command1,NULL);  // execute command
}


//two argument on the left of pipe command 
void child2(int *fd, char *command1, char *command2){
	close(fd[0]); // close reading 
	dup2(fd[1],1); // redirect stdout
	close(fd[1]); // close writing 
	   
	execlp(command1,command1,command2,NULL);// execute command

}


int main(int argc, const char *argv[]){

	char buffer[50];  //array to store input
     
	while(1){
		int count = 0;
		int status;
	    printf("\n**************************************"); 
		printf("\nEnter commands, or enter \"q\" to quit.\n");
		printf("**************************************\n");  
		int n;
		n=read(0, buffer, 50);// read from stdin
		buffer[n]='\0';  
		 
		if(buffer[0]=='q'){//quit
			break;
		}
		
	    char *token = strtok (buffer, " \n"); //  STRING tokenizer 
	    char *command[5] = {NULL,NULL,NULL,NULL,NULL}; // array to store each command
	     	       
	    while (token != NULL)
	    { 
			command[count++] = token;
	        token = strtok (NULL, " \n");  //"enter" and "space" are tokenized
	    }

		 
		if( (command[1]==NULL)||(command[2]==NULL) ){ //no pipe in command line condition			
			if(fork() == 0)
				execlp(command[0],command[0],command[1],NULL);
			else
        		wait(&status);			
		}

		else{

			if(fork()== 0) {

				int fd[2]; 
				pipe(fd);

				if( (fork()) == 0){
      				if(strcmp("|", command[1])==0) // if the second command is pipe				   
      					child2(fd,command[0],command[0]);  // one argument is executed on the left side of the pipe
      				else							// if the third command is pipe	
      					child1(fd,command[0],command[1]); // two arguments are executed on the left side of the pipe 
      				
      			}else{
					if(strcmp("|", command[1])==0) // if the second command is pipe	
						parent(fd,command[2],command[3]); // third and forth arguments are executed after pipe
	     			else 								//// if the third command is pipe	
						parent(fd,command[3],command[4]); // forth and fifth arguments are executed after pipe					
   				}
     		}else
				wait(&status);
			
		}
	}


	return 0;
}
