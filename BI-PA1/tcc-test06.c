#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>

#define CHECK(cond) \
	if (!(cond)) { \
		puts ("Nespravny vstup."); \
		exit (EXIT_FAILURE); \
	}

int
main (int argc, char *argv[])
{
	int n, i, k, count = 0, in = 0;

	puts ("Zadejte n:");
	CHECK (scanf ("%d", &n) == 1);
	CHECK (n > 0);

	puts ("Zadejte cisla:");
	for (i = 0; i < n; i++)
	{
		CHECK (scanf ("%d", &k) == 1);
		if (k % 2 == 0)
			in = 1;
		else if (in)
		{
			count++;
			in = 0;
		}
	}

	if (in)
		count++;

	printf ("Vysledek: %d\n", count);
	return 0;
}

