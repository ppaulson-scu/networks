#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "lab1.h"

int main(int argc, char *argv[]){
	
	if(strcmp(argv[1], "--encrypt") == 0 && argc >=5){
		return encrypt(argv[2], argv[3], (long)argv[4]);

	}else if(strcmp(argv[1], "--decrypt") == 0 && argc >=5){
		return decrypt(argv[2], argv[3], (long)argv[4]);

	}else{
		printf("Usage:\n--encrypt <source_file> <destination_file> <encryption_key>\n--decrypt <source_file> <destination_file> <decryption_key>\n");
		return 0;
	}
}

int encrypt(char *source, char *destination, int key){
	
	FILE *src = fopen(source, "r");
	FILE *dest = fopen(destination, "w");

	uint8_t current;
	while(fread(&current, 1, 1, src) == 1){
		current = current ^ key;	
		fwrite(&current, 1, 1, dest);
	}

	fclose(src);
	fclose(dest);
	return 0;
}

int decrypt(char *source, char *destination, int key){
	return encrypt(source, destination, key);
}
