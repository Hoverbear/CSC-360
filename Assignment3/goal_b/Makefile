all: clean simvm

simvm:
	gcc simvm.c -o simvm -Wall -pedantic -lreadline -std=gnu99

run: clean simvm
	./simvm --file=./trace01.out --framesize=12 --numframes=256 --scheme=lru --progress
	./simvm --file=./trace01.out --framesize=12 --numframes=256 --scheme=fifo --progress

clean:
	rm -rf simvm
