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
1. Execute keygen to generate a key: `keygen` for a random key or `pwd2key` for creating a key from a password.
2. Use encrypt to encrypt the file: `encrypt <inputFile> <outputFile> <keyFile>`
3. Use decrypt to decrypt the file: `decrypt <inputFile> <outputFile> <keyFile>`


If you don't  give a keyfile to `encrypt` or `decrypt`, `key.key` is assumed implicitly.
# TODO
- [x] Improve performance (Build with `make all-noassert`)
- [x] Add the possibility of generating a key from a password
- [ ] Improve documentation
- [ ] Replace asserts with better alternatives

# Compiling
## Prerequisites
SlowCrypt uses [`OpenSSL`](https://github.com/openssl/openssl/blob/master/LICENSE.txt) (Apache 2.0 License) for some hash functions.
Furthermore, at least `gcc` (Version 10 and upwards) is required for compiling.
## Build
- `make all` builds SlowCrypt with compiler optimizations and without debug infos.
- `make all-debug` builds SlowCrypt for debugging.
- `make all-noassert` is the same as `make all`, except `assert` is disabled.
- `make clean` removes the `bin` directory.
- `make format` formats the code.
- `make install` installs the code into `$PREFIX` (Default: `/usr/local`)
## Test
Use the supplied test-script `test`.
# Disclaimer
Don't use this algorithm for encrypting important data.
Anyone who has experience with cryptography, will probably break SlowCrypt in less than a weekend.
Use a battle-hardened, tested algorithm like AES-256 instead.
