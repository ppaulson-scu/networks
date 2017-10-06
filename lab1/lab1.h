#ifndef LAB_1
#define LAB_1

//Encrypts the data found in source and stores it in destination.
//Paramaters:
//Source: file to be encrypted
//Destination: file to store encrypted data in   
//Key: encryption key
//Side effects: destination will be created if it does not exist, and overwritten otherwise.
int decrypt(char *source, char *destination, int key);

//Decrypts the data found in source and stores it in destination.
//Paramaters:
//Source: file to be decrypted
//Destination: file to store decrypted data in   
//Key: decryption key
//Side effects: destination will be created if it does not exist, and overwritten otherwise.
int encrypt(char *source, char *destination, int key);

#endif
