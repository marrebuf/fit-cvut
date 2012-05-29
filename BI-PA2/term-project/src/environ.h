/**
 * @file environ.h
 * Environment.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 */

#ifndef __ENVIRON_H__
#define __ENVIRON_H__

/** Attempted to retrieve the value of a nonexistant variable. */
class EUndefinedVariable : public std::exception
{
	std::string message;
public:
	/** Initialize the exception using the variable @a name. */
	EUndefinedVariable (const std::string &name);
	virtual ~EUndefinedVariable () throw () {}
	/** Return a description of the event. */
	virtual const char *what () const throw () {return message.c_str ();}
};

/** Maps Value objects to variable names. */
class Environ
{
protected:
	/** Internal map of all the variables. */
	std::map<std::string, Value> vars;
public:
	/** Retrieve the value assigned to the name. */
	Value &get (const std::string &name) throw (EUndefinedVariable);
	/** Assign a value to the name. */
	void set (const std::string &name, const Value &value);
	/** Unset any value assigned to the name. */
	void unset (const std::string &name);
	/** Unset all of the variables. */
	void clear ();
};

#endif /* ! __ENVIRON_H__ */
