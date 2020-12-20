#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shared.h"

int main(int argc, char **argv) {
	Arguments *arguments = calloc(1, sizeof(Arguments));
	assert(arguments);
	evalArguments(argc, argv, arguments);
	FILE *toDecrypt = fopen(arguments->inFile, "rb");
	if (toDecrypt == NULL) {
		fprintf(stderr, "Couldn\'t open input file %s\n", arguments->inFile);
		free(arguments);
		return EXIT_FAILURE;
	}
	FILE *clearText = fopen(arguments->outFile, "wb");
	if (clearText == NULL) {
		fprintf(stderr, "Couldn\'t open output file %s\n", arguments->outFile);
		fclose(toDecrypt);
		free(arguments);
		return EXIT_FAILURE;
	}
	Key key = readKey(arguments->keyFile);
	if (key == NULL) {
		fprintf(stderr, "Couldn'\t load keyfile %s\n", arguments->keyFile);
		fclose(toDecrypt);
		fclose(clearText);
		free(arguments);
		return EXIT_FAILURE;
	}
	if (!arguments->useMemory) {
		decrypt(toDecrypt, clearText, key);
	} else {
		decryptInMemory(toDecrypt, clearText, key);
	}
	releaseKey(key);
	fclose(toDecrypt);
	fclose(clearText);
	free(arguments);
	return 0;
}
