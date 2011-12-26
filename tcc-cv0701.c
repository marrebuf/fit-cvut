#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>

static int
fact (int n)
{
	if (n <= 1)
		return 1;
	else
		return n * fact (n - 1);
}

static void
pascal_triangle (int n)
{
	int i, k;

	if (n > 1)
		pascal_triangle (n - 1);

	for (i = 0; i < n; i++)
		printf ("%3d ", fact (n - 1) / fact (n - 1 - i) / fact (i));
	putchar ('\n');
}

int
main (int argc, char *argv[])
{
	int n, r;

	puts ("Cislo?");
	r = scanf ("%d", &n);
	if (r == 0)
		exit (1);

	pascal_triangle (n);
	return 0;
}

