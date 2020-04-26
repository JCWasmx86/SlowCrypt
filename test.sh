#!/usr/bin/bash
cd bin
for v in {0..100}
do
	dd if=/dev/urandom of=$v.txt count=$v
done
for v in {0..100}
do
	./keygen
	./encrypt $v.txt $v.txt.enc key.key
	./decrypt $v.txt.enc $v.txt.dec key.key
	diff $v.txt $v.txt.dec
	echo $v
done
cd ..
