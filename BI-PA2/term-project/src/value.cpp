/**
 * @file value.h
 * Values and their operations.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 */

#include <iostream>
#include <map>
#include <exception>

#include <cstdio>
#include <cmath>

#include <config.h>

#include "matrix.h"
#include "value.h"

#include "gettext.h"
#define _(String) gettext (String)
#define N_(String) gettext_noop (String)


using namespace std;

EInvalidOperand::EInvalidOperand (const char *operation, const char *info)
{
	char buff[160];
	snprintf (buff, sizeof buff, info
		? _("Invalid operands to `%s': %s")
		: _("Invalid operands to `%s'"),
		operation, info);
	message = buff;
}

bool Value::describe = false;

Value::Value (const Value &value)
{
	type = value.type;
	switch (type = value.type)
	{
	case INTEGER:
		integer = value.integer;
		break;
	case REAL:
		real    = value.real;
		break;
	case MATRIX:
		matrix  = new Matrix (*value.matrix);
		break;
	}
}

Value &
Value::operator= (const Value &value)
{
	if (this != &value)
	{
		this->~Value ();
		new (this) Value (value);
	}
	return *this;
}

Value::~Value ()
{
	if (type == MATRIX)
		delete matrix;
}

ostream &
operator<< (ostream &os, const Value &v)
{
	switch (v.type)
	{
	case Value::INTEGER:
		return os <<  v.integer;
	case Value::REAL:
		if (v.real == -0.)
			return os << 0.;
		return os <<  v.real;
	case Value::MATRIX:
		return os << *v.matrix;
	default:
		return os;
	}
}


/** Describe steps of Gaussian elimination. */
static void
value_describe_steps_cb (Matrix::EliminateStep &step)
{
	switch (step.step)
	{
	case Matrix::EliminateStep::SWAP:
		printf (_("Swapping rows %u and %u\n"),
			(step.par1 + 1), (step.par2 + 1));
		break;
	case Matrix::EliminateStep::SUB:
		printf (_("Subtracting %g times row %u from row %u\n"),
			step.factor, (step.par1 + 1), (step.par2 + 1));
		break;
	case Matrix::EliminateStep::MUL:
		printf (_("Multiplying row %u by %g\n"),
			(step.par1 + 1), step.factor);
		break;
	case Matrix::EliminateStep::GROUP:
		cout << _("Resulting in:") << endl;
		cout << Matrix (step.ms->clone ()) << endl;
		break;
	case Matrix::EliminateStep::START:
		cout << _("Beginning Gaussian elimination:") << endl;
		cout << Matrix (step.ms->clone ()) << endl;
		break;
	}
}

/** Shorthand for matching types of binary operands. */
#define types(a,b) if (type == a && v.type == b)

Value
Value::binary_plus (const Value &v)
{
	static const char *name = N_("plus");
	Value value;

	types (INTEGER, INTEGER)
	{
		value.type = INTEGER;
		value.integer = integer + v.integer;
	}
	else types (REAL, REAL)
	{
		value.type = REAL;
		value.real = real + v.real;
	}
	else types (INTEGER, REAL)
	{
		value.type = REAL;
		value.real = integer + v.real;
	}
	else types (REAL, INTEGER)
	{
		value.type = REAL;
		value.real = real + v.integer;
	}
	else types (MATRIX, MATRIX)
	{
		try
		{
			value.type = MATRIX;
			value.matrix = new Matrix (*matrix + *v.matrix);
		}
		catch (const EIncompatibleMatrix &im)
		{
			throw EInvalidOperand (_(name), im.what ());
		}
	}
	else
		throw EInvalidOperand (_(name));

	return value;
}

Value
Value::binary_minus (const Value &v)
{
	static const char *name = N_("minus");
	Value value;

	types (INTEGER, INTEGER)
	{
		value.type = INTEGER;
		value.integer = integer - v.integer;
	}
	else types (REAL, REAL)
	{
		value.type = REAL;
		value.real = real - v.real;
	}
	else types (INTEGER, REAL)
	{
		value.type = REAL;
		value.real = integer - v.real;
	}
	else types (REAL, INTEGER)
	{
		value.type = REAL;
		value.real = real - v.integer;
	}
	else types (MATRIX, MATRIX)
	{
		try
		{
			value.type = MATRIX;
			value.matrix = new Matrix (*matrix - *v.matrix);
		}
		catch (const EIncompatibleMatrix &im)
		{
			throw EInvalidOperand (_(name), im.what ());
		}
	}
	else
		throw EInvalidOperand (_(name));

	return value;
}

Value
Value::binary_times (const Value &v)
{
	static const char *name = N_("multiplication");
	Value value;

	types (INTEGER, INTEGER)
	{
		value.type = INTEGER;
		value.integer = integer * v.integer;
	}
	else types (REAL, REAL)
	{
		value.type = REAL;
		value.real = real * v.real;
	}
	else types (INTEGER, REAL)
	{
		value.type = REAL;
		value.real = integer * v.real;
	}
	else types (REAL, INTEGER)
	{
		value.type = REAL;
		value.real = real * v.integer;
	}
	else types (MATRIX, INTEGER)
	{
		value.type = MATRIX;
		value.matrix = new Matrix (*matrix * v.integer);
	}
	else types (INTEGER, MATRIX)
	{
		value.type = MATRIX;
		value.matrix = new Matrix (*v.matrix * integer);
	}
	else types (MATRIX, REAL)
	{
		value.type = MATRIX;
		value.matrix = new Matrix (*matrix * v.real);
	}
	else types (REAL, MATRIX)
	{
		value.type = MATRIX;
		value.matrix = new Matrix (*v.matrix * real);
	}
	else types (MATRIX, MATRIX)
	{
		try
		{
			value.type = MATRIX;
			value.matrix = new Matrix (*matrix * *v.matrix);
		}
		catch (const EIncompatibleMatrix &im)
		{
			throw EInvalidOperand (_(name), im.what ());
		}
	}
	else
		throw EInvalidOperand (_(name));

	return value;
}

Value
Value::binary_power (const Value &v)
{
	static const char *name = N_("exponentiation");
	Value value;

	types (INTEGER, INTEGER)
	{
		value.type = INTEGER;
		value.integer = pow (integer, v.integer);
	}
	else types (REAL, REAL)
	{
		value.type = REAL;
		value.real = pow (real, v.real);
	}
	else types (INTEGER, REAL)
	{
		value.type = REAL;
		value.real = pow (integer, v.real);
	}
	else types (REAL, INTEGER)
	{
		value.type = REAL;
		value.real = pow (real, v.integer);
	}
	else types (MATRIX, INTEGER)
	{
		if (!v.integer)
			throw EInvalidOperand (_(name));

		try
		{
			Matrix m (*matrix);
			for (long i = abs (v.integer); --i; )
				m = m * *matrix;
			if (v.integer < 0)
			{
				if (describe)
				{
					cout << _("# VERBOSE INVERSION") << endl;
					m = m.inverse (value_describe_steps_cb);
					cout << endl;
				}
				else
					m = m.inverse ();
			}

			value.type = MATRIX;
			value.matrix = new Matrix (m);
			return value;
		}
		catch (const EIncompatibleMatrix &im)
		{
			throw EInvalidOperand (_(name), im.what ());
		}
	}
	else
		throw EInvalidOperand (_(name));

	return value;
}

Value
Value::unary_minus ()
{
	Value value;

	switch (type)
	{
	case INTEGER:
		value = *this;
		value.integer = -integer;
		return value;
	case REAL:
		value = *this;
		value.real = -real;
		return value;
	case MATRIX:
		value.type = MATRIX;
		value.matrix = new Matrix (*matrix * -1);
		return value;
	default:
		throw EInvalidOperand (_("unary minus"));
	}
}

Value
Value::unary_rank ()
{
	if (type != MATRIX)
		throw EInvalidOperand (_("matrix rank"));

	unsigned rank;
	matrix->eliminate (&rank);

	Value value;
	value.type = INTEGER;
	value.integer = rank;
	return value;
}

Value
Value::unary_det ()
{
	static const char *name = N_("matrix determinant");

	if (type != MATRIX)
		throw EInvalidOperand (_(name));

	try
	{
		Value value;
		value.type = REAL;
		value.real = matrix->get_determinant ();
		return value;
	}
	catch (const EIncompatibleMatrix &)
	{
		throw EInvalidOperand (_(name));
	}
}

Value
Value::unary_transpose ()
{
	if (type != MATRIX)
		throw EInvalidOperand (_("matrix transposition"));

	Value value;
	value.type = MATRIX;
	value.matrix = new Matrix (matrix->transpose ());
	return value;
}

Value
Value::unary_eliminate ()
{
	if (type != MATRIX)
		throw EInvalidOperand (_("matrix elimination"));

	Value value;
	value.type = MATRIX;

	if (describe)
	{
		cout << _("# VERBOSE ELIMINATION") << endl;
		value.matrix = new Matrix (matrix->eliminate
			(NULL, value_describe_steps_cb));
		cout << endl;
	}
	else
		value.matrix = new Matrix (matrix->eliminate ());

	return value;
}
