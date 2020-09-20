#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "shared.h"

void dumpXorShift(XorShift shift);

int main(int argc, char **argv) {
	char *filename = argc > 2 ? argv[1] : "key.key";
	Key key = readKey(filename);
	if (key == NULL) {
		fprintf(stderr, "Couldn't open key: %s\n", filename);
		return EXIT_FAILURE;
	}
	printf("xorValue: %" PRIu64 "\n", key->xorValue);
	printf("addValue: %" PRIu64 "\n", key->addValue);
	printf("shiftValue: %" PRId8 "\n", key->shiftValue);
	printf("firstValue: %" PRIu64 "\n", key->firstValue);
	// TODO
	dumpXorShift(key->state);
	dumpXorShift(key->xorState);
	dumpXorShift(key->addState);
	printf("howManyXors: %" PRIu8 "\n", key->howManyXors);
	printf("howManyBitSets: %" PRIu16 "\n", key->howManyBitSets);
	printf("addValue: %" PRIu64 "\n", key->startXorValue);
	printf("howManyAdds: %" PRIu16 "\n", key->howManyAdds);
	return EXIT_SUCCESS;
}

void dumpXorShift(XorShift shift) {
	printf("Xorstate: %" PRIu64 " states\n", shift->maxSize);
	for (uint64_t i = 0; i < shift->maxSize; i++) {
		printf("\t state[%" PRIu64 "] = %" PRIu64 "\n", i, shift->state[i]);
	}
}
