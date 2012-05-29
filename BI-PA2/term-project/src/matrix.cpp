/**
 * @file matrix.cpp
 * Matrix related classes.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 */

#include <iostream>
#include <map>
#include <cmath>
#include <cstring>
#include <cstdio>

#include <config.h>

#include "matrix.h"

#include "gettext.h"
#define _(String) gettext (String)

using namespace std;


/** To hack around some precision errors, values smaller than this
 *  are transformed to zeros in forward Gaussian elimination. */
#define ELIMINATES_TO_ZERO 1e-10


const char *
EDataError::what () const throw ()
{
	return _("Invalid matrix data");
}

MatrixArrayStorage::MatrixArrayStorage (unsigned rows, unsigned cols, bool init)
{
	this->rows = rows;
	this->cols = cols;
	ref_count = 0;
	values = new double[rows * cols];

	if (init)
	{
		for (unsigned i = rows * cols; i--; )
			values[i] = 0;
	}
}

MatrixArrayStorage::MatrixArrayStorage (std::istream &is) throw (EDataError)
{
	if (!is.read ((char *) &rows, sizeof rows)
	 || !is.read ((char *) &cols, sizeof cols)
	 || !rows || !cols)
		throw EDataError ();

	ref_count = 0;
	values = new double[rows * cols];
	if (!is.read ((char *) values, sizeof *values * cols * rows))
	{
		delete values;
		throw EDataError ();
	}
}

MatrixArrayStorage::~MatrixArrayStorage ()
{
	delete [] values;
}

double
MatrixArrayStorage::get (unsigned row, unsigned col) const
{
	if (row >= rows || col >= cols)
		return NAN;

	return values[row * cols + col];
}

void
MatrixArrayStorage::put (unsigned row, unsigned col, double value)
{
	if (row >= rows || col >= cols)
		return;

	values[row * cols + col] = value;
}

void
MatrixArrayStorage::swap_rows (unsigned row1, unsigned row2)
{
	if (row1 >= rows || row2 >= rows)
		return;

	double *tmp = new double[cols];
	memcpy (tmp,                  values + row1 * cols, cols * sizeof *values);
	memcpy (values + row1 * cols, values + row2 * cols, cols * sizeof *values);
	memcpy (values + row2 * cols, tmp,                  cols * sizeof *values);
	delete [] tmp;
}

bool
MatrixArrayStorage::serialize (std::ostream &os) const
{
	os.write ((char *) &rows, sizeof rows);
	os.write ((char *) &cols, sizeof cols);
	return os.write ((char *) values, sizeof *values * cols * rows);
}

MatrixStorage *
MatrixArrayStorage::clone () const
{
	MatrixArrayStorage *mas = new MatrixArrayStorage (rows, cols, false);
	memcpy (mas->values, values, sizeof *values * cols * rows);
	return mas;
}

MatrixStorage *
MatrixArrayStorage::create (unsigned rows, unsigned cols) const
{
	return new MatrixArrayStorage (rows, cols);
}


MatrixMapStorage::MatrixMapStorage (unsigned rows, unsigned cols)
{
	this->rows = rows;
	this->cols = cols;
	ref_count = 0;
}

MatrixMapStorage::MatrixMapStorage (std::istream &is) throw (EDataError)
{
	if (!is.read ((char *) &rows, sizeof rows)
	 || !is.read ((char *) &cols, sizeof cols)
	 || !rows || !cols)
		throw EDataError ();

	ref_count = 0;

	/* #n_rows { row #n_cols { col value }* }* */
	unsigned n_rows, n_cols, row, col;

	if (!is.read ((char *) &n_rows, sizeof n_rows))
		throw EDataError ();
	for (unsigned r = 0; r < n_rows; r++)
	{
		if (!is.read ((char *) &row,    sizeof row)
		 || !is.read ((char *) &n_cols, sizeof n_cols)
		 || row >= rows)
			throw EDataError ();

		map<unsigned, double> &col_map = values[row];
		for (unsigned c = 0; c < n_cols; c++)
		{
			double value;
			if (!is.read ((char *) &col,   sizeof col)
			 || !is.read ((char *) &value, sizeof value)
			 || col >= cols)
				throw EDataError ();

			col_map[col] = value;
		}
	}
}

double
MatrixMapStorage::get (unsigned row, unsigned col) const
{
	if (row >= rows || col >= cols)
		return NAN;

	if (!values.count (row))
		return 0;

	map<unsigned, double> &row_map = values[row];
	if (!row_map.count (col))
		return 0;

	return row_map[col];
}

void
MatrixMapStorage::put (unsigned row, unsigned col, double value)
{
	if (row >= rows || col >= cols)
		return;

	if (!value)
		values[row].erase (col);
	else
		values[row][col] = value;
}

void
MatrixMapStorage::swap_rows (unsigned row1, unsigned row2)
{
	if (row1 >= rows || row2 >= rows)
		return;

	map<unsigned, double> tmp = values[row1];
	values[row1] = values[row2];
	values[row2] = tmp;
}

bool
MatrixMapStorage::serialize (std::ostream &os) const
{
	unsigned n_rows, n_cols, row, col;

	n_rows = values.size ();
	if (!os.write ((char *) &rows,   sizeof rows)
	 || !os.write ((char *) &cols,   sizeof cols)
	 || !os.write ((char *) &n_rows, sizeof n_rows))
		return false;

	map<unsigned, map<unsigned, double> >::iterator ri;
	map<unsigned, double>::iterator ci;

	for (ri = values.begin (); ri != values.end (); ri++)
	{
		map<unsigned, double> &col_map = (*ri).second;

		row = (*ri).first;
		n_cols = col_map.size ();
		if (!os.write ((char *) &row,    sizeof row)
		 || !os.write ((char *) &n_cols, sizeof n_cols))
			return false;

		for (ci = col_map.begin (); ci != col_map.end (); ci++)
		{
			col = (*ci).first;
			double value = (*ci).second;
			if (!os.write ((char *) &col,   sizeof col)
			 || !os.write ((char *) &value, sizeof value))
				return false;
		}
	}

	return true;
}

MatrixStorage *
MatrixMapStorage::clone () const
{
	MatrixMapStorage *mms = new MatrixMapStorage (rows, cols);
	mms->values = values;
	return mms;
}

MatrixStorage *
MatrixMapStorage::create (unsigned rows, unsigned cols) const
{
	return new MatrixMapStorage (rows, cols);
}


Matrix::Matrix (MatrixStorage *storage)
{
	this->storage = storage;
	storage->ref_count++;
}

Matrix::Matrix (const Matrix &m)
{
	new (this) Matrix (m.storage);
}

Matrix::~Matrix ()
{
	if (!--storage->ref_count)
		delete storage;
}

Matrix &
Matrix::operator= (const Matrix &m)
{
	if (this != &m)
	{
		this->~Matrix ();
		new (this) Matrix (m);
	}
	return *this;
}

double
Matrix::get (unsigned row, unsigned col) const
{
	return storage->get (row, col);
}

void
Matrix::put (unsigned row, unsigned col, double value)
{
	if (storage->ref_count != 1)
	{
		storage->ref_count--;
		storage = storage->clone ();
		storage->ref_count++;
	}

	storage->put (row, col, value);
}

string *
Matrix::get_rows (unsigned &count) const
{
	unsigned r, rows = storage->get_rows ();
	unsigned c, cols = storage->get_cols ();

	/* Create and initialize the strings. */
	string *row_strings = new string[rows];
	for (r = 0; r < rows; r++)
		row_strings[r] = "| ";

	/* XXX: The numbers may still "overflow". */
	/* TODO: Choose the scientific format in that case. */
	struct Cell
	{
		char number[64];
		int len_fixed;
		int len_float;
	}
	*cells = new Cell[rows];

	for (c = 0; c < cols; c++)
	{
		int max_fixed = 0, max_float = 0;
		memset (cells, 0, sizeof *cells * rows);

		/* Compute the alignment and prepare number strings. */
		for (r = 0; r < rows; r++)
		{
			double value = storage->get (r, c);
			if (value == -0.)
				value = 0.;

			int len_fixed = snprintf (cells[r].number,
				sizeof cells->number, "%.f", floor (value));
			if (len_fixed > max_fixed)
				max_fixed = len_fixed;
			cells[r].len_fixed = len_fixed;

			int len_float = snprintf (cells[r].number,
				sizeof cells->number, "%f", value);

			/* Strip trailing zeros. */
			while (cells[r].number[len_float - 1] == '0')
				cells[r].number[--len_float] = '\0';
			/* And the decimal point, too, if possible. */
			if (len_float - 1 == len_fixed)
				cells[r].number[--len_float] = '\0';

			len_float -= len_fixed;
			if (len_float > max_float)
				max_float = len_float;
			cells[r].len_float = len_float;
		}

		/* Append the values according to the alignment. */
		for (r = 0; r < rows; r++)
		{
			if (c != 0)
				row_strings[r] += " , ";

			for (int i = cells[r].len_fixed; i < max_fixed; i++)
				row_strings[r] += ' ';
			row_strings[r] += cells[r].number;
			for (int i = cells[r].len_float; i < max_float; i++)
				row_strings[r] += ' ';
		}
	}

	/* Finalize the strings. */
	for (r = 0; r < rows; r++)
		row_strings[r] += " |";

	delete [] cells;
	count = rows;
	return row_strings;
}

ostream &
operator<< (ostream &os, const Matrix &m)
{
	unsigned r, rows;
	string *row_strings = m.get_rows (rows);
	for (r = 0; r < rows - 1; r++)
		os << row_strings[r] << endl;
	os << row_strings[r];
	delete [] row_strings;
	return os;
}

Matrix
Matrix::operator+ (const Matrix &m) const throw (EIncompatibleMatrix)
{
	unsigned r, rows = storage->get_rows ();
	unsigned c, cols = storage->get_cols ();

	if (m.storage->get_rows () != rows
	 || m.storage->get_cols () != cols)
		throw EIncompatibleMatrix
			(_("Cannot add matrices of different dimensions"));

	MatrixStorage *ms = storage->create (rows, cols);
	for (r = rows; r--; )
	for (c = cols; c--; )
		ms->put (r, c, storage->get (r, c) + m.get (r, c));
	return Matrix (ms);
}

Matrix
Matrix::operator- (const Matrix &m) const throw (EIncompatibleMatrix)
{
	try
	{
		return *this + (m * -1);
	}
	catch (const EIncompatibleMatrix &)
	{
		throw EIncompatibleMatrix
			(_("Cannot subtract matrices of different dimensions"));
	}
}

Matrix
Matrix::operator* (const Matrix &m) const throw (EIncompatibleMatrix)
{
	if (m.storage->get_rows () != storage->get_cols ())
		throw EIncompatibleMatrix
			(_("The matrix has wrong number of rows for multiplication"));

	unsigned r, rows =   storage->get_rows ();
	unsigned c, cols = m.storage->get_cols ();
	unsigned s, size =   storage->get_cols ();

	MatrixStorage *ms = storage->create (rows, cols);
	for (r = rows; r--; )
	for (c = cols; c--; )
	{
		double sum = 0;
		for (s = size; s--; )
			sum += storage->get (r, s) * m.get (s, c);
		ms->put (r, c, sum);
	}
	return Matrix (ms);
}

Matrix
Matrix::operator* (double n) const
{
	unsigned r, rows = storage->get_rows ();
	unsigned c, cols = storage->get_cols ();

	MatrixStorage *ms = storage->create (rows, cols);
	for (r = rows; r--; )
	for (c = cols; c--; )
		ms->put (r, c, storage->get (r, c) * n);
	return Matrix (ms);
}

Matrix
Matrix::transpose () const
{
	unsigned r, rows = storage->get_rows ();
	unsigned c, cols = storage->get_cols ();

	MatrixStorage *ms = storage->create (cols, rows);
	for (r = rows; r--; )
	for (c = cols; c--; )
		ms->put (c, r, storage->get (r, c));
	return Matrix (ms);
}

Matrix
Matrix::eliminate (unsigned *rank, EliminateCallback cb, void *extra) const
{
	unsigned rows = storage->get_rows ();
	unsigned cols = storage->get_cols ();

	MatrixStorage *ms = storage->clone ();

	EliminateStep step;
	step.extra = extra;
	step.ms = ms;

	if (cb)
	{
		step.step = EliminateStep::START;
		cb (step);
	}

	unsigned cur_row = 0;
	for (unsigned k = 0; k < cols; k++)
	{
		/* Find the pivot for this column. */
		double value_max = 0;
		unsigned pivot = 0;

		for (unsigned i = cur_row; i < rows; i++)
		{
			double value = abs (ms->get (i, k));
			if (value > value_max)
			{
				value_max = value;
				pivot = i;
			}
		}

		/* Nothing to do here. */
		if (!value_max)
			continue;

		bool done_anything = false;

		/* Possibly swap the current row with the pivot. */
		if (pivot != cur_row)
		{
			ms->swap_rows (cur_row, pivot);

			if (cb)
			{
				step.step = EliminateStep::SWAP;
				step.par1 = cur_row;
				step.par2 = pivot;
				cb (step);
				done_anything = true;
			}
		}

		/* Subtract the pivot from its following rows. */
		for (unsigned r = cur_row + 1; r < rows; r++)
		{
			double factor = ms->get (r, k) / ms->get (cur_row, k);

			if (!factor)
				continue;

			ms->put (r, k, 0);
			for (unsigned c = k + 1; c < cols; c++)
			{
				double value = ms->get (r, c) - ms->get (cur_row, c) * factor;
				ms->put (r, c, fabs (value) < ELIMINATES_TO_ZERO ? 0 : value);
			}

			if (cb)
			{
				step.step = EliminateStep::SUB;
				step.par1 = cur_row;
				step.par2 = r;
				step.factor = factor;
				cb (step);
				done_anything = true;
			}
		}

		cur_row++;

		if (done_anything)
		{
			step.step = EliminateStep::GROUP;
			cb (step);
		}
	}

	if (rank)
		*rank = cur_row;
	return Matrix (ms);
}

Matrix
Matrix::inverse (EliminateCallback cb, void *extra)
	const throw (EIncompatibleMatrix)
{
	unsigned size = storage->get_rows ();
	if (size != storage->get_cols ())
		throw EIncompatibleMatrix
			(_("Inverse matrix is only defined for rectangular matrices"));

	/* Create a matrix of form (M | E). */
	MatrixStorage *tmp = storage->create (size, 2 * size);
	for (unsigned r = 0; r < size; r++)
	{
		for (unsigned c = 0; c < size; c++)
			tmp->put (r, c, storage->get (r, c));
		tmp->put (r, size + r, 1);
	}

	Matrix m = Matrix (tmp).eliminate (NULL, cb, extra);
	if (!m.get (size - 1, size - 1))
		throw EIncompatibleMatrix (_("Only regular matrices are invertible"));

	EliminateStep step;
	step.ms = &m.get_storage ();
	step.extra = extra;

	/* Do the backward run of Gaussian elimination. */
	double factor;
	for (unsigned s = size; s--; )
	{
		bool done_anything = false;

		/* Make (s, s) = 1, divide the rest of the row. */
		factor = 1. / m.get (s, s);
		if (factor != 1.)
		{
			m.put (s, s, 1);
			for (unsigned c = size; c--; )
				m.put (s, size + c, m.get (s, size + c) * factor);

			if (cb)
			{
				step.step = EliminateStep::MUL;
				step.par1 = s;
				step.factor = factor;
				cb (step);
				done_anything = true;
			}
		}

		/* Make (0 .. s - 1; s) = 0, subtract rest of the row. */
		for (unsigned r = s; r--; )
		{
			factor = m.get (r, s);
			if (factor)
			{
				m.put (r, s, 0);
				for (unsigned c = size; c--; )
					m.put (r, size + c,
						m.get (r, size + c) - m.get (s, size + c) * factor);

				if (cb)
				{
					step.step = EliminateStep::SUB;
					step.par1 = s;
					step.par2 = r;
					step.factor = factor;
					cb (step);
					done_anything = true;
				}
			}
		}

		if (done_anything)
		{
			step.step = EliminateStep::GROUP;
			cb (step);
		}
	}

	/* Read out (E | M). */
	MatrixStorage *ms = storage->create (size, size);
	for (unsigned r = 0; r < size; r++)
		for (unsigned c = 0; c < size; c++)
			ms->put (r, c, m.get (r, c + size));

	return Matrix (ms);
}

/** Adjust the determinant value if needed. */
static void
matrix_determinant_eliminate_cb (Matrix::EliminateStep &step)
{
	if (step.step == Matrix::EliminateStep::SWAP)
		*static_cast<double *> (step.extra) *= -1;
}

double
Matrix::get_determinant () const throw (EIncompatibleMatrix)
{
	unsigned size = storage->get_rows ();
	if (size != storage->get_cols ())
		throw EIncompatibleMatrix
			(_("Determinant is only defined for rectangular matrices"));

	double factor = 1;
	Matrix m = eliminate (NULL, matrix_determinant_eliminate_cb, &factor);
	for (unsigned s = 0; s < size; s++)
		factor *= m.get (s, s);

	return factor;
}
