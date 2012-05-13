#include "common_fs.h"
#include <cassert>

#define DISK_SECTORS 87654 // 524288
static FILE *g_Fp = NULL;


/** Sample sector reading function. The function will be called by your FS
 *  driver implementation. Notice that the function is not called directly.
 *  Instead, the function will be invoked indirectly through a function
 *  pointer in the TBlkDev structure.
 */
static int
diskRead (int sectorNr, void *data, int sectorCnt)
{
	if (g_Fp == NULL)
		return 0;
	if (sectorCnt <= 0 || sectorNr + sectorCnt > DISK_SECTORS)
		return 0;

	fseek (g_Fp, sectorNr * SECTOR_SIZE, SEEK_SET);
	return fread (data, SECTOR_SIZE, sectorCnt, g_Fp);
}

/** Sample sector writing function. Similar to diskRead(). */
static int
diskWrite (int sectorNr, const void *data, int sectorCnt)
{
	if (g_Fp == NULL)
		return 0;
	if (sectorCnt <= 0 || sectorNr + sectorCnt > DISK_SECTORS)
		return 0;

	fseek (g_Fp, sectorNr * SECTOR_SIZE, SEEK_SET);
	return fwrite (data, SECTOR_SIZE, sectorCnt, g_Fp);
}

/** Create the file needed for the sector reading/writing functions above.
 *  This function is only needed for the particular implementation above.
 *  It could be understood as "buying a new disk".
 */
static TBlkDev *
createDisk ()
{
	unsigned buffer[SECTOR_SIZE / sizeof (unsigned)];
	TBlkDev *res = NULL;
	int i;

	for (unsigned i = SECTOR_SIZE / sizeof (unsigned); i--; )
		buffer[i] = 0xefbeadde;

	g_Fp = fopen ("/tmp/disk_content", "w+b");
	if (!g_Fp) return NULL;

	for (i = 0; i < DISK_SECTORS; i ++)
		if (fwrite (buffer, sizeof (buffer), 1, g_Fp) != 1)
			return NULL;

	res            = new TBlkDev;
	res->m_Sectors = DISK_SECTORS;
	res->m_Read    = diskRead;
	res->m_Write   = diskWrite;
	return res;
}

/** Open the files needed for the sector reading/writing functions above.
 *  This function is only needed for the particular implementation above.
 *  It could be understood as "turning the computer on".
 */
static TBlkDev *
openDisk ()
{
	TBlkDev *res = NULL;

	g_Fp = fopen ("/tmp/disk_content", "r+b");
	if (!g_Fp)
		return NULL;

	fseek (g_Fp, 0, SEEK_END);
	if (ftell (g_Fp) != DISK_SECTORS * SECTOR_SIZE)
	{
		fclose (g_Fp);
		g_Fp = NULL;
		return NULL;
	}

	res            = new TBlkDev;
	res->m_Sectors = DISK_SECTORS;
	res->m_Read    = diskRead;
	res->m_Write   = diskWrite;
	return res;
}

/** Release resources allocated by openDisk()/createDisk().  */
static void
doneDisk (TBlkDev *dev)
{
	delete dev;
	if (g_Fp)
	{
		fclose (g_Fp);
		g_Fp  = NULL;
	}
}

// ---------------------------------------------------------------------------

/* It won't write more bytes at once than this. */
#define BLOCK_UNIT 9000
/* Maximum filesize. */
#define MAX_FILE_SIZE 100000

/* Detect N pawnz before committing suicide to cut the crap. */
static int pwnlimit = 1;
#define PWNCHECK(cond) do { if (!(cond)) { \
	fputs ("LOL, YOU FAILED. LOL, LOL, YOU FAILED\n", stderr); \
	fprintf (stderr, "%s@%d: %s\n",__FILE__, __LINE__, #cond); \
	if (!--pwnlimit) abort (); } } while (0)

#define RAND_RANGE(b, e) ((b) + rand () % ((e) - (b) + 1))
#define MIN(a, b) ((a) < (b) ? (a) : (b))


/* Back-up for file information. */
static struct TableEntry
{
	TFile metadata;
	char *data;
	bool flagged;
}
g_table[DIR_ENTRIES_MAX];
unsigned g_table_len;

static int
compare_entries (const void *p1, const void *p2)
{
	TableEntry *e1 = (TableEntry *) p1;
	TableEntry *e2 = (TableEntry *) p2;
	return strcmp (e1->metadata.m_FileName, e2->metadata.m_FileName);
}

// ---------------------------------------------------------------------------

/* A bunch of helper functions. */
static void
str_random (char *out, size_t len)
{
	while (len--)
		*out++ = RAND_RANGE ('A', 127);
	*out = '\0';
}

static void
blk_random (char *out, size_t len)
{
	while (len--)
		*out++ = rand ();
}

static void
get_unique_filename (char *out)
{
	while (1)
	{
		str_random (out, RAND_RANGE (1, FILENAME_LEN_MAX));

		int fd = FileOpen (out, 0);
		if (fd == -1)
			return;
		FileClose (fd);
	}
}

// ---------------------------------------------------------------------------

/* Some trivial tests before going full frontal. */
static void
basic_check (void)
{
	/* Yeah, I lied. */
	int size = BLOCK_UNIT * 32;

	char *data   = new char [size];
	char *buffer = new char [size];
	blk_random (data, size);

	/* Start with nothing. */
	TFile info;
	PWNCHECK (FileFindFirst (&info) == 0);

	/* Create a file. */
	int fd;
	PWNCHECK ((fd = FileOpen ("test", 1)) != -1);
	PWNCHECK (fd != -1);

	/* Write as much as we can and close it. */
	size -= size - FileWrite (fd, data, size);
	PWNCHECK (size != 0);
	PWNCHECK (FileClose (fd) == 0);

	/* See if it's there how we remember it. */
	PWNCHECK (FileFindFirst (&info) == 1);
	PWNCHECK (FileFindNext (&info) == 0);
	PWNCHECK (strcmp ("test", info.m_FileName) == 0);
	PWNCHECK (info.m_FileSize == size);
	PWNCHECK (FileSize ("test") == size);

	/* Read it back. */
	PWNCHECK ((fd = FileOpen ("test", 0)) != -1);
	PWNCHECK (FileRead (fd, buffer, size + 1234) == size);
	for (int i = 0; i < size; i++)
		PWNCHECK (data[i] == buffer[i]);
	PWNCHECK (FileRead (fd, buffer, size) == 0);
	PWNCHECK (FileClose (fd) == 0);

	/* And delete it. */
	PWNCHECK (FileDelete ("test") == 1);
	PWNCHECK ((fd = FileOpen ("test", 0)) == -1);

	delete [] data;
	delete [] buffer;
}

/* Create some random files and back them up in a table. */
static void
fill_fs (void)
{
	unsigned i, written_total = 0;
	bool full = false;

	for (i = 0; !full && i < DIR_ENTRIES_MAX; i++)
	{
		get_unique_filename (g_table[i].metadata.m_FileName);

		/* Make some data. */
		unsigned size = RAND_RANGE (0, MAX_FILE_SIZE);
		blk_random (g_table[i].data = new char [size], size);
		g_table[i].metadata.m_FileSize = size;

		/* Write it into the file in random-sized blocks. */
		int fd = FileOpen (g_table[i].metadata.m_FileName, 1);
		PWNCHECK (fd != -1);

		for (unsigned iter = size; iter; )
		{
			unsigned long to_write = RAND_RANGE (0, MIN (iter, BLOCK_UNIT));
			int written = FileWrite (fd,
				g_table[i].data + size - iter, to_write);
			PWNCHECK (written >= 0);

			/* If we overshot the FS capacity, break out. */
			written_total += written;
			iter -= written;
			if ((unsigned long) written < to_write)
			{
				g_table[i].metadata.m_FileSize -= iter;
				full = true;
				break;
			}
		}

		/* Have to close it, so that we don't run out of FD's. */
		PWNCHECK (FileClose (fd) == 0);
		g_table_len++;
	}

	if (full)
	{
		/* Capacity check. */
		double usage = (double) written_total / SECTOR_SIZE / DISK_SECTORS;
		fprintf (stderr, "II Capacity ratio: %f (with %u files)\n",
			usage, g_table_len);
		PWNCHECK (usage <= 1.0);
		PWNCHECK (usage >= 0.9);
	}

	/* Sort the table. */
	qsort (g_table, g_table_len, sizeof *g_table, compare_entries);
}

/* Check whether the FS returns what we've stored in it. */
static void
check_fs_contents (void)
{
	/* Unflag all the entries. */
	for (unsigned i = 0; i < g_table_len; i++)
		g_table[i].flagged = false;

	/* Walk through the root directory. */
	TFile info;
	for (int r = FileFindFirst (&info); r; r = FileFindNext (&info))
	{
		/* Check if we really wrote this file, with this size. */
		TableEntry *entry = (TableEntry *) bsearch (&info,
			g_table, g_table_len, sizeof *g_table, compare_entries);
		PWNCHECK (entry != NULL);
		PWNCHECK (entry->metadata.m_FileSize == info.m_FileSize);

		/* And we haven't gotten here twice. */
		PWNCHECK (entry->flagged == false);
		entry->flagged = true;

		/* Read the data back. */
		int fd = FileOpen (info.m_FileName, 0);
		PWNCHECK (fd != -1);

		char buffer[BLOCK_UNIT];
		unsigned to_read;
		for (unsigned iter = entry->metadata.m_FileSize; iter; iter -= to_read)
		{
			to_read = RAND_RANGE (0, MIN (iter, BLOCK_UNIT));
			int read = FileRead (fd, buffer, to_read);
			PWNCHECK (read >= 0 && (unsigned) read == to_read);
			PWNCHECK (memcmp (entry->data + entry->metadata.m_FileSize - iter,
				buffer, to_read) == 0);
		}

		PWNCHECK (FileClose (fd) == 0);
	}
}

/* Create a small file and fill it with some contents. */
static char *
create_small_file (const char *filename, int *size)
{
	char *data = new char[*size = RAND_RANGE (1, 4096)];
	blk_random (data, *size);

	/* It has to fit in there in all circumstances. */
	int fd;
	PWNCHECK ((fd = FileOpen (filename, 1)) != -1);
	PWNCHECK (FileWrite (fd, data, *size) == *size);
	PWNCHECK (FileClose (fd) == 0);
	return data;
}

/* Similar but only with small files. */
static void
fill_fs_small (void)
{
	unsigned i, written_total = 0;

	for (i = 0; i < DIR_ENTRIES_MAX; i++)
	{
		char *filename = g_table[i].metadata.m_FileName;
		get_unique_filename (filename);

		/* Write something into the file... and overwrite it. */
		int size;

		delete [] create_small_file (filename, &size);
		PWNCHECK (FileSize (filename) == size);
		g_table[i].data = create_small_file (filename, &size);
		PWNCHECK (FileSize (filename) == size);

		g_table[i].metadata.m_FileSize = size;
		written_total += size;
		g_table_len++;
	}

	/* Sort the table. */
	qsort (g_table, g_table_len, sizeof *g_table, compare_entries);
}

/* Similar but only with small files. */
static void
check_fs_small (void)
{
	/* Unflag all the entries. */
	for (unsigned i = 0; i < g_table_len; i++)
		g_table[i].flagged = false;

	/* Walk through the root directory. */
	TFile info;
	for (int r = FileFindFirst (&info); r; r = FileFindNext (&info))
	{
		/* Check if we really wrote this file, with this size. */
		TableEntry *entry = (TableEntry *) bsearch (&info,
			g_table, g_table_len, sizeof *g_table, compare_entries);
		PWNCHECK (entry != NULL);
		PWNCHECK (entry->metadata.m_FileSize == info.m_FileSize);
		unsigned size = entry->metadata.m_FileSize;

		/* And we haven't gotten here twice. */
		PWNCHECK (entry->flagged == false);
		entry->flagged = true;

		/* Read the data back. */
		int fd = FileOpen (info.m_FileName, 0);
		PWNCHECK (fd != -1);

		char buffer[4096];
		int read = FileRead (fd, buffer, size);
		PWNCHECK (read >= 0 && (unsigned) read == size);
		PWNCHECK (memcmp (entry->data, buffer, size) == 0);

		PWNCHECK (FileClose (fd) == 0);
	}
}

/* Destroy the generated data both from the table, and the FS. */
static void
check_fs_finish (void)
{
	for (unsigned i = 0; i < g_table_len; i++)
	{
		/* The FS may have skipped the file completely. */
		PWNCHECK (g_table[i].flagged == true);
		/* Then this will probably fail, too. */
		PWNCHECK (FileDelete (g_table[i].metadata.m_FileName) == 1);
		/* Free the memory. */
		delete [] g_table[i].data;
	}

	g_table_len = 0;

	/* Everything should be gone now. */
	TFile info;
	PWNCHECK (FileFindFirst (&info) == 0);
}

/* Write to several file descriptors at once. */
static void
check_parallel (void)
{
	static int fds[OPEN_FILES_MAX];
	static TFile info[OPEN_FILES_MAX];
	static char data[OPEN_FILES_MAX][BLOCK_UNIT];

	/* Prepare a bunch of files and some file-specific data. */
	for (unsigned i = 0; i < OPEN_FILES_MAX; i++)
	{
		get_unique_filename (info[i].m_FileName);
		info[i].m_FileSize = 0;
		blk_random (data[i], sizeof data[i]);

		fds[i] = FileOpen (info[i].m_FileName, 1);
		PWNCHECK (fds[i] != -1);
	}

	/* Write while we can. */
	unsigned long written_total = 0;
	for (unsigned fd = 0; ; fd = (fd + 1) % OPEN_FILES_MAX)
	{
		unsigned written = FileWrite (fds[fd], data[fd], sizeof data[fd]);
		PWNCHECK (written <= sizeof data[fd]);

		info[fd].m_FileSize += written;
		written_total += written;

		if (written < sizeof data[fd])
			break;
	}

	/* Capacity check. */
	double usage = (double) written_total / SECTOR_SIZE / DISK_SECTORS;
	fprintf (stderr, "TT Capacity ratio: %f (with %d files)\n",
		usage, OPEN_FILES_MAX);
	PWNCHECK (usage <= 1.0);
	PWNCHECK (usage >= 0.9);

	/* Close the files and open them for reading. */
	for (unsigned i = 0; i < OPEN_FILES_MAX; i++)
	{
		PWNCHECK (FileClose (fds[i]) == 0);
		PWNCHECK ((fds[i] = FileOpen (info[i].m_FileName, 0)) != -1);
		PWNCHECK (FileSize (info[i].m_FileName) == info[i].m_FileSize);
	}

	/* Read them back. */
	static char buffer[BLOCK_UNIT];
	unsigned files_left = OPEN_FILES_MAX;

	for (unsigned fd = 0; files_left; fd = (fd + 1) % OPEN_FILES_MAX)
	{
		if (!info[fd].m_FileSize)
			continue;

		unsigned read = FileRead (fds[fd], buffer, sizeof buffer);
		info[fd].m_FileSize -= read;

		/* Check the data. */
		PWNCHECK (memcmp (buffer, data[fd], read) == 0);

		/* The FileRead() function should return less than it was asked
		 * to only in case we've arrived at the end of the file. */
		if (read < sizeof buffer)
		{
			PWNCHECK (FileRead (fds[fd], buffer, sizeof buffer) == 0);
			PWNCHECK (info[fd].m_FileSize == 0);
		}

		if (!info[fd].m_FileSize)
			files_left--;
	}

	/* Get rid of them for good. */
	for (unsigned i = 0; i < OPEN_FILES_MAX; i++)
	{
		PWNCHECK (FileClose (fds[i]) == 0);
		PWNCHECK (FileDelete (info[i].m_FileName) == 1);
	}
}

// ---------------------------------------------------------------------------

int
main (int argc, char *argv[])
{
	TBlkDev *dev = createDisk ();

	/* Don't use time(NULL), it makes debugging harder. */
	srand (0);

	/* Stage 1: Write something to the disk. */
	assert (FsCreate (dev) == 1);
	assert (FsMount  (dev) == 1);
	basic_check ();
	fill_fs ();
	check_fs_contents ();
	assert (FsUmount ()    == 1);
	doneDisk (dev);

	/* Stage 2: Check if we can still find it after remount. */
	dev = openDisk ();
	assert (FsMount  (dev) == 1);
	check_fs_contents ();
	check_fs_finish ();
	assert (FsUmount ()    == 1);
	doneDisk (dev);

	/* Stage 3: Small files. */
	dev = openDisk ();
	assert (FsMount  (dev) == 1);
	fill_fs_small ();
	assert (FsUmount ()    == 1);

	assert (FsMount  (dev) == 1);
	check_fs_small ();
	check_fs_finish ();
	assert (FsUmount ()    == 1);
	doneDisk (dev);

	/* Stage 4: Test parallel writing + capacity. */
	dev = openDisk ();
	assert (FsMount  (dev) == 1);
	check_parallel ();
	assert (FsUmount ()    == 1);
	doneDisk (dev);

	return 0;
}

