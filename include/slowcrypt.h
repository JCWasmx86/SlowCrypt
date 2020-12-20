#ifndef _SLOWCRYPT_H
#define _SLOWCRYPT_H
struct _key;
typedef struct _key *Key;
int decrypt(FILE *toDecrypt, FILE *clearText, Key key);
int encrypt(FILE *toEncrypt, FILE *encrypted, Key key);
int encryptInMemory(FILE *toEncrypt, FILE *encrypted, Key key);
int decryptInMemory(FILE *toDecrypt, FILE *decrypted, Key key);
#endif
