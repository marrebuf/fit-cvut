#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define CHECK(cond) \
	if (!(cond)) { \
		puts ("Nespravny vstup."); \
		return 1; \
	}

int
main (int argc, char *argv[])
{
	int cislo, x, cnt = 0;

	srand (time (NULL));
	cislo = rand () % 101;

	puts ("Myslim si cislo v <0; 100>, zkus jej uhodnout.");
	while (1)
	{
		cnt++;
		CHECK (scanf ("%u", &x) == 1);
		if (x == cislo)
		{
			printf ("Uhodl jsi na %d. pokus.\n", cnt);
			break;
		}
		else if (x < cislo)
			puts ("Moje cislo je vetsi.");
		else
			puts ("Moje cislo je mensi.");
	}

	return 0;
}

