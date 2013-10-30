all: mosh

mosh:
	gcc mosh.c -o mosh -Wall -pedantic -lreadline -std=gnu99

run: clean mosh
	./mosh

clean:
	rm -rf mosh
