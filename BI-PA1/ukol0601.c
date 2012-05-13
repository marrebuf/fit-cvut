#ifndef __PROGTEST__
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
#endif /* ! __PROGTEST__ */

struct Buffer
{
	char *s;
	size_t size, len;
};

static void
buffer_append (Buffer *b, const char *text, int text_len)
{
	if (text_len <= 0)
		return;

	if (!b->size)
		b->s = (char *) malloc (b->size = 16);
	while (b->size <= b->len + text_len)
		b->s = (char *) realloc (b->s, b->size <<= 1);

	memcpy (b->s + b->len, text, text_len);
	b->len += text_len;
}

static void
buffer_append_z (Buffer *b, const char *text)
{
	buffer_append (b, text, strlen (text));
}

static void
append_small_number (Buffer *b, int number, int set)
{
	static const char
	*s_basic[][10] =
	{
		/* Generic. */
		{"nula", "jedna", "dva", "tri", "ctyri",
		 "pet", "sest", "sedm", "osm", "devet"},
		/* Hundreds. */
		{"nula", "jedno", "dve", "tri", "ctyri",
		 "pet", "sest", "sedm", "osm", "devet"},
		/* Thousands, millions, ... */
		{"nula", "jeden", "dva", "tri", "ctyri",
		 "pet", "sest", "sedm", "osm", "devet"},
		/* Billions, ... */
		{"nula", "jedna", "dve", "tri", "ctyri",
		 "pet", "sest", "sedm", "osm", "devet"}
	},
	*s_xteen[] =
	{
		"deset", "jedenact", "dvanact", "trinact", "ctrnact",
		"patnact", "sestnact", "sedmnact", "osmnact", "devatenact"
	},
	*s_tens[] =
	{
		NULL, NULL, "dvacet", "tricet", "ctyricet",
		"padesat", "sedesat", "sedmdesat", "osmdesat", "devadesat"
	},
	*s_hundreds[] =
	{
		NULL, "sto", "ste", "sta", "sta",
		"set", "set", "set", "set", "set"
	};

	int hundreds, tens;

	hundreds = number / 100;
	number   = number % 100;
	tens     = number / 10;
	number   = number % 10;

	if (hundreds)
	{
		buffer_append_z (b, s_basic[1][hundreds]);
		buffer_append (b, " ", 1);
		buffer_append_z (b, s_hundreds[hundreds]);

		if (tens || number)
			buffer_append (b, " ", 1);
	}

	if (tens >= 2)
	{
		buffer_append_z (b, s_tens[tens]);
		if (number)
		{
			buffer_append (b, " ", 1);
			buffer_append_z (b, s_basic[set][number]);
		}
	}
	else if (tens)
		buffer_append_z (b, s_xteen[number]);
	else if (number || !hundreds)
		buffer_append_z (b, s_basic[set][number]);
}

static void
append_number (Buffer *b, long int number, int order)
{
	static const char *s_orders[][3] =
	{
		{"tisic",     "tisice",    "tisic"},
		{"milion",    "miliony",   "milionu"},
		{"miliarda",  "miliardy",  "miliard"},
		{"bilion",    "biliony",   "bilionu"},
		{"biliarda",  "biliardy",  "biliard"},
		{"trilion",   "triliony",  "trilionu"},
		{"triliarda", "triliardy", "triliard"}
	};

	if (number < 0)
	{
		buffer_append_z (b, "minus ");
		number = -number;
	}
	if (number >= 1000)
	{
		append_number (b, number / 1000, order + 1);

		number %= 1000;
		if (!number)
			return;

		buffer_append (b, " ", 1);
	}

	if (!order--)
		append_small_number (b, number, 0);
	else
	{
		unsigned x_cent;

		append_small_number (b, number,
			3 - (order & 1) - !order);
		buffer_append (b, " ", 1);

		x_cent = number % 100;
		if (x_cent > 10 && x_cent < 20)
			buffer_append_z (b, s_orders[order][2]);
		else switch (number % 10)
		{
		case 1:
			buffer_append_z (b, s_orders[order][0]);
			break;
		case 2:
		case 3:
		case 4:
			buffer_append_z (b, s_orders[order][1]);
			break;
		default:
			buffer_append_z (b, s_orders[order][2]);
			break;
		}
	}
}

char *
replaceNumbers (char *str)
{
	Buffer b = {NULL, 0, 0};
	char *endptr, *marker;
	int accept_num;
	long int i;

	if (!str)
		return NULL;

	accept_num = 1;
	marker = str;

	for (; *str; str++)
	{
		if (accept_num && (*str == '-' || isdigit (*str)))
		{
			i = strtol (str, &endptr, 10);
			if (!*endptr || isspace (*endptr))
			{
				buffer_append (&b, marker, str - marker);
				append_number (&b, i, 0);

				marker = str = endptr;
				if (!*str)
					break;
			}
			else
				str = endptr;
		}
		accept_num = isspace (*str);
	}
	buffer_append (&b, marker, str - marker + 1);
	return b.s;
}

#ifndef __PROGTEST__
void
test (const char *in, const char *out)
{
	static int nr;
	char *r;

	printf ("-- Test %d: ", ++nr);
	r = replaceNumbers ((char *) in);
	if (!r || strcmp (r, out))
	{
		puts ("FAIL");
		printf ("Should be: %s\n", out);
		printf ("Returned:  %s\n", r);
	}
	else
		puts ("OK");

	free (r);
}

int
main (int argc, char *argv[])
{
	/* Reference values. */
	test ("CVUT FIT byla zalozena 1 cervence 2009",
		"CVUT FIT byla zalozena jedna cervence dva tisice devet");
	test ("PA1 je muj nejoblibenejsi predmet, davam 5 z 5ti.",
		"PA1 je muj nejoblibenejsi predmet, davam pet z 5ti.");
	test ("Datovy typ int ma rozsah -2147483648 az 2147483647 vcetne.",
		"Datovy typ int ma rozsah minus dve miliardy jedno sto ctyricet"
		" sedm milionu ctyri sta osmdesat tri tisice sest set ctyricet"
		" osm az dve miliardy jedno sto ctyricet sedm milionu"
		" ctyri sta osmdesat tri tisice sest set ctyricet sedm vcetne.");
	test ("Cislo 123123 je delitelne 1001\n",
		"Cislo jedno sto dvacet tri tisice jedno sto dvacet tri"
		" je delitelne jeden tisic jedna\n");
	test ("Ulohy na Progtestu resi na 1. pokus Chuck Norris a mozna agent 007",
		"Ulohy na Progtestu resi na 1. pokus Chuck Norris a mozna agent sedm");

	test (" 740484", " sedm set ctyricet tisic ctyri sta osmdesat ctyri");
	test ("       846203    710262     ",
		"       osm set ctyricet sest tisic dve ste tri"
		"    sedm set deset tisic dve ste sedesat dva     ");
	test ("111111111111", "jedno sto jedenact miliard jedno sto jedenact"
		" milionu jedno sto jedenact tisic jedno sto jedenact");
	test ("20020020020", "dvacet miliard dvacet milionu dvacet tisic dvacet");

	char arr[80], *s;

	puts ("\nTestbed:");
	while (fgets (arr, sizeof arr, stdin))
	{
		s = replaceNumbers (arr);
		fputs (s, stdout);
		free (s);
	}

	return 0;
}
#endif /* ! __PROGTEST__ */

