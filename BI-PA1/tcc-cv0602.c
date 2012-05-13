#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>

#define CHECK(cond) \
	if (!(cond)) { \
		puts ("Nespravny vstup."); \
		return 1; \
	}

/** Return the index of the next point in a triangle. */
#define next(n) (((n) + 1) % 3)

/** Return rough distance of our point from the edge
 *  identified by its first point.
 */
#define location_to_edge(edge) \
	((point[1] - tri[edge][1]) * (tri[next (edge)][0] - tri[edge][0]) \
		- (point[0] - tri[edge][0]) * (tri[next (edge)][1] - tri[edge][1]))

/* Index names for clarity. */
enum {X, Y};
enum {A, B, C};
enum {AB, BC, CA};

int
main (int argc, char *argv[])
{
	int tri[3][2], point[2], loc[3];

	puts ("Zadejte trojúhelník [A1 A2 B1 B2 C1 C2]:");
	CHECK (scanf ("%d %d %d %d %d %d",
		&tri[A][X], &tri[A][Y],
		&tri[B][X], &tri[B][Y],
		&tri[C][X], &tri[C][Y]) == 6);

	puts ("Zadejte bod [P1 P2]:");
	CHECK (scanf ("%d %d", &point[X], &point[Y]) == 2);

	loc[AB] = location_to_edge (A);
	loc[BC] = location_to_edge (B);
	loc[CA] = location_to_edge (C);

	if (loc[AB] * loc[BC] > 0 && loc[BC] * loc[CA] > 0)
		puts ("Bod leží uvnitř trojúhelníku.");
	else
		puts ("Bod leží vně trojúhelníku.");

	return 0;
}

