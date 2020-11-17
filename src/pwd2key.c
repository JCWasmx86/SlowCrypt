#include <assert.h>
#include <openssl/evp.h>
#include <shared.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>

#define MAX_PASSWORD_LENGTH 1024
#define HASH_LENGTH (87 + sizeof(unsigned int))
#define SALT "slowcrypt"
#define SALT_LENGTH strlen(SALT)
#define NUM_ITERATIONS 1048576

ssize_t getpassword(char **lineptr, size_t *n, FILE *stream);
void generateKey(unsigned char *digest, char *outputFile);
void hash(const char *password, const unsigned char *salt, int32_t iterations,
		  uint32_t outputBytes, unsigned char *digest);
char *evalArgumentsForPwd2key(int argc, char **argv);

void hash(const char *password, const unsigned char *salt, int32_t iterations,
		  uint32_t outputBytes, unsigned char *digest) {
	assert(PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_LENGTH,
							 iterations, EVP_sha512(), outputBytes, digest));
}

// Based on https://stackoverflow.com/a/6869218
ssize_t getpassword(char **lineptr, size_t *n, FILE *stream) {
	struct termios old;
	if (tcgetattr(fileno(stream), &old) != 0)
		return -1;
	struct termios new = old;
	new.c_lflag &= ~ECHO;
	if (tcsetattr(fileno(stream), TCSAFLUSH, &new) != 0)
		return -1;
	int nread = getline(lineptr, n, stream);
	assert(tcsetattr(fileno(stream), TCSAFLUSH, &old) == 0);
	return nread;
}

int main(int argc, char **argv) {
	char *keyName = evalArgumentsForPwd2key(argc, argv);
	char *password = NULL; // Initialized by getpassword
	size_t lengthOfPassword = 0;
	puts("Please enter your password:");
	assert(getpassword(&password, &lengthOfPassword, stdin) != -1);
	printf("Generating keyfile: %s\n", keyName);
	unsigned char *digest = malloc(HASH_LENGTH);
	assert(digest);
	unsigned char *salt = (unsigned char *)SALT;
	hash(password, salt, 100, HASH_LENGTH, digest);
	generateKey(digest, keyName);
	memset(password, 0, lengthOfPassword);
	free(digest);
	free(password);
	return EXIT_SUCCESS;
}
void generateKey(unsigned char *digest, char *keyName) {
	FILE *fp = fopen(keyName, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Couldn't open file %s\n", keyName);
		return;
	}
	srandom(*((unsigned int *)digest));
	digest += sizeof(unsigned int);
	uint64_t xorV = *((uint64_t *)digest);
	digest += 8;
	fwrite(&xorV, 8, 1, fp); // 8 bytes
	uint64_t addV = *((uint64_t *)digest);
	fwrite(&addV, 8, 1, fp); // 16 bytes
	digest += 8;
	int8_t shiftValue = digest[0] % 64 * (digest[1] & 1 ? -1 : 1); // 18 bytes
	fwrite(&shiftValue, 1, 1, fp);
	digest += 2;
	uint64_t firstValue = *((uint64_t *)digest); // 24 bytes
	digest += 8;
	fwrite(&firstValue, 8, 1, fp);
	for (int i = 0; i < 3; i++) {
		uint64_t maxSize = *((uint64_t *)digest) & 0xFFFF; // 32,48,64 bytes
		digest += 8;
		maxSize = maxSize < 4 ? 4 : maxSize;
		fwrite(&maxSize, 8, 1, fp);
		for (uint64_t i = 0; i < maxSize; i++) {
			uint64_t a = generate64BitValue();
			fwrite(&a, 8, 1, fp);
		}
		uint64_t index = *((uint64_t *)digest) % maxSize; // 40,56,72 bytes
		digest += 8;
		fwrite(&index, 8, 1, fp);
	}
	uint8_t xorC = digest[0]; // 73 bytes
	fwrite(&xorC, 1, 1, fp);
	digest++;
	uint16_t bitC = *((uint16_t *)digest); // 75 bytes
	digest += 2;
	fwrite(&bitC, 1, 2, fp);
	uint64_t sX = *((uint64_t *)digest); // 83 bytes
	digest += 8;
	fwrite(&sX, 1, 8, fp);
	uint16_t howManyAdds = *((uint16_t *)digest); // 85 bytes
	digest += 2;
	fwrite(&howManyAdds, 2, 1, fp);
	fclose(fp);
}

char *evalArgumentsForPwd2key(int argc, char **argv) {
	char *keyFile = "key.key";
	int shouldExit = 0;
	for (int i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
			printf("%s:\n", argv[0]);
			puts("Usage:");
			printf("%s <options>* outputFile?\n", argv[0]);
			puts("--help, -h    Print this help");
			puts("--version, -v Print the version");
			puts("If no outputFile is given, key.key is assumed silently.");
			shouldExit = 1;
		} else if (strcmp(arg, "--version") == 0 || strcmp(arg, "-v") == 0) {
			printf("%s: Version 0.0.1-Beta\n", argv[0]);
			shouldExit = 1;
		} else {
			if (strcmp("key.key", keyFile) != 0) {
				fprintf(stderr, "Overwriting outputfile %s with %s!\n", keyFile,
						arg);
			}
			keyFile = arg;
		}
	}
	if (shouldExit)
		exit(0);
	return keyFile;
}
