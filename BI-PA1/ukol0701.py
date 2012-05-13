#!/usr/bin/env python

def sum_bribes (budget, table):
	m = [0]
	for j in xrange (1, budget + 1):
		m.append (reduce (max, [m[j - i] + table[i]
			for i in xrange (1, min (len (table), j + 1))],
			m[j - 1]))
	return m[budget]

def test (budget, table, table_len, expected):
	result = sum_bribes (budget, table[:table_len + 1])
	if expected != result:
		print "FAIL: Should be %d, returned %d" % (expected, result)

test1 = [0, 100, 0, 0, 350, 0, 750]
test2 = [0, 1, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 53]
test3 = [0, 0, 0, 0, 0, 0, 0, 1]

test (1,  test1, 7, 100)
test (1,  test1, 7, 100)
test (2,  test1, 7, 200)
test (3,  test1, 7, 300)
test (4,  test1, 7, 400)
test (5,  test1, 7, 500)
test (6,  test1, 7, 750)
test (7,  test1, 7, 850)
test (8,  test1, 7, 950)
test (9,  test1, 7, 1050)
test (10, test1, 7, 1150)
test (11, test1, 7, 1250)
test (12, test1, 7, 1500)
test (13, test1, 7, 1600)
test (14, test1, 7, 1700)
test (15, test1, 7, 1800)
test (16, test1, 7, 1900)
test (17, test1, 7, 2000)

test (1,  test2, 16, 1)
test (2,  test2, 16, 7)
test (3,  test2, 16, 8)
test (4,  test2, 16, 14)
test (5,  test2, 16, 15)
test (6,  test2, 16, 21)
test (7,  test2, 16, 22)
test (8,  test2, 16, 28)
test (9,  test2, 16, 29)
test (10, test2, 16, 35)
test (11, test2, 16, 36)
test (12, test2, 16, 42)
test (13, test2, 16, 43)
test (14, test2, 16, 49)
test (15, test2, 16, 53)
test (16, test2, 16, 56)
test (17, test2, 16, 60)

test (1,  test3, 8, 0)
test (2,  test3, 8, 0)
test (3,  test3, 8, 0)
test (4,  test3, 8, 0)
test (5,  test3, 8, 0)
test (6,  test3, 8, 0)
test (7,  test3, 8, 1)
test (8,  test3, 8, 1)
test (9,  test3, 8, 1)
test (10, test3, 8, 1)
test (11, test3, 8, 1)
test (12, test3, 8, 1)
test (13, test3, 8, 1)
test (14, test3, 8, 2)
test (15, test3, 8, 2)
test (16, test3, 8, 2)
test (17, test3, 8, 2)

