#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

static int
dow (int y, int m, int d)
{
	static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	y -= m < 3;
	return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

int
main (int argc, char *argv[])
{
	int r;
	unsigned int d_in, m_in, y_in;

	const char *dny[] =
		{"nedele", "pondeli", "utery", "streda", "ctvrtek", "patek", "sobota"};

	printf ("Zadej datum [d m y]: ");
	r = scanf ("%u %u %u", &d_in, &m_in, &y_in);
	assert (r == 3);
	assert (d_in > 0 && m_in > 0 && y_in > 0);

	/* Pan Sakamoto. (Gregoriansky kalendar.) */
	printf ("Den v tydnu je sakamotovsky: %s\n", dny[dow (y_in, m_in, d_in)]);

	/* A ted pan Gauss. (Juliansky kalendar.) */
	unsigned int Y, d, m, y, c, w;

	Y = y_in - (m < 3);
	y = Y % 100;
	c = Y / 100;
	d = d_in;
	m = ((m_in + 9) % 12) + 1;

	w = (d + (int) (2.6 * m - 0.2)
		+ y + y / 4 + c / 4. - 2 * c) % 7;
	printf ("Den v tydnu je gaussovsky: %s\n", dny[w]);

	return 0;
}

