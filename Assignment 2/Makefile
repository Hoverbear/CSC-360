all: mosh

mosh:
	gcc mosh.c -o mosh -Wall -lreadline -std=gnu99

run: clean mosh
	./mosh

clean:
	rm -rf mosh
