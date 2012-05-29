/**
 * @file parser.h
 * Parser.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 */

#ifndef __PARSER_H__
#define __PARSER_H__

/** An unexpected Token appeared in the input. */
class EParserUnexpected : public std::exception
{
	std::string message;
public:
	/** Initialize the exception using the invalid @a token. */
	EParserUnexpected (const Token &token);
	virtual ~EParserUnexpected () throw () {}
	/** Return a description of the event. */
	virtual const char *what () const throw ();
};

/** Wrapper for the Tokenizer. */
class Parser
{
	/** The underlying tokenizer. */
	Tokenizer &scanner;
	/** A deque containing all of the tokens read so far. */
	std::deque<Token> backtrace;

	/** Current position within the backtrace. */
	unsigned pos;
public:
	/** Initialize the object. */
	Parser (Tokenizer &scanner);
	/** Accept a token of given type. */
	bool accept (TokenType type);
	/** Accept any token. */
	void accept_any ();
	/** Expect a token of given type. */
	void expect (TokenType type) throw (EParserUnexpected);
	/** Return a reference to the last token.
	 *  If no token has been accepted or expected before,
	 *  the behaviour is undefined. */
	Token &last ();
	/** Go back a token. */
	bool go_back (int how_much = 1);
};

#endif /* ! __PARSER_H__ */

