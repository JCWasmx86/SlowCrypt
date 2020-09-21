#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"

static void dumpXorShift(XorShift shift);
static void processArgs(int argc, char **argv);
static void printHelp(char *this);
static void printKey(char *fileName);

int main(int argc, char **argv) {
	processArgs(argc, argv);
	int numberOfKeysPrinted = 0;
	int canSkip = 0; // After -- argument
	for (int i = 1; i < argc; i++) {
		char *filename = argv[i];
		if (strcmp("--", filename) == 0) {
			canSkip = 1;
			continue;
		}
		if (!canSkip &&
			filename[0] == '-') { // Skip options, but only before --
			continue;
		}
		numberOfKeysPrinted++;
		printKey(filename);
	}
	if (numberOfKeysPrinted == 0) {
		printKey("key.key");
	}
	return EXIT_SUCCESS;
}
static void printKey(char *filename) {
	Key key = readKey(filename);
	if (key == NULL) {
		fprintf(stderr, "Couldn't open key: %s\n", filename);
		return;
	}
	printf("Dumping key: %s\n", filename);
	printf("xorValue: %" PRIu64 "\n", key->xorValue);
	printf("addValue: %" PRIu64 "\n", key->addValue);
	printf("shiftValue: %" PRId8 "\n", key->shiftValue);
	printf("firstValue: %" PRIu64 "\n", key->firstValue);
	dumpXorShift(key->state);
	dumpXorShift(key->xorState);
	dumpXorShift(key->addState);
	printf("howManyXors: %" PRIu8 "\n", key->howManyXors);
	printf("howManyBitSets: %" PRIu16 "\n", key->howManyBitSets);
	printf("addValue: %" PRIu64 "\n", key->startXorValue);
	printf("howManyAdds: %" PRIu16 "\n", key->howManyAdds);
	releaseKey(key);
}
static void dumpXorShift(XorShift shift) {
	printf("Xorstate: %" PRIu64 " states\n", shift->maxSize);
	for (uint64_t i = 0; i < shift->maxSize; i++) {
		printf("\t state[%" PRIu64 "] = %" PRIu64, i, shift->state[i]);
		if (i == shift->index) {
			printf(" < start index");
		}
		printf("\n");
	}
}

static void processArgs(int argc, char **argv) {
	int shouldExit = 0;
	for (int i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (strcmp("--", arg) == 0) {
			break;
		} else if (strcmp("--version", arg) == 0 || strcmp("-v", arg) == 0) {
			printf("%s: Version 0.0.1-Beta\n", argv[0]);
			shouldExit = 1;
		} else if (strcmp("--help", arg) == 0 || strcmp("-h", arg) == 0) {
			printHelp(argv[0]);
			shouldExit = 1;
		}
	}
	if (shouldExit)
		exit(0);
}
static void printHelp(char *this) {
	puts(this);
	printf("Usage: %s <outputFile>* (Default: key.key) \n", this);
	puts("--version -v Print version");
	puts("--help    -h Print this help");
}
