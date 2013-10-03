# cat words | palin
all: clean palin

run: clean palin
	echo "test\ntests\ntset" | ./palin

palin:
	gcc --ansi palin.c -o palin -pthread -Wall

clean:
	rm -rf *.out palin