#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define ABS(n) ((n) < 0 ? -(n) : (n))
#define MAX(a,b) ((a) + (b) + ABS ((a) - (b))) / 2
#define MIN(a,b) ((a) + (b) - ABS ((a) - (b))) / 2

int main (int argc, char *argv[])
{
	int a, b, c, r;
	int max, min;

	printf ("Zadej prvni cislo: ");
	r = scanf ("%d", &a);
	assert (r == 1);

	printf ("Zadej druhe cislo: ");
	r = scanf ("%d", &b);
	assert (r == 1);

	printf ("Zadej treti cislo: ");
	r = scanf ("%d", &c);
	assert (r == 1);

	max = MAX (a, MAX (b, c));
	min = MIN (a, MIN (b, c));

	printf ("Nejvetsi %d\n", max);
	printf ("Prostredni %d\n", a + b + c - min - max);
	printf ("Nejmensi %d\n", min);

	return 0;
}

