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

uint64_t encrypt(uint64_t v, uint64_t lastValue, Key k);

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
	fputc(0, encrypted); // How many padded zeroes? Will be filled later.
	int paddedZeroes = 0;
	uint64_t cnt = 0;
	uint64_t lastValue = 0;
	for (int i = 0; i < 7; i++)
		fputc(rand() & 0xFF, encrypted); // Padding to 8
	int index=0;
	while (1) {
		char rawBytes[8];
		int readBytes = fread(rawBytes, 1, 8, toEncrypt);
		if (readBytes == 0) {
			break;
		} else {
			for (int i = readBytes; i < 8; i++)
				rawBytes[i] = '\0';
			paddedZeroes = 8 - readBytes;
		}
		uint64_t read = *((uint64_t *)rawBytes);
		read^=key->hash[index++];
		if(index==8){
			index=0;
		}
		uint64_t encryptedInt =
			encrypt(read, cnt == 0 ? key->firstValue : lastValue, key);
		fwrite(&encryptedInt, 8, 1, encrypted);
		cnt++;
		lastValue = encryptedInt;
	}
	rewind(encrypted);
	fputc(paddedZeroes, encrypted);
	releaseKey(key);
	fclose(toEncrypt);
	fclose(encrypted);
	free(arguments);
	return EXIT_SUCCESS;
}

uint64_t encrypt(uint64_t v, uint64_t lastValue, Key k) {
	uint64_t start = v;
	start ^= k->xorValue;
	start += k->addValue;
	start = rotate(start, k->shiftValue);
	uint64_t generatedRandomNumber = k->startXorValue;
	for (int i = 0; i < k->howManyBitSets; i++) {
		uint8_t toggle = !!xorshift(k->state);
		uint64_t i = xorshift(k->state) % 64;
		if (toggle)
			generatedRandomNumber ^= (1L << i);
	}
	start ^= generatedRandomNumber;
	for (uint16_t i = 0; i < k->howManyAdds; i++)
		start += xorshift(k->addState);
	for (uint8_t i = 0; i < k->howManyXors; i++)
		start ^= xorshift(k->xorState);
	start ^= lastValue;
	return reverse(start);
}
