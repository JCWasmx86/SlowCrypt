#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shared.h"

static void decryptInMemory(Key key, FILE *toDecrypt, FILE *decrypted);
static void *readIntoBuffer(FILE *fp, uint64_t *size);

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
		uint8_t paddedZeroes = fgetc(toDecrypt);
		for (int i = 0; i < 7; i++) {
			fgetc(toDecrypt);
		}
		int fd = fileno(toDecrypt);
		struct stat buf;
		fstat(fd, &buf);
		off_t size = buf.st_size;
		uint64_t cnt = 0;
		uint64_t lastValue = 0;
		uint64_t numBlocks = (size - 8) / 8;
		int index = 0;
		while (1) {
			char rawBytes[8];
			int readBytes = fread(rawBytes, 1, 8, toDecrypt);
			if (readBytes == 0) {
				break;
			}
			uint64_t read = *((uint64_t *)rawBytes);
			uint64_t decrypted =
				decrypt(read, cnt == 0 ? key->firstValue : lastValue, key);
			decrypted ^= key->hash[index++];
			if (index == 8) {
				index = 0;
			}
			if (cnt < (numBlocks - 1)) {
				fwrite(&decrypted, 8, 1, clearText);
			} else {
				fwrite(&decrypted, 8 - paddedZeroes, 1, clearText);
			}
			lastValue = read;
			memset(rawBytes, 0, 8);
			cnt++;
		}
	} else {
		decryptInMemory(key, toDecrypt, clearText);
	}
	releaseKey(key);
	fclose(toDecrypt);
	fclose(clearText);
	free(arguments);
	return 0;
}

static void decryptInMemory(Key key, FILE *toDecrypt, FILE *decrypted) {
	uint64_t size = 0;
	uint64_t *buffer = (uint64_t *)readIntoBuffer(toDecrypt, &size);
	uint64_t *backupBuffer = buffer;
	assert(buffer);
	uint8_t *bufferToWriteTo = calloc(size, 1);
	assert(bufferToWriteTo);
	uint8_t paddedZeroes = ((uint8_t *)buffer)[0];
	buffer++; // Skip 8-byte header.
	uint64_t cnt = 0;
	uint64_t lastValue = 0;
	uint64_t numBlocks = (size - 8) / 8;
	int index = 0;
	while (1) {
		uint64_t read = *buffer;
		buffer++;
		uint64_t decryptedValue =
			decrypt(read, cnt == 0 ? key->firstValue : lastValue, key);
		if (cnt == numBlocks) {
			break;
		}
		decryptedValue ^= key->hash[index++];
		if (index == 8) {
			index = 0;
		}
		if (cnt < (numBlocks - 0)) {
			memcpy(&bufferToWriteTo[cnt * 8], &decryptedValue, 8);
		} else {
			memcpy(&bufferToWriteTo[cnt * 8], &decryptedValue,
				   8 - paddedZeroes);
			break;
		}
		lastValue = read;
		if (cnt == numBlocks) {
			break;
		}
		cnt++;
	}
	fwrite(bufferToWriteTo, 1, (cnt - 1) * 8 + (8 - paddedZeroes), decrypted);
	free(bufferToWriteTo);
	free(backupBuffer);
}
static void *readIntoBuffer(FILE *fp, uint64_t *size) {
	uint8_t *ret = calloc(32 * 1024, 1);
	assert(ret);
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
	assert(readBytes % 8 == 0);
	*size = readBytes;
	return readBytes != 0 ? realloc(ret, readBytes) : ret;
}
