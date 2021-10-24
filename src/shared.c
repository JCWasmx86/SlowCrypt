#include "shared.h"
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
static Key _readKey(FILE *fp);
static uint64_t encrypt_(uint64_t v, uint64_t lastValue, Key k);
static uint64_t decrypt_(uint64_t v, uint64_t lastValue, Key k);
static void *readIntoBuffer(FILE *fp, uint64_t *size, int *numZeroes);

Key readKey(char *name) {
	FILE *fp = fopen(name, "rb");
	if (fp == NULL) {
		return NULL;
	}
	return _readKey(fp);
}
static Key _readKey(FILE *fp) {
	Key key = calloc(1, sizeof(struct _key));
	if (key == NULL) {
		return NULL;
	}
	size_t status = fread(&key->xorValue, 1, 8, fp);
	if (status != 8)
		goto end;
	status = fread(&key->addValue, 1, 8, fp);
	if (status != 8)
		goto end;
	status = fread(&key->shiftValue, 1, 1, fp);
	if (status != 1)
		goto end;
	status = fread(&key->firstValue, 1, 8, fp);
	if (status != 8)
		goto end;
	key->state = readXorShift(fp);
	if (!key->state) {
		goto end;
	}
	key->xorState = readXorShift(fp);
	if (!key->xorState) {
		free(key->state->state);
		free(key->state);
		goto end;
	}
	key->addState = readXorShift(fp);
	if (!key->addState) {
		free(key->xorState->state);
		free(key->state->state);
		free(key->state);
		free(key->xorState);
		goto end;
	}
	status = fread(&key->howManyXors, 1, 1, fp);
	if (status != 1)
		goto end2;
	status = fread(&key->howManyBitSets, 1, 2, fp);
	if (status != 2)
		goto end2;
	status = fread(&key->startXorValue, 1, 8, fp);
	if (status != 8)
		goto end2;
	status = fread(&key->howManyAdds, 1, 2, fp);
	if (status != 2)
		goto end2;
	rewind(fp);
	int fd = fileno(fp);
	struct stat buf;
	int retValue = fstat(fd, &buf);
	if (retValue == -1)
		goto end2;
	off_t size = buf.st_size;
	void *buffer = malloc(size);
	if (buffer == NULL)
		goto end2;
	status = fread(buffer, 1, size, fp);
	if (status != (off_t)size)
		goto end3;
	SHA512(buffer, size, (unsigned char *)key->hash);
	free(buffer);
	fclose(fp);
	return key;
end3:
	free(buffer);
end2:
	free(key->xorState->state);
	free(key->addState->state);
	free(key->state->state);
	free(key->state);
	free(key->xorState);
	free(key->addState);
end:
	free(key);
	fclose(fp);
	return NULL;
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
	if (!xs)
		return NULL;
	size_t status = fread(&xs->maxSize, 1, 8, fp);
	if (status != 8)
		goto end;
	xs->state = malloc(8 * xs->maxSize);
	if (!xs->state)
		goto end;
	for (uint64_t i = 0; i < xs->maxSize; i++) {
		status = fread(&xs->state[i], 1, 8, fp);
		if (status != 8) {
			free(xs->state);
			goto end;
		}
	}
	status = fread(&xs->index, 1, 8, fp);
	if (status != 8) {
		free(xs->state);
		goto end;
	}
	return xs;
end:
	free(xs);
	return NULL;
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
		exit(EXIT_SUCCESS);
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
static uint64_t encrypt_(uint64_t v, uint64_t lastValue, Key k) {
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
static uint64_t decrypt_(uint64_t v, uint64_t lastValue, Key k) {
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
int encrypt(FILE *toEncrypt, FILE *encrypted, Key key) {
	if (toEncrypt == NULL || encrypted == NULL || key == NULL) {
		return 1;
	}
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
			encrypt_(read, cnt == 0 ? key->firstValue : lastValue, key);
		fwrite(&encryptedInt, 8, 1, encrypted);
		cnt++;
		lastValue = encryptedInt;
	}
	rewind(encrypted);
	fputc(paddedZeroes, encrypted);
	return 0;
}
int decrypt(FILE *toDecrypt, FILE *clearText, Key key) {
	if (toDecrypt == NULL || clearText == NULL || key == NULL) {
		return 1;
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
	int index = 0;
	while (1) {
		char rawBytes[8];
		int readBytes = fread(rawBytes, 1, 8, toDecrypt);
		if (readBytes == 0) {
			break;
		}
		uint64_t read = *((uint64_t *)rawBytes);
		uint64_t decrypted =
			decrypt_(read, cnt == 0 ? key->firstValue : lastValue, key);
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
	return 0;
}
int encryptInMemory(FILE *toEncrypt, FILE *encrypted, Key key) {
	if (toEncrypt == NULL || encrypted == NULL || key == NULL) {
		return 1;
	}
	uint64_t size = 0;
	int paddedZeroes = 0;
	uint64_t *buffer =
		(uint64_t *)readIntoBuffer(toEncrypt, &size, &paddedZeroes);
	if (!buffer)
		return 1;
	uint64_t *backupBuffer = buffer;
	uint64_t numberOfBlocks = size / 8;
	size += 8; // For the first leading 8 bytes
	uint8_t *bufferToWriteTo = calloc(size, 1);
	if (!bufferToWriteTo) {
		free(buffer);
		return 1;
	}
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
			encrypt_(read, cnt == 0 ? key->firstValue : lastValue, key);
		memcpy(bufferToWriteTo, &encryptedInt, 8);
		bufferToWriteTo += 8;
		cnt++;
		lastValue = encryptedInt;
	}
	size_t num = fwrite(base, 1, (numberOfBlocks * 8 + 8), encrypted);
	if (num != (numberOfBlocks * 8 + 8)) {
		return 1;
	}
	fflush(encrypted);
	free(base);
	free(backupBuffer);
	return 0;
}
int decryptInMemory(FILE *toDecrypt, FILE *decrypted, Key key) {
	if (toDecrypt == NULL || decrypted == NULL || key == NULL) {
		return 1;
	}
	uint64_t size = 0;
	int nZ = 0;
	uint64_t *buffer = (uint64_t *)readIntoBuffer(toDecrypt, &size, &nZ);
	if (!buffer)
		return 1;
	else if (nZ != 0) {
		free(buffer);
		return 1;
	}
	uint64_t *backupBuffer = buffer;
	uint8_t *bufferToWriteTo = calloc(size, 1);
	if (!bufferToWriteTo) {
		free(buffer);
		return 1;
	}
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
			decrypt_(read, cnt == 0 ? key->firstValue : lastValue, key);
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
	size_t toWrite = (cnt - 1) * 8 + (8 - paddedZeroes);
	size_t num = fwrite(bufferToWriteTo, 1, toWrite, decrypted);
	free(bufferToWriteTo);
	free(backupBuffer);
	return num != toWrite;
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
			if (!ptr) {
				free(ret);
				return NULL;
			}
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
	readBytes += 8;
	return readBytes != 0 ? realloc(ret, readBytes) : ret;
}
