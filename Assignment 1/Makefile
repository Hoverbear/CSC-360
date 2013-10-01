# cat words | palin
all: clean palin

run: clean palin
	cat words | palin

palin:
	gcc --ansi palin.c -o palin -pthread -Wall

clean:
	rm -rf *.out palin