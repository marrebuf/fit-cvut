#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_BRANCHES 3
enum {DECORATION_NONE, DECORATION_CANDLE, DECORATION_SPARKLER};

struct TNODE
{
	TNODE *m_Parent;
	TNODE *m_Branches[MAX_BRANCHES];
	int m_Decoration;
};
#endif /* ! __PROGTEST__ */

/** Check if the path is valid. */
static int
check_path (char *path)
{
	for (; *path; path++)
		if (*path < '0' || *path >= '0' + MAX_BRANCHES)
			return 0;

	return 1;
}

int
setDecoration (TNODE **root, char *path, int decor)
{
	TNODE *node, *new_node, **branch;

	if (!check_path (path))
		return 0;

	if (!*root)
		*root = (TNODE *) calloc (1, sizeof (TNODE));

	node = *root;
	for (; *path; path++)
	{
		branch = &node->m_Branches[*path - '0'];

		if (!*branch)
		{
			new_node = (TNODE *) calloc (1, sizeof (TNODE));
			new_node->m_Parent = node;
			*branch = new_node;
		}
		node = *branch;
	}

	node->m_Decoration = decor;
	return 1;
}

void
destroyTree (TNODE *root)
{
	int i;

	if (!root)
		return;

	for (i = 0; i < MAX_BRANCHES; i++)
		destroyTree (root->m_Branches[i]);

	free (root);
}

int
cutBranch (TNODE **root, char *path)
{
	if (!check_path (path))
		return 0;

	if (!*root)
		return 0;

	for (; *path; path++)
	{
		root = &(*root)->m_Branches[*path - '0'];
		if (!*root)
			return 0;
	}

	destroyTree (*root);
	*root = NULL;
	return 1;
}

/** Check whether the two TNODE's can catch fire. */
static int
can_catch_fire (TNODE *a, TNODE *b)
{
	return (a->m_Decoration == DECORATION_CANDLE
	    &&  b->m_Decoration == DECORATION_SPARKLER)
	    || (b->m_Decoration == DECORATION_CANDLE
	    &&  a->m_Decoration == DECORATION_SPARKLER);
}

int
easyToCatchFire (TNODE *root)
{
	TNODE *branch, *last_branch;
	int i;

	if (!root)
		return 0;

	last_branch = NULL;
	for (i = 0; i < MAX_BRANCHES; i++)
	{
		branch = root->m_Branches[i];

		if (branch)
		{
			if (can_catch_fire (branch, root))
				return 1;
			if (last_branch && can_catch_fire (branch, last_branch))
				return 1;
			if (easyToCatchFire (branch))
				return 1;
		}
		last_branch = branch;
	}
	return 0;
}

#ifndef __PROGTEST__
int
main (int argc, char *argv[])
{
	TNODE *n;

	n = NULL;
	assert (setDecoration (&n, (char *) "000", DECORATION_SPARKLER) == 1);
	assert (setDecoration (&n, (char *) "001", DECORATION_SPARKLER) == 1);
	assert (setDecoration (&n, (char *) "002", DECORATION_SPARKLER) == 1);
	assert (setDecoration (&n, (char *) "1", DECORATION_CANDLE) == 1);
	assert (setDecoration (&n, (char *) "01", DECORATION_NONE) == 1);
	assert (setDecoration (&n, (char *) "", DECORATION_CANDLE) == 1);
	assert (easyToCatchFire (n) == 0);
	destroyTree (n);

	n = NULL;
	assert (setDecoration (&n, (char *) "000", DECORATION_SPARKLER) == 1);
	assert (setDecoration (&n, (char *) "002", DECORATION_CANDLE) == 1);
	assert (setDecoration (&n, (char *) "2", DECORATION_CANDLE) == 1);
	assert (easyToCatchFire (n) == 0);
	destroyTree (n);

	n = NULL;
	assert (setDecoration (&n, (char *) "0001", DECORATION_SPARKLER) == 1);
	assert (setDecoration (&n, (char *) "000", DECORATION_CANDLE) == 1);
	assert (easyToCatchFire (n) == 1);
	destroyTree (n);

	n = NULL;
	assert (setDecoration (&n, (char *) "012001", DECORATION_SPARKLER) == 1);
	assert (setDecoration (&n, (char *) "012002", DECORATION_CANDLE) == 1);
	assert (easyToCatchFire (n) == 1);
	assert (cutBranch (&n, (char *) "0120") == 1);
	assert (easyToCatchFire (n) == 0);
	destroyTree (n);

	n = NULL;
	assert (setDecoration (&n, (char *) "0123", DECORATION_SPARKLER) == 0);
	assert (cutBranch (&n, (char *) "012") == 0);
	assert (easyToCatchFire (n) == 0);
	destroyTree (n);

	n = NULL;
	assert (setDecoration (&n, (char *) "012", DECORATION_SPARKLER) == 1);
	assert (setDecoration (&n, (char *) "011", DECORATION_CANDLE) == 1);
	assert (easyToCatchFire (n) == 1);
	assert (cutBranch (&n, (char *) "") == 1 && n == NULL);
	assert (easyToCatchFire (n) == 0);
	assert (cutBranch (&n, (char *) "") == 0);
	destroyTree (n);

	return 0;
}
#endif /* ! __PROGTEST__ */

