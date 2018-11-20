//Author: Jingchao Wu
//        Mingyang Chen
//        The server connect to a client on another computer. It is able to execute commang sent from the client and send result to client to print out.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

void serviceClient (char  clAddr[], int sd, int client);

int main(int argc, char *argv[])
{
  	
  	int sd, client, portNumber; 
	int status;
	
  	struct sockaddr_in serv_addr, clt_addr;
  	socklen_t addrlen;
	

	char clAddr[INET6_ADDRSTRLEN]; // used to store ip address of the client
	

	//if port is invalid
  	if(argc != 2)
	{ 
    	printf("Call model: %s <Port Number>\n", argv[0]);
    	return 1;
  	} 
  	portNumber = atoi(argv[1]); //convert port number to int

  	sd = socket(AF_INET, SOCK_STREAM, 0);  //ipv4
  	if(sd < 0) {
		perror("can't open socket");
		exit(-1);
	}
  	printf("Creating socket...\n");

  	//clean buffer
  	memset(&serv_addr, 0, sizeof(serv_addr));
  	serv_addr.sin_family = AF_INET;  //socket in family IPV4;
  	serv_addr.sin_addr.s_addr = INADDR_ANY; //IP
  	serv_addr.sin_port = htons(portNumber); //host to network short  

  	if(bind(sd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("can't bind");
	exit(-1);
	}
		printf("bind socket to port %d...\n", portNumber);
		listen(sd, 5);  //this socket handles incoming requests

  		while(1)
		{
			printf("wait on port %d...\n", portNumber);
  			addrlen = sizeof(clt_addr); 
  			//client picks up that specific phone call:
			client = accept(sd, (struct sockaddr*)&clt_addr, &addrlen);

			if(client < 0){ //newsock is for that specific socket
				perror("can't accept");
				exit(-1);
			}
			//client IP
			void *clientIP;
			struct in_addr ip = clt_addr.sin_addr;
			clientIP = &ip.s_addr;
			
			inet_ntop(AF_INET, clientIP, clAddr, sizeof(clAddr));
			printf("\nIP %s connected ", clAddr);					
			//fork:
			pid_t pid = fork();

			if(pid < 0) 
			{
				perror("Failed to fork...");
				exit(1);
			}
			if(pid == 0) //child
			{
				serviceClient (clAddr, sd, client);	// child process take care of the client in a separate function							
			}
			else{//parent
			    wait(&status);
			close(client); }//close socket to client
		}//while(1) loop
  	close(sd); //close socket
  	return 0;
}

void serviceClient (char  clAddr[], int sd, int client){

	printf("Handler assigned for client %s\n", clAddr);
				close(sd); //close the general (recepcionist) socket	

				char msgBuf[256];	//incoming massage buffer
				int b,fileSize;
  	            char FSizeBuf[256];
				//clean buffer
				memset(&msgBuf, 0, sizeof(msgBuf));
				do
				{
					memset(&msgBuf, 0, sizeof(msgBuf));		
					//read client message:
					printf("Connected to a client...\n");
  					
					//we receive on specific socket:		
					b = recv(client, msgBuf, sizeof(msgBuf), 0);
					
					//server blocks on receive (waiting) 	 
					if(b < 0){
					perror("Failed to receive from client"); 
					exit(-1);}
					else
						msgBuf[b] = '\0';
					
					printf("Server got message: %s\n", msgBuf);
					
					if(strcmp(msgBuf, "quit") == 0)
					{
						//send exit back to client
						send(client, msgBuf, sizeof(msgBuf), 0);

						printf("Terminating connection...\n");
						close(client);
						exit(0);
					}
		
					//user calls 'get file' 
					else if(msgBuf[0] == 'g' &&
						msgBuf[1] == 'e' &&
						msgBuf[2] == 't' &&
						msgBuf[3] == ' ')
					{
						printf("Client called get\n");
						
						//parse the string
						int j = 0;
						for(int i = 4; i <= strlen(msgBuf); i++)
						{
							msgBuf[j] = msgBuf[i];
							j++;
						}
                        //open file
						FILE* fp;
						fp = fopen(msgBuf, "rb");
						if(fp == NULL)
							printf("Error opening file.");

						printf("File opened successfully!\n");
							
                  
						//calculate file size:
						int file_size = 0;
						if(fseek(fp, 0, SEEK_END) != 0)
							printf("Error calculate file size\n");
						
						file_size = ftell(fp);
						rewind(fp);
						printf("File size: %d bytes\n", file_size);
						
						//pass file size to a buffer
						memset(&FSizeBuf, 0, sizeof(FSizeBuf));
						sprintf(FSizeBuf, "%d", file_size);
							
						//send file size:
						b = send(client, FSizeBuf, sizeof(FSizeBuf), 0);
						if(b < 0) //n < 0
							printf("Error sending file size.\n");
						
						//receive an ACK from client
						//give time for client to get file size
						b = recv(client, FSizeBuf, sizeof(FSizeBuf), 0);
						if(b < 0)
							printf("Error receiving handshake");

						//we create a byte array:
						char FilebyteArray[256];
						memset(&FilebyteArray, 0, sizeof(FilebyteArray));
							
						int bufferRead = 0;
						int bytesRemaining = file_size;
						
						//while there are still bytes to be sent:
						while(bytesRemaining != 0)
						{						 
								bufferRead = fread(FilebyteArray, 1, bytesRemaining, fp);
								bytesRemaining = bytesRemaining - bufferRead;
								b = send(client, FilebyteArray, 256, 0);
								if(b < 0)
									printf("Sending Error...\n");							 
					
						}
						printf("File sent!\n");
						//clean buffers
						memset(&msgBuf, 0, sizeof(msgBuf));
						memset(&FilebyteArray, 0, sizeof(FilebyteArray));
					}//end get file command
					//user calls 'put file', receive file from the client
					else if(msgBuf[0] == 'p' &&
						msgBuf[1] == 'u' &&
						msgBuf[2] == 't' &&
						msgBuf[3] == ' ')
					{
						printf("Client called put\n");
						
						//the client  got the file name
						b = send(client, msgBuf, sizeof(msgBuf), 0);
                                                if(b < 0)
                                                        printf("Error sending file ACK\n");

						//receive the FSizeBuf
						memset(&FSizeBuf, 0, sizeof(FSizeBuf));
						b = recv(client, FSizeBuf, sizeof(FSizeBuf), 0);
                                                if(b < 0)
                                                        printf("Error receiving file size\n");   
						printf("size should be: %s\n", FSizeBuf);

						//send an ACK for file size
                            b = send(client, FSizeBuf, sizeof(FSizeBuf), 0);
                            if(b < 0)
                                printf("Error sending ACK for file size\n");

						//catch the file name
						char fileName[256];
						memset(&fileName, 0, sizeof(fileName));

						//parse
						int j = 0;
						for(int i = 4; i <= strlen(msgBuf); i++)
						{
							//pass to name buffer
							fileName[j] = msgBuf[i];
							j++;
						}
						fileSize = atoi(FSizeBuf);
						
						//print file name and size:
						printf("File: '%s' (%d bytes)\n", fileName, fileSize);

						//receive data
						memset(&msgBuf, 0, sizeof(msgBuf));
						int remainingData = 0;
						ssize_t len;
						
						FILE* fileprocessor;
						fileprocessor = fopen(fileName, "wb"); //overwrite if existing
										   //create if not
						remainingData = fileSize;
	
						
						while(remainingData != 0)
						{                                                    
                            len = recv(client, msgBuf, remainingData, 0);
                            fwrite(msgBuf, sizeof(char), len, fileprocessor);
                            remainingData -= len;
                            printf("Received %lu bytes\n", len);
						    break;
                                                        						
						}						
						fclose(fileprocessor);
                          b = recv(client, msgBuf, 256, 0); //receive packet.
                                                //clean buffer
						memset(&msgBuf, 0 , sizeof(msgBuf));
					}//end put 
					else//send message got from client
					{	
						b = send(client, msgBuf, sizeof(msgBuf), 0);
		
						if(b < 0){
							perror("Error sending to server");
							exit(-1);
						}
						printf("Send message...%s\n", msgBuf);
					}
					
					//clean buffer
					memset(&msgBuf, 0, sizeof(msgBuf));
				}while(strcmp(msgBuf, "quit") != 0); //close while loop here
	            exit(0);
}
