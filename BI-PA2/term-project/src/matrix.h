/**
 * @file matrix.h
 * Matrix related classes.
 *
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 */

#ifndef __MATRIX_H__
#define __MATRIX_H__

/** Corrupted data encountered when loading data from a stream. */
class EDataError : public std::exception
{
	/** Return a description of the event. */
	virtual const char *what () const throw ();
};

/** Storage for values in a Matrix. */
class MatrixStorage
{
protected:
	unsigned rows;       //!< Number of rows.
	unsigned cols;       //!< Number of columns.
public:
	virtual ~MatrixStorage () {}
	unsigned ref_count;  //!< Reference count.  Not used internally.

	unsigned get_rows () const {return rows;}  //!< Get number of rows.
	unsigned get_cols () const {return cols;}  //!< Get number of columns.

	/** Get the value at the given position. */
	virtual double get (unsigned row, unsigned col) const = 0;
	/** Set the value at the given position. */
	virtual void put (unsigned row, unsigned col, double value) = 0;
	/** Swap two rows. */
	virtual void swap_rows (unsigned row1, unsigned row2) = 0;

	/** Serialize the storage into an output stream. */
	virtual bool serialize (std::ostream &os) const = 0;

	/** Create a new object using the same values. */
	virtual MatrixStorage *clone () const = 0;
	/** Create a new empty object of the same type. */
	virtual MatrixStorage *create (unsigned rows, unsigned cols) const = 0;
};

/** Array-based storage for dense matrices. */
class MatrixArrayStorage : public MatrixStorage
{
	double *values;
public:
	/** Initialize the storage. */
	MatrixArrayStorage (unsigned rows, unsigned cols, bool init = true);
	/** Initialize the storage using binary values from a stream. */
	MatrixArrayStorage (std::istream &is) throw (EDataError);
	virtual ~MatrixArrayStorage ();

	virtual double get (unsigned row, unsigned col) const;
	virtual void put (unsigned row, unsigned col, double value);
	virtual void swap_rows (unsigned row1, unsigned row2);

	virtual bool serialize (std::ostream &os) const;

	virtual MatrixStorage *clone () const;
	virtual MatrixStorage *create (unsigned rows, unsigned cols) const;
};

/** Map-based storage for sparse matrices. */
class MatrixMapStorage : public MatrixStorage
{
	mutable std::map<unsigned, std::map<unsigned, double> > values;
public:
	/** Initialize the storage. */
	MatrixMapStorage (unsigned rows, unsigned cols);
	/** Initialize the storage using binary values from a stream. */
	MatrixMapStorage (std::istream &is) throw (EDataError);

	virtual double get (unsigned row, unsigned col) const;
	virtual void put (unsigned row, unsigned col, double value);
	virtual void swap_rows (unsigned row1, unsigned row2);

	virtual bool serialize (std::ostream &os) const;

	virtual MatrixStorage *clone () const;
	virtual MatrixStorage *create (unsigned rows, unsigned cols) const;
};

/** The Matrix is incompatible with the specified operation. */
class EIncompatibleMatrix : public std::exception
{
	std::string message;
public:
	/** Initialize the exception with the given message. */
	EIncompatibleMatrix (const std::string &message) : message (message) {}
	virtual ~EIncompatibleMatrix () throw () {}
	/** Return a description of the event. */
	virtual const char *what () const throw () {return message.c_str ();}
};

/** Implements traditional matrix operations on top of MatrixStorage. */
class Matrix
{
	MatrixStorage *storage;  //!< Underlying storage.

	/** Get number-aligned string representation of rows. */
	std::string *get_rows (unsigned &count) const;
public:
	/** Initialize the matrix to use the given storage.
	 *  The Matrix delete's this object when it's not needed anymore. */
	Matrix (MatrixStorage *storage);
	/** Initialize the matrix using values from another Matrix. */
	Matrix (const Matrix &m);
	~Matrix ();

	/** Returns the storage object used by this matrix. */
	MatrixStorage &get_storage () const {return *storage;}

	/** Retrieve the (@a row, @a col)-th value. */
	double get (unsigned row, unsigned col) const;
	/** Set the (@a row, @a col)-th value to @a value. */
	void put (unsigned row, unsigned col, double value);

	/** Print the matrix out into an output stream. */
	friend std::ostream &operator<< (std::ostream &os, const Matrix &m);
	/** Accept the storage of another Matrix. */
	Matrix &operator= (const Matrix &m);

	/** Add the matrix to another. */
	Matrix operator+ (const Matrix &m) const throw (EIncompatibleMatrix);
	/** Subtract another matrix from this one. */
	Matrix operator- (const Matrix &m) const throw (EIncompatibleMatrix);
	/** Multiply with another matrix. */
	Matrix operator* (const Matrix &m) const throw (EIncompatibleMatrix);

	/** Multiply all values by a constant. */
	Matrix operator* (double n) const;
	/** Tranpose the matrix. */
	Matrix transpose () const;

	/** Describes the elimination step. */
	struct EliminateStep
	{
		/** The step that's been performed. */
		enum
		{
			SWAP,   //!< Rows @a par1 and @a par2 were swapped.
			SUB,    //!< @a factor * @a par1 was subtracted from row @a par2.
			MUL,    //!< Row @a par1 was multiplied by @a factor.

			START,  //!< The elimination has started.
			GROUP   //!< A group of operations has been finished.
		}
		step;

		void *extra;        //!< Caller-specific data.
		MatrixStorage *ms;  //!< Current matrix data (after the step).
		unsigned par1;      //!< First integer parameter.
		unsigned par2;      //!< Second integer parameter.
		double factor;      //!< Row multiplication factor.
	};

	/** Callback for single steps of Gaussian elimination. */
	typedef void (*EliminateCallback) (EliminateStep &step);

	/** Apply Gaussian elimination, get the @a rank as a by-product. */
	Matrix eliminate (unsigned *rank = NULL,
		EliminateCallback cb = NULL, void *extra = NULL) const;
	/** Compute an inverse matrix via Gauss-Jordan elimination. */
	Matrix inverse (EliminateCallback cb = NULL,
		void *extra = NULL) const throw (EIncompatibleMatrix);

	/** Compute the determinant of this matrix. */
	double get_determinant () const throw (EIncompatibleMatrix);
};

#endif /* ! __MATRIX_H__ */

