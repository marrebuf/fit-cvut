#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Token {END, EMPTY, NODE, SECTION};

static enum Token
get_token (char *buffer, size_t size)
{
	if (!fgets (buffer, size, stdin))
		return END;
	size_t len = strlen (buffer);

	/* Fix the buffer first. */
	if (buffer[len - 1] == '\n')
		buffer[--len] = '\0';

	/* Empty line? */
	if (!len)
		return EMPTY;

	/* Section header? */
	bool is_header = false;
	if (buffer[len - 1] == ':')
	{
		buffer[--len] = '\0';
		if (buffer[len - 1] == '/')
			buffer[--len] = '\0';

		is_header = true;
	}

	/* Only use the last component. */
	char *slash = strrchr (buffer, '/');
	if (slash++)
	{
		len -= slash - buffer;
		memmove (buffer, slash, len + 1);
	}

	return is_header ? SECTION : NODE;
}

int
main (int argc, char *argv[])
{
	puts ("graph Dir {\n"
	      "\trankdir = LR\n"
	      "\tnode [shape = note]");

	char token[1024];

	char section[1024] = "/";
	bool had_first = false;

	int last_point, point = 0;
	int counter = 0;

	bool running = true;
	while (running)
	{
		enum Token type = get_token (token, sizeof token);

		if ((type == SECTION || type == END) && counter)
		{
			printf ("\n\tsubgraph {rank = same\n\t\t\"%s\"", section);
			for (int i = last_point; i < point; i++)
				printf (" -- p%d", i);
			putchar ('\n');
			for (int i = last_point; i < point; i++)
				printf ("\t\tp%d [shape = point, width = 0]\n", i);
			printf ("\t}\n");
		}

		switch (type)
		{
		case END:
			running = false;
		case EMPTY:
			break;
		case NODE:
			printf ("\tp%d -- \"%s\" [label = %d]\n",
				point++, token, ++counter);
			break;
		case SECTION:
			if (had_first)
				strcpy (section, token);
			had_first = true;

			printf ("\n\t\"%s\" [shape = folder]\n", section);
			last_point = point;
			counter = 0;
		}
	}

	puts ("}");
	return 0;
}

