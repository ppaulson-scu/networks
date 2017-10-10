 /**************************
 *     Based on:
 *     socket example, server
 *     fall 2017
 ***************************/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <math.h>

int main (int, char *[]); 


/*********************
 * main
 *********************/
int main (int argc, char *argv[])
{
	int n = 0;
	char *p; 
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr; 
	char buff[5];
	char large_buff[20];
	struct addrinfo hints, *res;

	// set up
	memset (&serv_addr, '0', sizeof (serv_addr));
	memset (buff, '0', sizeof (buff)); 
	memset (&hints, 0, sizeof (hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, argv[1], &hints, &res);
	
	// create socket, bind, and listen
	listenfd = socket (res->ai_family, res->ai_socktype, 0);
	bind (listenfd, res->ai_addr, res->ai_addrlen); 
	listen (listenfd, 10); 	
	
	// accept and interact
	connfd = accept (listenfd, (struct sockaddr*)NULL, NULL); 

	//get destination file name
	read (connfd, large_buff, sizeof(large_buff));
	char *filename = large_buff;
	FILE *dest = fopen(filename, "w");	
	
	//get file length		
	int file_len;
	read (connfd, &file_len, sizeof(file_len)); 

	//receive data
	while (file_len > 0)
	{
		memset(buff, 0, 10);
		read (connfd, buff, (5 > file_len)? file_len : 5);	
		fwrite (buff, 1, (5 > file_len)? file_len : 5, dest);
		file_len -= 5;
	}
	close (connfd);
}
