# Slowcrypt
This is an encryption algorithm that uses a variable key-length.
I don't know, whether it's secure but I don't think so. It's too slow
for nearly all applications (Encryption is around 10KB/s and decryption 
at around 11KB/s). At least the memory usage is very low (Around 64KB).
Slowcrypt uses XOR, unsigned-overflow and xorshift for the encryption.
This algorithm operates on 8-byte-numbers, so it will be padded with zeroes at the end, if the length of the plaintext 
is not a multiple of 8.
## How Slowcrypt works
The plaintext is divided into blocks of 8-bytes, the last block may be extended with zeroes to have this length.
After that following operations are applied to each block:
```c
u64 encrypt(u64 toEncrypt,u64 lastValue,Key key){
	u64 ret=toEncrypt;
	ret^=key->xorValue;
	ret+=key->addValue;
	ret=rotate(ret,key->rotateAmount);//Bitwise rotate, for example 0010->0100
	u64 generated=key->startSeed;
	for(int i=0;i<k->howManyBitSets;i++){
		u8 toggle=!!xorshift(k->state);
		u64 i=xorshift(k->state)%64;
		if(toggle)
			generated^=(1L<<i);//Toggle i'th bit
	}
	ret^=generatedRandomNumber;
	for(uint16_t i=0;i<k->howManyAdds;i++)
		ret+=xorshift(k->addState);
	for(uint8_t i=0;i<k->howManyXors;i++)
		ret^=xorshift(k->xorState);
	start^=lastValue;//Last encrypted value, or a value from the key.
	return reverse(start);//Reverse bits, 10101011==>11010101
}
```
# Disclaimer
I wouldn't recommend to use this algorithm to encrypt important data, because Slowcrypt may be unsecure.
Use something like AES-256 instead.
