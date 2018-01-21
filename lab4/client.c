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
#include <sys/time.h>
#include "header.h"

int main (int, char *[]);

#define BUFF_LEN 100

#define CHECKSUM_FAIL_CHANCE 2
#define SEND_FAIL_CHANCE 2
#define TIMEOUT_TIME 1


/********************
 * main
 ********************/
int main (int argc, char *argv[])
{
	UDPSocket mysocket;
	char buff[BUFF_LEN];
	struct addrinfo hints, *res;
	int seq = 0;
	struct timeval tval; 

	if (argc != 5)
	{
		printf ("Usage: %s <srcfile> <destfile> <ip of server> <port>\n",argv[0]);
		return 1;
	}

	if(strlen(argv[2]) > 10){
		printf ("destfile max length: 10 characters. Input: %d characters.\n", strlen(argv[2]));
		return 1;
	}

	// set up
	memset (buff, '0', sizeof (buff));
	memset (&hints, 0, sizeof (hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(argv[3], argv[4], &hints, &res);

	srand(time(NULL));

	// open socket
	if ((mysocket.sockfd = socket (res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
	{
		printf ("Error : Could not create socket \n");
		return 1;
	}
	
	tval.tv_sec = TIMEOUT_TIME;
	tval.tv_usec = 0;
	setsockopt(mysocket.sockfd, SOL_SOCKET, SO_RCVTIMEO, &tval, sizeof(tval));

	mysocket.socklen = sizeof(mysocket.remoteInfo);
	printf("sending handshake...\n");

	//send handshake
	packet handshake_message;
	handshake_message.type = HANDSHAKE;
	handshake_message.seq_num = seq;
	memcpy(buff, &handshake_message, sizeof(handshake_message));
	while(1){
		printf("attempted to send handshake\n");
		sendto(mysocket.sockfd, buff, BUFF_LEN - 1, 0, res->ai_addr, res->ai_addrlen);
		recvfrom(mysocket.sockfd, buff, BUFF_LEN - 1, 0, &mysocket.remoteInfo, &mysocket.socklen);
		printf("recieved handshake response\n");
		packet response;
		memcpy(&response, buff, sizeof(response));
		if(response.seq_num == seq && response.type == HANDSHAKE){
			seq = (seq == 0) ? 1 : 0;
			printf("handshake good\n");
			break;
		}
		printf("handshake not good\n");
	}

	//send filename
	printf("sending filename...\n");
	packet filename_message; 
	filename_message.type = FILENAME;
	filename_message.checksum = calc_checksum(argv[2], sizeof(argv[2]));
	memcpy( filename_message.data, argv[2], sizeof(argv[2]));
	filename_message.numbytes = sizeof(argv[2]);
	filename_message.seq_num = seq;

	printf("filename message type: %d\n", filename_message.type);
	memcpy( buff, &filename_message, sizeof(filename_message));
	while(1){
		printf("attempted to send filename\n");
		sendto(mysocket.sockfd, buff, BUFF_LEN - 1, 0, res->ai_addr, res->ai_addrlen);
		recvfrom(mysocket.sockfd, buff, BUFF_LEN - 1, 0, &mysocket.remoteInfo, &mysocket.socklen);
		printf("recieved filename response\n");
		packet response;
		memcpy(&response, buff, sizeof(response));
		if(validate_msg(response, seq) == 1 && response.type == FILENAME){
			printf("filename good\n");
			seq = (seq == 0) ? 1 : 0;
			break;
		}
		printf("filename not good, type = %d\n", response.type);
	}
		
	//send data
	printf("sending data...\n");
	FILE *src = fopen(argv[1], "r");
	do{				//Send the entire message
		printf("constructing a data packet\n");
		packet message;
		message.seq_num = seq;
		message.type = DATA;
		message.numbytes = fread(message.data, 1, 10, src);
		printf("Bytes read: %d\n", message.numbytes);
		message.checksum = calc_checksum(message.data, message.numbytes);
		while(1){						//Send this specific chunk
			printf("sending a data packet with sequence %d when the client seq is %d\n", message.seq_num, seq);
			if(rand() % 10 < CHECKSUM_FAIL_CHANCE){ 
				message.checksum = 0;
			}else{
				message.checksum = calc_checksum(message.data, message.numbytes);
			}
				
			memcpy(buff, &message, sizeof(message));
			if(rand() % 10 >= SEND_FAIL_CHANCE){
				sendto(mysocket.sockfd, buff, BUFF_LEN - 1, 0, res->ai_addr, res->ai_addrlen);
			} else {
				sleep(TIMEOUT_TIME * 2);
			}

			

			int message_len = recvfrom(mysocket.sockfd, buff, BUFF_LEN - 1, 0, &mysocket.remoteInfo, &mysocket.socklen);
			if (message_len < 0){
				perror("");
				continue;
			}
			printf("recieved a data response\n");
			packet response;
			memcpy(&response, buff, sizeof(response));
			if(validate_msg(response, seq) == 1 && response.type == DATA){
				printf("data good\n");
				seq = (seq == 0) ? 1 : 0;
				break;
			}
			printf("data not good\n");
		}
	}while(feof(src) == 0);

	fclose(src);
	//send last packet
	packet final_message;
	printf("sending fileend...\n");
	final_message.type = FILEEND;
	final_message.seq_num = seq;
	final_message.numbytes = 0;
	final_message.checksum = calc_checksum(final_message.data, final_message.numbytes);
	memcpy(buff, &final_message, sizeof(final_message));
	printmetadata(final_message);
	while(1){
		printf("sending fileend packet\n");
		sendto(mysocket.sockfd, buff, BUFF_LEN - 1, 0, res->ai_addr, res->ai_addrlen);
		recvfrom(mysocket.sockfd, buff, BUFF_LEN - 1, 0, &mysocket.remoteInfo, &mysocket.socklen);
		printf("recieved fileend response\n");
		packet response;
		memcpy(&response, buff, sizeof(response));
		if(response.seq_num == seq && response.type == FILEEND){
			printf("fileend good\n");
			seq = (seq == 0) ? 1 : 0;
			break;
		}
		printf("fileend not good\n");
	}

	close(mysocket.sockfd);
	return 0;	
	
}
