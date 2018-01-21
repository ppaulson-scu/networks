#include <stdint.h>
#include <time.h>

#ifndef LAB3_HEADER
#define LAB3_HEADER


typedef struct udpSocket{
	int sockfd;
	struct sockaddr remoteInfo;
	socklen_t socklen;
} UDPSocket;

typedef enum messagetype{ HANDSHAKE, FILENAME, DATA, FILEEND } m_type;

typedef struct packet {
	uint8_t seq_num;
	uint8_t checksum;
	char data[10];
	uint8_t numbytes;
	m_type type;
} packet;

uint8_t calc_checksum(char data[], int nbytes){
	int i;
	uint8_t result = 0;
	for(i = 0; i < nbytes; i++){
		result = result ^ data[i];
	}
	return result;
}

int validate_msg(packet pack, int seqn){
	if(seqn != pack.seq_num){
		printf("sequence incorrect. seq_num: %d, seqn: %d\n", pack.seq_num, seqn);
		return 0;	
	}

	 
	if(calc_checksum(pack.data, pack.numbytes) != pack.checksum){
		printf("checksum incorrect: %d vs %d\n", calc_checksum(pack.data, pack.numbytes), pack.checksum);		
		return 0;
	}
	return 1;
}

void printmetadata(packet pack){

	printf("seq_num: %d\nchecksum: %d\nnumbytes: %d\ntype: %d\n", pack.seq_num, pack.checksum, pack.numbytes, pack.type);
	return;
}

#endif 
