#CFLAGS=-O2

all: attacker victim shared_file

victim: victim.c
	gcc $(CFLAGS) victim.c -o victim

attacker: attacker.c
	gcc $(CFLAGS) attacker.c -o attacker

shared_file: create_shared_file.py
	./create_shared_file.py

clean:
	rm -f attacker victim
