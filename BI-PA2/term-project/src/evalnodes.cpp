/**
 * @file evalnodes.cpp
 * Nodes of the evaluation tree.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 */

#include <iostream>
#include <exception>
#include <map>

#include <config.h>

#include "matrix.h"
#include "value.h"
#include "environ.h"
#include "evalnodes.h"

using namespace std;

Value
IntegerNode::evaluate (Environ &e) const
{
	Value v;
	v.type = Value::INTEGER;
	v.integer = value;
	return v;
}

Value
RealNode::evaluate (Environ &e) const
{
	Value v;
	v.type = Value::REAL;
	v.real = value;
	return v;
}

Value
VarNode::evaluate (Environ &e) const
{
	return e.get (name);
}

UnaryNode::~UnaryNode ()
{
	delete op;
}

Value
UnaryNode::evaluate (Environ &e) const
{
	return (op->evaluate (e).*delegate) ();
}

BinaryNode::~BinaryNode ()
{
	delete op1;
	delete op2;
}

Value
BinaryNode::evaluate (Environ &e) const
{
	return (op1->evaluate (e).*delegate) (op2->evaluate (e));
}
