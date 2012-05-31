/**
 * @file tokenizer.cpp
 * Tokenizer.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 */

#include <iostream>
#include <cctype>

#include <config.h>

#include "tokenizer.h"

#include "gettext.h"
#define _(String) gettext (String)

using namespace std;

string
Token::to_string () const
{
	switch (type)
	{
	default:
	case INVALID:    return _("invalid input");
	case END:        return _("end of input");

	case INPUT:
	case DELETE:
	case LOAD:
	case SAVE:
	case TYPEOF:
	case EXIT:
	case HELP:
	case SET:        return s;

	case EQUALS:     return "=";
	case PLUS:       return "+";
	case MINUS:      return "-";
	case TIMES:      return "*";
	case POWER:      return "^";

	case RANK:
	case DET:
	case TRANSPOSE:
	case ELIMINATE:  return s;

	case LPAREN:     return "(";
	case RPAREN:     return ")";
	case COMMA:      return ",";

	case INTEGER:    return _("integer");
	case REAL:       return _("real number");
	case IDENT:      return _("identifier");
	case STRING:     return _("string");
	}
}

/* Since this is a hand-written tokenizer, simplify the code by first reading
 * all strings as identifiers before trying to reinterpret them as keywords.
 *
 * This is the table used for the conversion.
 */
static const struct
{
	const char *word;
	enum TokenType token;
}
keywords[] =
{
	{"input",     INPUT},
	{"delete",    DELETE},
	{"load",      LOAD},
	{"save",      SAVE},
	{"typeof",    TYPEOF},
	{"exit",      EXIT},
	{"help",      HELP},
	{"set",       SET},

	{"rank",      RANK},
	{"det",       DET},
	{"transpose", TRANSPOSE},
	{"eliminate", ELIMINATE}
};

static const unsigned n_keywords = sizeof keywords / sizeof *keywords;


Tokenizer::Tokenizer (istream &s) : is (s)
{
	cur_col = cur_line = 0;
}

int
Tokenizer::get ()
{
	int ch = is.get ();
	if (ch == '\n')
	{
		cur_line++;
		cur_col = 0;
	}
	else
		cur_col++;

	return ch;
}

void
Tokenizer::put_back (int ch)
{
	if (ch == '\n')
		cur_line--;
	else
		cur_col--;

	is.putback (ch);
}

/** Declare the result being of type @a t and return. */
#define RETURN(t) \
	do {result.type = (t); return result;} while (0)

Token
Tokenizer::next_token ()
{
	Token result;
	int ch;

	/* Ignore whitespace at the beginning. */
	do
		ch = get ();
	while (isspace (ch));

	result.col  = cur_col;
	result.line = cur_line;

	if (!is.good ())
		RETURN (END);

	switch (ch)
	{
	case '=': RETURN (EQUALS);
	case '+': RETURN (PLUS);
	case '-': RETURN (MINUS);
	case '*': RETURN (TIMES);
	case '^': RETURN (POWER);
	case '(': RETURN (LPAREN);
	case ')': RETURN (RPAREN);
	case ',': RETURN (COMMA);
	}

	/* Regexp: [[:digit:]]+[.]?[[:digit:]]* */
	if (isdigit (ch))
	{
		result.n = 0;
		result.i = 0;

		do
		{
			result.n = result.n * 10 + (ch - '0');
			result.i = result.i * 10 + (ch - '0');
			ch = get ();
		}
		while (isdigit (ch));

		if (ch == '.')
		{
			double e = 1;
			for (ch = get (); isdigit (ch); ch = get ())
				result.n += (ch - '0') * (e /= 10);
			result.type = REAL;
		}
		else
			result.type = INTEGER;

		put_back (ch);
		return result;
	}

	/* Regexp: [[:alpha:]_][[:alnum:]_]* */
	if (isalpha (ch) || ch == '_')
	{
		do
		{
			result.s += ch;
			ch = get ();
		}
		while (isalnum (ch) || ch == '_');
		put_back (ch);

		/* Is it a keyword? */
		for (unsigned i = 0; i < n_keywords; i++)
			if (result.s == keywords[i].word)
				RETURN (keywords[i].token);

		RETURN (IDENT);
	}

	/* Regexp: "[^"]*" */
	if (ch == '"')
	{
		while ((ch = get ()) != '"' && is.good ())
			result.s += ch;

		if (ch == '"')
			RETURN (STRING);

		/* Fall through. */
		result.s.clear ();
	}

	RETURN (INVALID);
}
