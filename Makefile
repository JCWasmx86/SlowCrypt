CFLAGS+=-Wall -Wextra -pedantic -Wno-unused-parameter
ifndef PREFIX
export PREFIX=/usr/local
endif
all:
	mkdir -p bin/obj
	$(CC) $(CFLAGS) -fanalyzer -march=native -O3 -s -I"include" -c -o bin/obj/encrypt.o src/encrypt.c
	$(CC) $(CFLAGS) -fanalyzer -march=native -O3 -s -I"include" -c -o bin/obj/decrypt.o src/decrypt.c
	$(CC) $(CFLAGS) -fanalyzer -march=native -O3 -s -I"include" -c -o bin/obj/shared.o src/shared.c
	$(CC) $(CFLAGS) -fanalyzer -march=native -O3 -s -I"include" -c -o bin/obj/keygen.o src/keygen.c
	$(CC) $(CFLAGS) -fanalyzer -march=native -O3 -s -I"include" -c -o bin/obj/keydump.o src/keydump.c
	$(CC) $(CFLAGS) -fanalyzer -march=native -O3 -s -I"include" -c -o bin/obj/pwd2key.o src/pwd2key.c
	$(CC) -o bin/encrypt bin/obj/encrypt.o bin/obj/shared.o $(LIBS) -lcrypto
	$(CC) -o bin/decrypt bin/obj/decrypt.o bin/obj/shared.o $(LIBS) -lcrypto
	$(CC) -o bin/keygen bin/obj/keygen.o bin/obj/shared.o $(LIBS) -lcrypto
	$(CC) -o bin/keydump bin/obj/keydump.o bin/obj/shared.o $(LIBS) -lcrypto
	$(CC) -o bin/pwd2key bin/obj/pwd2key.o bin/obj/shared.o $(LIBS) -lcrypto
all-debug:
	mkdir -p bin/obj
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/encrypt.o src/encrypt.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/decrypt.o src/decrypt.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/shared.o src/shared.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/keygen.o src/keygen.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/keydump.o src/keydump.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/pwd2key.o src/pwd2key.c
	$(CC) -o bin/encrypt bin/obj/encrypt.o bin/obj/shared.o $(LIBS) -lcrypto
	$(CC) -o bin/decrypt bin/obj/decrypt.o bin/obj/shared.o $(LIBS) -lcrypto
	$(CC) -o bin/keygen bin/obj/keygen.o bin/obj/shared.o $(LIBS) -lcrypto
	$(CC) -o bin/keydump bin/obj/keydump.o bin/obj/shared.o $(LIBS) -lcrypto
	$(CC) -o bin/pwd2key bin/obj/pwd2key.o bin/obj/shared.o $(LIBS) -lcrypto
all-noassert:
	mkdir -p bin/obj
	$(CC) $(CFLAGS) -DNDEBUG -fanalyzer -march=native -O3 -s -I"include" -c -o bin/obj/encrypt.o src/encrypt.c
	$(CC) $(CFLAGS) -DNDEBUG -fanalyzer -march=native -O3 -s -I"include" -c -o bin/obj/decrypt.o src/decrypt.c
	$(CC) $(CFLAGS) -DNDEBUG -fanalyzer -march=native -O3 -s -I"include" -c -o bin/obj/shared.o src/shared.c
	$(CC) $(CFLAGS) -DNDEBUG -fanalyzer -march=native -O3 -s -I"include" -c -o bin/obj/keygen.o src/keygen.c
	$(CC) $(CFLAGS) -DNDEBUG -fanalyzer -march=native -O3 -s -I"include" -c -o bin/obj/keydump.o src/keydump.c
	$(CC) $(CFLAGS) -DNDEBUG -fanalyzer -march=native -O3 -s -I"include" -c -o bin/obj/pwd2key.o src/pwd2key.c
	$(CC) -o bin/encrypt bin/obj/encrypt.o bin/obj/shared.o $(LIBS) -lcrypto
	$(CC) -o bin/decrypt bin/obj/decrypt.o bin/obj/shared.o $(LIBS) -lcrypto
	$(CC) -o bin/keygen bin/obj/keygen.o bin/obj/shared.o $(LIBS) -lcrypto
	$(CC) -o bin/keydump bin/obj/keydump.o bin/obj/shared.o $(LIBS) -lcrypto
	$(CC) -o bin/pwd2key bin/obj/pwd2key.o bin/obj/shared.o $(LIBS) -lcrypto
clean:
	rm -rf bin
install: all-noassert
	cp bin/encrypt $(PREFIX)/bin/encrypt
	cp bin/decrypt $(PREFIX)/bin/decrypt
	cp bin/keygen $(PREFIX)/bin/keygen
	cp bin/keydump $(PREFIX)/bin/keydump
	cp bin/pwd2key $(PREFIX)/bin/pwd2key
uninstall:
	rm $(PREFIX)/bin/encrypt
	rm $(PREFIX)/bin/decrypt
	rm $(PREFIX)/bin/keygen
	rm $(PREFIX)/bin/keydump
	rm $(PREFIX)/bin/pwd2key
format:
	find src include -iname *.h -o -iname *.c | xargs clang-format --style=file -verbose -i
