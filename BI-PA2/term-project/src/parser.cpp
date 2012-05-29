/**
 * @file parser.cpp
 * Parser.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 */

#include <iostream>
#include <stack>
#include <exception>
#include <cstdio>

#include <config.h>

#include "tokenizer.h"
#include "parser.h"

#include "gettext.h"
#define _(String) gettext (String)

using namespace std;


EParserUnexpected::EParserUnexpected (const Token &token)
{
	char buff[80];
	snprintf (buff, sizeof buff, _("Unexpected `%s' at %d:%d"),
		token.to_string ().c_str (), token.line + 1, token.col);
	message = buff;
}

const char *
EParserUnexpected::what () const throw ()
{
	return message.c_str ();
}

Parser::Parser (Tokenizer &scanner) : scanner (scanner)
{
	pos = 0;
	accept_any ();
}

Token &
Parser::last ()
{
	/* The parser is always one step ahead. */
	return backtrace.at (pos + 1);
}

bool
Parser::go_back (int how_much)
{
	if (pos + how_much >= backtrace.size ())
		return false;

	pos += how_much;
	return true;
}

bool
Parser::accept (TokenType type)
{
	if (backtrace.at (pos).type != type)
		return false;

	accept_any ();
	return true;
}

void
Parser::accept_any ()
{
	if (pos)
		pos--;
	else
		backtrace.push_front (scanner.next_token ());
}

void
Parser::expect (TokenType type) throw (EParserUnexpected)
{
	if (!accept (type))
		throw EParserUnexpected (backtrace.at (pos));
}

