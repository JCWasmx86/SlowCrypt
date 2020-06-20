#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "shared.h"
int main(int argc,char** argv){
	FILE* r=fopen("/dev/urandom","rb");
	uint32_t seed=0;
	if(r==NULL){
		fprintf(stderr,"Couldn't open /dev/urandom, falling back to rand()");
		seed=rand();
	}else{
		fread(&seed,4,1,r);
		fclose(r);
	}
	srandom(seed);
	FILE *fp =fopen(argv[1]==NULL?"key.key":argv[1],"wb");
	uint64_t xorV=generate64BitValue();//8
	fwrite(&xorV,8,1,fp);
	uint64_t addV=generate64BitValue();//16
	fwrite(&addV,8,1,fp);
	int8_t shiftValue=random()%64*(random()&1?-1:1);//17
	fwrite(&shiftValue,1,1,fp);
	uint64_t firstValue=generate64BitValue();//25
	fwrite(&firstValue,8,1,fp);
	for(int i=0;i<3;i++){
		uint64_t maxSize=generate64BitValue()&0xFFFF;
		maxSize=maxSize<4?4:maxSize;
		fwrite(&maxSize,8,1,fp);
		for(uint64_t i=0;i<maxSize;i++){
			uint64_t a=generate64BitValue();
			fwrite(&a,8,1,fp);
		}
		uint64_t index=generate64BitValue()%maxSize;
		fwrite(&index,8,1,fp);
	}
	uint8_t xorC=random()&0xFF;//26
	fwrite(&xorC,1,1,fp);
	uint16_t bitC=random()&0xFFFF;//28
	fwrite(&bitC,2,1,fp);
	uint64_t sX=generate64BitValue();//36
	fread(&sX,8,1,fp);
	uint16_t howManyAdds=random()&0xFFFF;//38
	fwrite(&howManyAdds,2,1,fp);
	fclose(fp);
	return 0;
}
