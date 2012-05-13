#include <stdio.h>
#include <stdlib.h>

#define CHECK(cond) \
	if (!(cond)) { \
		puts ("Nespravny vstup."); \
		return 1; \
	}

int
main (int argc, char *argv[])
{
	double a, b, aa, bb, h;
	double objem;

	a = b = aa = bb = h = 0.;

	puts ("Zadejte velikosti a a b:");
	scanf ("%lf %lf", &a, &b);
	CHECK (a > 0 && b > 0);

	puts ("Zadejte velikosti a' a b':");
	scanf ("%lf %lf", &aa, &bb);
	CHECK (aa > 0 && bb > 0 && aa < a && bb < b);

	puts ("Zadejte velikost h:");
	scanf ("%lf", &h);
	CHECK (h > 0);

	objem = h * ((aa * b + a * bb) / 2 + (a - aa) * (b - bb) / 3);

	printf ("Objem: %f\n", objem);
	return 0;
}

