#ifndef _SLOWCRYPT_H
#define _SLOWCRYPT_H
// Opaque struct that holds the key
struct _key;
typedef struct _key *Key;
// Read key from specified file. If it wasn't found, NULL will be returned
Key readKey(char *name);
// Free key and resources
void releaseKey(Key key);
// No argument can be NULL!

// Decrypt toDecrypt to clearText with the key. Returns 1 on error, 0 otherwise
int decrypt(FILE *toDecrypt, FILE *clearText, Key key);
// Encrypt toEncrypt to encrypted with the key. Returns 1 on error, 0 otherwise
int encrypt(FILE *toEncrypt, FILE *encrypted, Key key);
// Encrypt toEncrypt in memory and write everything at once to encrypted.
// Returns 1 on error
int encryptInMemory(FILE *toEncrypt, FILE *encrypted, Key key);
// Decrypt toDecrypt in memory and write it to decrypted.
int decryptInMemory(FILE *toDecrypt, FILE *decrypted, Key key);
#endif
