/**
 * @file parseexpr.h
 * Expression parsing.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 */

#ifndef __PARSEEXPR_H__
#define __PARSEEXPR_H__

/** The expression was invalid. */
class EInvalidExpression : public std::exception
{
	std::string message;
public:
	/** Initialize the exception with the message. */
	EInvalidExpression (const std::string &message) : message (message) {}
	virtual ~EInvalidExpression () throw () {}
	/** Return a description of the event. */
	virtual const char *what () const throw () {return message.c_str ();}
};

/** Parse an expression. */
EvalNode *parse_expression (Parser &p)
	throw (EInvalidExpression, EParserUnexpected);

#endif /* ! __PARSEEXPR_H__ */
