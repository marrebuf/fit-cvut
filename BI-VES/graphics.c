#include <p24fxxxx.h>

#define _ADDED_C_LIB 1
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "libves/display.h"
#include "libves/delay.h"
#include "libves/font.h"

// Set the clock to 16MHz etc.
_CONFIG1 (JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF \
     & FWDTEN_OFF & ICS_PGx2)
_CONFIG2 (0xF7FF & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_OFF \
     & POSCMOD_HS & FNOSC_PRIPLL & PLLDIV_DIV3 & IOL1WAY_ON)

#define ABS(x)  ((x) < 0 ? -(x) : x)

// http://www.christianpinder.com/articles/pseudo-random-number-generation/
static long
rand_int (void)
{
	static long rnd_seed = 456L;
	long k1, ix = rnd_seed;

	k1 = ix / 127773L;
	ix = 16807L * (ix - k1 * 127773L) - k1 * 2836L;
	if (ix < 0)  ix += 2147483647L;
	return rnd_seed = ix;
}

static void
swap_int (int *a, int *b)
{
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static void
line (int x1, int y1, int x2, int y2)
{
	int i;

	if (ABS (x1 - x2) > ABS (y1 - y2))
	{
		if (x1 > x2)
		{
			swap_int (&x1, &x2);
			swap_int (&y1, &y2);
		}

		for (i = x1; i <= x2; i++)
		{
			long z = y1;
			if (x2 != x1)
				z += (long) (y2 - y1) * (i - x1) / (x2 - x1);
			put_pixel (i, z, COLOR_WHITE);
		}
	}
	else
	{
		if (y1 > y2)
		{
			swap_int (&x1, &x2);
			swap_int (&y1, &y2);
		}

		for (i = y1; i <= y2; i++)
		{
			long z = x1;
			if (y2 != y1)
				z += (long) (x2 - x1) * (i - y1) / (y2 - y1);
			put_pixel (z, i, COLOR_WHITE);
		}
	}
}

static void
poly (int n, int *coord)
{
	int i;

	for (i = 0; i < n - 1; i++)
		line (coord[2 * i],     coord[2 * i + 1],
		      coord[2 * i + 2], coord[2 * i + 3]);
}

static void
show_big (char *str)
{
	char s[2];

	if (!str[0])  return;
	if (!str[1])  { s[0] = ' ';    s[1] = str[0]; }
	else          { s[0] = str[0]; s[1] = str[1]; }

	disp_clear ();

	unsigned char *p1 = get_font_data_ptr (s[0]);
	unsigned char *p2 = get_font_data_ptr (s[1]);

	int page, col, i;

	for (page = 0; page < 8; page++)
	{
		sh1101_write (COMMAND, SET_PAGE_ADDRESS | page);

		// 16px + invisible 2px
		sh1101_write (COMMAND, SET_LOWER_COLUMN_ADDRESS  | 2);
		sh1101_write (COMMAND, SET_HIGHER_COLUMN_ADDRESS | 1);

		for (col = 0; col < 6; col++)
			for (i = 0; i < 8; i++)
				sh1101_write (DATA, ((p1[col] >> page) & 1) ? 255 : 0);

		// 72px + invisible 2px
		sh1101_write (COMMAND, SET_LOWER_COLUMN_ADDRESS  | 10);
		sh1101_write (COMMAND, SET_HIGHER_COLUMN_ADDRESS | 4);

		for (col = 0; col < 6; col++)
			for (i = 0; i < 8; i++)
				sh1101_write (DATA, ((p2[col] >> page) & 1) ? 255 : 0);
	}
}

static void
show_big_num (int num)
{
	char s[4] = {0, 0, 0};

	if (num < 0 || num >= 100)
		strcpy (s, "??");
	else
		snprintf (s, sizeof s, "%d", num);

	show_big (s);
}

int
main ()
{
	int i;

	disp_init ();

// ----- Scrolling numbers counting from 0 to 99, then ?? ---------------------

	sh1101_write (COMMAND, SET_CONTRAST_CONTROL_REGISTER);
	sh1101_write (COMMAND, 0xFF);

	for (i = 0; i <= 100; i++)
	{
		sh1101_write (COMMAND, SET_CONTRAST_CONTROL_REGISTER);
		sh1101_write (COMMAND, 0x80 + cos (i / 5.) * 0x78);

		show_big_num (i);
		sh1101_write (COMMAND, SET_DISPLAY_START_LINE | (i % SCREEN_HEIGHT));
		delay_loop_ms (50);
	}

// ----- Keep the question marks there for a while ----------------------------

	for (; i < 200; i++)
	{
		sh1101_write (COMMAND, SET_CONTRAST_CONTROL_REGISTER);
		sh1101_write (COMMAND, 0x80 + cos (i / 5.) * 0x78);

		sh1101_write (COMMAND, SET_DISPLAY_START_LINE | (i % SCREEN_HEIGHT));
		delay_loop_ms (50);
	}

// ----- Shapes and formatted text --------------------------------------------

	sh1101_write (COMMAND, SET_CONTRAST_CONTROL_REGISTER);
	sh1101_write (COMMAND, 0x60);

	sh1101_write (COMMAND, SET_DISPLAY_START_LINE | 0);

	for (i = 9; i >= 0; i--)
	{
		disp_clear ();

		char s[42];
		snprintf (s, sizeof s, "Cekam %d %.2f 0x%x...", i, 1.2, 255);

		disp_at (1, 1);
		disp_str (s);

		line (5, 20, 5 + 10 * i, 20);

		int sou[] = {10, 30, 50, 30, 20, 50, 60, 50};
		poly (sizeof sou / sizeof *sou / 2, sou);

//		draw_ellipse (80, 40, 20, 12);

		delay_loop_ms (250);
	}

// ----- Falling letters ------------------------------------------------------

#define COUNT 8
#define MIN_LENGTH 3
#define MAX_LENGTH 8

	char words[COUNT][MAX_LENGTH] = {""};
	int words_cnt[COUNT] = {0};
	int words_coord[COUNT][2] = {{0, 0}};
	int words_len[COUNT] = {0};

	for (i = 0; i < COUNT; i++)
	{
		words_coord[i][0] = rand_int() %  SCREEN_WIDTH;
		words_coord[i][1] = rand_int() % (SCREEN_HEIGHT / 2);
		words_len[i] = MIN_LENGTH + rand_int() % (MAX_LENGTH - MIN_LENGTH);
	}

	for (;;)
	{
		disp_clear ();

		for (i = 0; i < COUNT; i++)
		{
			int k, x, y;

			x = words_coord[i][0];
			y = words_coord[i][1];

			for (k = 0; k < words_cnt[i]; k++)
			{
				draw_char (x - get_char_width (words[i][k]) / 2,
						   y, words[i][k]);
				y += 8;
			}

			if (words_cnt[i] >= words_len[i])
			{
				words_cnt[i] = 0;
				words_coord[i][0] = rand_int() %  SCREEN_WIDTH;
				words_coord[i][1] = rand_int() % (SCREEN_HEIGHT / 2);
				words_len[i] = MIN_LENGTH
							  + rand_int() % (MAX_LENGTH - MIN_LENGTH);
			}
			else
				words[i][words_cnt[i]++] = 'a' + (rand_int() % 26);
		}

		delay_loop_ms (100);
	}

	return 0;
}

