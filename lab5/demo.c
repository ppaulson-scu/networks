#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *increment();
void *decrement();

int main(){
	srand(time(NULL));
	pthread_t thread1, thread2;
	int iret1, iret2;

	iret1 = pthread_create(&thread1, NULL, increment, NULL);
	iret2 = pthread_create(&thread2, NULL, decrement, NULL);

	if(iret1){
		fprintf(stderr, 
		exit(EXIT_FAILURE);
	}

	pthread_join( thread1, NULL);

	exit(EXIT_SUCCESS);

}

void *increment(){

	int i = 0;

	for(i = 0; i < 6; i++){
		pthread_mutex_lock(&mutex);
		//do stuff
		pthread_mutex_unlock(&mutex);
	}
	
}
	

