#!/usr/bin/tcc -run

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
	int n, i, need_sign = 0;

	puts ("Číslo k faktorizaci:");
	CHECK (scanf ("%d", &n) == 1);
	CHECK (n > 0);

	if (n == 1)
	{
		puts ("Jednička je moc malá.");
		return 0;
	}

	for (i = 2; n > 1; i++)
	{
		int power = 0;

		while (n % i == 0)
		{
			n /= i;
			power++;
		}

		if (power)
		{
			if (need_sign)
				printf (" * ");
			need_sign = 1;

			printf ("%d^%d", i, power);
		}
	}
	putchar ('\n');

	return 0;
}

