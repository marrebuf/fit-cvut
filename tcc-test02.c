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
	int n, i, k, current = 0, longest = 0;

	puts ("Zadejte n:");
	CHECK (scanf ("%d", &n) == 1);
	CHECK (n > 0);

	puts ("Zadejte cisla:");
	for (i = 0; i < n; i++)
	{
		CHECK (scanf ("%d", &k) == 1);
		if (k > 0)
			current++;
		else if (current > longest)
		{
			longest = current;
			current = 0;
		}
	}

	if (current > longest)
		longest = current;

	printf ("Vysledek: %d\n", longest);
	return 0;
}

