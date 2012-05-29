/**
 * @file environ.cpp
 * Environment.
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

#include "gettext.h"
#define _(String) gettext (String)

using namespace std;


EUndefinedVariable::EUndefinedVariable (const std::string &name)
{
	message = _("Undefined variable: ") + name;
}

Value &
Environ::get (const std::string &name) throw (EUndefinedVariable)
{
	if (!vars.count (name))
		throw EUndefinedVariable (name);
	return vars[name];
}

void
Environ::set (const string &name, const Value &value)
{
	vars[name] = value;
}

void
Environ::unset (const string &name)
{
	vars.erase (name);
}

void
Environ::clear ()
{
	vars.clear ();
}
