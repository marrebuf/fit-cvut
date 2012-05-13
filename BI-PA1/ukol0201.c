#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define SQ(n) ((n) * (n))

#define CHECK(cond) \
	if (!(cond)) { \
		puts ("Nespravny vstup."); \
		exit (EXIT_FAILURE); \
	}

struct Shape
{
	enum {RECT, CIRCLE} type;
	union
	{
		struct {long x, y, r;} c;
		struct {long x1, x2, y1, y2;} r;
	};
};

static void
read_circle (Shape *s)
{
	assert (s != NULL);

	s->type = Shape::CIRCLE;

	puts ("Stred:");
	CHECK (scanf ("%ld %ld", &s->c.x, &s->c.y) == 2);

	puts ("Polomer:");
	CHECK (scanf ("%ld", &s->c.r) == 1);
	CHECK (s->c.r > 0);
}

static void
read_rect (Shape *s)
{
	long x1, x2, y1, y2;

	assert (s != NULL);

	s->type = Shape::RECT;

	puts ("Prvni bod:");
	CHECK (scanf ("%ld %ld", &x1, &y1) == 2);
	puts ("Druhy bod:");
	CHECK (scanf ("%ld %ld", &x2, &y2) == 2);

	CHECK (x1 != x2 && y1 != y2);

	if (x1 < x2)
	{
		s->r.x1 = x1;
		s->r.x2 = x2;
	}
	else
	{
		s->r.x1 = x2;
		s->r.x2 = x1;
	}

	if (y1 < y2)
	{
		s->r.y1 = y1;
		s->r.y2 = y2;
	}
	else
	{
		s->r.y1 = y2;
		s->r.y2 = y1;
	}
}

static int
check_rect_rect (Shape *s1, Shape *s2)
{
	assert (s1 != NULL && s2 != NULL);
	assert (s1->type == Shape::RECT && s2->type == Shape::RECT);

	return s1->r.x1 <= s2->r.x2 && s1->r.x2 >= s2->r.x1
		&& s1->r.y1 <= s2->r.y2 && s1->r.y2 >= s2->r.y1;
}

static int
check_circle_circle (Shape *s1, Shape *s2)
{
	long dx, dy;

	assert (s1 != NULL && s2 != NULL);
	assert (s1->type == Shape::CIRCLE && s2->type == Shape::CIRCLE);

	dx = s1->c.x - s2->c.x;
	dy = s1->c.y - s2->c.y;

	return sqrt (SQ (dx) + SQ (dy)) <= s1->c.r + s2->c.r;
}

static int
check_rect_circle (Shape *s1, Shape *s2)
{
	double w_2, h_2, dist_x, dist_y;

	assert (s1 != NULL && s2 != NULL);
	assert (s1->type == Shape::RECT && s2->type == Shape::CIRCLE);

	w_2 = (s1->r.x2 - s1->r.x1) / 2.;
	h_2 = (s1->r.y2 - s1->r.y1) / 2.;

	dist_x = fabs (s2->c.x - s1->r.x1 - w_2);
	dist_y = fabs (s2->c.y - s1->r.y1 - h_2);

	if (dist_x > (w_2 + s2->c.r) || dist_y > (h_2 + s2->c.r))
		return 0;
	if (dist_x <= w_2 || dist_y <= h_2)
		return 1;

	dist_x -= w_2;
	dist_y -= h_2;

	return SQ (dist_x) + SQ (dist_y) <= SQ (s2->c.r);
}

static int
check_intersection (Shape *s1, Shape *s2)
{
	assert (s1 != NULL && s2 != NULL);

	if (s1->type == Shape::RECT)
		return s2->type == Shape::RECT
			? check_rect_rect     (s1, s2)
			: check_rect_circle   (s1, s2);
	else
		return s2->type == Shape::RECT
			? check_rect_circle   (s2, s1)
			: check_circle_circle (s1, s2);
}

int
main (int argc, char *argv[])
{
	Shape shapes[2];
	int i;
	char c;

	for (i = 0; i < 2; i++)
	{
		printf ("Tvar %d (R=rectangle, C=circle):\n", i + 1);
		CHECK (scanf (" %c", &c) == 1);

		switch (c)
		{
		case 'C':
			read_circle (&shapes[i]);
			break;
		case 'R':
			read_rect (&shapes[i]);
			break;
		default:
			CHECK (0);
		}
	}

	puts (check_intersection (&shapes[0], &shapes[1])
		? "Prunik je neprazdny."
		: "Prunik je prazdny.");

	return 0;
}

