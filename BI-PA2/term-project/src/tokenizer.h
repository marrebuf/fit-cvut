/**
 * @file tokenizer.h
 * Tokenizer.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 */

#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

/** Token identifier. */
enum TokenType
{
	INVALID, END,

	/* Commands. */
	INPUT, DELETE, LOAD, SAVE, TYPEOF, EXIT, HELP, SET,

	/* Operators. */
	EQUALS, PLUS, MINUS, TIMES, POWER,
	RANK, DET, TRANSPOSE, ELIMINATE,

	LPAREN, RPAREN, COMMA,
	INTEGER, REAL, IDENT, STRING
};

/** Describes a token that's been read by the Tokenizer. */
struct Token
{
	TokenType type;  //!< Type of the token.
	std::string s;   //!< A string value.
	double n;        //!< A real number value.
	long i;          //!< An integer value.
	int col;         //!< "Horizontal" position of the token.
	int line;        //!< "Vertical" position of the token.

	/** Initialize the token with given type. */
	Token (TokenType type = INVALID) : type (type) {n = col = line = 0;}
	/** Get string representation of the token. */
	std::string to_string () const;
};

/** Input stream tokenizer. */
class Tokenizer
{
	std::istream &is;         //!< The underlying input stream.
	int cur_col;              //!< The current position within a line.
	int cur_line;             //!< The current line number.

	/** Retrieve a character from the input stream.
	 *  @return The character as returned by istream::get().
	 */
	int get ();

	/** Put a character back into the input stream.
	 *  @param[in] ch  The character to be put back.
	 */
	void put_back (int ch);

public:
	/** Initialize the tokenizer to use the @a s stream. */
	Tokenizer (std::istream &s);
	/** Retrieve the next token from the input stream. */
	Token next_token ();
};

#endif /* ! __TOKENIZER_H__ */
