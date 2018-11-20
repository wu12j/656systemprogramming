//Author: Jingchao Wu
//        Mingyang Chen
//        The client connects to a server then send shell command to server to execute. The feedback from server will be printed.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

 
int main(int argc, char* argv[])
{
  int sd, portNumber, n, fileSize;
  struct hostent* server;
  struct sockaddr_in serv_addr;// Server socket address.
  char buffer[256];
  char fileSizeBuffer[256];
 

  if(argc != 3) {
    printf("Call mode: %s <hostname> <port>\n", argv[0]);
    return 1;
  }
  server = gethostbyname(argv[1]);
  if(!server) {
    fprintf(stderr, "Error: no such host: %s\n", argv[1]);
    return 2;
  }
  portNumber = atoi(argv[2]);
  


  //socket file descriptor
  sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sd < 0) {perror("can't open socket");
  exit(-1); }
  printf("create socket...\n");

  //when socket is created:
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET; //IPV4
  serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
  serv_addr.sin_port = htons(portNumber); //port

  if(connect(sd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
    perror("can't connect to server");
	exit(-1); }
  printf("connect...\n");

  do
  {
	  printf("\nENTER MESSAGE: ");
	  fgets(buffer, 255, stdin);
	  n = strlen(buffer);
  
	  if(n > 0 && buffer[n-1] == '\n') //line break
		  buffer[n-1] = '\0';
	  
	  //send
	  n = send(sd, buffer, strlen(buffer), 0);
	  printf("Client sent %s\n", buffer);

	  if(n < 0){ //couldn't send
		  perror("can't send to server");
		  exit(-1); }
		  
	
	  //client calls get
	  if(buffer[0] == 'g' &&
		 buffer[1] == 'e' &&
		 buffer[2] == 't' &&
		 buffer[3] == ' ')
	  {
		  printf("Client requested a download.\n");

		  //catch the file name
		  char fileName[256];
		  memset(&fileName, 0, sizeof(fileName));
          //parse file name
		  int j = 0;
		  for(int i = 4; i <= strlen(buffer); i++)
		  {
			  fileName[j] = buffer[i];
			  j++;
		  }

		  //catch file size:
		  recv(sd, buffer, sizeof(buffer), 0);
		  fileSize = atoi(buffer);

		  //send size back as ACK:
		  send(sd, buffer, sizeof(buffer), 0);

		  //print file name & size 
		  printf("File: '%s' (%d bytes)\n",fileName, fileSize);
		  

		  //receive data:
		  memset(&buffer, 0, sizeof(buffer));
		  int remainingData = 0;
		  ssize_t len;
 
		  FILE* fp;
		  fp = fopen(fileName, "wb");
		  							 
		  remainingData = fileSize;
		   
		  //printf("remainingData: %d", remainingData);
		  while(remainingData != 0)
		  {
			  
				  len = recv(sd, buffer, remainingData, 0);
				  fwrite(buffer, sizeof(char), len, fp);
				  remainingData -= len;
				  printf("Received %lu bytes\n", len);
				  break;
			   
		  }
		  fclose(fp);
		  n = recv(sd, buffer, 256, 0); //receive bizarre lingering packet.

		  //clean buffer
		  memset(&buffer, 0, sizeof(buffer));
	  }
	//client call put
	  else if(buffer[0] == 'p' &&
	          buffer[1] == 'u' &&
		  buffer[2] == 't' &&
		  buffer[3] == ' ')
	  {
		  printf("Client requested an upload\n");

                  //wait for the server's ACK
                  n = recv(sd, buffer, sizeof(buffer), 0);
                  if(n < 0)
                      printf("Server didn't acknowledge name");

		  //parse the string
		  int j = 0;
		  for(int i = 4; i <= strlen(buffer); i++)
		  {
			  buffer[j] = buffer[i];
			  j++;
		  }
	 
                  FILE* fp;
		  fp = fopen(buffer, "rb"); //read bytes of file name
		  if(fp == NULL)
			  printf("error opening file in: %s\n", buffer);
		  printf("File opened successfully!\n");

		  //calculate file size:
		  int file_size = 0;
		  if(fseek(fp, 0, SEEK_END) != 0)
			printf("Error determining file size\n");

		  file_size = ftell(fp);
		  rewind(fp);
		  printf("File size: %lu bytes\n", file_size);
		  
		  //pass file size to a buffer                  
		  memset(&fileSizeBuffer, 0, sizeof(fileSizeBuffer));
		  sprintf(fileSizeBuffer, "%d", file_size);
		  
		  //send file size:
		  n = send(sd, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
		  if(n < 0)
			  printf("Error sending file size information\n"); 
		  
		  //receive ACK for file size:
                  n = recv(sd, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
                  if(n < 0)
                          printf("Error receiving handshake");
                  
		  //create a byte array:
                  char FilebyteArray[256];
                  memset(&FilebyteArray, 0, sizeof(FilebyteArray));
 
                  int buffRead = 0;
                  int bytesRemaining = file_size;

                  //while there are still bytes to be sent:
                  while(bytesRemaining != 0)
                  {
                 
                           buffRead = fread(FilebyteArray, 1, bytesRemaining, fp);
                           bytesRemaining = bytesRemaining - buffRead;
                           n = send(sd, FilebyteArray, 256, 0);
                           if(n < 0)
                                   printf("Error sending file\n");

                            
                  }
                  printf("File sent!\n");
                  //clean buffers
                  memset(&buffer, 0, sizeof(buffer));
                  memset(&FilebyteArray, 0, sizeof(FilebyteArray));
	  }
	  
	  //user type quit
	  else if(strcmp(buffer, "quit") == 0)
	  {
		  break;
	  }
	  else//client sent a normal message
	  {	  
		   
		  n = recv(sd, buffer, sizeof(buffer), 0);
  
		  if(n < 0) {//couldn't receive 
			  perror("can't receive from server"); 
		  }
		  else
			  buffer[n] = '\0';
	  	  
		  printf("Client received message: %s\n", buffer);
		  
		  //clean buffer
		  memset(&buffer, 0, sizeof(buffer));
	  }
	  //clean buffer  
	  memset(&buffer, 0, sizeof(buffer));
  } while(strcmp(buffer, "quit") != 0);
  
  close(sd);
  return 0;
}
