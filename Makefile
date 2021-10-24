CFLAGS+=-Wall -Wextra -pedantic -Wno-unused-parameter
LIBS+= -lcrypto
ifndef PREFIX
export PREFIX=/usr/local
endif
all:
	mkdir -p bin/obj
	$(CC) $(CFLAGS) -O3 -I"include" -c -o bin/obj/encrypt.o src/encrypt.c
	$(CC) $(CFLAGS) -O3 -I"include" -c -o bin/obj/decrypt.o src/decrypt.c
	$(CC) $(CFLAGS) -O3 -I"include" -c -o bin/obj/shared.o src/shared.c
	$(CC) $(CFLAGS) -O3 -I"include" -c -o bin/obj/keygen.o src/keygen.c
	$(CC) $(CFLAGS) -O3 -I"include" -c -o bin/obj/keydump.o src/keydump.c
	$(CC) $(CFLAGS) -O3 -I"include" -c -o bin/obj/pwd2key.o src/pwd2key.c
	$(CC) -o bin/encrypt bin/obj/encrypt.o bin/obj/shared.o $(LIBS)
	$(CC) -o bin/decrypt bin/obj/decrypt.o bin/obj/shared.o $(LIBS)
	$(CC) -o bin/keygen bin/obj/keygen.o bin/obj/shared.o $(LIBS)
	$(CC) -o bin/keydump bin/obj/keydump.o bin/obj/shared.o $(LIBS)
	$(CC) -o bin/pwd2key bin/obj/pwd2key.o bin/obj/shared.o $(LIBS)
all-debug:
	mkdir -p bin/obj
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/encrypt.o src/encrypt.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/decrypt.o src/decrypt.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/shared.o src/shared.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/keygen.o src/keygen.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/keydump.o src/keydump.c
	$(CC) $(CFLAGS) -g3 -I"include" -c -o bin/obj/pwd2key.o src/pwd2key.c
	$(CC) -o bin/encrypt bin/obj/encrypt.o bin/obj/shared.o $(LIBS)
	$(CC) -o bin/decrypt bin/obj/decrypt.o bin/obj/shared.o $(LIBS)
	$(CC) -o bin/keygen bin/obj/keygen.o bin/obj/shared.o $(LIBS)
	$(CC) -o bin/keydump bin/obj/keydump.o bin/obj/shared.o $(LIBS)
	$(CC) -o bin/pwd2key bin/obj/pwd2key.o bin/obj/shared.o $(LIBS)
clean:
	rm -rf bin
install: all
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
make-deb: all
	mkdir -p bin/debian/DEBIAN
	echo Package: slowcrypt >bin/debian/DEBIAN/control
	echo Version: 0.0.1-Beta >> bin/debian/DEBIAN/control
	echo Section: custom >> bin/debian/DEBIAN/control
	echo Architecture: `dpkg --print-architecture`>> bin/debian/DEBIAN/control
	echo Depends: libssl1.1>>bin/debian/DEBIAN/control
	echo Maintainer: JCWasmx86 >> bin/debian/DEBIAN/control
	echo Description: Slowcrypt tools >> bin/debian/DEBIAN/control
	mkdir -p bin/debian/usr/bin
	mkdir -p bin/debian/usr/include
	mkdir -p bin/debian/usr/lib
	cp bin/encrypt bin/decrypt bin/keygen bin/keydump bin/pwd2key bin/debian/usr/bin
	cp include/slowcrypt.h bin/debian/usr/include
	ar csr bin/debian/usr/lib/libslowcrypt.a bin/obj/shared.o
	dpkg-deb --build bin/debian slowcrypt.deb
format:
	find src include -iname *.h -o -iname *.c | xargs clang-format --style=file -verbose -i
