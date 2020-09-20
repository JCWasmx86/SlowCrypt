#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shared.h"

uint64_t decrypt(uint64_t v, uint64_t lastValue, Key k);

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("%s <infile> <outfile> [<keyfile>(default key.key)]\n", argv[0]);
		return -1;
	}
	FILE *toDecrypt = fopen(argv[1], "rb");
	if (toDecrypt == NULL) {
		fprintf(stderr, "Couldn\'t open %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	FILE *clearText = fopen(argv[2], "wb");
	if (clearText == NULL) {
		fprintf(stderr, "Couldn\'t open %s\n", argv[2]);
		fclose(toDecrypt);
		return EXIT_FAILURE;
	}
	Key key = readKey(argv[3] == NULL ? "key.key" : argv[3]);
	if (key == NULL) {
		fprintf(stderr, "Couldn'\t load key!\n");
		fclose(toDecrypt);
		fclose(clearText);
		return EXIT_FAILURE;
	}
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
	while (1) {
		char rawBytes[8];
		int readBytes = fread(rawBytes, 1, 8, toDecrypt);
		if (readBytes == 0) {
			break;
		}
		uint64_t read = *((uint64_t *)rawBytes);
		uint64_t decrypted =
			decrypt(read, cnt == 0 ? key->firstValue : lastValue, key);
		if (cnt < (numBlocks - 1)) {
			fwrite(&decrypted, 8, 1, clearText);
		} else {
			fwrite(&decrypted, 8 - paddedZeroes, 1, clearText);
		}
		lastValue = read;
		memset(rawBytes, 0, 8);
		cnt++;
	}
	free(key->state->state);
	free(key->xorState->state);
	free(key->addState->state);
	free(key->state);
	free(key->xorState);
	free(key->addState);
	free(key);
	fclose(toDecrypt);
	fclose(clearText);
	return 0;
}
uint64_t decrypt(uint64_t v, uint64_t lastValue, Key k) {
	uint64_t start = reverse(v);
	uint64_t generatedRandomNumber = k->startXorValue;
	for (int i = 0; i < k->howManyBitSets; i++) {
		uint8_t toggle = !!xorshift(k->state);
		uint64_t i = xorshift(k->state) % 64;
		if (toggle) {
			generatedRandomNumber ^= (1L << i);
		}
	}
	uint64_t tmp = 0;
	for (uint16_t i = 0; i < k->howManyAdds; i++)
		tmp += xorshift(k->addState);
	uint64_t tmp2 = 0;
	for (uint16_t i 4 = 0; i < k->howManyXors; i++)
		tmp2 ^= xorshift(k->xorState);
	start ^= lastValue;
	start ^= tmp2;
	start -= tmp;
	start ^= generatedRandomNumber;
	start = rotate(start, -k->shiftValue);
	start -= k->addValue;
	start ^= k->xorValue;
	return start;
}
