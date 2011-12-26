#include <stdio.h>
#include <stdlib.h>

#define CHECK(cond) \
	if (!(cond)) { \
		puts ("Nespravny vstup."); \
		exit (EXIT_FAILURE); \
	}

int
main (int argc, char *argv[])
{
	int n, i, k, biggest_sum = 0, biggest_k = 0;

	puts ("Zadejte n:");
	CHECK (scanf ("%d", &n) == 1);
	CHECK (n > 0);

	puts ("Zadejte cisla:");
	while (n--)
	{
		int sum = 0;

		CHECK (scanf ("%d", &k) == 1);

		for (i = k; i; i /= 10)
			sum += i % 10;
		if (sum < 0)
			sum = -sum;

		if (sum > biggest_sum)
		{
			biggest_sum = sum;
			biggest_k = k;
		}
		else if (sum == biggest_sum && k > biggest_k)
			biggest_k = k;
	}

	printf ("Vysledek: %d\n", biggest_k);
	return 0;
}

