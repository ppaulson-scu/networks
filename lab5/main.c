#include <sys/wait.h>
#include <pthread.h>
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

#define INF 10000
#define BUFF_LEN 100
#define N 4
void *thread1();
void *thread2();
void *thread3();

void sendMessage(int, int, int);

int neighbor_table[N][N];
int r_id;
char* c_table;
char* r_info;

typedef struct message {
	int src;
	int dest;
	int cost;
} message;

pthread_mutex_t neighbor_table_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]){

	srand(time(NULL));
	pthread_t t1, t2, t3;
	int iret1, iret2, iret3;
	int i, j;
	char str [100];

	if(argc != 4){
		fprintf(stderr, "Usage: ./main <router_id> <cost_table> <routing_info>\n");
		return 1;
	}

	//fprintf(stderr, "test\n");
	
	r_id = atoi(argv[1]);
	c_table = argv[2];
	r_info = argv[3];

	FILE* cost_file = fopen(c_table, "r");

	//char c = fgetc(cost_file);
	//while(c != EOF){
	//	if (c == '\n') N++;
	//	c = fgetc(cost_file); 
	//}

	//rewind(cost_file);
	//neighbor_table = (int **) malloc (N * sizeof(int *));
	//for(i = 0; i < N; i++){
		//neighbor_table[i] = (int *) malloc (N * sizeof(int));
	//}

	//fprintf(stderr, "%d\n", neighbor_table[i][j]);
	
	for(i = 0; i < N; i++){
		for(j = 0; j < N; j++){;
			int x;
			fscanf(cost_file, "%d", &x);
			neighbor_table[i][j] = x;
		}
	}

	//fprintf(stderr, "test\n");
	iret1 = pthread_create(&t1, NULL, thread1, NULL);
	iret2 = pthread_create(&t2, NULL, thread2, NULL);
	iret3 = pthread_create(&t3, NULL, thread3, NULL);

	if(iret1 || iret2 || iret3){
		fprintf(stderr, "error creating threads\n");
	}

	//fprintf(stderr, "test\n");
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	
	fclose(cost_file);
	exit(EXIT_SUCCESS);
	
}

//Recieves messages from other 'routers.' Updates the neighbor cost table.
void *thread1(){ 
	
	int rcvd_bytes = 0;
	char buff[BUFF_LEN];
	struct addrinfo hints, *res;
	UDPSocket mysocket;
	message msg;

	memset (buff, '0', sizeof (buff)); 
	memset (&hints, 0, sizeof (hints));	
	
	FILE* fp = fopen(r_info, "r");
	char machine [10];
	char ip[13];
	char port[5];
	int i;

	//fprintf(stderr, "%d\n", r_id);
	for(i = 0; i <= r_id; i++){
		fscanf(fp, "%s %s %s", machine, ip, port);
	}

	//fprintf(stderr, "%s\n", port);

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, port, &hints, &res);

	if((mysocket.sockfd = socket (res->ai_family, res->ai_socktype, 0)) < 0){
		printf("socket creation error\n");
		return 0;
	}

	//fprintf(stderr, "test (t1)\n");

	if((bind (mysocket.sockfd, res->ai_addr, res->ai_addrlen)) < 0){
		printf("socket bind error - %s\n", strerror(errno));
		return 0;
	}
	
	//fprintf(stderr, "test (t1)\n");

	mysocket.socklen = sizeof mysocket.remoteInfo;
	while(1){
		memset(buff, '0', sizeof(buff));
		rcvd_bytes = recvfrom(mysocket.sockfd, buff, BUFF_LEN - 1, 0, &mysocket.remoteInfo, &mysocket.socklen);

		//fprintf(stderr, "Recieved Bytes: %d\n", rcvd_bytes);

		memcpy(&msg, buff, sizeof(msg));
		if(msg.src > N || msg.dest > N) continue;
		//fprintf(stderr, "Recieved a message! %d %d %d\n", msg.src, msg.dest, msg.cost);

		pthread_mutex_lock(&neighbor_table_mutex);
		neighbor_table[msg.src][msg.dest] = msg.cost;
		neighbor_table[msg.dest][msg.src] = msg.cost;
		//fprintf(stderr, "New cost recieved: %d\n", neighbor_table[msg.src][msg.dest]);
		pthread_mutex_unlock(&neighbor_table_mutex);
	
		int j, k;

		for(j = 0; j < N; j++){
			for(k = 0; k < N; k++){
				//fprintf(stderr, "%d ", neighbor_table[j][k]);
			}
			//fprintf(stderr, "\n");
		}
	}
	
}

//Reads changes from keyboard. Updates neighbor cost table. Sends messages to other nodes.
void *thread2(){

	while(1){
		//read input from keyboard
		int dest, cost;
		scanf("%d %d", &dest, &cost);


		//update table
		pthread_mutex_lock(&neighbor_table_mutex);
		neighbor_table[r_id][dest] = cost;
		neighbor_table[dest][r_id] = cost;
		//fprintf(stderr, "New cost: %d\n", neighbor_table[r_id][dest]);
		pthread_mutex_unlock(&neighbor_table_mutex);

		//send messages to other nodes
		int i;
		for(i = 0; i < N; i++){
			if(i == r_id) continue;
			int j;
			for(j = 0; j < N; j++){
				//fprintf(stderr, "Sending to %d...\n", i);
				sendMessage(i, j, neighbor_table[r_id][j]);
			}	
		}
	}
}

//Runs Djikstra's Algorithm. Keeps the appropriate row of the neighbor table updated.
void *thread3(){

	while(1){
		sleep(10);
		pthread_mutex_lock(&neighbor_table_mutex);
		int solved_set [N];
		memset(solved_set, 0, sizeof(solved_set));
		solved_set[r_id] = 1;
		int min_costs [N];
		int i, j;

		//fprintf(stderr, "\n");
		for(i = 0; i < N; i++){
			for(j = 0; j < N; j++){
				//fprintf(stderr, "%d ", neighbor_table[i][j]);
			}
			//fprintf(stderr, "\n");
		}
		//fprintf(stderr, "\n");

		for(i = 0; i < N; i++){
			min_costs[i] = neighbor_table[r_id][i];
			//fprintf(stderr, "%d ", min_costs[i]);
		}

		//fprintf(stderr, "\n");

		for(i = 0; i < N-1; i++){
			//determine the minimum unadded cost node
			int minimum = INF;
			int min_node;
			for(j = 0; j < N; j++){
				if(solved_set[j] == 0 && min_costs[j] <= minimum){
					min_node = j;
					minimum = min_costs[j];
				}
			}

			//fprintf(stderr, "Min: %d Minnode: %d\n", minimum, min_node);

			//update min cost set and visited array
			min_costs[min_node] = minimum;
			solved_set[min_node] = 1;

			//update min cost array with new visited node
			for(j = 0; j < N; j++){
				if(solved_set[j] == 0 && min_costs[j] > minimum + neighbor_table[min_node][j]){
					//fprintf(stderr, "min: %d neighbor_table[min_node][j]: %d\n", minimum, 1+neighbor_table[min_node][j]);
					min_costs[j] = minimum + neighbor_table[min_node][j];
					//fprintf(stderr, "Sum: %d %d\n", min_costs[j], minimum + neighbor_table[min_node][j]);
				}
			}
		}

		for(i = 0; i < N; i++){
			neighbor_table[r_id][i] = min_costs[i];
			neighbor_table[i][r_id] = min_costs[i];
			fprintf(stderr, "%d ", min_costs[i]);
		}
		fprintf(stderr, "\n");

		pthread_mutex_unlock(&neighbor_table_mutex);
	}
}

void sendMessage(int target, int dest, int cost){
	UDPSocket mysocket;
	struct addrinfo hints, *res;
	char str[100];
	message msg;

	// set up
	FILE* fp = fopen(r_info, "r");
	char machine [10];
	char ip[13];
	char port[5];
	int i;
	for(i = 0; i <= target; i++){
		fscanf(fp, "%s %s %s", machine, ip, port);
	}
	
		
	memset (str, ' ', sizeof (str));
	memset (&hints, 0, sizeof (hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, port, &hints, &res);
	if ((mysocket.sockfd = socket (res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
	{
		printf ("Error : Could not create socket, %s \n", strerror(errno));
		return;
	}

	mysocket.socklen = sizeof(mysocket.remoteInfo);

	msg.src = r_id;
	msg.dest = dest;
	msg.cost = cost;

	memcpy(str, &msg, sizeof(msg));

	sendto(mysocket.sockfd, str, sizeof(msg), 0, res->ai_addr, res->ai_addrlen);
	//fprintf(stderr, "Transmitted to router %d: %d %d %d\n", target, msg.src, msg.dest, msg.cost);

	fclose(fp);

}
