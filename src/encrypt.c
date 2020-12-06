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

static void encryptInMemory(Key key, FILE *toEncrypt, FILE *encrypted);
static void *readIntoBuffer(FILE *fp, uint64_t *size, int *numZeroes);

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
		// How many padded zeroes? Will be filled later.
		fputc(0, encrypted);
		int paddedZeroes = 0;
		uint64_t cnt = 0;
		uint64_t lastValue = 0;
		for (int i = 0; i < 7; i++)
			fputc(rand() & 0xFF, encrypted); // Padding to 8
		int index = 0;
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
			read ^= key->hash[index++];
			if (index == 8) {
				index = 0;
			}
			uint64_t encryptedInt =
				encrypt(read, cnt == 0 ? key->firstValue : lastValue, key);
			fwrite(&encryptedInt, 8, 1, encrypted);
			cnt++;
			lastValue = encryptedInt;
		}
		rewind(encrypted);
		fputc(paddedZeroes, encrypted);
	} else {
		encryptInMemory(key, toEncrypt, encrypted);
	}
	releaseKey(key);
	free(arguments);
	fclose(toEncrypt);
	fclose(encrypted);
	return EXIT_SUCCESS;
}

static void encryptInMemory(Key key, FILE *toEncrypt, FILE *encrypted) {
	uint64_t size = 0;
	int paddedZeroes = 0;
	uint64_t *buffer =
		(uint64_t *)readIntoBuffer(toEncrypt, &size, &paddedZeroes);
	uint64_t *backupBuffer = buffer;
	assert(buffer);
	uint64_t numberOfBlocks = size / 8;
	size += 8; // For the first leading 8 bytes
	uint8_t *bufferToWriteTo = calloc(size, 1);
	assert(bufferToWriteTo);
	uint8_t *base = bufferToWriteTo;
	*bufferToWriteTo = paddedZeroes;
	bufferToWriteTo++;
	for (int i = 0; i < 7; i++) {
		*bufferToWriteTo = rand() & 0xFF;
		bufferToWriteTo++;
	}
	uint64_t cnt = 0;
	uint64_t lastValue = 0;
	int index = 0;
	while (1) {
		uint8_t rawBytes[8];
		memcpy(rawBytes, buffer, 8);
		buffer++;
		if (cnt == numberOfBlocks) {
			break;
		}
		uint64_t read = *((uint64_t *)rawBytes);
		read ^= key->hash[index++];
		if (index == 8) {
			index = 0;
		}
		uint64_t encryptedInt =
			encrypt(read, cnt == 0 ? key->firstValue : lastValue, key);
		memcpy(bufferToWriteTo, &encryptedInt, 8);
		bufferToWriteTo += 8;
		cnt++;
		lastValue = encryptedInt;
	}
	size_t num = fwrite(base, 1, (numberOfBlocks * 8 + 8), encrypted);
	assert((numberOfBlocks * 8 + 8) == num);
	fflush(encrypted);
	free(base);
	free(backupBuffer);
}
static void *readIntoBuffer(FILE *fp, uint64_t *size, int *numZeroes) {
	uint8_t *ret = malloc(32 * 1024);
	uint64_t currSize = 32 * 1024;
	uint64_t readBytes = 0;
	while (1) {
		size_t cnt = fread(&ret[readBytes], 1, 1024, fp);
		readBytes += cnt;
		if (cnt < 1024) {
			break;
		}
		if (currSize == readBytes) {
			void *ptr = realloc(ret, currSize * 2);
			currSize *= 2;
			assert(ptr);
			ret = ptr;
		}
	}
	int cnt = 0;
	while (readBytes % 8 != 0) {
		readBytes++;
		cnt++;
	}
	*numZeroes = cnt;
	*size = readBytes;
	return readBytes != 0 ? realloc(ret, readBytes) : ret;
}
