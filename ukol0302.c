#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CHECK(cond) \
	if (!(cond)) { \
		puts ("Nespravny vstup."); \
		exit (EXIT_FAILURE); \
	}

/** Print out a separator made of dashes. */
static void
print_separator (int len)
{
	while (len--)
		putchar ('-');
	putchar ('\n');
}

int
main (int argc, char *argv[])
{
	/* The product of our multiplication may be larger than the size
	 * of long int, therefore we save the resulting digits into
	 * an array to print out later. We need to buffer just half of
	 * the result. Each byte contains slightly more than two digits,
	 * so we just multiply the count of bytes by four.
	 */
	char result[sizeof (long int) * 4], c;

	long int n, m, rest, product;
	int i, len, indent, result_len, zero_indent, zero_factor;
	double len_n, len_m;

	n = m = -1;

	puts ("Zadejte dve nezaporna cisla:");
	scanf ("%ld %ld", &n, &m);
	CHECK (n >= 0 && m >= 0);

	puts ("Vypocet:");

	len_n = log10 (n ? n : 1) + 1;
	len_m = log10 (m ? m : 1) + 1;

	len = n && m ? len_n + len_m - 1 : 1;

	if (len_n > len)
		len = len_n;
	if (len_m > len)
		len = len_m;

	printf ("  %*ld\n", len, n);
	printf ("x %*ld\n", len, m);

	print_separator (len + 2);

	/* Compute the steps between. */
	result_len = 0;
	rest = 0;

	indent = len;
	zero_indent = 0;
	zero_factor = 1;

	do
	{
		product = n * (m % 10);
		rest += product;

		/* Leave out the zero if we can. */
		if (product || !n || !m)
		{
			printf ("  %*ld\n",
				indent  + zero_indent,
				product * zero_factor);
			zero_indent = 0;
			zero_factor = 1;
		}
		else
		{
			zero_indent += 1;
			zero_factor *= 10;
		}
		indent--;

		if (!result_len || (m && n))
			result[result_len++] = '0' + rest % 10;
		rest /= 10;
	}
	while ((m /= 10));

	print_separator (len + 2);

	/* Revert the result and end the string. */
	for (i = result_len >> 1; i--; )
	{
		c = result[i];
		result[i] = result[result_len - i - 1];
		result[result_len - i - 1] = c;
	}
	result[result_len] = '\0';

	/* Prepend the remainder. */
	if (rest)
		printf ("  %*ld%s\n", len - result_len, rest, result);
	else
		printf ("  %*s\n", len, result);
	return 0;
}

