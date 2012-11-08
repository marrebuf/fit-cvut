#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>

#if (defined __x86_64__ || defined __amd64__) && defined __unix__
	#define RUN_MACHINE_CODE_UNIX
	#include <sys/mman.h>
#elif defined __WIN64__
	#define RUN_MACHINE_CODE_WIN64
	#include <windows.h>
	#define TokenType MyTokenType
#endif // __WIN64__

/* --- Utilities ------------------------------------------------------------ */

#define XALLOC_WRAP(value)                                                     \
	void *x = (value);                                                         \
	if (!x) {                                                                  \
		fputs ("Memory allocation failed\n", stderr);                          \
		exit (EXIT_FAILURE);                                                   \
	} return x;

static void *
xmalloc (size_t n)
{
	XALLOC_WRAP (malloc (n))
}

static void *
xcalloc (size_t n, size_t m)
{
	XALLOC_WRAP (calloc (n, m))
}

static void *
xrealloc (void *z, size_t n)
{
	XALLOC_WRAP (realloc (z, n))
}

static char *
xstrdup (const char *s)
{
	return strcpy (xmalloc (strlen (s) + 1), s);
}

/* --- String buffer -------------------------------------------------------- */

typedef struct Buffer Buffer;

struct Buffer
{
	char *s;
	size_t alloc, used;
};

#define BUFFER_INITIALIZER {NULL, 0, 0}

static void
buffer_append (Buffer *this, char c)
{
	if (!this->s)
		this->s = xmalloc (this->alloc = 8);
	else if (this->used == this->alloc)
		this->s = xrealloc (this->s, this->alloc <<= 1);

	this->s[this->used++] = c;
}

static void
buffer_append_n (Buffer *this, const void *s, size_t n)
{
	if (!this->s)
		this->s = xmalloc (this->alloc = 8);
	while (this->used + n > this->alloc)
		this->s = xrealloc (this->s, this->alloc <<= 1);

	memcpy (this->s + this->used, s, n);
	this->used += n;
}

/* --- Tokens --------------------------------------------------------------- */

typedef struct Token Token;

typedef enum {TOKEN_ALNUM, TOKEN_EPS, TOKEN_DASH, TOKEN_PIPE,
		TOKEN_LT, TOKEN_GT, TOKEN_EOL, TOKEN_END} TokenType;

struct Token
{
	TokenType type;                    //! Type of the token.
	unsigned line, col;                //! Location of the token within input.
	char *value;                       //! A token string, for TOKEN_ALNUM.
};

static Token *
token_new (unsigned line, unsigned col)
{
	Token *this = xmalloc (sizeof *this);
	this->line = line;
	this->col = col;
	this->value = NULL;
	return this;
}

static void
token_free (Token *this)
{
	free (this->value);
	free (this);
}

static const char *
token_type_to_string (TokenType type)
{
	switch (type)
	{
	case TOKEN_ALNUM:  return "alphanumeric";
	case TOKEN_EPS:    return "\\eps";
	case TOKEN_DASH:   return "-";
	case TOKEN_PIPE:   return "|";
	case TOKEN_LT:     return "<";
	case TOKEN_GT:     return ">";
	case TOKEN_EOL:    return "end of line";
	case TOKEN_END:    return "end of file";

	default:
		abort ();
	}
}

/* --- Scanner -------------------------------------------------------------- */

typedef struct Scanner Scanner;

struct Scanner
{
	unsigned line, col;                //! Current position within input.
	FILE *fp;                          //! Input stream.
};

static Scanner *
scanner_new (FILE *fp)
{
	assert (fp != NULL);

	Scanner *this = xmalloc (sizeof *this);
	this->line = 0;
	this->col = 0;
	this->fp = fp;
	return this;
}

static inline void
scanner_free (Scanner *this)
{
	free (this);
}

static Token *
scanner_next (Scanner *this)
{
	Token *token = token_new (this->line + 1, this->col + 1);
	int c;

next_start:
	switch ((c = fgetc (this->fp)))
	{
	case ' ':
	case '\t':
		this->col++;
	case '\r':
		goto next_start;

	case '-':  token->type = TOKEN_DASH;  this->col++;  return token;
	case '|':  token->type = TOKEN_PIPE;  this->col++;  return token;
	case '<':  token->type = TOKEN_LT;    this->col++;  return token;
	case '>':  token->type = TOKEN_GT;    this->col++;  return token;

	case '\n':
		token->type = TOKEN_EOL;
		this->col = 0;
		this->line++;
		return token;
	case EOF:
		token->type = TOKEN_END;
		return token;

	case '\\':
		if (fgetc (this->fp) != 'e'
		 || fgetc (this->fp) != 'p'
		 || fgetc (this->fp) != 's')
			goto next_fail;

		token->type = TOKEN_EPS;
		this->col += 4;
		return token;
	default:
		if (!isalnum (c))
			goto next_fail;

		Buffer buf = BUFFER_INITIALIZER;
		do buffer_append (&buf, c);
		while (isalnum ((c = fgetc (this->fp))));
		ungetc (c, this->fp);
	
		this->col += buf.used;
		buffer_append (&buf, '\0');

		token->value = buf.s;
		token->type = TOKEN_ALNUM;
		return token;
	}

next_fail:
	fprintf (stderr, "Unexpected input at %d:%d\n",
		token->line, token->col);
	token_free (token);
	ungetc (c, this->fp);
	return NULL;
}

/* --- Automata ------------------------------------------------------------- */

typedef struct State State;
typedef struct Table Table;

struct State
{
	char *name;                        //! The name of the state.
	unsigned defined  : 1;             //! Has the state been defined yet?
	unsigned is_start : 1;             //! Is this the start state?
	unsigned is_final : 1;             //! Is this a final state?

	uint64_t *trans;                   //! Transitions for each symbol.
	                                   //! This is a bitmap.
};

struct Table
{
	enum TableType {TABLE_E_NFA, TABLE_NFA, TABLE_DFA} type;

	char *symbols;                     //! Input symbols. 0 is for epsilon.
	size_t symbols_len;                //! Count of input symbols.
	size_t symbols_alloc;              //! How many symbols were allocated.

	State *states;                     //! All the states.
	size_t states_len;                 //! Count of the states.
	size_t states_alloc;               //! How many states were allocated.

	int epsilon_index;                 //! Index of the epsilon symbol.
};

static void
table_free (Table *this)
{
	size_t i;
	for (i = 0; i < this->states_len; i++)
	{
		free (this->states[i].name);
		free (this->states[i].trans);
	}

	free (this->states);
	free (this->symbols);
	free (this);
}

static inline void
_table_print_transition (Table *this, uint64_t trans)
{
	putchar (' ');
	if (!trans)
	{
		putchar ('-');
		return;
	}

	unsigned state;
	bool need_pipe = false;
	for (state = 0; state < sizeof trans * 8; state++)
	{
		if (!(trans & (1ULL << state)))
			continue;

		assert (state < this->states_len);
		if (need_pipe)
			putchar ('|');
		fputs (this->states[state].name, stdout);
		need_pipe = true;
	}
}

static void
table_print (Table *this)
{
	switch (this->type)
	{
	case TABLE_E_NFA:
	case TABLE_NFA:    fputs ("NFA", stdout);  break;
	case TABLE_DFA:    fputs ("DFA", stdout);  break;
	default:           abort ();
	}

	size_t i;
	for (i = 0; i < this->symbols_len; i++)
	{
		if (this->symbols[i])
			printf (" %c", this->symbols[i]);
		else
			fputs (" \\eps", stdout);
	}
	putchar ('\n');

	for (i = 0; i < this->states_len; i++)
	{
		State *s = &this->states[i];
		if (s->is_start)  putchar ('>');
		if (s->is_final)  putchar ('<');
		fputs (s->name, stdout);

		size_t k;
		for (k = 0; k < this->symbols_len; k++)
			_table_print_transition (this, s->trans[k]);
		putchar ('\n');
	}
}

static void
_table_epsilon_union (Table *this, Table *orig,
	uint64_t *output, size_t cur_state, uint64_t *visited, bool *final)
{
	assert (cur_state < this->states_len);

	unsigned state;
	uint64_t trans = orig->states[cur_state].trans[orig->epsilon_index];
	for (state = 0; state < sizeof trans * 8; state++)
	{
		uint64_t mask = 1ULL << state;
		if (!(trans & mask))
			continue;

		size_t i;
		for (i = 0; i < this->symbols_len; i++)
			output[i] |= this->states[state].trans[i];
		*final |= this->states[state].is_final;

		// Keep track of visited states to avoid infinite recursion.
		if (!(*visited & mask))
		{
			*visited |= mask;
			_table_epsilon_union (this, orig, output, state, visited, final);
		}
	}
}

static Table *
table_remove_epsilon (Table *this)
{
	assert (this->type == TABLE_E_NFA);

	Table *new = xcalloc (sizeof *new, 1);
	new->type = TABLE_NFA;
	new->symbols = xmalloc (new->symbols_len = this->symbols_len - 1);
	new->states = xmalloc ((new->states_len = this->states_len)
		* sizeof *new->states);
	new->epsilon_index = -1;

	assert (this->epsilon_index >= 0);
	assert ((unsigned) this->epsilon_index < this->symbols_len);

	// Copy all symbols except epsilon.
	int epsilon = this->epsilon_index;
	memcpy (new->symbols, this->symbols, epsilon);
	memcpy (new->symbols + epsilon, this->symbols + epsilon + 1,
		new->symbols_len - epsilon);

	// Copy all transitions except epsilon.
	size_t i;
	for (i = 0; i < this->states_len; i++)
	{
		State *s_new = &new ->states[i];
		State *s_old = &this->states[i];

		s_new->name     = xstrdup (s_old->name);
		s_new->defined  = s_old->defined;
		s_new->is_start = s_old->is_start;
		s_new->is_final = s_old->is_final;

		s_new->trans = xmalloc (new->symbols_len * sizeof *s_new->trans);

		memcpy (s_new->trans, s_old->trans, epsilon * sizeof *s_new->trans);
		memcpy (s_new->trans + epsilon, s_old->trans + epsilon + 1,
			(new->symbols_len - epsilon) * sizeof *s_new->trans);
	}

	// Compute epsilon closures.
	for (i = 0; i < this->states_len; i++)
	{
		uint64_t visited = 1ULL << i;
		bool final = new->states[i].is_final;
		_table_epsilon_union (new, this,
			new->states[i].trans, i, &visited, &final);
		new->states[i].is_final = final;
	}

	return new;
}

static bool
_table_add_state (Table *this, const char *name)
{
	if (this->states_len == 64)
		return false;

	if (this->states_len == this->states_alloc)
		this->states = xrealloc (this->states,
			sizeof *this->states * (this->states_alloc <<= 1));

	State *s = &this->states[this->states_len++];
	s->name = xstrdup (name);
	s->defined = false;
	s->is_start = false;
	s->is_final = false;
	s->trans = xcalloc (this->symbols_len, sizeof *s->trans);
	return true;
}

static unsigned
_table_determinize_state (Table *new, Table *this,
	uint64_t state, uint64_t **states_masks)
{
	// Similar to _table_get_state() somewhere below.
	size_t i;
	for (i = 0; i < new->states_len; i++)
		if ((*states_masks)[i] == state)
			return i;

	unsigned new_state = i;
	char buf[8];
	snprintf (buf, sizeof buf, "%d", new_state);

	if (!_table_add_state (new, buf))
	{
		fputs ("Too many states for me (>64)\n", stderr);
		exit (EXIT_FAILURE);
	}

	*states_masks = xrealloc (*states_masks,
		sizeof **states_masks * new->states_alloc);

	(*states_masks)[new_state] = state;
	State *s = &new->states[new_state];

	// Combine the substates into it.
	unsigned substate;
	for (substate = 0; substate < sizeof state * 8; substate++)
	{
		if (!((1ULL << substate) & state))
			continue;

		s->is_final |= this->states[substate].is_final;
		for (i = 0; i < this->symbols_len; i++)
			s->trans[i] |= this->states[substate].trans[i];
	}

	// Translate state subsets into determinized states.
	for (i = 0; i < new->symbols_len; i++)
	{
		if (!s->trans[i])
			continue;

		unsigned trans = _table_determinize_state
			(new, this, s->trans[i], states_masks);
		s = &new->states[new_state];
		s->trans[i] = 1ULL << trans;
	}

	s->defined = true;
	return new_state;
}

static Table *
table_determinize (Table *this)
{
	assert (this->type != TABLE_E_NFA);

	Table *new = xcalloc (sizeof *new, 1);
	new->type = TABLE_DFA;
	new->symbols = xmalloc (new->symbols_len = this->symbols_len);
	memcpy (new->symbols, this->symbols, this->symbols_len);
	new->states = xmalloc (sizeof *new->states * (new->states_alloc = 16));
	new->states_len = 0;
	new->epsilon_index = -1;

	// Find the start states.
	uint64_t new_start = 0;
	size_t i;
	for (i = 0; i < this->states_len; i++)
		if (this->states[i].is_start)
			new_start |= 1ULL << i;
	assert (new_start != 0);

	// Scan the original table and add new numbered states.
	uint64_t *states_masks = xmalloc (sizeof *states_masks * new->states_alloc);
	_table_determinize_state (new, this, new_start, &states_masks);
	free (states_masks);

	new->states[0].is_start = true;
	return new;
}

// Not needed anymore.  It might come handy later, though...
static void
_table_reduce_start_states (Table *this)
{
	assert (this->type != TABLE_E_NFA);

	uint64_t states = 0;
	size_t i;
	for (i = 0; i < this->states_len; i++)
		if (this->states[i].is_start)
			states |= 1ULL << i;

	assert (states != 0);
	if (!(states & (states - 1)))
		return;

	// Find a unique name for the new state.
	char name[8];
	unsigned start_id = 0;

	do
	{
		snprintf (name, sizeof name, "%d", start_id++);
		for (i = 0; i < this->states_len; i++)
			if (!strcmp (this->states[i].name, name))
				break;
	}
	while (i != this->states_len);

	// Allocate the entry.
	unsigned new_state = i;
	if (!_table_add_state (this, name))
	{
		fputs ("Too many states for me (>64)\n", stderr);
		exit (EXIT_FAILURE);
	}

	// Make the new state go on epsilon to all of the original
	// start states; only without any actual epsilon involved.
	State *s = &this->states[new_state];
	s->defined  = true;
	s->is_start = true;

	unsigned m;
	for (m = 0; m < sizeof states * 8; m++)
	{
		if (!(states & (1ULL << m)))
			continue;
		this->states[m].is_start = false;
		this->states[new_state].is_final |= this->states[m].is_final;

		size_t k;
		for (k = 0; k < this->symbols_len; k++)
			this->states[new_state].trans[k] |=
				this->states[m].trans[k];
	}
}

static Table *
table_revert_edges (Table *this)
{
	assert (this->type != TABLE_E_NFA);

	Table *new = xcalloc (sizeof *new, 1);
	new->type = TABLE_NFA;
	new->symbols = xmalloc (new->symbols_len = this->symbols_len);
	memcpy (new->symbols, this->symbols, this->symbols_len);
	new->states = xmalloc ((new->states_alloc =
		new->states_len = this->states_len) * sizeof *new->states);
	new->epsilon_index = -1;

	// Initialize an empty copy of the transition table.
	size_t i;
	for (i = 0; i < this->states_len; i++)
	{
		State *s_new = &new ->states[i];
		State *s_old = &this->states[i];

		s_new->name     = xstrdup (s_old->name);
		s_new->defined  = s_old->defined;
		s_new->is_final = s_old->is_start;
		s_new->is_start = s_old->is_final;

		s_new->trans = xcalloc (new->symbols_len, sizeof *s_new->trans);
	}

	// Revert edges.
	size_t k;
	for (k = 0; k < this->symbols_len; k++)
		for (i = 0; i < this->states_len; i++)
	{
		uint64_t state = this->states[i].trans[k];
		if (!state)
			continue;

		unsigned m;
		for (m = 0; m < sizeof state * 8; m++)
			if (((1ULL << m) & state))
				new->states[m].trans[k] |= 1ULL << i;
	}

	return new;
}

static Table *
table_minimize (Table *this)
{
	assert (this->type == TABLE_DFA);

	// Brzozowski's algorithm:
	Table *rev = table_revert_edges (this);
	Table *det = table_determinize (rev);
	table_free (rev);
	rev = table_revert_edges (det);
	table_free (det);
	det = table_determinize (rev);
	table_free (rev);

	return det;
}

static size_t
_table_algorithmize_find_start_state (Table *this)
{
	assert (this->type == TABLE_DFA);

	bool found_start = false;
	size_t i, start_state = 0;
	for (i = 0; i < this->states_len; i++)
	{
		if (!this->states[i].is_start)
			continue;

		if (found_start)
		{
			fputs ("Error: I can't produce an algorithm "
				"for multiple start states\n", stderr);
			exit (EXIT_FAILURE);
		}
		start_state = i;
		found_start = true;
	}

	assert (found_start == true);
	return start_state;
}

static void
table_to_code (Table *this)
{
	size_t i, start_state = _table_algorithmize_find_start_state (this);

	fputs ("bool checkString (const char *s) {\n"
		"  #define ACCEPT case 0: return true;\n"
		"  #define END default: return false; }\n"
#ifdef USE_GOTO
		"  #define S(a) a: switch (*s++) {\n"
		"  #define T(a,b) case a: goto b;\n\n", stdout);

	size_t xi;
	for (xi = 0; xi < this->states_len; xi++)
	{
		if (!xi)  i = start_state;
		else      i = (xi == start_state) ? 0 : xi;
#else // ! USE_GOTO
		"  #define BEGIN while (true) switch (q) {\n"
		"  #define S(a) case a: switch (*s++) {\n"
		"  #define T(a,b) case a: q = b; continue;\n\n"
		"  enum {", stdout);

	bool need_comma = false;
	for (i = 0; i < this->states_len; i++)
	{
		if (need_comma)
		{
			if (i != 0 && i % 10 == 0)
				fputs (",\n    ", stdout);
			else
				fputs (", ", stdout);
		}
		printf ("S%s", this->states[i].name);
		need_comma = true;
	}

	printf ("} q = S%s;\n\n  BEGIN\n", this->states[start_state].name);

	for (i = 0; i < this->states_len; i++)
	{
#endif // ! USE_GOTO
		printf ("  S(S%s)", this->states[i].name);

		size_t k;
		for (k = 0; k < this->symbols_len; k++)
		{
			uint64_t b = this->states[i].trans[k];
			if (!b)
				continue;

			unsigned state = 0;
			while (b >>= 1)
				state++;

			printf (" T('%c',S%s)",
				this->symbols[k], this->states[state].name);
		}

		if (this->states[i].is_final)
			fputs (" ACCEPT", stdout);
		puts (" END");
	}

#ifndef USE_GOTO
	puts ("  END");
#endif // ! USE_GOTO
	puts ("}");
}

/* --- Automata - Parsing --------------------------------------------------- */

typedef struct Parser Parser;

struct Parser
{
	Scanner *s;                        //! Where tokens come from.
	Token *token, *last;               //! The current and the previous token.
	jmp_buf error;                     //! Error handler.
};

static void
parser_error (Parser *this, const char *format, ...)
{
	va_list ap;

	va_start (ap, format);
	fputs ("Parse error: ", stderr);
	vfprintf (stderr, format, ap);
	fputc ('\n', stderr);
	va_end (ap);

	longjmp (this->error, 1);
}

static void
parser_read (Parser *this)
{
	if (this->last)
		token_free (this->last);
	this->last = this->token;
	this->token = scanner_next (this->s);
	if (!this->token)
		longjmp (this->error, 1);
}

static void
parser_free (Parser *this)
{
	if (this->last)   token_free (this->last);
	if (this->token)  token_free (this->token);

	scanner_free (this->s);
	free (this);
}

static Parser *
parser_new (Scanner *s)
{
	assert (s != NULL);

	Parser *this = xmalloc (sizeof *this);
	this->s = s;
	this->last = NULL;
	this->token = NULL;

	if (setjmp (this->error))
	{
		parser_free (this);
		return NULL;
	}

	// Could also have an init() method.
	parser_read (this);
	return this;

}

static bool
parser_accept (Parser *this, TokenType type)
{
	if (this->token->type != type)
		return false;
	parser_read (this);
	return true;
}

static void
parser_expect (Parser *this, TokenType type)
{
	if (!parser_accept (this, type))
		parser_error (this, "Expected ‘%s’, got ‘%s’ at %d:%d",
			token_type_to_string (type),
			token_type_to_string (this->token->type),
			this->token->line, this->token->col);
}

static char
_table_load_symbol (Parser *p)
{
	if (parser_accept (p, TOKEN_EPS))
		return 0;
	else
	{
		parser_expect (p, TOKEN_ALNUM);
		if (strlen (p->last->value) != 1)
			parser_error (p, "Input symbols must be of length 1: %s",
				p->last->value);
		return p->last->value[0];
	}
}

static inline void
_table_load_alphabet (Table *this, Parser *p)
{
	this->symbols = xmalloc (this->symbols_alloc = 16);
	this->symbols_len = 0;
	this->epsilon_index = -1;

	do
	{
		char symbol = _table_load_symbol (p);
		size_t i;
		for (i = 0; i < this->symbols_len; i++)
			if (this->symbols[i] == symbol)
				parser_error (p, "Can't have repeating input symbols: %c",
					symbol);
		if (!symbol)
			this->epsilon_index = this->symbols_len;
		if (this->symbols_len == this->symbols_alloc)
			this->symbols = xrealloc (this->symbols, this->symbols_alloc <<= 1);
		this->symbols[this->symbols_len++] = symbol;
	}
	while (!parser_accept (p, TOKEN_EOL));

	if (this->epsilon_index != -1)
	{
		if (this->type == TABLE_DFA)
			parser_error (p, "Can't have DFA's with epsilon transitions");
		this->type = TABLE_E_NFA;
	}
}

static unsigned
_table_get_state (Table *this, Parser *p, const char *name)
{
	size_t i;
	for (i = 0; i < this->states_len; i++)
		if (!strcmp (name, this->states[i].name))
			return i;

	if (!_table_add_state (this, name))
		parser_error (p, "Too many states for me (>64)");
	return i;
}

static void
_table_load_transition (Table *this, Parser *p, uint64_t *trans)
{
	if (parser_accept (p, TOKEN_DASH))
		return;

	do
	{
		parser_expect (p, TOKEN_ALNUM);
		unsigned state = _table_get_state (this, p, p->last->value);
		*trans |= 1ULL << state;
	}
	while (this->type != TABLE_DFA && parser_accept (p, TOKEN_PIPE));
}

static inline void
_table_load_states (Table *this, Parser *p)
{
	this->states = xmalloc (sizeof *this->states * (this->states_alloc = 16));
	this->states_len = 0;

	bool have_start = false;
	bool have_final = false;

	while (!parser_accept (p, TOKEN_END))
	{
		bool is_start = false;
		if (parser_accept (p, TOKEN_GT))
			have_start = is_start = true;

		bool is_final = false;
		if (parser_accept (p, TOKEN_LT))
			have_final = is_final = true;

		parser_expect (p, TOKEN_ALNUM);
		unsigned state = _table_get_state (this, p, p->last->value);
		State *s = &this->states[state];

		if (s->defined)
			parser_error (p, "Redefining state %s on line %d",
				p->last->value, p->last->line);

		s->defined = true;
		s->is_start = is_start;
		s->is_final = is_final;

		size_t i;
		for (i = 0; i < this->symbols_len; i++)
		{
			_table_load_transition (this, p, &s->trans[i]);
			s = &this->states[state];
		}

		if (!parser_accept (p, TOKEN_END))
			parser_expect (p, TOKEN_EOL);
	}

	if (!this->states_len)  parser_error (p, "No states");
	if (!have_start)        parser_error (p, "No start state");
	if (!have_final)        parser_error (p, "No final state");

	size_t i;
	for (i = 0; i < this->states_len; i++)
		if (!this->states[i].defined)
			parser_error (p, "Undefined state: %s", this->states[i].name);
}

static Table *
table_load (FILE *fp)
{
	Parser *p = parser_new (scanner_new (fp));
	if (!p)
		return NULL;

	Table *this = xcalloc (sizeof *this, 1);
	if (setjmp (p->error))
	{
		table_free (this);
		parser_free (p);
		return NULL;
	}

	parser_expect (p, TOKEN_ALNUM);
	if      (!strcmp (p->last->value, "NFA"))  this->type = TABLE_NFA;
	else if (!strcmp (p->last->value, "DFA"))  this->type = TABLE_DFA;
	else     parser_error (p, "Invalid table type: %s", p->last->value);

	_table_load_alphabet (this, p);
	_table_load_states (this, p);

	parser_free (p);
	return this;
}

/* --- Compiling ------------------------------------------------------------ */

typedef struct AddressLink AddressLink;

struct AddressLink
{
	unsigned state;                    // ID of the state to jump to.
	unsigned offset;                   // Offset to the address within code.
	AddressLink *next;                 // The next link in order.
};

typedef enum {AMD64_MS, AMD64_SYSV} Amd64Convention;

static void *
table_compile_amd64 (Table *this, Amd64Convention convention, size_t *size)
{
	size_t i, start_state = _table_algorithmize_find_start_state (this);

	#define APPEND_CODE(x)  buffer_append_n (&code, x, sizeof x - 1)

	static const char
		xloadms[] = "\x0F\xB6\x01"               // movzx eax, byte [rcx]
		            "\x48\x83\xC1\x01",          // add rcx, byte 1
		xloadsv[] = "\x0F\xB6\x07"               // movzx eax, byte [rdi]
		            "\x48\x83\xC7\x01",          // add rdi, byte 1
		xinput [] = "\x3C" /* id */,             // cmp al, <id>
		xdelta [] = "\x0F\x84\x00\x00\x00\x00",  // jz <addr32>
		xaccept[] = "\x84\xC0"                   // test al, al
		            "\x0F\x94\xC0"               // setz al
		            "\x0F\xB6\xC0",              // movzx eax, al
		xreject[] = "\x31\xC0",                  // xor eax, eax
		xreturn[] = "\xC3";                      // ret

	Buffer code = BUFFER_INITIALIZER;
	unsigned state_offsets[this->states_len];
	AddressLink *fixups = NULL, *iter;

	size_t xi;
	for (xi = 0; xi < this->states_len; xi++)
	{
		if (!xi)  i = start_state;
		else      i = (xi == start_state) ? 0 : xi;

		state_offsets[i] = code.used;
		if (convention == AMD64_MS)
			APPEND_CODE (xloadms);
		else
			APPEND_CODE (xloadsv);

		size_t k;
		for (k = 0; k < this->symbols_len; k++)
		{
			uint64_t b = this->states[i].trans[k];
			if (!b)
				continue;

			unsigned state = 0;
			while (b >>= 1)
				state++;

			APPEND_CODE (xinput);
			buffer_append (&code, this->symbols[k]);
			APPEND_CODE (xdelta);

			AddressLink *link = xmalloc (sizeof *link);
			link->state = state;
			link->offset = code.used - 4;
			link->next = fixups;
			fixups = link;
		}

		if (this->states[i].is_final)
			APPEND_CODE (xaccept);
		else
			APPEND_CODE (xreject);
		APPEND_CODE (xreturn);
	}

	// Fix relative jumps to other states.
	for (iter = fixups; iter; iter = fixups)
	{
		int32_t rel32
			= (int32_t) state_offsets[iter->state]
			- (int32_t) (iter->offset + 4);

		code.s[iter->offset]     =  rel32        & 0xFF;
		code.s[iter->offset + 1] = (rel32 >> 8)  & 0xFF;
		code.s[iter->offset + 2] = (rel32 >> 16) & 0xFF;
		code.s[iter->offset + 3] = (rel32 >> 24) & 0xFF;

		fixups = iter->next;
		free (iter);
	}

	if (size)
		*size = code.used;

	return code.s;
}

/* --- Processing ----------------------------------------------------------- */

static void
print_machine_code (unsigned char *code, size_t size)
{
	printf ("const char *code = \"");

	unsigned i;
	for (i = 0; i < size; i++)
	{
		if (i != 0 && !(i % 16))
			fputs ("\"\n\t\"", stdout);
		printf ("\\x%02x", code[i]);
	}

	fputs ("\";\n\n", stdout);
}

static void
test_machine_code (void *executable)
{
	puts ("Here you can test inputs on the machine. "
#if defined __WIN32__ || defined __WIN64__
		"Press ^Z and Enter to terminate.");
#else // ! __WIN32__ && ! __WIN64__
		"Press ^D to terminate.");
#endif // ! __WIN32__ && ! __WIN64__

	char buf[80];
	fputs ("> ", stdout);
	fflush (stdout);
	while (fgets (buf, sizeof buf, stdin))
	{
		char *end = strpbrk (buf, "\r\n");
		if (end)  *end = '\0';

		puts (((uint32_t (*) (const char *)) executable) (buf)
			? "< accepted" : "< rejected");
		fputs ("> ", stdout);
		fflush (stdout);
	}
	fputs ("\n\n", stdout);
}

static inline void
create_machine_code_sysv (Table *table)
{
	size_t size;
	unsigned char *compiled = table_compile_amd64 (table, AMD64_SYSV, &size);

	puts ("/* amd64 machine code, System V calling convention */");
	print_machine_code (compiled, size);

#ifdef RUN_MACHINE_CODE_UNIX
	if (feof (stdin))
		goto cmcs_cleanup;

	void *executable = mmap (NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (!executable)
	{
		printf ("Error running the machine: mmap: %s\n", strerror (errno));
		goto cmcs_cleanup;
	}

	memcpy (executable, compiled, size);
	test_machine_code (executable);
	munmap (executable, size);

cmcs_cleanup:
#endif // RUN_MACHINE_CODE_UNIX
	free (compiled);
}

static inline void
create_machine_code_win64 (Table *table)
{
	size_t size;
	unsigned char *compiled = table_compile_amd64 (table, AMD64_MS, &size);

	puts ("/* amd64 machine code, Microsoft x64 calling convention */");
	print_machine_code (compiled, size);

#ifdef RUN_MACHINE_CODE_WIN64
	if (feof (stdin))
		goto cmcw_cleanup;

	void *executable = VirtualAlloc (NULL, size,
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!executable)
	{
		LPTSTR message = NULL;
		FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER
			| FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
			GetLastError (), MAKELANGID (LANG_ENGLISH, SUBLANG_DEFAULT),
			(LPTSTR) &message, 0, NULL);
		printf ("Error running the machine: VirtualAlloc: %s\n", message);
		LocalFree (message);
		goto cmcw_cleanup;
	}

	memcpy (executable, compiled, size);
	FlushInstructionCache (GetCurrentProcess (), executable, size);
	test_machine_code (executable);
	VirtualFree (executable, 0, MEM_RELEASE);

cmcw_cleanup:
#endif // RUN_MACHINE_CODE_WIN64
	free (compiled);
}

static void
transform (const char *msg, Table **ptable, Table *(*fun) (Table *))
{
	puts (msg);
	Table *new = fun (*ptable);
	table_free (*ptable);
	table_print (*ptable = new);
	putchar ('\n');
}

int
main (int argc, char *argv[])
{
	FILE *input = stdin;

	if (argc > 2)
	{
		fprintf (stderr, "Usage: %s [input-file]\n", argv[0]);
		exit (EXIT_FAILURE);
	}
	if (argc == 2)
	{
		const char *file = argv[1];
		if (!(input = fopen (file, "r")))
		{
			fprintf (stderr, "Load error: %s: %s\n", file, strerror (errno));
			exit (EXIT_FAILURE);
		}
	}

	Table *table = table_load (input);
	fclose (input);

	if (!table)
	{
		fputs ("Failed to load the table\n", stderr);
		exit (EXIT_FAILURE);
	}

	if (table->type == TABLE_E_NFA)
		transform ("Removing epsilon transitions...",
			&table, table_remove_epsilon);
	if (table->type == TABLE_NFA)
		transform ("Determinizing the NFA...", &table, table_determinize);

	transform ("Minimizing the DFA...", &table, table_minimize);

	table_to_code (table);
	putchar ('\n');

	create_machine_code_win64 (table);
	create_machine_code_sysv (table);

	transform ("Forming an NFA for the reversed language...",
		&table, table_revert_edges);

	table_free (table);
	return 0;
}

