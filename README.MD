# Slowcrypt
This is an encryption algorithm that uses a variable key-length.
It's too slow for nearly all applications (Encryption is around 6-7KB/s and decryption 
at around 7-10KB/s). At least the memory usage is very low (Around 1MB).
Slowcrypt uses XOR,xorshift and exploits unsigned-overflow for the encryption.
This algorithm operates on 8-byte-numbers, so it will be padded with zeroes at the end, if the length of the plaintext 
is not a multiple of 8.
## How Slowcrypt works
The plaintext is divided into blocks of 8-bytes, the last block may be padded with zeroes to have 8 bytes.
After that multiple transformations are applied to each block. (See [here](src/encrypt.c))
# Usage
1. Execute keygen to generate a key: `keygen`
2. Use encrypt to encrypt the file: `encrypt <inputFile> <outputFile> <keyFile>`
3. Use decrypt to decrypt the file: `decrypt <inputFile> <outputFile> <keyFile>`


If you don't  give a keyfile to `encrypt` or `decrypt`, `key.key` is assumed implicitly.
# TODO
- [ ] Improve performance
- [x] Add the possibility of generating a key from a password
- [ ] Improve documentation

# Disclaimer
Don't use this algorithm for encrypting important data.
Anyone who has experience with cryptography, will probably break SlowCrypt in less than a weekend.
Use a battle-hardened, tested algorithm like AES-256 instead.
