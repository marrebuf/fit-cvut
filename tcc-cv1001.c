#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int
main (int argc, char *argv[])
{
	FILE *fp;
	long i, file_len;

	assert (argc == 2);

	fp = fopen (argv[1], "r");
	assert (fp != NULL);

	fseek (fp, 0, SEEK_END);
	file_len = ftell (fp);

	for (i = 0; i < file_len / 2; i++)
	{
		int cs, ce;

		fseek (fp, i, SEEK_SET);
		cs = fgetc (fp);

		fseek (fp, -1 - i, SEEK_END);
		ce = fgetc (fp);

		if (cs != ce)
		{
			puts ("Soubor neni palindromem.");
			return 0;
		}
	}

	puts ("Soubor je palindromem.");
	return 0;
}


