#include <stdint.h>

#ifndef _SHARED_H
#define _SHARED_H

typedef struct _xorShift {
	uint64_t *state;
	uint64_t index;
	uint64_t maxSize;
} * XorShift;

typedef struct _key {
	uint64_t xorValue;	 // val^=xorValue
	uint64_t addValue;	 // val+=xorValue
	int8_t shiftValue;	 // shift val(left or right)
	uint64_t firstValue; // if the first value is xored
	XorShift state;		 // To generate a random xor value for a start value
	XorShift xorState;
	XorShift addState;
	uint8_t howManyXors;	 // How many xors with xorState?
	uint16_t howManyBitSets; // How many bits will be toggled from state?
	uint64_t startXorValue;	 // Start for state+howManyXors
	uint16_t howManyAdds;	 // How many random numbers will be added?
} * Key;

uint64_t rotate(uint64_t v, int n);
Key readKey(char *name);
uint64_t xorshift(XorShift state);
XorShift readXorShift(FILE *fp);
uint64_t reverse(uint64_t x);
uint64_t reinterpret(int64_t i);
uint64_t generate64BitValue(void);
#endif
