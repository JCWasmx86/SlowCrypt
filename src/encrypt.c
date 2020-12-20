#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shared.h"

int main(int argc, char **argv) {
	Arguments *arguments = calloc(1, sizeof(Arguments));
	assert(arguments);
	evalArguments(argc, argv, arguments);
	FILE *toEncrypt = fopen(arguments->inFile, "rb");
	if (toEncrypt == NULL) {
		fprintf(stderr, "Couldn\'t open input file %s\n", arguments->inFile);
		free(arguments);
		return EXIT_FAILURE;
	}
	FILE *encrypted = fopen(arguments->outFile, "wb");
	if (encrypted == NULL) {
		fprintf(stderr, "Couldn\'t open output file %s\n", arguments->outFile);
		free(arguments);
		fclose(toEncrypt);
		return EXIT_FAILURE;
	}
	Key key = readKey(arguments->keyFile);
	if (key == NULL) {
		fprintf(stderr, "Couldn\'t load key %s\n", arguments->keyFile);
		free(arguments);
		fclose(toEncrypt);
		fclose(encrypted);
		return EXIT_FAILURE;
	}
	if (!arguments->useMemory) {
		encrypt(toEncrypt, encrypted, key);
	} else {
		encryptInMemory(toEncrypt, encrypted, key);
	}
	releaseKey(key);
	free(arguments);
	fclose(toEncrypt);
	fclose(encrypted);
	return EXIT_SUCCESS;
}
