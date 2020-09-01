CFLAGS+=-Wall -Wextra -pedantic -Wno-unused-parameter
all:
	mkdir -p bin/obj
	$(CC) $(CFLAGS) -march=native -O3 -s -I"include" -c -o bin/obj/encrypt.o src/encrypt.c
	$(CC) $(CFLAGS) -march=native -O3 -s -I"include" -c -o bin/obj/decrypt.o src/decrypt.c
	$(CC) $(CFLAGS) -march=native -O3 -s -I"include" -c -o bin/obj/shared.o src/shared.c
	$(CC) $(CFLAGS) -march=native -O3 -s -I"include" -c -o bin/obj/keygen.o src/keygen.c
	$(CC) -o bin/encrypt bin/obj/encrypt.o bin/obj/shared.o
	$(CC) -o bin/decrypt bin/obj/decrypt.o bin/obj/shared.o 
	$(CC) -o bin/keygen bin/obj/keygen.o bin/obj/shared.o  
all-debug:
	mkdir -p bin/obj
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/encrypt.o src/encrypt.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/decrypt.o src/decrypt.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/shared.o src/shared.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/keygen.o src/keygen.c
	$(CC) -o bin/encrypt bin/obj/encrypt.o bin/obj/shared.o
	$(CC) -o bin/decrypt bin/obj/decrypt.o bin/obj/shared.o
	$(CC) -o bin/keygen bin/obj/keygen.o bin/obj/shared.o 
clean:
	rm -rf bin
install: all
	cp bin/encrypt /usr/local/bin/encrypt
	cp bin/decrypt /usr/local/bin/decrypt
	cp bin/keygen /usr/local/bin/keygen
