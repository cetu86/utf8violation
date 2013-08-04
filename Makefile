all:
	gcc -ggdb -O0 -std=c99 -o utf8violation utf8violation.c

release:
	gcc -O3 -std=c99 -o utf8violation utf8violation.c
	strip utf8violation
