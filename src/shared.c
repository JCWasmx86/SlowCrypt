#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <openssl/sha.h>
#include "shared.h"
Key readKey(char* name){
	FILE* fp=fopen(name,"rb");
	if(fp==NULL)
		return NULL;
	Key key=calloc(1,sizeof(struct _key));
	assert(key!=NULL);
	fread(&key->xorValue,8,1,fp);
	fread(&key->addValue,8,1,fp);
	fread(&key->shiftValue,1,1,fp);
	fread(&key->firstValue,8,1,fp);
	key->state=readXorShift(fp);
	key->xorState=readXorShift(fp);
	key->addState=readXorShift(fp);
	fread(&key->howManyXors,1,1,fp);
	fread(&key->howManyBitSets,2,1,fp);
	fread(&key->startXorValue,8,1,fp);
	fread(&key->howManyAdds,2,1,fp);
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
XorShift readXorShift(FILE *fp){
	XorShift xs=calloc(1,sizeof(struct _xorShift));
	assert(xs!=NULL);
	fread(&xs->maxSize,8,1,fp);
	xs->state=malloc(8*xs->maxSize);
	assert(xs->state);
	for(uint64_t i=0;i<xs->maxSize;i++)
		fread(&xs->state[i],8,1,fp);
	fread(&xs->index,8,1,fp);
	return xs;
}
uint64_t xorshift(XorShift state){
	uint64_t index = state->index;
	if(index>=state->maxSize)
		index=0;
	const uint64_t s = state->state[index++];
	uint64_t t = state->state[s%state->maxSize];
	t ^= t << 31;
	t ^= t >> 11;
	t ^= s ^ (s >> 30);
	state->state[index>=state->maxSize-1?0:index] = t;
	state->index = index==state->maxSize-1?0:index;
	return t * (uint64_t)6787578757473601083ul;
}
uint64_t rotate(uint64_t v, int n) {
    n = n & 63U;
    if (n)
        v = (v >> n) | (v << (64-n));
    return v;
}
void* hash(const char* password){
	uint8_t* ptr=calloc(HASH_LENGTH,1);
	size_t len=strlen(password);
	assert(len>3);
	memcpy(ptr,password,len);
	//Stretch the init array to 86 bytes.
	for(size_t i=len;i<HASH_LENGTH;i++){
		ptr[i]=((uint64_t)pow((ptr[i-1]^ptr[i-2]),ptr[i-3]))&0xFF;
	}
	unsigned char hash[SHA256_DIGEST_LENGTH];
	for(int i=0;i<HASH_LENGTH;i++){
		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, &ptr[i], len-i);
		SHA256_Final(hash, &sha256);
		for(int idx=i;i<SHA256_DIGEST_LENGTH;i++){
			ptr[idx%HASH_LENGTH]^=hash[idx-i];
		}
	}
	memset(hash,0,SHA256_DIGEST_LENGTH);
	return ptr;
}
uint64_t reinterpret(int64_t i){
	return *((uint64_t*)&i);
}
uint64_t generate64BitValue(void){
	uint64_t upperHalf=((uint64_t)random())<<32;
	uint64_t lowerHalf=random();
	uint64_t complete=upperHalf|lowerHalf;
	return reinterpret(complete);
}
