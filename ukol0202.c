#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

/** Jenom takové výpomocné makro. */
#define CHECK(cond) \
	if (!(cond)) { \
		puts ("Nespravny vstup."); \
		exit (EXIT_FAILURE); \
	}

/* Love it or hate it. */
enum {FROM, TO};
enum {A, B, C};
enum {AB, BC, CA, ABC};

/** Zjistí, zda je daný interval validní. */
#define is_valid(i) ((i)[FROM] < (i)[TO])

/** Výpis intervalu. */
static void
print_interval (const int i[2])
{
	printf ("%d:%02d - %d:%02d",
		i[FROM] / 60, i[FROM] % 60,
		i[TO]   / 60, i[TO]   % 60);
}

/** Výpis intervalu do řádku, pokud je validní. */
static void
print_interval_ex (const int i[2], int *need_comma)
{
	if (!is_valid (i))
		return;

	if (*need_comma)
		printf (", ");
	else
		*need_comma = 1;
	
	print_interval (i);
}

/** Spojit dva intervaly, pokud na sebe přímo navazují. */
static void
join_interval (const int i1[2], const int i2[2], int out[2])
{
	if (is_valid (i1) && is_valid (i2))
	{
		if (i1[TO] == i2[FROM])
		{
			out[FROM] = i1[FROM];
			out[TO]   = i2[TO];
		}
		else if (i2[TO] == i1[FROM])
		{
			out[FROM] = i2[FROM];
			out[TO]   = i1[TO];
		}
	}
}

/** Společný čas dvou manažerů. */
static void
get_common (const int m1[2], const int m2[2], int out[2])
{
	out[FROM] = m1[FROM] > m2[FROM] ? m1[FROM] : m2[FROM];
	out[TO]   = m1[TO]   < m2[TO]   ? m1[TO]   : m2[TO];
}

int
main (int argc, char **argv)
{
	int managers[3][2], common[4][2], i;

	for (i = A; i < ABC; i++)
	{
		int from_h, from_m, to_h, to_m;

		printf ("Manazer %c:\n", 'A' + i);
		CHECK (scanf ("%2d:%2d - %2d:%2d",
			&from_h, &from_m, &to_h, &to_m) == 4);
		CHECK (from_m >= 0 && to_m >= 0
			&& from_h >= 0 && to_h >= 0);
		CHECK (from_m < 60 && to_m < 60
			&& from_h < 24 && to_h < 24);

		managers[i][FROM] = from_h * 60 + from_m;
		managers[i][TO]   = to_h   * 60 + to_m;
		CHECK (managers[i][FROM] < managers[i][TO]);
	}

	/* Zjistí průnik časů manažerů po dvou. */
	get_common (managers[A], managers[B], common[AB]);
	get_common (managers[B], managers[C], common[BC]);
	get_common (managers[C], managers[A], common[CA]);

	/* Zjistí průnik časů všech manažerů najednou. */
	get_common (common[AB], common[BC], common[ABC]);

	if (is_valid (common[ABC]))
	{
		printf ("Vsichni tri manazeri: ");
		print_interval (common[ABC]);
	}
	else if (is_valid (common[AB])
	      || is_valid (common[BC])
	      || is_valid (common[CA]))
	{
		printf ("Dva manazeri: ");

		/* Pokud na sebe dva intervaly navazují, spojit je. */
		join_interval (common[AB], common[BC], common[ABC]);
		join_interval (common[BC], common[CA], common[ABC]);
		join_interval (common[CA], common[AB], common[ABC]);

		/* Pokud jsme něco spojili do jednoho, sjednocený interval
		 * ABC se stane platným, pokud se tak ovšem nestalo,
		 * prostě vypíšeme všechny intervaly jednotlivě.
		 */
		if (is_valid (common[ABC]))
			print_interval (common[ABC]);
		else
		{
			int need_comma = 0;

			print_interval_ex (common[AB], &need_comma);
			print_interval_ex (common[BC], &need_comma);
			print_interval_ex (common[CA], &need_comma);
		}
	}
	else
		printf ("Spolecny cas neexistuje.");

	putchar ('\n');
	return 0;
}

