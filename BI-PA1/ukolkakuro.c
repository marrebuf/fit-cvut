/*
 *  Kakuro solver
 *  Copyright Přemysl Janouch 2011, WTFPL
 *
 *  X     X     16\X  16\X  X     X     15\X  16\X  X     X     X     X
 *  X     X\8   .     .     7\X   X\13  .     .     X     7\X   12\X  X
 *  X     16\17 .     .     .     29\9  .     .     19\15 .     .     X
 *  X\9   .     .     6\23  .     .     .     11\11 .     .     .     X
 *  X\16  .     .     .     23\19 .     .     .     .     9\X   X     X
 *  X     X     X\22  .     .     .     19\8  .     .     .     30\X  6\X
 *  X     X     6\X   15\21 .     .     .     .     11\19 .     .     .
 *  X     X\22  .     .     .     11\21 .     .     .     4\8   .     .
 *  X     X\7   .     .     X\3   .     .     X\9   .     .     .     X
 *  X     X     X     X     X\12  .     .     X     X\12  .     .     X
 *
 *  Basically a bruteforce solver, makes use of just a few hints,
 *  hence it may be slow for some puzzles.
 *
 *  Apparently it's got a few bugs, too, since it didn't pass all
 *  of the "corner case" tests. I couldn't find the cause.
 *  (I don't have access to the tests.)
 *
 *  It's got a nice visual debug output mode (-DDEBUG). Yay!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <assert.h>


/* ===== Declarations ====================================================== */

#define MAX_SIZE 32

#define N1    0x001
#define N2    0x002
#define N3    0x004
#define N4    0x008
#define N5    0x010
#define N6    0x020
#define N7    0x040
#define N8    0x080
#define N9    0x100
#define NALL  0x1FF

#define N_CONV(i)   ((long) 1 << ((i) - 1))
#define N_ALONE(n)  !((n) & ((n) - 1))


enum {DOWN, RIGHT};
enum {OK, ERROR};

struct Field
{
	enum {BLANK, CLUE, NUMBER} type;

	/* NUMBER: Current number. */
	int number;
	/* NUMBER: Clue indexes.
	 * CLUE:   Clue values. */
	int clues[2];
	/* CLUE:   Bitmask of allowed numbers. */
	int masks[2];
	/* CLUE:   Number of fields following. */
	int nflds[2];
};

struct Kakuro
{
	/* One more for parsing (NL before EOF). */
	Field fields[MAX_SIZE][MAX_SIZE + 1];
	int width, height;
};


/* ===== Scanner =========================================================== */

#define GETC()      (c->c = fgetc (c->fp))
#define UNGETC()    ungetc (c->c, c->fp)
#define SCANINT(i)  fscanf (c->fp, "%d", (i))
#define RETURN(s)   longjmp (c->jb, 1 + s)

/** Scanner context structure. */
struct ScannerCtx
{
	FILE *fp;
	int c;

	Kakuro *k;
	int cur_x, cur_y;

	jmp_buf jb;
};

/** Check for an end of field. */
static int
load_check_end (ScannerCtx *c)
{
	switch (GETC ())
	{
	case EOF:
		/* Ignore blank line before end. */
		if (c->cur_x)
		{
			if (++c->cur_y >= MAX_SIZE)
				RETURN (ERROR);
			if (++c->cur_x != c->k->width)
				RETURN (ERROR);
		}

		/* Retrieve height information. */
		c->k->height = c->cur_y;
		if (c->k->width < 2 || c->k->height < 2)
			RETURN (ERROR);
		RETURN (OK);
	case '\n':
		/* Retrieve width information. */
		c->cur_x++;
		if (c->cur_y == 0)
			c->k->width = c->cur_x;
		else if (c->cur_x != c->k->width)
			RETURN (ERROR);

		c->cur_x = 0;
		if (++c->cur_y  > MAX_SIZE)
			RETURN (ERROR);
		return 1;
	case ' ':
		if (++c->cur_x == MAX_SIZE)
			RETURN (ERROR);
		return 1;
	default:
		UNGETC ();
		return 0;
	}
}

/** Load a single field of a Kakuro puzzle. */
static void
load_field (ScannerCtx *c)
{
	Field *f;

	f = &c->k->fields[c->cur_x][c->cur_y];

	switch (GETC ())
	{
	case EOF:
		load_check_end (c);
	case ' ':
		/* Just ignore spaces at this position. */
		return;
	case '.':
		/* Placeholder for a number. */
		f->type = Field::NUMBER;
		f->number = 0;
		break;
	case 'X':
		/* Might be no man's land. */
		f->type = Field::BLANK;
		if (load_check_end (c))
			return;

		/* Or a horizontal clue. */
		f->type = Field::CLUE;
		f->clues[DOWN] = 0;
		if (GETC () != '\\' ||
		    SCANINT (&f->clues[RIGHT]) != 1 ||
		    f->clues[RIGHT] <= 0)
			RETURN (ERROR);
		break;
	default:
		/* This has to be a vertical clue. */
		f->type = Field::CLUE;
		UNGETC ();
		if (SCANINT (&f->clues[DOWN]) != 1 ||
		    f->clues[DOWN] <= 0 ||
		    GETC () != '\\')
			RETURN (ERROR);

		/* Possibly with a horizontal clue, too. */
		if (SCANINT (&f->clues[RIGHT]) == 1)
		{
			if (f->clues[RIGHT] <= 0)
				RETURN (ERROR);
			break;
		}
		f->clues[RIGHT] = 0;
		if (GETC () != 'X')
			RETURN (ERROR);
	}

	/* Excpect an end of field. */
	if (!load_check_end (c))
		RETURN (ERROR);
}

/** Load a Kakuro puzzle. */
static int
load (Kakuro *k, FILE *fp)
{
	ScannerCtx c = {0};
	int status;

	c.fp = fp;
	c.k  = k;

	if ((status = setjmp (c.jb)))
		return status - 1;

	while (1)
		load_field (&c);
}


/* ===== Stolen optimization table ========================================= */

#define KMB(i)     static int kctm##i (int c) {switch (c) {
#define KMR(i, r)  case (i): return (r);
#define KME        default: return (N_CONV (c) - 1) & NALL;}}

KMB (2)
KMR (3,  N1|N2)
KMR (4,  N1|N3)
KMR (16, N7|N9)
KMR (17, N8|N9)
KME
KMB (3)
KMR (6,  N1|N2|N3)
KMR (7,  N1|N2|N4)
KMR (23, N6|N8|N9)
KMR (24, N7|N9|N9)
KME
KMB (4)
KMR (10, N1|N2|N3|N4)
KMR (11, N1|N2|N3|N5)
KMR (29, N5|N7|N8|N9)
KMR (30, N6|N7|N8|N9)
KME
KMB (5)
KMR (15, N1|N2|N3|N4|N5)
KMR (16, N1|N2|N3|N4|N6)
KMR (34, N4|N6|N7|N8|N9)
KMR (35, N5|N6|N7|N8|N9)
KME
KMB (6)
KMR (21, N1|N2|N3|N4|N5|N6)
KMR (22, N1|N2|N3|N4|N5|N7)
KMR (38, N3|N5|N6|N7|N8|N9)
KMR (39, N4|N5|N6|N7|N8|N9)
KME
KMB (7)
KMR (28, N1|N2|N3|N4|N5|N6|N7)
KMR (29, N1|N2|N3|N4|N5|N6|N8)
KMR (41, N2|N4|N5|N6|N7|N8|N9)
KMR (42, N3|N4|N5|N6|N7|N8|N9)
KME
KMB (8)
KMR (36, N1|N2|N3|N4|N5|N6|N7|N8)
KMR (37, N1|N2|N3|N4|N5|N6|N7|N9)
KMR (38, N1|N2|N3|N4|N5|N6|N8|N9)
KMR (39, N1|N2|N3|N4|N5|N7|N8|N9)
KMR (40, N1|N2|N3|N4|N6|N7|N8|N9)
KMR (41, N1|N2|N3|N5|N6|N7|N8|N9)
KMR (42, N1|N2|N4|N5|N6|N7|N8|N9)
KMR (43, N1|N3|N4|N5|N6|N7|N8|N9)
KMR (44, N2|N3|N4|N5|N6|N7|N8|N9)
KME
KMB (9)
KMR (45, N1|N2|N3|N4|N5|N6|N7|N8|N9)
KME


static int
clue_to_mask (int clue, int in)
{
	static int (*helpers[]) (int) =
		{kctm2, kctm3, kctm4, kctm5,
		 kctm6, kctm7, kctm8, kctm9};
	int i, n = 9, check_clue = 0;

	assert (in >= 1 && in <= 9);

	for (i = in; i--; )
		check_clue += n--;
	if (check_clue < clue)
		return 0;
	if (in == 1)
		return N_CONV (clue) & NALL;

	return helpers[in - 2](clue);
}


/* ===== Validity checking & preprocessing ================================= */

struct CheckCtx
{
	int dir, i, not_last;
	int clue, cnt;
	int clue_i;
	Field *clue_field;
};

static int
check_inner (CheckCtx *c, Field *f)
{
	switch (f->type)
	{
	case Field::CLUE:
		/* A clue without fields,
		 * invalid clue value. */
		if (c->clue * !c->cnt ||
			f->clues[c->dir] * !c->not_last ||
			f->clues[c->dir] > 45)
			return ERROR;

		c->clue = f->clues[c->dir];
		c->cnt = 0;
		break;
	case Field::BLANK:
		/* A clue without fields. */
		if (c->clue * !c->cnt)
			return ERROR;

		c->clue = 0;
		break;
	case Field::NUMBER:
		/* Fields without a clue,
		 * 9+ fields next to each other. */
		if (!c->clue || c->cnt == 9)
			return ERROR;

		c->cnt++;
	}

	return OK;
}

static void
preprocess_end_clue (CheckCtx *c, int n)
{
	c->clue_field->masks[c->dir] = clue_to_mask (c->clue, n);
	c->clue_field->nflds[c->dir] = n;
}

static int
preprocess (CheckCtx *c, Field *f)
{
	switch (f->type)
	{
	case Field::CLUE:
		if (f->clues[c->dir])
			c->clue_i = c->i;
	case Field::BLANK:
		if (c->clue * c->cnt)
			preprocess_end_clue (c, c->cnt);

		c->clue_field = f;
		break;
	case Field::NUMBER:
		if (c->clue * !c->not_last)
			preprocess_end_clue (c, c->cnt + 1);

		f->clues[c->dir] = c->clue_i;
	}

	return check_inner (c, f);
}

/** Check the validity of a Kakuro puzzle. */
static int
check (Kakuro *k)
{
	CheckCtx c = {0};
	int x, y;

	c.dir = RIGHT;
	for (y = 0; y < k->height; y++)
	{
		c.clue = 0;
		c.not_last = k->width;
		for (c.i = 0; c.not_last--; c.i++)
			if (preprocess (&c, &k->fields[c.i][y]))
				return ERROR;
	}

	c.dir = DOWN;
	for (x = 0; x < k->width; x++)
	{
		c.clue = 0;
		c.not_last = k->height;
		for (c.i = 0; c.not_last--; c.i++)
			if (preprocess (&c, &k->fields[x][c.i]))
				return ERROR;
	}

	return OK;
}


/* ===== Printing ========================================================== */

#define CLUE_DOWN(k, f, x)   (k)->fields[(x)] [(f)->clues[DOWN]]
#define CLUE_RIGHT(k, f, y)  (k)->fields[(f)->clues[RIGHT]][(y)]
#define MASK_DOWN(k, f, x)   CLUE_DOWN  (k, f, x).masks[DOWN]
#define MASK_RIGHT(k, f, y)  CLUE_RIGHT (k, f, y).masks[RIGHT]

#define MASK(k, f, x, y) \
	 (MASK_DOWN  ((k), (f), (x)) \
	& MASK_RIGHT ((k), (f), (y)))

#ifdef DEBUG
	#define DEBUG_PRINT(k) \
		do {print (k); putchar ('\n');} while (0)
#else
	#define DEBUG_PRINT(k)
#endif


static int
decode_n (int n)
{
	int i;

	for (i = 0; n; n >>= 1)
		i++;
	return i;
}

static int
print_field (Kakuro *k, int x, int y)
{
	Field *f;
	int r;

	f = &k->fields[x][y];
	switch (f->type)
	{
	case Field::BLANK:
#ifdef DEBUG
		return printf ("\033[30mX\033[0m") - 9;
#else /* ! DEBUG */
		return printf ("X");
#endif /* ! DEBUG */
	case Field::CLUE:
#ifdef DEBUG
		r  = f->clues[DOWN]
		   ? printf ("\033[1;31m%d\033[0m",
		     f->clues[DOWN])  - 11
		   : printf ("\033[30mX\033[0m")   - 9;
		r += f->clues[RIGHT]
		   ? printf ("\033[30m\\\033[1;32m%d\033[0m",
		     f->clues[RIGHT]) - 16
		   : printf ("\033[30m\\X\033[0m") - 9;
#else /* ! DEBUG */
		r  = f->clues[DOWN]
		   ? printf ("%d", f->clues[DOWN])
		   : printf ("X");
		r += f->clues[RIGHT]
		   ? printf ("\\%d", f->clues[RIGHT])
		   : printf ("\\X");
#endif /* ! DEBUG */
		return r;
	case Field::NUMBER:
		return f->number
		   ? printf ("%d", decode_n (f->number))
#ifdef DEBUG
		   : printf ("\033[33m%x\033[0m",
		     MASK (k, f, x, y)) - 9;
#else /* ! DEBUG */
		   : printf (".");
#endif /* ! DEBUG */
	default:
		return 0;
	}
}

/** Print out the Kakuro puzzle. */
static void
print (Kakuro *k)
{
	int x, y, space;

	for (y = 0; y < k->height; y++)
	for (x = 0; x < k->width;  x++)
	{
		space = 6 - print_field (k, x, y);

		if (k->width == x + 1)
			putchar ('\n');
		else while (space-- > 0)
			putchar (' ');
	}
}


/* ===== Solver ============================================================ */

#define FLIP(x, y, i) \
	do {MASK_DOWN  (k, f, (x)) ^= (i); \
	    MASK_RIGHT (k, f, (y)) ^= (i);} while (0)

#define ADD(x, y, i) \
	do {int __n = decode_n (i); \
	    Field *__d = &CLUE_DOWN  (k, f, (x)); \
	    Field *__r = &CLUE_RIGHT (k, f, (y)); \
	    __d->nflds[DOWN]  ++; \
	    __r->nflds[RIGHT] ++; \
	    __d->clues[DOWN]  += __n; \
	    __r->clues[RIGHT] += __n; \
	    FLIP ((x), (y), i);} while (0)
#define SUB(x, y, i) \
	do {int __n = decode_n (i); \
	    Field *__d = &CLUE_DOWN  (k, f, (x)); \
	    Field *__r = &CLUE_RIGHT (k, f, (y)); \
	    __d->nflds[DOWN]  --; \
	    __r->nflds[RIGHT] --; \
	    __d->clues[DOWN]  -= __n; \
	    __r->clues[RIGHT] -= __n; \
	    FLIP ((x), (y), i);} while (0)


struct FieldLoc
{
	int x, y;
	Field *f;
};


/* Filter out fields to fill using bruteforce. */
static int
filter_fields (Kakuro *k, FieldLoc fields[])
{
	Field *f;
	int x, y, mask, n_fields = 0;

	for (x = 0; x < k->width;  x++)
	for (y = 0; y < k->height; y++)
	{
		f = &k->fields[x][y];
		if (f->type != Field::NUMBER)
			continue;

		mask = MASK (k, f, x, y);

		/* Nothing can be put in here. */
		if (!mask)
			return 0;

		/* Only one solution possible. */
		if (N_ALONE (mask))
		{
			f->number = mask;
			SUB (x, y, mask);
			continue;
		}

		fields[n_fields].x = x;
		fields[n_fields].y = y;
		fields[n_fields].f = f;
		n_fields++;
	}

	return n_fields;
}


static void
copy_numbers (Kakuro *src, Kakuro *dst)
{
	int x, y;

	for (x = 0; x < src->width;  x++)
	for (y = 0; y < src->height; y++)
		dst->fields[x][y].number = src->fields[x][y].number;
}

static int
check_sums (Kakuro *k, Field *f, int x, int y)
{
	int diff_down, diff_right;
	int flds_down, flds_right;
	Field *clue_down, *clue_right;

	clue_down  = &CLUE_DOWN  (k, f, x);
	clue_right = &CLUE_RIGHT (k, f, y);

	flds_down  = clue_down ->nflds[DOWN];
	diff_down  = clue_down ->clues[DOWN];
	flds_right = clue_right->nflds[RIGHT];
	diff_right = clue_right->clues[RIGHT];

	/* There are still some fields, they can be filled;
	 * no other fields, nothing to fill them with. */
	return ((flds_down  * (diff_down  > 0))
	    || (!flds_down  * !diff_down))
	    && ((flds_right * (diff_right > 0))
	    || (!flds_right * !diff_right));
}

/** Solve the Kakuro puzzle. */
static int
solve (Kakuro *kk)
{
	FieldLoc *i, fields[MAX_SIZE * MAX_SIZE / 2];
	Kakuro *k, kw;
	int n_fields, n_solutions = 0;

	/* Make a copy of the input since we destroy
	 * the clues while solving. */
	kw = *kk;
	k  = &kw;

	DEBUG_PRINT (k);

	n_fields = filter_fields (k, fields);
	if (!n_fields)
		return 0;

	DEBUG_PRINT (k);

	for (i = fields; i >= fields; )
	{
		Field *f;
		int x, y, n, mask;

		f = i->f;
		x = i->x;
		y = i->y;

		mask = MASK (k, f, x, y);

		/* Get the minimal next variation
		 * and reset the number. */
		if (f->number)
		{
			n = f->number << 1;

			ADD (x, y, f->number);
			f->number = 0;
		}
		else
			n = 1;

		/* No next variation exists. */
		if (!mask || !(~(n - 1) & mask))
		{
			i--;
			continue;
		}

		/* Compute its value. */
		while (!(n & mask))
			n <<= 1;

		/* Place the number into the grid. */
		f->number = n;
		SUB (x, y, n);

		/* See if there's a problem. */
		if (!check_sums (k, f, x, y))
			continue;

		DEBUG_PRINT (k);

		/* If we find a solution, copy it
		 * back and try to find others
		 * by pretending it wasn't. */
		if (i - fields + 1 == n_fields)
		{
			if (!n_solutions++)
				copy_numbers (k, kk);
		}
		else
			i++;
	}

	return n_solutions;
}


/* ===== Main ============================================================== */

int
main (int argc, char *argv[])
{
	static Kakuro k;
	int n_solutions;

	puts ("Zadejte kakuro:");
	if (load (&k, stdin) || check (&k))
	{
		puts ("Nespravny vstup.");
		exit (EXIT_FAILURE);
	}

	n_solutions = solve (&k);
	switch (n_solutions)
	{
	case 0:
		puts ("Reseni neexistuje.");
		break;
	case 1:
		puts ("Kakuro ma jedno reseni:");
		print (&k);
		break;
	default:
		printf ("Celkem ruznych reseni: %d\n", n_solutions);
	}

	return 0;
}

