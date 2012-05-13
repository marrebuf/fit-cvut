#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define CHECK(cond) \
	if (!(cond)) { \
		puts ("Nespravny vstup."); \
		return 1; \
	}

#define IDX(aa,xx,yy) (aa)[sirka * (yy) + (xx)]

int
main (int argc, char *argv[])
{
	int sirka, i, k;
	char *pole;

	puts ("Zadej sirku:");
	CHECK (scanf ("%u", &sirka) == 1);
	CHECK (sirka % 2 == 1);

	pole = (char *) malloc (sirka * sirka * sizeof *pole);
	memset (pole, ' ', sirka * sirka * sizeof *pole);

	for (i = 0; i < sirka; i++)
	{
		IDX (pole, i, i) = '*';
		IDX (pole, sirka - 1 - i, i) = '*';

		IDX (pole, 0, i) = '*';
		IDX (pole, i, 0) = '*';
		IDX (pole, sirka - 1, i) = '*';
		IDX (pole, i, sirka - 1) = '*';
	}

	for (i = 0; i < sirka; i++)
	{
		for (k = 0; k < sirka; k++)
			putchar (IDX (pole, i, k));

		putchar ('\n');
	}

	return 0;
}

