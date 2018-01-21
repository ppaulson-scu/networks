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
#include "header.h"

#define BUFF_LEN 100

int main (int, char *[]); 


/*********************
 * main
 *********************/
int main (int argc, char *argv[])
{
	int rcvd_bytes; 
	char buff[BUFF_LEN];
	struct addrinfo hints, *res;
	UDPSocket mysocket;
	int seq = 0;
	char *filename;
	FILE *dest;

	// set up
	memset (buff, '0', sizeof (buff)); 
	memset (&hints, 0, sizeof (hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, argv[1], &hints, &res);
	
	// create socket, bind, and listen
	if((mysocket.sockfd = socket (res->ai_family, res->ai_socktype, 0)) < 0){
		printf("socket creation error\n");
		return 0;
	}
		
	if((bind (mysocket.sockfd, res->ai_addr, res->ai_addrlen)) < 0){
		printf("socket bind error\n");
		return 0;
	}

		
	
	mysocket.socklen = sizeof mysocket.remoteInfo;
	while(1){
		memset(buff, '0', sizeof(buff));
		rcvd_bytes = recvfrom(mysocket.sockfd, buff, BUFF_LEN - 1, 0, &mysocket.remoteInfo, &mysocket.socklen);
		
		//reassemble packet
		packet message;
		memcpy(&message, buff, sizeof(message));
		printf("Message type: %d\n", message.type);
		if(message.type == HANDSHAKE){
			printf("server recieved HANDSHAKE\n");
			//send handshake ACK
			seq = (seq == 0) ? 1 : 0;
			sendto(mysocket.sockfd, buff, BUFF_LEN - 1, 0, &mysocket.remoteInfo, mysocket.socklen);
			continue;
		}

		if(validate_msg(message, seq) == 0){
			printf("server recieved corrupt message, discarding\n");
			printmetadata(message);
			message.checksum = message.checksum + 1;
			memcpy(buff, &message, sizeof(message));
			sendto(mysocket.sockfd, buff, BUFF_LEN - 1, 0, &mysocket.remoteInfo, mysocket.socklen);
			continue;
		}

		if(message.type == FILENAME){
			printf("server recieved FILENAME\n");
			filename = message.data;
			filename[message.numbytes] = '\0';
			printf("Filename: %s\n", filename);
			dest = fopen(filename, "w");
			seq = (seq == 0) ? 1 : 0;
			sendto(mysocket.sockfd, buff, BUFF_LEN - 1, 0, &mysocket.remoteInfo, mysocket.socklen);
		}

		if(message.type == DATA){
			printf("server recieved DATA\n");
			printf("Message data: %s\n", message.data);
			fwrite(message.data, 1, message.numbytes, dest);

			seq = (seq == 0) ? 1 : 0;
			sendto(mysocket.sockfd, buff, BUFF_LEN - 1, 0, &mysocket.remoteInfo, mysocket.socklen);
		}
		
		if(message.type == FILEEND){
			printf("server recieved FILEEND\n");
			seq = (seq == 0) ? 1 : 0;
			sendto(mysocket.sockfd, buff, BUFF_LEN - 1, 0, &mysocket.remoteInfo, mysocket.socklen);
			break;
		}
	}
	
	fclose(dest);
	close(mysocket.sockfd);
	return 0;
}














