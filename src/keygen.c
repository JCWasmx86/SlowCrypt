#include "shared.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *processArgs(int argc, char **argv);
static void printHelp(char *this);

int main(int argc, char **argv) {
	const char *keyFile = processArgs(argc, argv);
	FILE *r = fopen("/dev/urandom", "rb");
	uint32_t seed = 0;
	if (r == NULL) {
		fprintf(stderr, "Couldn't open /dev/urandom, falling back to rand()");
		seed = rand();
	} else {
		fread(&seed, 4, 1, r);
		fclose(r);
	}
	srandom(seed);
	FILE *fp = fopen(keyFile == NULL ? "key.key" : keyFile, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Couldn't open %s for writing!\n",
				keyFile == NULL ? "key.key" : keyFile);
		return -1;
	}
	// Each block is XOR'd with this value
	uint64_t xorV = generate64BitValue();
	fwrite(&xorV, 8, 1, fp);
	// To each block, this value is added ==>Exploiting Overflow
	uint64_t addV = generate64BitValue();
	fwrite(&addV, 8, 1, fp);
	// Rotate the bits in the block
	int8_t shiftValue = random() % 64 * (random() & 1 ? -1 : 1);
	fwrite(&shiftValue, 1, 1, fp);
	// If the first block is encrypted, use this value for XOR
	uint64_t firstValue = generate64BitValue();
	fwrite(&firstValue, 8, 1, fp);
	for (int i = 0; i < 3; i++) {
		uint64_t maxSize = generate64BitValue() & 0xFFFF;
		maxSize = maxSize < 4 ? 4 : maxSize;
		fwrite(&maxSize, 8, 1, fp);
		for (uint64_t i = 0; i < maxSize; i++) {
			uint64_t a = generate64BitValue();
			fwrite(&a, 8, 1, fp);
		}
		uint64_t index = generate64BitValue() % maxSize;
		fwrite(&index, 8, 1, fp);
	}
	uint8_t xorC = random() & 0xFF;
	fwrite(&xorC, 1, 1, fp);
	uint16_t bitC = random() & 0xFFFF;
	fwrite(&bitC, 1, 2, fp);
	uint64_t sX = generate64BitValue();
	fwrite(&sX, 1, 8, fp);
	uint16_t howManyAdds = random() & 0xFFFF;
	fwrite(&howManyAdds, 2, 1, fp);
	fclose(fp);
	return 0;
}

static const char *processArgs(int argc, char **argv) {
	int shouldExit = 0;
	char *currentKeyFile = NULL;
	for (int i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (strcmp("--version", arg) == 0 || strcmp("-v", arg) == 0) {
			printf("%s: Version 0.0.1-Beta\n", argv[0]);
			shouldExit = 1;
		} else if (strcmp("--help", arg) == 0 || strcmp("-h", arg) == 0) {
			printHelp(argv[0]);
			shouldExit = 1;
		} else {
			if (currentKeyFile != NULL) {
				fprintf(stderr, "Warning: Changing keyfile to %s (from %s)\n",
						arg, currentKeyFile);
			}
			currentKeyFile = arg;
		}
	}
	if (shouldExit)
		exit(0);
	return currentKeyFile;
}

static void printHelp(char *this) {
	puts(this);
	printf("Usage: %s <options>* <outputFile>? (Default: key.key) \n", this);
	puts("--version -v Print version");
	puts("--help    -h Print this help");
	puts("Everything else: The last keyfile specified is written:");
	printf(
		"\"%s a.key b.key\"  will write a keyfile called \"b.key\" and discard "
		"the name \"a.key\"\n",
		this);
}
