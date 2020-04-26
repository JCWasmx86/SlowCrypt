#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
typedef struct _xorShift {
	uint64_t *state;
	uint64_t index;
	uint64_t maxSize;
}*XorShift;
typedef struct _key{
	uint64_t xorValue;//val^=xorValue
	uint64_t addValue;//val+=xorValue
	int8_t shiftValue;//shift val(left or right)
	uint64_t firstValue;//if the first value is xored
	XorShift state;//To generate a random xor value for a start value
	XorShift xorState;
	XorShift addState;
	uint8_t howManyXors;//How many xors with xorState?
	uint16_t howManyBitSets;//How many bits will be toggled from state
	uint64_t startXorValue;//Start for state+howManyXors
	uint16_t howManyAdds;//How many random numbers will be added?
}*Key;
uint64_t reinterpret(int64_t i);
int main(int argc,char** argv){
	char* line=NULL;
	uint64_t len=0;
	srandom(time(NULL));
	FILE *fp =fopen("key.key","wb");
	uint64_t xorV=reinterpret(((((uint64_t)random())<<32|((uint64_t)random()))));
	fwrite(&xorV,8,1,fp);
	uint64_t addV=reinterpret(((((uint64_t)random())<<32|((uint64_t)random()))));
	fwrite(&addV,8,1,fp);
	int8_t shiftValue=random()%64*(random()&1?-1:1);
	fwrite(&shiftValue,1,1,fp);
	uint64_t firstValue=reinterpret(((((uint64_t)random())<<32|((uint64_t)random()))));
	fwrite(&firstValue,8,1,fp);
	for(int i=0;i<3;i++){
		uint64_t maxSize=reinterpret(((((uint64_t)random())<<32|((uint64_t)random()))))&0xFF;
		maxSize=maxSize<4?4:maxSize;
		fwrite(&maxSize,8,1,fp);
		for(uint64_t i=0;i<maxSize;i++){
			uint64_t a=reinterpret(((((uint64_t)random())<<32|((uint64_t)random()))));
			fwrite(&a,8,1,fp);
		}
		uint64_t index=reinterpret(((((uint64_t)random())<<32|((uint64_t)random()))))%maxSize;
		fwrite(&index,8,1,fp);
	}
	uint8_t xorC=random()%256;
	fwrite(&xorC,1,1,fp);
	uint16_t bitC=random()&0xFFFF;
	fwrite(&bitC,2,1,fp);
	uint64_t sX=reinterpret(((((uint64_t)random())<<32|((uint64_t)random()))));
	fread(&sX,8,1,fp);
	uint16_t howManyAdds=random()&0xFFFF;
	fwrite(&howManyAdds,2,1,fp);
	fclose(fp);
	return 0;
}
uint64_t reinterpret(int64_t i){
	return *((uint64_t*)&i);
}
