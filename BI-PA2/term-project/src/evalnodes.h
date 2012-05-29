/**
 * @file evalnodes.h
 * Nodes of the evaluation tree.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 */

#ifndef __EVALNODES_H__
#define __EVALNODES_H__

/** Base class for all nodes in an evaluation tree. */
class EvalNode
{
public:
	virtual ~EvalNode () {}
	/** Evaluate the subtree rooted at this node. */
	virtual Value evaluate (Environ &e) const = 0;
};

/** Wrapper class for integer values. */
class IntegerNode : public EvalNode
{
	long value;
public:
	/** Initialize the node with the given integer. */
	IntegerNode (long i) : value (i) {}
	virtual Value evaluate (Environ &e) const;
};

/** Wrapper class for real number values. */
class RealNode : public EvalNode
{
	double value;
public:
	/** Initialize the node with the given real number. */
	RealNode (double n) : value (n) {}
	virtual Value evaluate (Environ &e) const;
};

/** Wrapper class for references to variables. */
class VarNode : public EvalNode
{
	std::string name;
public:
	/** Initialize the node with the given variable name. */
	VarNode (std::string s) : name (s) {}
	virtual Value evaluate (Environ &e) const;
};

/** Unary operation. */
class UnaryNode : public EvalNode
{
public:
	/** Pointer to a unary evaluation delegate method. */
	typedef Value (Value::*UnaryEvaluate) ();
protected:
	UnaryEvaluate delegate;   //!< Delegate that performs the operation.
	EvalNode *op;             //!< Operand.
public:
	/** Initialize the node with the given delegate and its operand. */
	UnaryNode (UnaryEvaluate delegate, EvalNode *op)
		: delegate (delegate), op (op) {}
	virtual ~UnaryNode ();
	virtual Value evaluate (Environ &e) const;
};

/** Binary operation. */
class BinaryNode : public EvalNode
{
public:
	/** Pointer to a binary evaluation delegate method. */
	typedef Value (Value::*BinaryEvaluate) (const Value &v);
protected:
	BinaryEvaluate delegate;  //!< Delegate that performs the operation.
	EvalNode *op1;            //!< First operand.
	EvalNode *op2;            //!< Second operand.
public:
	/** Initialize the node with the given delegate and its operands. */
	BinaryNode (BinaryEvaluate delegate, EvalNode *op1, EvalNode *op2)
		: delegate (delegate), op1 (op1), op2 (op2) {}
	virtual ~BinaryNode ();
	virtual Value evaluate (Environ &e) const;
};

#endif /* ! __EVALNODES_H__ */
