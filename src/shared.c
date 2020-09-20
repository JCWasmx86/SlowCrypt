#include "shared.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
Key readKey(char *name) {
	FILE *fp = fopen(name, "rb");
	if (fp == NULL) {
		return NULL;
	}
	Key key = calloc(1, sizeof(struct _key));
	assert(key != NULL);
	assert(fread(&key->xorValue, 1, 8, fp) == 8);
	assert(fread(&key->addValue, 1, 8, fp) == 8);
	assert(fread(&key->shiftValue, 1, 1, fp) == 1);
	assert(fread(&key->firstValue, 1, 8, fp) == 8);
	key->state = readXorShift(fp);
	key->xorState = readXorShift(fp);
	key->addState = readXorShift(fp);
	assert(fread(&key->howManyXors, 1, 1, fp) == 1);
	assert(fread(&key->howManyBitSets, 1, 2, fp) == 2);
	assert(fread(&key->startXorValue, 1, 8, fp) == 8);
	assert(fread(&key->howManyAdds, 1, 2, fp) == 2);
	fclose(fp);
	return key;
}
uint64_t reverse(uint64_t x) {
	long r = 0;
	for (int i = 63; i >= 0; i--) {
		r |= ((x >> i) & 0x1L) << (63 - i);
	}
	return r;
}
XorShift readXorShift(FILE *fp) {
	XorShift xs = calloc(1, sizeof(struct _xorShift));
	assert(xs);
	assert(fread(&xs->maxSize, 1, 8, fp) == 8);
	xs->state = malloc(8 * xs->maxSize);
	assert(xs->state);
	for (uint64_t i = 0; i < xs->maxSize; i++)
		assert(fread(&xs->state[i], 1, 8, fp) == 8);
	assert(fread(&xs->index, 1, 8, fp) == 8);
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
