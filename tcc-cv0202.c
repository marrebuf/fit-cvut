#!/usr/bin/tcc -run

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
	double t, v1, v2, t_vasek, s;

	printf ("Zadej náskok Pepy v s: ");
	scanf ("%lf", &t);

main_repeat:
	printf ("Zadej rychlost Pepy (v1) v m/s: ");
	scanf ("%lf", &v1);

	printf ("Zadej rychlost Vaška (v2) v m/s: ");
	scanf ("%lf", &v2);

	if (v2 <= v1)
	{
		printf ("Vašek neni dost rychlej na Pepu, znova\n");
		goto main_repeat;
	}

	t_vasek = (v1 * t - 250.) / (v2 - v1);
	s = v2 * t_vasek;

	printf ("Vzdálenost od startu je %lf\n", s);
	return 0;
}

