#ifndef __PROGTEST__
#include "common_ssvc.h"
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#endif /* ! __PROGTEST__ */

/* ===== Utilities ========================================================= */
static void
check (int condition, const char *fmt, ...)
{
	va_list ap;

	if (!condition)
	{
		va_start (ap, fmt);
		vfprintf (stderr, fmt, ap);
		va_end (ap);
		exit (EXIT_FAILURE);
	}
}


/* ===== Test environment ================================================== */

typedef struct
{
	TMESSAGE enc_msg, dec_msg;
	int shift;
}
TestUnit;

static TestUnit *g_units;
static unsigned g_n_units;


/* This is a very simple receiver. The code generates a total of 3 messages,
 * followed by NULL answers. The messages are taken from a pre-computed
 * table, the first two entries are correct (with respect to the keys),
 * the third is not.
 */
static const TMESSAGE *
sample_receiver (void)
{
	static unsigned cnt = 0;

	if (cnt >= g_n_units)  return NULL;

	printf ("Message received!\n");
	return &g_units[cnt++].enc_msg;
}

/* This is a very simple officer. It accepts the message and the list of
 * possible agent/shift pairs. The code here only displays the numbers.
 */
static void
sample_officer (const TMESSAGE *msg, TRESULTS *res, int res_nr)
{
	int i;

	// TODO: Check the results
	printf ("Message processing finished!\n");
	for (i = 0; i < res_nr; i++)
		printf ("\tagent %d, shift %d\n", res[i].m_Agent, res[i].m_Shift);
	if (res_nr == 0)
		printf ("An invalid message, perhaps KGB?\n");

	/* res is not freed here. It is the responsibility of the caller. */
}

/** Read out agents' keys from the specified file. */
static int
read_keys (const char *file, unsigned char out[AGENTS_MAX][KEY_LENGTH])
{
	int n_keys = 0;
	FILE *fp;

	fp = fopen (file, "rb");
	check (fp != NULL, "Failed to open the key file: %s\n",
		strerror (errno));

	while (fread (out[n_keys], KEY_LENGTH, 1, fp))
		n_keys++;

	fclose (fp);
	return n_keys;
}

/** Open `ctr.suffix' for binary reading. */
static FILE *
ctr_open (unsigned ctr, const char *suffix)
{
	char fname[10];
	
	snprintf (fname, sizeof (fname), "%03u.%s", ctr, suffix);
	return fopen (fname, "rb");
}

/** Read test inputs out of the current directory.
 *  .out  -- encrypted message
 *  .txt  -- text message
 *  .shift -- how many bytes `out' is shifted by
 */
static unsigned
load_unit_files (TestUnit **output)
{
	TestUnit *units;
	unsigned len = 0, size = 8, ctr;

	units = (TestUnit *) malloc (size * sizeof *units);

	printf ("Loading units... ");
	for (ctr = 0; ; ctr++)
	{
		if (len == size)
			units = (TestUnit *) realloc (units,
				(size <<= 1) * sizeof *units);

		FILE *out   = ctr_open (ctr, "out"),
		     *txt   = ctr_open (ctr, "txt"),
		     *shift = ctr_open (ctr, "shift");

		if (!out || !shift || !txt)
		{
			if (out)    fclose (out);
			if (txt)    fclose (txt);
			if (shift)  fclose (shift);

			break;
		}

		if (ctr)  printf (", ");
		printf ("%u", ctr);

		units[ctr].enc_msg.m_Length =
			fread (units[ctr].enc_msg.m_Message, 1, MESSAGE_MAX, out);
		units[ctr].dec_msg.m_Length =
			fread (units[ctr].dec_msg.m_Message, 1, MESSAGE_MAX, txt);

		if (ferror (out) || ferror (txt))
			printf (" [error reading messages]");
		else if (!fscanf (shift, "%d", &units[ctr].shift))
			printf (" [error reading shift value]");
		else
			len++;

		fclose (out);
		fclose (txt);
		fclose (shift);
	}

	if (len)  putchar ('\n');
	else      puts ("none present");

	*output = units;
	return len;
}

int
main (int argc, char *argv [])
{
	unsigned char keys[AGENTS_MAX][KEY_LENGTH];
	int n_keys;

	check (chdir ("sample_ssvc") == 0, "Failed to chdir\n");
	n_keys = read_keys ("keys", keys);
	check (n_keys != 0, "No keys\n");
	g_n_units = load_unit_files (&g_units);
	check (g_units != NULL, "No test units\n");

	SecretService (n_keys, keys, 2, sample_receiver, sample_officer);
	return 0;
}

