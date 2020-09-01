# Slowcrypt
This is an encryption algorithm that uses a variable key-length.
It's too slow for nearly all applications (Encryption is around 30KB/s and decryption 
at around 35KB/s). At least the memory usage is very low (Around 1MB).
Slowcrypt uses XOR,xorshift and exploits unsigned-overflow for the encryption.
This algorithm operates on 8-byte-numbers, so it will be padded with zeroes at the end, if the length of the plaintext 
is not a multiple of 8.
## How Slowcrypt works
The plaintext is divided into blocks of 8-bytes, the last block may be padded with zeroes to have 8 bytes.
After that multiple transformations are applied to each block. (See [here](src/encrypt.c))
# Usage
1. Execute keygen to generate a key. (key.key file)
2. Use encrypt to encrypt the file (encrypt <inFile> <outFile> [<keyfile>(default: key.key)]
2. Use decrypt to decrypt the file (decrypt <inFile> <outFile> [<keyfile>(default: key.key)]

# TODO
- [] Improve performance
- [] Add the possibility of generating a key from a password
- [] Improve documentation

# Disclaimer
I wouldn't recommend to use this algorithm to encrypt important data, because Slowcrypt may be unsecure.
Use something like AES-256 instead.
