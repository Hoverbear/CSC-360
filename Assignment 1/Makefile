# cat words | palin
all: clean palin threaded_palin

threaded_run: clean threaded_palin
	cat smallwords | ./threaded_palin

run: clean palin
	cat smallwords | ./palin

treaded_palin:
	gcc --ansi threaded_palin.c -o threaded_palin -pthread -Wal

palin:
	gcc --ansi palin.c -o palin -pthread -Wall

clean:
	rm -rf *.out palin