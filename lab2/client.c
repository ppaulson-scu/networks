/***************************
 *	Based on
 *	socket example, client
 *	fall 2017
 **************************/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

int main (int, char *[]);


/********************
 * main
 ********************/
int main (int argc, char *argv[])
{
	int i;
	int sockfd = 0, n = 0;
	char buff[10];
	char *p;
	struct sockaddr_in serv_addr;
	struct addrinfo hints, *res;

	if (argc != 5)
	{
		printf ("Usage: %s <ip of server> \n",argv[0]);
		return 1;
	} 

	// set up
	memset (buff, '0', sizeof (buff));
	memset (&serv_addr, '0', sizeof (serv_addr)); 
	memset (&hints, 0, sizeof (hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	//File setup
	FILE *src = fopen(argv[1], "r");

	getaddrinfo(argv[3], argv[4], &hints, &res);
	

	// open socket
	if ((sockfd = socket (res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
	{
		printf ("Error : Could not create socket \n");
		return 1;
	} 

	// connect
	if (connect (sockfd, res->ai_addr, res->ai_addrlen) < 0)
	{
		int erno = errno;
		printf ("Error : Connect Failed \nErrno: %d\n", erno);
		return 1;
	} 

	//send file name
	write(sockfd, argv[2], 16);

	//send file size
	int count = 0;
	int current;
	while(fread(&current, 1, 1, src)){
		count++;
	}
	rewind(src);
	write(sockfd, &count, sizeof(count));

	//send file
	while (1)
	{	
		int read_in = fread(&buff, 1, 10, src);
		write (sockfd, buff, read_in);
		if (read_in < 10) break;
	} 
	close (sockfd);

	return 0;
}
