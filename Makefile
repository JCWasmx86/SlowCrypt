all:
	mkdir -p bin/obj
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/encrypt.o src/encrypt.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/decrypt.o src/decrypt.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/shared.o src/shared.c
	$(CC) $(CFLAGS) -g3 -c -o bin/obj/keygen.o src/keygen.c
	$(CC) -o bin/encrypt bin/obj/encrypt.o bin/obj/shared.o
	$(CC) -o bin/decrypt bin/obj/decrypt.o bin/obj/shared.o
	$(CC) -o bin/keygen bin/obj/keygen.o
clean:
	rm -rf bin
