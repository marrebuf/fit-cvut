#!/usr/bin/env python
# Shit in, shit out. This tool generates two implementations of
# the finite automaton used in ticket-machine.c and a graphviz
# source to create a huge-ass graph for it, as required.

import sys

def w (x): sys.stdout.write (x)

functions = {}
action_table = []
states_table = []

for q in range (0, 33):
	actions = []
	action_table.append (actions)

	states = []
	states_table.append (states)

	if q > 0:
		states.append (0)
		functions['r%d' % q] = \
			["c ()", 'd ("vraceno %d Kc")' % q]
		actions.append ("r%d" % q)
	else:
		states.append (0)
		actions.append ('c')

	for v in [1, 2, 5, 10, 20]:
		if (q <= 12):
			states.append (q + v)
			functions['p%d' % (q + v)] = \
				["c ()", 'd ("vhozeno %d Kc")' % (q + v)]
			actions.append ('p%d' % (q + v))
		else:
			states.append (q)
			actions.append (None)

	for v in [6, 8, 12]:
		if q > v:
			states.append (0)
			functions['g%dr%d' % (v, q - v)] = \
				["c ()", 'd ("vydavam za %d Kc")' % v,
			             'd ("vraceno %d Kc")' % (q - v)]
			actions.append ('g%dr%d' % (v, q - v))
		elif v == q:
			states.append (0)
			functions['g%d' % v] = \
				["c ()", 'd ("vydavam za %d Kc")' % v]
			actions.append ('g%d' % v)
		else:
			states.append (q)
			actions.append (None)

for name, f in functions.items ():
	w ('static void %s (void) { ' % name)
	for s in f: w ('%s; ' % s)
	w ('}\n')
w ("\n")

w ("static int g_prechody[N_STAVU][N_SYMBOLU] =\n")
w ("{\n");
for q in states_table:
	w ("\t{ ")
	for trans in q:
		w ("%2d, " % trans)
	w ("},\n")
w ("};\n\n")

w ("static akce_t g_vystupy[N_STAVU][N_SYMBOLU] =\n")
w ("{\n");
for q in action_table:
	w ("\t{ ")
	for action in q:
		w ("%-7s " % ((action if action else "0") + ","))
	w ("},\n")
w ("};\n\n")

w ("enum { ")
for q in range (len (action_table)): w ("q%d, " % q)
w ("};\n\n")

actions = ["STORNO", "VHOZENO_1", "VHOZENO_2", "VHOZENO_5", "VHOZENO_10",
           "VHOZENO_20", "JIZDENKA_6", "JIZDENKA_8", "JIZDENKA_12"]
w ("enum { ")
for a in range (len (actions)): w ("%s, " % actions[a])
w ("};\n\n")

w ("void\nmain ()\n{\n")
w ("\tint q = 0;\n")
w ("\twhile (1)\n\t{\n")
w ("\t\tint i = read_input ();\n")
w ("\t\tswitch (q)\n\t\t{\n")
for q in range (0, len (states_table)):
	w ("\t\tcase q%d:\n" % q)
	w ("\t\t\tswitch (i)\n\t\t\t{\n")
	for i in range (len (states_table[q])):
		a = action_table[q][i]
		nq = states_table[q][i]
		if not a and nq == q: continue
		w ("\t\t\tcase %s:\n" % actions[i])
		w ("\t\t\t\tq = q%d;\n" % nq)
		if a and a in functions:
			for c in functions[a]:
				w ("\t\t\t\t%s;\n" % c)
		w ("\t\t\t\tbreak;\n")
	w ("\t\t\tdefault:\n")
	w ("\t\t\t\tbreak;\n")
	w ("\t\t\t}\n")
	w ("\t\t\tbreak;\n")
w ("\t\tdefault:\n")
w ("\t\t\tbreak;\n")
w ("\t\t}\n")
w ("\t}\n")
w ("}\n\n")

w ("digraph G {\n")
for q in range (0, len (states_table)):
	for i in range (len (states_table[q])):
		a = action_table[q][i]
		nq = states_table[q][i]
		w ('\tq%d -> q%d [label = "%d/%s"];\n'
		       % (q, nq, i, #actions[i].lower ().replace ("_", " "),
		          a if a else ""))
w ("}\n\n")

