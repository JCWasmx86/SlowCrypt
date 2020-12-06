#include "shared.h"
#include <assert.h>
#include <math.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static XorShift readXorShift(FILE *fp);
static void printHelp(char *this);
static Key _readKey(FILE* fp);

Key readKey(char *name) {
	FILE *fp = fopen(name, "rb");
	if (fp == NULL) {
		return NULL;
	}
	return _readKey(fp);
}
static Key _readKey(FILE* fp){
	Key key = calloc(1, sizeof(struct _key));
	assert(key != NULL);
	size_t status = fread(&key->xorValue, 1, 8, fp);
	assert(status == 8);
	status = fread(&key->addValue, 1, 8, fp);
	assert(status == 8);
	status = fread(&key->shiftValue, 1, 1, fp);
	assert(status == 1);
	status = fread(&key->firstValue, 1, 8, fp);
	assert(status == 8);
	key->state = readXorShift(fp);
	key->xorState = readXorShift(fp);
	key->addState = readXorShift(fp);
	status = fread(&key->howManyXors, 1, 1, fp);
	assert(status == 1);
	status = fread(&key->howManyBitSets, 1, 2, fp);
	assert(status == 2);
	status = fread(&key->startXorValue, 1, 8, fp);
	assert(status == 8);
	status = fread(&key->howManyAdds, 1, 2, fp);
	assert(status == 2);
	rewind(fp);
	int fd = fileno(fp);
	struct stat buf;
	fstat(fd, &buf);
	off_t size = buf.st_size;
	void *buffer = malloc(size);
	assert(buffer);
	status = fread(buffer, 1, size, fp);
	assert(status == (size_t)size);
	SHA512(buffer, size, (unsigned char *)key->hash);
	free(buffer);
	fclose(fp);
	return key;
}
void releaseKey(Key key) {
	if (!key) {
		return;
	}
	free(key->state->state);
	free(key->xorState->state);
	free(key->addState->state);
	free(key->state);
	free(key->xorState);
	free(key->addState);
	free(key);
}
uint64_t reverse(uint64_t x) {
	long r = 0;
	for (int i = 63; i >= 0; i--) {
		r |= ((x >> i) & 0x1L) << (63 - i);
	}
	return r;
}
static XorShift readXorShift(FILE *fp) {
	XorShift xs = calloc(1, sizeof(struct _xorShift));
	assert(xs);
	size_t status = fread(&xs->maxSize, 1, 8, fp);
	assert(status == 8);
	xs->state = malloc(8 * xs->maxSize);
	assert(xs->state);
	for (uint64_t i = 0; i < xs->maxSize; i++) {
		status = fread(&xs->state[i], 1, 8, fp);
		assert(status == 8);
	}
	status = fread(&xs->index, 1, 8, fp);
	assert(status == 8);
	return xs;
}
uint64_t xorshift(XorShift state) {
	uint64_t index = state->index;
	if (index >= state->maxSize) {
		index = 0;
	}
	const uint64_t s = state->state[index++];
	uint64_t t = state->state[s % state->maxSize];
	t ^= t << 31;
	t ^= t >> 11;
	t ^= s ^ (s >> 30);
	state->state[index >= state->maxSize - 1 ? 0 : index] = t;
	state->index = index == state->maxSize - 1 ? 0 : index;
	return t * (uint64_t)6787578757473601083ul;
}
uint64_t rotate(uint64_t v, int n) {
	n = n & 63U;
	if (n) {
		v = (v >> n) | (v << (64 - n));
	}
	return v;
}
uint64_t reinterpret(int64_t i) { return *((uint64_t *)&i); }
uint64_t generate64BitValue(void) {
	uint64_t upperHalf = ((uint64_t)random()) << 32;
	uint64_t lowerHalf = random();
	uint64_t complete = upperHalf | lowerHalf;
	return reinterpret(complete);
}
void evalArguments(int argc, char **argv, Arguments *arguments) {
	int shouldExit = 0;
	int skip = 0;
	int givenFiles = 0;
	for (int i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (!skip && (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0)) {
			printHelp(argv[0]);
			shouldExit = 1;
		} else if (!skip &&
				   (strcmp(arg, "--version") == 0 || strcmp(arg, "-v") == 0)) {
			printf("%s: Version 0.0.1-Beta\n", argv[0]);
			shouldExit = 1;
		} else if (strcmp(arg, "--") == 0) {
			skip = 1;
		} else if (!skip && memcmp(arg, "--in=", strlen("--in=")) == 0) {
			givenFiles++;
		} else if (!skip && memcmp(arg, "--out=", strlen("--out=")) == 0) {
			givenFiles++;
		} else if (!skip && memcmp(arg, "--key=", strlen("--key=")) == 0) {
			givenFiles++;
		} else if (skip) { // Unconditionally add files after --
			givenFiles++;
		} else if (!skip &&
				   arg[0] !=
					   '-') { // Add a file, if the file doesn't start with '-'.
			givenFiles++;
		} else if (!skip && strcmp(arg, "--memory") == 0) {
			// Nothing
		} else {
			printf("Unknown option: %s\n", arg);
		}
	} // Finished 1. pass of parsing the options
	if (shouldExit)
		exit(0);
	int status = 0;
	for (int i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (arg == NULL) {
			break;
		}
		if (!skip && (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0)) {
			// Nothing
			continue;
		} else if (!skip &&
				   (strcmp(arg, "--version") == 0 || strcmp(arg, "-v") == 0)) {
			// Nothing
			continue;
		} else if (strcmp(arg, "--") == 0) {
			// Nothing
			continue;
		} else if (!skip && memcmp(arg, "--in=", strlen("--in=")) == 0) {
			if (status == 0) {
				arguments->inFile = &arg[4];
				status++;
			} else if (status != 0 && arguments->inFile != NULL) {
				fprintf(stderr, "Overwriting input file: %s. (Now %s)\n",
						arguments->inFile, &arg[4]);
				arguments->inFile = &arg[4];
			} else {
				arguments->inFile = &arg[4];
			}
		} else if (!skip && memcmp(arg, "--out=", strlen("--out=")) == 0) {
			if (status == 1) {
				arguments->outFile = &arg[5];
				status++;
			} else if (status != 1 && arguments->outFile != NULL) {
				fprintf(stderr, "Overwriting output file: %s. (Now %s)\n",
						arguments->outFile, &arg[5]);
				arguments->outFile = &arg[5];
			} else {
				arguments->outFile = &arg[5];
			}
		} else if (!skip && memcmp(arg, "--key=", strlen("--key=")) == 0) {
			if (status == 2) {
				arguments->keyFile = &arg[5];
				status++;
			} else if (status != 2 && arguments->keyFile != NULL) {
				fprintf(stderr, "Overwriting keyfile: %s. (Now %s)\n",
						arguments->keyFile, &arg[5]);
				arguments->keyFile = &arg[5];
			} else {
				arguments->keyFile = &arg[5];
			}
		} else if (!skip && memcmp(arg, "--memory", strlen("--memory")) == 0) {
			arguments->useMemory = 1;
		} else if (!skip && arg[0] != '-') {
			switch (status) {
				case 0:
					if (arguments->inFile != NULL) {
						fprintf(stderr,
								"Overwriting input file: %s. (Now %s)\n",
								arguments->inFile, arg);
					}
					arguments->inFile = arg;
					break;
				case 1:
					if (arguments->outFile != NULL) {
						fprintf(stderr,
								"Overwriting output file: %s. (Now %s)\n",
								arguments->outFile, arg);
					}
					arguments->outFile = arg;
					break;
				case 2:
					if (arguments->keyFile != NULL) {
						fprintf(stderr, "Overwriting keyfile: %s. (Now %s)\n",
								arguments->keyFile, arg);
					}
					arguments->keyFile = arg;
					break;
				default:
					fprintf(stderr, "Don't know, what to do with %s!\n", arg);
					break;
			}
			status++;
		} else if (skip) {
			switch (status) {
				case 0:
					if (arguments->inFile != NULL) {
						fprintf(stderr,
								"Overwriting input file: %s. (Now %s)\n",
								arguments->inFile, &arg[4]);
					}
					arguments->inFile = arg;
					break;
				case 1:
					if (arguments->outFile != NULL) {
						fprintf(stderr,
								"Overwriting output file: %s. (Now %s)\n",
								arguments->outFile, &arg[5]);
					}
					arguments->outFile = arg;
					break;
				case 2:
					if (arguments->keyFile != NULL) {
						fprintf(stderr, "Overwriting keyfile: %s. (Now %s)\n",
								arguments->keyFile, &arg[5]);
					}
					arguments->keyFile = arg;
					break;
				default:
					fprintf(stderr, "Don't know, what to do with %s!\n", arg);
					break;
			}
			status++;
		}
	}
	if (arguments->keyFile == NULL)
		arguments->keyFile = "key.key";
	if (arguments->outFile == NULL || arguments->inFile == NULL) {
		fprintf(stderr, "Didn\'t give enough arguments!\n");
		exit(-1);
	}
}
static void printHelp(char *this) {
	puts(this);
	printf("Usage: %s <optionsOrFiles> \n", this);
	puts("--version -v Print version");
	puts("--help    -h Print this help");
	puts("--in         Set the input file");
	puts("--out        Set the output file");
	puts("--key        Set the key file. (Default key.key)");
	puts("--memory	   Encrypt/decrypt in memory (Leads to more a higher "
		 "usage");
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
		if (toggle) {
			generatedRandomNumber ^= (1L << i);
		}
	}
	start ^= generatedRandomNumber;
	for (uint16_t i = 0; i < k->howManyAdds; i++)
		start += xorshift(k->addState);
	for (uint8_t i = 0; i < k->howManyXors; i++)
		start ^= xorshift(k->xorState);
	start ^= lastValue;
	return reverse(start);
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
	for (uint16_t i = 0; i < k->howManyXors; i++)
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
