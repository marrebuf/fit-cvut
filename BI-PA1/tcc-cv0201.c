#!/usr/bin/tcc -run

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static double
surface (double r, double h)
{
	return M_PI * r * (r + sqrt (r * r + h * h));
}

static double
volume (double r, double h)
{
	return 1 / 3. * M_PI * r * r * h;
}

int main (int argc, char *argv[])
{
	double radius, height;

	printf ("Zadej poloměr podstavy: ");
	scanf ("%lf", &radius);

	printf ("Zadej výšku kuželu: ");
	scanf ("%lf", &height);

	printf ("Povrch kuželu: %.4lf\n",
		surface (radius, height));
	printf ("Objem kuželu: %.4lf\n",
		volume (radius, height));

	return 0;
}

