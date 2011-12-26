#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

struct TSOLDIER
{
	TSOLDIER *m_Next;
	int m_PersonalID;
	char m_SecretRecord[64];
};
#endif /* __PROGTEST__ */

TSOLDIER *
mergePlatoons (TSOLDIER *p1, TSOLDIER *p2)
{
	TSOLDIER *result, **append;

	append = &result;

	while (p1 != NULL || p2 != NULL)
	{
		if (p1 != NULL)
		{
			*append = p1;
			append = &p1->m_Next;
			p1 = p1->m_Next;
		}
		if (p2 != NULL)
		{
			*append = p2;
			append = &p2->m_Next;
			p2 = p2->m_Next;
		}
	}

	*append = NULL;
	return result;
}
 
void
splitPlatoon (TSOLDIER *src, TSOLDIER **p1, TSOLDIER **p2)
{
	TSOLDIER *tmp, **append;
	int i, length = 0;

	*p1 = NULL;
	*p2 = NULL;

	for (tmp = src; tmp != NULL; tmp = tmp->m_Next)
	  length++;

	tmp = src;

	append = p1;
	for (i = length / 2; i != 0; i --)
	{
		*append = tmp;
		append = &tmp->m_Next;
		tmp = tmp->m_Next;
	}

	*append = NULL;

	append = p2;
	for (i = length / 2; i != 0; i --)
	{
		*append = tmp;
		append = &tmp->m_Next;
		tmp = tmp->m_Next;
	}

	*append = NULL;

	free (tmp);
}
 
void
destroyPlatoon (TSOLDIER *src)
{
	TSOLDIER *tmp;

	while (src != NULL)
	{
		tmp = src->m_Next;
		free (src);
		src = tmp;
	}
}
  
#ifndef __PROGTEST__
static TSOLDIER *
gen (int first, ...)
{
	va_list ap;
	TSOLDIER *result, **append, *tmp;
	int i, arrow = 0;

	append = &result;
	printf ("Generating... ");

	va_start (ap, first);
	for (i = first; i != -1; i = va_arg (ap, int))
	{
		if (arrow)
		  printf ("-> ");
		printf ("%d ", i);
		arrow = 1;

		tmp = (TSOLDIER *) calloc (1, sizeof *tmp);
		tmp->m_PersonalID = i;
		*append = tmp;
		append = &tmp->m_Next;
	}
	va_end (ap);

	*append = NULL;
	printf ("\n");
	return result;
}

static void
test (TSOLDIER *list, int first, ...)
{
	va_list ap;
	int i, arrow = 0;

	printf ("Testing... ");

	va_start (ap, first);
	for (i = first; i != -1; i = va_arg (ap, int))
	{
		if (arrow)
		  printf ("-> ");
		arrow = 1;

		if (list != NULL)
		  printf ("%d ", list->m_PersonalID);
		else
		  printf ("-1\n");

		assert (list != NULL);
		assert (list->m_PersonalID == i);
		list = list->m_Next;
	}
	va_end (ap);

	printf ("\n");
	assert (list == NULL);
}

int
main (int argc, char *argv [])
{
	TSOLDIER *a, *b, *c;

	a = gen (0, 1, 2, 3, 4, -1);
	b = gen (10, 11, 12, 13, 14, -1);

	c = mergePlatoons (a, b);
	test (c, 0, 10, 1, 11, 2, 12, 3, 13, 4, 14, -1);

	splitPlatoon (c, &a, &b);
	test (a, 0, 10, 1, 11, 2, -1);
	test (b, 12, 3, 13, 4, 14, -1);

	destroyPlatoon (a);
	destroyPlatoon (b);


	a = gen (0, 1, 2, -1);
	b = gen (10, 11, 12, 13, 14, -1);
	c = mergePlatoons (a, b);
	test (c, 0, 10, 1, 11, 2, 12, 13, 14, -1);

	splitPlatoon (c, &a, &b);
	test (a, 0, 10, 1, 11, -1);
	test (b, 2, 12, 13, 14, -1);

	destroyPlatoon (a);
	destroyPlatoon (b);


	a = gen (0, 1, 2, -1);
	b = gen (10, 11, 12, 13, -1);
	c = mergePlatoons (a, b);
	test (c, 0, 10, 1, 11, 2, 12, 13, -1);

	splitPlatoon (c, &a, &b);
	test (a, 0, 10, 1, -1);
	test (b, 11, 2, 12, -1);

	destroyPlatoon (a);
	destroyPlatoon (b);
}
#endif /* __PROGTEST__ */

