#include <stdint.h>
#include <time.h>

#ifndef LAB5_HEADER
#define LAB5_HEADER

//	Representation of a UDP socket.
typedef struct udpSocket{
	int sockfd;
	struct sockaddr remoteInfo;
	socklen_t socklen;
} UDPSocket;

//	Representation of the a message's type.
typedef enum messagetype{ HANDSHAKE, FILENAME, DATA, FILEEND } m_type;

//	Representation of a packet.
typedef struct packet {
	uint8_t seq_num;
	uint8_t checksum;
	char data[10];
	uint8_t numbytes;
	m_type type;
} packet;

/*	uint8_t calc_checksum(char data[], int nbytes)
 *	
 *	Calculates a uint8_t checksum for the input array.
 *
 *	@param 	{char []} data:	array for which checksum is to be calculated
 *	@param	{int} nbytes:	number of relevant bytes
 *	@return	{uint8_t}	a 8-bit XOR checksum that corresponds to the input array.
 */
uint8_t calc_checksum(char data[], int nbytes){
	int i;
	uint8_t result = 0;
	for(i = 0; i < nbytes; i++){
		result = result ^ data[i];
	}
	return result;
}

/*	int validate_msg(packet pack, int seqn)
 *	
 *	Checks to see if a packet's checksum and sequence number are correct.
 *
 *	@param	{packet} pack: the packet to be verified.
 *	@param	{int} seqn:  the current sequence number of the calling client or server.
 *	@return {int} 0 if packet is invalid, 1 otherwise
 */	
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

/*	void printmetadata(packet pack)
 *
 *	Prints the metadata of a given packet: seq_num, checksum, numbytes, and type.
 *	
 *	@param {packet} pack: the packet to be printed.
 *	
 *	
 */
void printmetadata(packet pack){

	printf("seq_num: %d\nchecksum: %d\nnumbytes: %d\ntype: %d\n", pack.seq_num, pack.checksum, pack.numbytes, pack.type);
	return;
}

#endif 
