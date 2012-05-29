/**
 * @file parseexpr.cpp
 * Expression parsing.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 */

#include <iostream>
#include <stack>
#include <map>

#include <config.h>

#include "matrix.h"
#include "tokenizer.h"
#include "parser.h"
#include "value.h"
#include "environ.h"
#include "evalnodes.h"
#include "parseexpr.h"

#include "gettext.h"
#define _(String) gettext (String)

using namespace std;


#define BINARY   0x10000
#define UNARY    0x20000
#define SPECIAL  0x40000

#define BI BINARY  |
#define UN UNARY   |
#define SP SPECIAL |

/** Abstract delegate pointer. */
typedef Value (Value::*OpDelegate) ();

#define DEL (OpDelegate) &Value::

enum Associativity {LEFT, RIGHT, NONE};

/** Maps tokens to operators. */
static const struct Operator
{
	int id;         //!< Compound operator identifier.
	OpDelegate d;   //!< Operation delegate.
	int prec;       //!< Operator precedence.
	int assoc;      //!< Operator associativity.
}
g_operators[] =
{
	{BI PLUS,      DEL binary_plus,     4, LEFT},
	{BI MINUS,     DEL binary_minus,    4, LEFT},
	{BI TIMES,     DEL binary_times,    3, LEFT},
	{BI POWER,     DEL binary_power,    2, RIGHT},
	{UN MINUS,     DEL unary_minus,     2, RIGHT},
	{UN RANK,      DEL unary_rank,      1, RIGHT},
	{UN DET,       DEL unary_det,       1, RIGHT},
	{UN TRANSPOSE, DEL unary_transpose, 1, RIGHT},
	{UN ELIMINATE, DEL unary_eliminate, 1, RIGHT},
	{SP LPAREN,    NULL,                0, NONE}
};

static const unsigned int g_n_operators
	= sizeof g_operators / sizeof *g_operators;

#define SHOULD_NOT_HAPPEN \
	throw EInvalidExpression (_("Expression parsing error"))


/** Translate a token mask to an operator using the given arity. */
static const Operator *
translate (const Token &token, int mask)
{
	for (unsigned i = 0; i < g_n_operators; i++)
	{
		if (g_operators[i].id == (token.type | mask))
			return &g_operators[i];
	}

	throw EParserUnexpected (token);
}

/** Apply the top of the operator stack on the output. */
static void
apply (stack<EvalNode *> &output, stack<const Operator *> &op_stack)
{
	if (op_stack.empty ())
		SHOULD_NOT_HAPPEN;

	const Operator *oper = op_stack.top ();
	op_stack.pop ();

	if (oper->id & UNARY)
	{
		if (output.empty ())
			SHOULD_NOT_HAPPEN;

		UnaryNode *node = new UnaryNode
			((UnaryNode::UnaryEvaluate) oper->d, output.top ());
		output.pop ();
		output.push (node);
	}
	else if (oper->id & BINARY)
	{
		if (output.size () < 2)
			SHOULD_NOT_HAPPEN;

		EvalNode *op2 = output.top ();
		output.pop ();
		EvalNode *op1 = output.top ();
		output.pop ();

		BinaryNode *node = new BinaryNode
			((BinaryNode::BinaryEvaluate) oper->d, op1, op2);
		output.push (node);
	}
	else
		SHOULD_NOT_HAPPEN;
}

static void
process_tokens (Parser &p,
	stack<EvalNode *> &output, stack<const Operator *> &op_stack)
{
	int expect = UNARY;
	while (!p.accept (END))
	{
		int expect_next = BINARY;

		if      (p.accept (REAL))
			output.push (new    RealNode (p.last ().n));
		else if (p.accept (INTEGER))
			output.push (new IntegerNode (p.last ().i));
		else if (p.accept (IDENT))
			output.push (new     VarNode (p.last ().s));
		else if (p.accept (LPAREN))
		{
			op_stack.push (translate (p.last (), SPECIAL));
			expect_next = UNARY;
		}
		else if (p.accept (RPAREN))
		{
			while (1)
			{
				if (op_stack.empty ())
					throw EInvalidExpression (_("Mismatched right parenthesis"));

				if (op_stack.top ()->id == (SP LPAREN))
				{
					op_stack.pop ();
					break;
				}

				apply (output, op_stack);
			}
		}
		else
		{
			p.accept_any ();
			const Operator *op = translate (p.last (), expect);

			while (!op_stack.empty () && !(op_stack.top ()->id & SPECIAL) &&
				((op->assoc == LEFT  && op->prec >= op_stack.top ()->prec) ||
				 (op->assoc == RIGHT && op->prec >  op_stack.top ()->prec)))
				apply (output, op_stack);

			op_stack.push (op);
			expect_next = UNARY;
		}

		expect = expect_next;
	}
}

EvalNode *
parse_expression (Parser &p) throw (EInvalidExpression, EParserUnexpected)
{
	stack<EvalNode *> output;
	stack<const Operator *> op_stack;

	try
	{
		process_tokens (p, output, op_stack);

		while (!op_stack.empty ())
		{
			if (op_stack.top ()->id == (SP LPAREN))
				throw EInvalidExpression (_("Mismatched left parenthesis"));

			apply (output, op_stack);
		}

		/* We expect only the root node to be left on the stack. */
		if (output.size () == 1)
			return output.top ();

		if (output.empty ())
			throw EInvalidExpression (_("The expression is empty"));

		SHOULD_NOT_HAPPEN;
	}
	catch (...)
	{
		while (!output.empty ())
		{
			delete output.top ();
			output.pop ();
		}

		throw;
	}
}
