#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
uint64_t reinterpret(int64_t i);
int main(int argc,char** argv){
	char* line=NULL;
	uint64_t len=0;
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
