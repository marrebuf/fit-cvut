/**
 * @file matrixcalc.cpp
 * Matrix calculator.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 * @mainpage
 *
 * @b matrixcalc is a rather simple, user-friendly matrix calculator.  You
 * should be able to start using the program just a short while after seeing
 * the output of the integrated `help' command.
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <stack>
#include <exception>

#include <clocale>
#include <cstdlib>
#include <cstring>

#include <config.h>

#include "matrix.h"
#include "value.h"
#include "tokenizer.h"
#include "parser.h"
#include "environ.h"
#include "evalnodes.h"
#include "parseexpr.h"

#include "gettext.h"
#define _(String) gettext (String)

#ifdef HAVE_LIBREADLINE
	#if defined (HAVE_READLINE_READLINE_H)
		#include <readline/readline.h>
	#elif defined (HAVE_READLINE_H)
		#include <readline.h>
	#else /* ! HAVE_READLINE_H */
		extern "C" char *readline (...);
	#endif /* ! HAVE_READLINE_H */
#endif /* HAVE_LIBREADLINE */

#ifdef HAVE_READLINE_HISTORY
	#if defined (HAVE_READLINE_HISTORY_H)
		#include <readline/history.h>
	#elif defined (HAVE_HISTORY_H)
		#include <history.h>
	#else /* ! HAVE_HISTORY_H */
		extern "C" void add_history (...);
		extern "C" int write_history (...);
		extern "C" int read_history (...);
	#endif /* ! HAVE_HISTORY_H */
#endif /* HAVE_READLINE_HISTORY */

/* FIXME: Use autoconf for this. */
#ifdef _WIN32
	#include <io.h>
	#define fileno _fileno
	#define isatty _isatty
#else /* ! _WIN32 */
	#include <unistd.h>
#endif /* ! _WIN32 */

using namespace std;


/** The expression evaluation environment. */
Environ g_environ;
/** Creating sparse matrices. */
bool g_create_sparse;

/** Show program help. */
static void
show_help ()
{
	cout << _(
	"Commands:\n"
	"  input NAME (ROWS, COLS)  Enter a matrix from keyboard\n"
	"  delete NAME | *          Delete the named variable, or everything\n"
	"  load NAME FILENAME       Load variable from a file\n"
	"  save NAME FILENAME       Save variable to a file\n"
	"  typeof NAME              Print out the type of the variable\n"
	"  set OPTION               Set program options\n"
	"  help                     Show this help\n"
	"  exit                     Exit the program\n"
	"\n"
	"  <expression>             Compute the expression and display\n"
	"                           the result on standard output\n"
	"  NAME = <expression>      Compute the expression and assign\n"
	"                           the result to a variable\n"
	"\n"
	"Options:\n"
	"  sparse  | dense          Use sparse or dense storage for new matrices\n"
	"  verbose | quiet          Be verbose when performing computations\n"
	"\n"
	"Operators in <expression>:\n"
	"  +, -, *, ^               Their usual meaning\n"
	"                           Note that ^ is capable of inverting matrices\n"
	"  det                      Compute matrix determinant\n"
	"  rank                     Compute rank of a matrix\n"
	"  transpose                Produce a transposed matrix\n"
	"  eliminate                Apply Gaussian elimination\n") << endl;
}

/** Let the user enter a single row of a matrix. */
static bool
enter_row (MatrixStorage *storage, int cols, int row)
{
	char prompt[16] = "";
	if (isatty (fileno (stdin)))
		snprintf (prompt, sizeof prompt, "#%d: ", row + 1);

	istringstream is;

#ifdef HAVE_LIBREADLINE
	char *s = readline (prompt);
	if (!s)  return false;
	is.str (s);
	free (s);
#else /* ! HAVE_LIBREADLINE */
	cout << prompt;
	char buff[80];
	if (!cin.getline (buff, sizeof buff))
		return false;
	is.str (buff);
#endif /* ! HAVE_LIBREADLINE */

	double value;
	for (int col = 0; col < cols; col++)
	{
		if (!(is >> value))
		{
			cout << _("Not enough values") << endl;
			return false;
		}
		storage->put (row, col, value);
	}

	if (is >> value)
	{
		cout << _("Too many values") << endl;
		return false;
	}

	return true;
}

/** Let the user enter a @a rows by @a cols matrix. */
static void
enter_matrix (const string &var, int rows, int cols)
{
	MatrixStorage *ms = g_create_sparse
		? static_cast<MatrixStorage *> (new MatrixMapStorage   (rows, cols))
		: static_cast<MatrixStorage *> (new MatrixArrayStorage (rows, cols));

	printf (_("Enter a %dx%d matrix:\n"), rows, cols);

	for (int r = 0; r < rows; r++)
		if (!enter_row (ms, cols, r))
		{
			cout << _("Matrix input aborted") << endl;
			delete ms;
			return;
		}

	Value value;
	value.type = Value::MATRIX;
	value.matrix = new Matrix (ms);
	g_environ.set (var, value);
}

/** Load a variable from a file. */
static bool
load_var (const string &var, const string &filename)
{
	ifstream ifs (filename.c_str (), ios::binary);
	Value value;

	/* Load type of variable. */
	if (!ifs || !ifs.read ((char *) &value.type, sizeof value.type))
		return false;

	/* Read in the actual value. */
	switch (value.type)
	{
	case Value::INTEGER:
		if (!ifs.read ((char *) &value.integer, sizeof value.integer))
			return false;
		break;
	case Value::REAL:
		if (!ifs.read ((char *) &value.real,    sizeof value.real))
			return false;
		break;
	case Value::MATRIX:
		/* So far we only distinguish two kinds of matrix storage. */
		bool sparse;
		if (!ifs.read ((char *) &sparse, sizeof sparse))
			return false;

		try
		{
			value.matrix = new Matrix (sparse
				? static_cast<MatrixStorage *> (new MatrixMapStorage   (ifs))
				: static_cast<MatrixStorage *> (new MatrixArrayStorage (ifs)));
		}
		catch (const EDataError &)
		{
			return false;
		}
		break;
	default:
		return false;
	}

	/* And assign it to the variable name. */
	g_environ.set (var, value);
	return true;
}

/** Save a variable to a file. */
static bool
save_var (const string &var, const string &filename)
{
	ofstream ofs (filename.c_str (), ios::binary);
	Value value;

	try
	{
		value = g_environ.get (var);
	}
	catch (const EUndefinedVariable &)
	{
		return false;
	}

	/* First write the type of variable. */
	if (!ofs || !ofs.write ((char *) &value.type, sizeof value.type))
		return false;

	/* Then the actual value. */
	MatrixStorage *storage;
	bool sparse;

	switch (value.type)
	{
	case Value::INTEGER:
		return ofs.write ((char *) &value.integer, sizeof value.integer);
	case Value::REAL:
		return ofs.write ((char *) &value.real,    sizeof value.real);
	case Value::MATRIX:
		storage = &value.matrix->get_storage ();
		sparse = !!dynamic_cast<MatrixMapStorage *> (storage);
		if (!ofs.write ((char *) &sparse, sizeof sparse))
			return false;

		return storage->serialize (ofs);
	default:
		return false;
	}
}

/** Try to read a command from the input. */
static bool
parse_command (Parser &parser)
{
	if (parser.accept (INPUT))
	{
		parser.expect (IDENT);
		string var = parser.last ().s;
		parser.expect (LPAREN);
		parser.expect (INTEGER);
		long rows = parser.last ().i;
		parser.expect (COMMA);
		parser.expect (INTEGER);
		long cols = parser.last ().i;
		parser.expect (RPAREN);
		parser.expect (END);

		if (rows < 1 || cols < 1)
			cout << _("Wrong dimensions") << endl;
		else
			enter_matrix (var, rows, cols);
	}
	else if (parser.accept (DELETE))
	{
		if (parser.accept (TIMES))
		{
			parser.expect (END);
			g_environ.clear ();
		}
		else
		{
			parser.expect (IDENT);
			string var = parser.last ().s;
			parser.expect (END);
			g_environ.unset (var);
		}
	}
	else if (parser.accept (LOAD))
	{
		parser.expect (IDENT);
		string var = parser.last ().s;
		parser.expect (STRING);
		string filename = parser.last ().s;
		parser.expect (END);

		if (!load_var (var, filename))
			cout << _("Loading failed") << endl;
	}
	else if (parser.accept (SAVE))
	{
		parser.expect (IDENT);
		string var = parser.last ().s;
		parser.expect (STRING);
		string filename = parser.last ().s;
		parser.expect (END);

		if (!save_var (var, filename))
			cout << _("Saving failed") << endl;
	}
	else if (parser.accept (TYPEOF))
	{
		parser.expect (IDENT);
		string var = parser.last ().s;
		parser.expect (END);

		Value value;
		value = g_environ.get (var);

		switch (value.type)
		{
		case Value::INTEGER:
			cout << _("Integer") << endl;
			break;
		case Value::REAL:
			cout << _("Real number") << endl;
			break;
		case Value::MATRIX:
			MatrixStorage *ms = &value.matrix->get_storage ();
			if (dynamic_cast<MatrixMapStorage *> (ms))
				cout << _("Sparse matrix") << endl;
			else
				cout << _("Dense matrix") << endl;
		}
	}
	else if (parser.accept (SET))
	{
		parser.expect (IDENT);
		string option = parser.last ().s;
		parser.expect (END);

		if (option == "sparse")
		{
			cout << _("Creating sparse matrices") << endl;
			g_create_sparse = true;
		}
		else if (option == "dense")
		{
			cout << _("Creating dense matrices") << endl;
			g_create_sparse = false;
		}
		else if (option == "verbose")
		{
			cout << _("Verbose mode set to on") << endl;
			Value::describe = true;
		}
		else if (option == "quiet")
		{
			cout << _("Verbose mode set to off") << endl;
			Value::describe = false;
		}
		else
			cout << _("Unsupported option: ") << option << endl;
	}
	else if (parser.accept (EXIT))
	{
		parser.expect (END);
		cout << _("Bye!") << endl;
		exit (EXIT_SUCCESS);
	}
	else if (parser.accept (HELP))
	{
		parser.expect (END);
		show_help ();
	}
	else
		return false;

	return true;
}

/** Process a single line of input. */
static void
process_input (const char *s)
{
	istringstream is (s);
	Tokenizer scanner (is);
	Parser parser (scanner);

	EvalNode *tree = NULL;

	/* Ignore empty lines. */
	if (parser.accept (END))
		return;

	try
	{
		/* First try parsing as a command. */
		if (parse_command (parser))
			return;

		/* Or as an expression,
		 * possibly assigned to a variable. */
		bool assigning = false;
		string ident;

		if (parser.accept (IDENT))
		{
			ident = parser.last ().s;
			if (parser.accept (EQUALS))
				assigning = true;
			else
				parser.go_back ();
		}

		tree = parse_expression (parser);
		Value val = tree->evaluate (g_environ);
		if (assigning)
			g_environ.set (ident, val);
		else
			cout << val << endl;

		delete tree;
	}
	catch (const exception &e)
	{
		cout << _("Error: ") << e.what () << endl;
		delete tree;
	}
}

/** Program entry point. */
int
main (int argc, char *argv[])
{
	setlocale (LC_ALL, "");

#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);
#endif /* ENABLE_NLS */

	printf (_("Welcome to %s\n"), PACKAGE_STRING);
	cout <<   "Copyright Přemysl Janouch 2012" << endl << endl;
	cout << _("Type `help' to get started")    << endl;

	const char *prompt = "> ";
	if (!isatty (fileno (stdin)))
		prompt = "";

#ifdef HAVE_LIBREADLINE
	char *s;
	while ((s = readline (prompt)))
	{
		if (strlen (s))
		{
			process_input (s);

#ifdef HAVE_READLINE_HISTORY
			add_history (s);
#endif /* HAVE_READLINE_HSITORY */
		}

		free (s);
	}
#else /* ! HAVE_LIBREADLINE */
	char buff[80];

	cout << prompt;
	while (cin.getline (buff, sizeof buff))
	{
		if (!strlen (buff))
			continue;
		process_input (buff);

		cout << prompt;
	}
#endif /* ! HAVE_LIBREADLINE */

	return 0;
}

