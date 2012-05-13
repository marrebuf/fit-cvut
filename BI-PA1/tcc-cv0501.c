#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define CHECK(cond) \
	if (!(cond)) { \
		puts ("Nespravny vstup."); \
		return 1; \
	}

int
main (int argc, char *argv[])
{
	int vyska, sirka, i, k, *pole;

	puts ("Zadej vysku:");
	CHECK (scanf ("%u", &vyska) == 1);

	sirka = (vyska - 1) * 2 + 1;
	pole = (int *) malloc (sizeof (int) * sirka);
	assert (pole != NULL);

	for (i = 0; i < vyska; i++)
	{
		for (k = 0; k < vyska; k++)
		{
			pole[vyska - k - 2] = pole[vyska + k] = i > k;
		}
		pole[vyska - 1] = 1;

		for (k = 0; k < sirka; k++)
			putchar (pole[k] ? '*' : ' ');

		putchar ('\n');
	}

	return 0;
}

