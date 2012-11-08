#include <vector>
#include <iostream>
#include <fstream>
#include <utility>
#include <algorithm>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <functional>

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdio>

// Just guessing; adjust if needed.
#ifdef __BYTE_ORDER
	#if __BYTE_ORDER == __LITTLE_ENDIAN
		#define WORDS_LITTLE_ENDIAN
	#endif
#elif defined (WIN32) || defined (WIN64)
	#define WORDS_LITTLE_ENDIAN
#endif

// --- Heap Sort ---------------------------------------------------------------

inline int bh_left   (int i) {return (i + 1) * 2     - 1;}
inline int bh_right  (int i) {return (i + 1) * 2 + 1 - 1;}
inline int bh_parent (int i) {return (i + 1) / 2     - 1;}

template<typename T>
static void heapify (std::vector<T> &v, int heap_size, int item)
{
	int l = bh_left  (item);
	int r = bh_right (item);

	int largest = item;
	if (l < heap_size && v[l] > v[largest])
		largest = l;
	if (r < heap_size && v[r] > v[largest])
		largest = r;

	if (largest != item)
	{
		std::swap (v[item], v[largest]);
		heapify (v, heap_size, largest);
	}
}

template<typename T>
static void heapsort (std::vector<T> &v)
{
	int heap_size = v.size ();
	for (int i = heap_size / 2; i--; )
		heapify (v, heap_size, i);

	for (int i = heap_size - 1; i > 0; i--)
	{
		std::swap (v[0], v[i]);
		heapify (v, --heap_size, 0);
	}
}

// --- Radix Sort --------------------------------------------------------------

static inline void
radix_counting_sort (const void *in, void *out, int *count,
	size_t nmemb, size_t size, unsigned pos)
{
	memset (count, 0, 256 * sizeof *count);
	for (unsigned i = 0; i < nmemb; i++)
		count[*((const unsigned char *) in + (size * i) + pos)]++;
	for (unsigned i = 1; i < 256; i++)
		count[i] += count[i - 1];
	for (unsigned i = nmemb; i--; )
	{
		const unsigned char *x = (const unsigned char *) in + (size * i);
		memcpy ((unsigned char *) out + size * --count[*(x + pos)], x, size);
	}
}

static void
radixsort_8 (void *a, size_t nmemb, size_t size)
{
	assert (a != NULL);
	assert (size > 0);

	auto count = new int[256];
	auto tmp = new char[nmemb * size];

	void *in = a, *out = tmp;
#ifdef WORDS_LITTLE_ENDIAN
	for (unsigned i = 0; i < size; i++)
#else // ! WORDS_LITTLE_ENDIAN
	for (unsigned i = size; i--; )
#endif // ! WORDS_LITTLE_ENDIAN
	{
		radix_counting_sort (in, out, count, nmemb, size, i);
		std::swap (in, out);
	}

	if (in != a)
		memcpy (a, tmp, nmemb * size);

	delete[] count;
	delete[] tmp;
}

// --- Utilities ---------------------------------------------------------------

template<typename T>
static int
int_compare (const void *a, const void *b)
{
	auto ta = static_cast<const T *> (a);
	auto tb = static_cast<const T *> (b);

	return *ta < *tb ? -1 : *ta != *tb;
}

// --- Main --------------------------------------------------------------------

typedef unsigned long long my_sorted_t;
typedef std::vector<my_sorted_t> my_vector_t;

static struct
{
	const char *name;
	std::function<void (my_vector_t &)> fun;
}
g_tests[] =
{
	{"STL vector sort", [] (my_vector_t &v) {
		std::sort (v.begin (), v.end ());
	}},
	{"Vector heapsort", [] (my_vector_t &v) {
		heapsort (v);
	}},
	{"STL qsort", [] (my_vector_t &v) {
		qsort (v.data (), v.size (), sizeof *(v.data ()),
			int_compare<my_sorted_t>);
	}},
	{"8-bit radixsort", [] (my_vector_t &v) {
		radixsort_8 (v.data (), v.size (), sizeof *(v.data ()));
	}}
};


template<typename T, typename U>
static int
measure (const std::vector<T> &v, const std::vector<T> &ref, U f)
{
	auto x = v;

	auto start = std::chrono::high_resolution_clock::now ();
	f (x);
	auto end   = std::chrono::high_resolution_clock::now ();

	if (!std::equal (ref.begin (), ref.end (), x.begin ()))
		std::cout << "Not sorted correctly! ";

	return std::chrono::duration_cast<std::chrono::milliseconds>
		(end - start).count ();
}

static void
test_sorts (std::ofstream &data, unsigned power)
{
	unsigned n_elements = 1 << power;

	std::cout << "Sorting " << n_elements << " elements..." << std::endl;
	data << power << ' ';

	my_vector_t a (n_elements);
	for (auto &i : a)
		i = rand ();

	// For checking if the array was sorted correctly.
	auto ref = a;
	std::sort (ref.begin (), ref.end ());

	for (auto &test : g_tests)
	{
		int elapsed = measure (a, ref, test.fun);
		std::cout << test.name << ": " << elapsed << " ms" << std::endl;
		data << elapsed << ' ';
	}

	std::cout << std::endl;
	data << std::endl;
}

int
main (int argc, char *argv[])
{
	srand (time (nullptr));

	std::ofstream data ("sorttest.dat");
	for (unsigned i = 12; i <= 24; i++)
		test_sorts (data, i);

	if (!data)
	{
		std::cerr << "Couldn't write statistic data to file" << std::endl;
		return 1;
	}

	FILE *gp = popen ("gnuplot -persist", "w");
	fprintf (gp,
		"set log y\n"
		"set termoption enhanced\n"
		"set style data linespoints\n"
		"set title 'Sorting algorithm comparison'\n"
		"set xlabel 'Array size [2^x]'\n"
		"set ylabel 'Time [ms]'\n"
		"set key top left\n"
		"plot ");

	int i = 0;
	for (auto &test : g_tests)
	{
		if (i)  fprintf (gp, ", ");
		fprintf (gp, "'sorttest.dat' using 1:%d title '%s'",
			i++ + 2, test.name);
	}
	fputc ('\n', gp);

	pclose (gp);
	return 0;
}

