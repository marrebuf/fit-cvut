/**
 * @file value.h
 * Values and their operations.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 */

#ifndef __VALUE_H__
#define __VALUE_H__

/** Invalid operand in relation to the operation. */
class EInvalidOperand : public std::exception
{
	std::string message;
public:
	/** Initialize the exception, using @a operation as
	 *  a textual description of the invalid operand. */
	EInvalidOperand (const char *operation, const char *info = NULL);
	virtual ~EInvalidOperand () throw () {}
	/** Return a description of the event. */
	virtual const char *what () const throw () {return message.c_str ();}
};

/** Wrapper for values. */
struct Value
{
	enum {INTEGER, REAL, MATRIX} type;  //!< The type of this value.

	union
	{
		long integer;    //!< Integer value.
		double real;     //!< Real number value.
		Matrix *matrix;  //!< Matrix value.
	};

	/** Initialize the value. */
	Value () : type (INTEGER), integer (0) {}
	/** Initialize the value with another one. */
	Value (const Value &value);
	~Value ();

	/** A flag toggling describing of operations onto standard output. */
	static bool describe;

	/** Use the contents of another Value. */
	Value &operator= (const Value &value);
	/** Print the value out into an output stream. */
	friend std::ostream &operator<< (std::ostream &os, const Value &v);

	Value binary_plus   (const Value &v);  //!< Addition.
	Value binary_minus  (const Value &v);  //!< Subtraction.
	Value binary_times  (const Value &v);  //!< Multiplication.
	Value binary_power  (const Value &v);  //!< Exponentiation.

	Value unary_minus      ();  //!< Unary subtraction.
	Value unary_rank       ();  //!< Matrix rank.
	Value unary_det        ();  //!< Matrix determinant.
	Value unary_transpose  ();  //!< Matrix transposition.
	Value unary_eliminate  ();  //!< Matrix elimination.
};

#endif /* ! __VALUE_H__ */

