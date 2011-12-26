#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#define DIE(msg) \
	do { \
		puts (msg); \
		return 1; \
	} while (0)

void
print_angle (const char *name, double a)
{
	double deg, degmin, degsec;

	deg    = a / M_PI * 180;
	degmin = modf (deg,    &deg)    * 60.;
	degsec = modf (degmin, &degmin) * 60.;

	if (degsec >= 59.995)
	{
		degmin += 1.;
		degsec = 0.;
	}
	if (degmin >= 59.5)
	{
		deg += 1.;
		degmin = 0.;
	}

	printf ("Uhel %s: %.4f rad = %.0f s %02.0f'%05.2f''\n",
		name, a, deg, degmin, degsec);
}

int
main (int argc, char **argv)
{
	double a, b, c;
	double s, area;
	double alpha, beta, gamma;

	a = b = c = 0.;

	puts ("Zadejte velikost stran a b c:");
	scanf ("%lf %lf %lf", &a, &b, &c);
	if (!(a > 0 && b > 0 && c > 0))
		DIE ("Nespravny vstup.");

	if (!(a + b > c && a + c > b && b + c > a))
		DIE ("Trojuhelnik neexistuje.");

	if (a == b && b == c)
		puts ("Trojuhelnik je rovnostranny.");
	else if (a == b || b == c || a == c)
		puts ("Trojuhelnik je rovnoramenny.");
	else
		puts ("Trojuhelnik neni ani rovnostranny ani rovnoramenny.");

	s = (a + b + c) / 2;
	area = sqrt (s * (s - a) * (s - b) * (s - c));

	alpha = acos (sqrt (s * (s - a) / b / c)) * 2;
	beta  = acos (sqrt (s * (s - b) / a / c)) * 2;
	gamma = acos (sqrt (s * (s - c) / a / b)) * 2;

	/* Přesnost * konstanta * vztaženo k velikosti vstupu */
	if (fabs (a * a + b * b - c * c) <= DBL_EPSILON * 64 * c * c
	 || fabs (a * a + c * c - b * b) <= DBL_EPSILON * 64 * b * b
	 || fabs (b * b + c * c - a * a) <= DBL_EPSILON * 64 * a * a)
		puts ("Trojuhelnik je pravouhly.");
	else if (alpha > M_PI_2 || beta > M_PI_2 || gamma > M_PI_2)
		puts ("Trojuhelnik je tupouhly.");
	else
		puts ("Trojuhelnik je ostrouhly.");

	print_angle ("alfa", alpha);
	print_angle ("beta", beta);
	print_angle ("gama", gamma);

	printf ("Obvod: %.4f\n", a + b + c);
	printf ("Obsah: %.4f\n", area);

	printf ("Polomer kruznice vepsane: %.4f\n", area / s);
	printf ("Polomer kruznice opsane: %.4f\n",  a / 2 / sin (alpha));

	printf ("Vyska va: %.4f\n", b * sin (gamma));
	printf ("Vyska vb: %.4f\n", a * sin (gamma));
	printf ("Vyska vc: %.4f\n", a * sin (beta));

	return 0;
}

