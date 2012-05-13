#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>

static int
collatz (int n)
{
	int r = 1;

	if (n != 1)
	{
		printf ("%d, ", n);

		if (n % 2 == 1)
			n = 3 * n + 1;
		else
			n = n / 2;

		r += collatz (n);
	}
	else
		printf ("%d\n", n);

	return r;
}

int
main (int argc, char *argv[])
{
	int n, r;

	puts ("Cislo?");
	r = scanf ("%d", &n);
	if (r == 0)
		exit (1);

	printf ("-- Pocet: %d\n", collatz (n));
	return 0;
}

