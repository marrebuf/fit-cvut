#include <p24fxxxx.h>

#define _ADDED_C_LIB 1
#include <stdio.h>
#include <string.h>

#include "libves/display.h"
#include "libves/delay.h"
#include "libves/font.h"

#include "touch.h"

// Set the clock to 16MHz etc.
_CONFIG1 (JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF \
     & FWDTEN_OFF & ICS_PGx2)
_CONFIG2 (0xF7FF & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_OFF \
     & POSCMOD_HS & FNOSC_PRIPLL & PLLDIV_DIV3 & IOL1WAY_ON)

// ----- Utilities, framework -------------------------------------------------

static void
disp_str_inverse (const char *s)
{
	int r = disp_get_row (), c = disp_get_col ();
	sh1101_write (COMMAND, SET_PAGE_ADDRESS | (7 - r));

	int c_off = 2 + c * 6;
	sh1101_write (COMMAND, SET_LOWER_COLUMN_ADDRESS  | (c_off & 0xf));
	sh1101_write (COMMAND, SET_HIGHER_COLUMN_ADDRESS | (c_off >> 4));

	int i;
	for (; *s; s++, c++)
		for (i = 0; i < 6; i++)
			sh1101_write (DATA, get_font_data_ptr (*s)[i] ^ 0xff);

	disp_at (r, c);
}

struct
{
	char *name;
	int x;
	int y;
	int value;
}
g_menu[] =
{
	{ " 1 Kc",       0, 3, 1 },
	{ " 2 Kc",       0, 4, 2 },
	{ " 5 Kc",       0, 5, 3 },
	{ "10 Kc",       0, 6, 4 },
	{ "20 Kc",       0, 7, 5 },

	{ "Vyber  6 Kc", 8, 3, 6 },
	{ "Vyber  8 Kc", 8, 4, 7 },
	{ "Vyber 12 Kc", 8, 5, 8 },

	{ "Storno     ", 8, 7, 0 },
};

#define N_ELEMENTS(a) (sizeof (a) / sizeof (*(a)))

static unsigned g_menu_current;

static void
draw_menu (void)
{
	size_t i;
	for (i = 0; i < N_ELEMENTS (g_menu); i++)
	{
		disp_at (g_menu[i].y, g_menu[i].x);
		if (g_menu_current == i)
		{
			disp_str_inverse ("> ");
			disp_str_inverse (g_menu[i].name);
		}
		else
		{
			disp_str ("  ");
			disp_str (g_menu[i].name);
		}
	}
}

static void
c (void)
{
	disp_clear ();
	delay_loop_ms (50);

	draw_menu ();
	disp_at (0, 0);
}

static void
d (char *s)
{
	disp_line (s);
}

static int
read_input (void)
{
	while (1)
	{
		switch (touch_readkey ())
		{
		case 1:
			g_menu_current = (g_menu_current + N_ELEMENTS (g_menu) - 1)
				% N_ELEMENTS (g_menu);
			draw_menu ();
			break;
		case 3:
			g_menu_current = (g_menu_current + 1) % N_ELEMENTS (g_menu);
			draw_menu ();
			break;
		case 5:
			return g_menu[g_menu_current].value;
		default:
			/* ignore other keys */
			break;
		}
	}
}

static void main_table (void);
static void main_switch (void);

int
main ()
{
	disp_init ();
	touch_init ();

	c ();

//	main_table ();
	main_switch ();
	return 0;
}

// ----- Table ----------------------------------------------------------------

static void g8r9 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 9 Kc"); }
static void g8r8 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 8 Kc"); }
static void r28 (void) { c (); d ("vraceno 28 Kc"); }
static void g8r3 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 3 Kc"); }
static void g8r2 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 2 Kc"); }
static void g8r1 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 1 Kc"); }
static void g8r7 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 7 Kc"); }
static void g8r6 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 6 Kc"); }
static void g8r5 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 5 Kc"); }
static void g8r4 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 4 Kc"); }
static void r16 (void) { c (); d ("vraceno 16 Kc"); }
static void g6 (void) { c (); d ("vydavam za 6 Kc"); }
static void r14 (void) { c (); d ("vraceno 14 Kc"); }
static void r15 (void) { c (); d ("vraceno 15 Kc"); }
static void r12 (void) { c (); d ("vraceno 12 Kc"); }
static void r13 (void) { c (); d ("vraceno 13 Kc"); }
static void r10 (void) { c (); d ("vraceno 10 Kc"); }
static void r11 (void) { c (); d ("vraceno 11 Kc"); }
static void r18 (void) { c (); d ("vraceno 18 Kc"); }
static void r19 (void) { c (); d ("vraceno 19 Kc"); }
static void p10 (void) { c (); d ("vhozeno 10 Kc"); }
static void p11 (void) { c (); d ("vhozeno 11 Kc"); }
static void p12 (void) { c (); d ("vhozeno 12 Kc"); }
static void p13 (void) { c (); d ("vhozeno 13 Kc"); }
static void p14 (void) { c (); d ("vhozeno 14 Kc"); }
static void p15 (void) { c (); d ("vhozeno 15 Kc"); }
static void p16 (void) { c (); d ("vhozeno 16 Kc"); }
static void p17 (void) { c (); d ("vhozeno 17 Kc"); }
static void p18 (void) { c (); d ("vhozeno 18 Kc"); }
static void p19 (void) { c (); d ("vhozeno 19 Kc"); }
static void r25 (void) { c (); d ("vraceno 25 Kc"); }
static void g6r24 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 24 Kc"); }
static void g6r25 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 25 Kc"); }
static void g6r26 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 26 Kc"); }
static void g6r20 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 20 Kc"); }
static void g6r21 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 21 Kc"); }
static void g6r22 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 22 Kc"); }
static void g6r23 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 23 Kc"); }
static void g6r5 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 5 Kc"); }
static void g6r4 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 4 Kc"); }
static void g6r7 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 7 Kc"); }
static void g6r6 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 6 Kc"); }
static void g6r1 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 1 Kc"); }
static void g6r3 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 3 Kc"); }
static void g6r2 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 2 Kc"); }
static void g6r9 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 9 Kc"); }
static void g6r8 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 8 Kc"); }
static void p25 (void) { c (); d ("vhozeno 25 Kc"); }
static void p24 (void) { c (); d ("vhozeno 24 Kc"); }
static void p27 (void) { c (); d ("vhozeno 27 Kc"); }
static void p26 (void) { c (); d ("vhozeno 26 Kc"); }
static void p21 (void) { c (); d ("vhozeno 21 Kc"); }
static void p20 (void) { c (); d ("vhozeno 20 Kc"); }
static void p23 (void) { c (); d ("vhozeno 23 Kc"); }
static void p22 (void) { c (); d ("vhozeno 22 Kc"); }
static void p29 (void) { c (); d ("vhozeno 29 Kc"); }
static void p28 (void) { c (); d ("vhozeno 28 Kc"); }
static void r21 (void) { c (); d ("vraceno 21 Kc"); }
static void p32 (void) { c (); d ("vhozeno 32 Kc"); }
static void p30 (void) { c (); d ("vhozeno 30 Kc"); }
static void p31 (void) { c (); d ("vhozeno 31 Kc"); }
static void r4 (void) { c (); d ("vraceno 4 Kc"); }
static void r5 (void) { c (); d ("vraceno 5 Kc"); }
static void r6 (void) { c (); d ("vraceno 6 Kc"); }
static void r7 (void) { c (); d ("vraceno 7 Kc"); }
static void r1 (void) { c (); d ("vraceno 1 Kc"); }
static void r2 (void) { c (); d ("vraceno 2 Kc"); }
static void r3 (void) { c (); d ("vraceno 3 Kc"); }
static void r8 (void) { c (); d ("vraceno 8 Kc"); }
static void r9 (void) { c (); d ("vraceno 9 Kc"); }
static void g12r18 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 18 Kc"); }
static void g12r9 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 9 Kc"); }
static void g12r8 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 8 Kc"); }
static void g12r5 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 5 Kc"); }
static void g12r4 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 4 Kc"); }
static void g12r7 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 7 Kc"); }
static void g12r6 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 6 Kc"); }
static void g12r1 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 1 Kc"); }
static void g12r3 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 3 Kc"); }
static void g12r2 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 2 Kc"); }
static void g12r16 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 16 Kc"); }
static void g8r22 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 22 Kc"); }
static void g8r23 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 23 Kc"); }
static void g8r20 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 20 Kc"); }
static void g8r21 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 21 Kc"); }
static void g8r24 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 24 Kc"); }
static void p2 (void) { c (); d ("vhozeno 2 Kc"); }
static void p3 (void) { c (); d ("vhozeno 3 Kc"); }
static void p1 (void) { c (); d ("vhozeno 1 Kc"); }
static void p6 (void) { c (); d ("vhozeno 6 Kc"); }
static void p7 (void) { c (); d ("vhozeno 7 Kc"); }
static void p4 (void) { c (); d ("vhozeno 4 Kc"); }
static void p5 (void) { c (); d ("vhozeno 5 Kc"); }
static void p8 (void) { c (); d ("vhozeno 8 Kc"); }
static void p9 (void) { c (); d ("vhozeno 9 Kc"); }
static void g12r19 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 19 Kc"); }
static void r29 (void) { c (); d ("vraceno 29 Kc"); }
static void r17 (void) { c (); d ("vraceno 17 Kc"); }
static void g12r15 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 15 Kc"); }
static void g12r14 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 14 Kc"); }
static void g12r17 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 17 Kc"); }
static void g8r18 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 18 Kc"); }
static void g12r11 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 11 Kc"); }
static void g12r10 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 10 Kc"); }
static void g12r13 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 13 Kc"); }
static void g12r12 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 12 Kc"); }
static void r27 (void) { c (); d ("vraceno 27 Kc"); }
static void g8r19 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 19 Kc"); }
static void r26 (void) { c (); d ("vraceno 26 Kc"); }
static void g8r17 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 17 Kc"); }
static void g8r16 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 16 Kc"); }
static void g8r15 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 15 Kc"); }
static void g8r14 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 14 Kc"); }
static void g8r13 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 13 Kc"); }
static void g8r12 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 12 Kc"); }
static void g8r11 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 11 Kc"); }
static void g8r10 (void) { c (); d ("vydavam za 8 Kc"); d ("vraceno 10 Kc"); }
static void r24 (void) { c (); d ("vraceno 24 Kc"); }
static void r23 (void) { c (); d ("vraceno 23 Kc"); }
static void r22 (void) { c (); d ("vraceno 22 Kc"); }
static void g6r15 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 15 Kc"); }
static void g6r14 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 14 Kc"); }
static void g6r17 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 17 Kc"); }
static void g6r16 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 16 Kc"); }
static void g6r11 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 11 Kc"); }
static void g6r10 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 10 Kc"); }
static void g6r13 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 13 Kc"); }
static void g6r12 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 12 Kc"); }
static void r20 (void) { c (); d ("vraceno 20 Kc"); }
static void g6r19 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 19 Kc"); }
static void g6r18 (void) { c (); d ("vydavam za 6 Kc"); d ("vraceno 18 Kc"); }
static void g12r20 (void) { c (); d ("vydavam za 12 Kc"); d ("vraceno 20 Kc"); }
static void g8 (void) { c (); d ("vydavam za 8 Kc"); }
static void r30 (void) { c (); d ("vraceno 30 Kc"); }
static void r31 (void) { c (); d ("vraceno 31 Kc"); }
static void r32 (void) { c (); d ("vraceno 32 Kc"); }
static void g12 (void) { c (); d ("vydavam za 12 Kc"); }

#define N_STAVU   33
#define N_SYMBOLU  9

static int g_prechody[N_STAVU][N_SYMBOLU] =
{
	{  0,  1,  2,  5, 10, 20,  0,  0,  0 },
	{  0,  2,  3,  6, 11, 21,  1,  1,  1 },
	{  0,  3,  4,  7, 12, 22,  2,  2,  2 },
	{  0,  4,  5,  8, 13, 23,  3,  3,  3 },
	{  0,  5,  6,  9, 14, 24,  4,  4,  4 },
	{  0,  6,  7, 10, 15, 25,  5,  5,  5 },
	{  0,  7,  8, 11, 16, 26,  0,  6,  6 },
	{  0,  8,  9, 12, 17, 27,  0,  7,  7 },
	{  0,  9, 10, 13, 18, 28,  0,  0,  8 },
	{  0, 10, 11, 14, 19, 29,  0,  0,  9 },
	{  0, 11, 12, 15, 20, 30,  0,  0, 10 },
	{  0, 12, 13, 16, 21, 31,  0,  0, 11 },
	{  0, 13, 14, 17, 22, 32,  0,  0,  0 },
	{  0, 13, 13, 13, 13, 13,  0,  0,  0 },
	{  0, 14, 14, 14, 14, 14,  0,  0,  0 },
	{  0, 15, 15, 15, 15, 15,  0,  0,  0 },
	{  0, 16, 16, 16, 16, 16,  0,  0,  0 },
	{  0, 17, 17, 17, 17, 17,  0,  0,  0 },
	{  0, 18, 18, 18, 18, 18,  0,  0,  0 },
	{  0, 19, 19, 19, 19, 19,  0,  0,  0 },
	{  0, 20, 20, 20, 20, 20,  0,  0,  0 },
	{  0, 21, 21, 21, 21, 21,  0,  0,  0 },
	{  0, 22, 22, 22, 22, 22,  0,  0,  0 },
	{  0, 23, 23, 23, 23, 23,  0,  0,  0 },
	{  0, 24, 24, 24, 24, 24,  0,  0,  0 },
	{  0, 25, 25, 25, 25, 25,  0,  0,  0 },
	{  0, 26, 26, 26, 26, 26,  0,  0,  0 },
	{  0, 27, 27, 27, 27, 27,  0,  0,  0 },
	{  0, 28, 28, 28, 28, 28,  0,  0,  0 },
	{  0, 29, 29, 29, 29, 29,  0,  0,  0 },
	{  0, 30, 30, 30, 30, 30,  0,  0,  0 },
	{  0, 31, 31, 31, 31, 31,  0,  0,  0 },
	{  0, 32, 32, 32, 32, 32,  0,  0,  0 }
};

typedef void (*action_t) (void);

static action_t g_vystupy[N_STAVU][N_SYMBOLU] =
{
	{ c,      p1,     p2,     p5,     p10,    p20,    0,      0,      0      },
	{ r1,     p2,     p3,     p6,     p11,    p21,    0,      0,      0      },
	{ r2,     p3,     p4,     p7,     p12,    p22,    0,      0,      0      },
	{ r3,     p4,     p5,     p8,     p13,    p23,    0,      0,      0      },
	{ r4,     p5,     p6,     p9,     p14,    p24,    0,      0,      0      },
	{ r5,     p6,     p7,     p10,    p15,    p25,    0,      0,      0      },
	{ r6,     p7,     p8,     p11,    p16,    p26,    g6,     0,      0      },
	{ r7,     p8,     p9,     p12,    p17,    p27,    g6r1,   0,      0      },
	{ r8,     p9,     p10,    p13,    p18,    p28,    g6r2,   g8,     0      },
	{ r9,     p10,    p11,    p14,    p19,    p29,    g6r3,   g8r1,   0      },
	{ r10,    p11,    p12,    p15,    p20,    p30,    g6r4,   g8r2,   0      },
	{ r11,    p12,    p13,    p16,    p21,    p31,    g6r5,   g8r3,   0      },
	{ r12,    p13,    p14,    p17,    p22,    p32,    g6r6,   g8r4,   g12    },
	{ r13,    0,      0,      0,      0,      0,      g6r7,   g8r5,   g12r1  },
	{ r14,    0,      0,      0,      0,      0,      g6r8,   g8r6,   g12r2  },
	{ r15,    0,      0,      0,      0,      0,      g6r9,   g8r7,   g12r3  },
	{ r16,    0,      0,      0,      0,      0,      g6r10,  g8r8,   g12r4  },
	{ r17,    0,      0,      0,      0,      0,      g6r11,  g8r9,   g12r5  },
	{ r18,    0,      0,      0,      0,      0,      g6r12,  g8r10,  g12r6  },
	{ r19,    0,      0,      0,      0,      0,      g6r13,  g8r11,  g12r7  },
	{ r20,    0,      0,      0,      0,      0,      g6r14,  g8r12,  g12r8  },
	{ r21,    0,      0,      0,      0,      0,      g6r15,  g8r13,  g12r9  },
	{ r22,    0,      0,      0,      0,      0,      g6r16,  g8r14,  g12r10 },
	{ r23,    0,      0,      0,      0,      0,      g6r17,  g8r15,  g12r11 },
	{ r24,    0,      0,      0,      0,      0,      g6r18,  g8r16,  g12r12 },
	{ r25,    0,      0,      0,      0,      0,      g6r19,  g8r17,  g12r13 },
	{ r26,    0,      0,      0,      0,      0,      g6r20,  g8r18,  g12r14 },
	{ r27,    0,      0,      0,      0,      0,      g6r21,  g8r19,  g12r15 },
	{ r28,    0,      0,      0,      0,      0,      g6r22,  g8r20,  g12r16 },
	{ r29,    0,      0,      0,      0,      0,      g6r23,  g8r21,  g12r17 },
	{ r30,    0,      0,      0,      0,      0,      g6r24,  g8r22,  g12r18 },
	{ r31,    0,      0,      0,      0,      0,      g6r25,  g8r23,  g12r19 },
	{ r32,    0,      0,      0,      0,      0,      g6r26,  g8r24,  g12r20 }
};

void
main_table (void)
{
	int q = 0;
	while (1)
	{
		int i = read_input ();
		action_t a = g_vystupy[q][i];
		if (a) a ();
		q = g_prechody[q][i];
	}
}

// ----- Switch ---------------------------------------------------------------

enum { q0, q1, q2, q3, q4, q5, q6, q7, q8, q9,
	   q10, q11, q12, q13, q14, q15, q16, q17, q18, q19,
	   q20, q21, q22, q23, q24, q25, q26, q27, q28, q29,
	   q30, q31, q32 };

enum { STORNO, VHOZENO_1, VHOZENO_2, VHOZENO_5, VHOZENO_10, VHOZENO_20,
	   JIZDENKA_6, JIZDENKA_8, JIZDENKA_12 };

void
main_switch (void)
{
	int q = 0;
	while (1)
	{
		int i = read_input ();
		switch (q)
		{
		case q0:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				break;
			case VHOZENO_1:
				q = q1;
				c ();
				d ("vhozeno 1 Kc");
				break;
			case VHOZENO_2:
				q = q2;
				c ();
				d ("vhozeno 2 Kc");
				break;
			case VHOZENO_5:
				q = q5;
				c ();
				d ("vhozeno 5 Kc");
				break;
			case VHOZENO_10:
				q = q10;
				c ();
				d ("vhozeno 10 Kc");
				break;
			case VHOZENO_20:
				q = q20;
				c ();
				d ("vhozeno 20 Kc");
				break;
			default:
				break;
			}
			break;
		case q1:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 1 Kc");
				break;
			case VHOZENO_1:
				q = q2;
				c ();
				d ("vhozeno 2 Kc");
				break;
			case VHOZENO_2:
				q = q3;
				c ();
				d ("vhozeno 3 Kc");
				break;
			case VHOZENO_5:
				q = q6;
				c ();
				d ("vhozeno 6 Kc");
				break;
			case VHOZENO_10:
				q = q11;
				c ();
				d ("vhozeno 11 Kc");
				break;
			case VHOZENO_20:
				q = q21;
				c ();
				d ("vhozeno 21 Kc");
				break;
			default:
				break;
			}
			break;
		case q2:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 2 Kc");
				break;
			case VHOZENO_1:
				q = q3;
				c ();
				d ("vhozeno 3 Kc");
				break;
			case VHOZENO_2:
				q = q4;
				c ();
				d ("vhozeno 4 Kc");
				break;
			case VHOZENO_5:
				q = q7;
				c ();
				d ("vhozeno 7 Kc");
				break;
			case VHOZENO_10:
				q = q12;
				c ();
				d ("vhozeno 12 Kc");
				break;
			case VHOZENO_20:
				q = q22;
				c ();
				d ("vhozeno 22 Kc");
				break;
			default:
				break;
			}
			break;
		case q3:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 3 Kc");
				break;
			case VHOZENO_1:
				q = q4;
				c ();
				d ("vhozeno 4 Kc");
				break;
			case VHOZENO_2:
				q = q5;
				c ();
				d ("vhozeno 5 Kc");
				break;
			case VHOZENO_5:
				q = q8;
				c ();
				d ("vhozeno 8 Kc");
				break;
			case VHOZENO_10:
				q = q13;
				c ();
				d ("vhozeno 13 Kc");
				break;
			case VHOZENO_20:
				q = q23;
				c ();
				d ("vhozeno 23 Kc");
				break;
			default:
				break;
			}
			break;
		case q4:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 4 Kc");
				break;
			case VHOZENO_1:
				q = q5;
				c ();
				d ("vhozeno 5 Kc");
				break;
			case VHOZENO_2:
				q = q6;
				c ();
				d ("vhozeno 6 Kc");
				break;
			case VHOZENO_5:
				q = q9;
				c ();
				d ("vhozeno 9 Kc");
				break;
			case VHOZENO_10:
				q = q14;
				c ();
				d ("vhozeno 14 Kc");
				break;
			case VHOZENO_20:
				q = q24;
				c ();
				d ("vhozeno 24 Kc");
				break;
			default:
				break;
			}
			break;
		case q5:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 5 Kc");
				break;
			case VHOZENO_1:
				q = q6;
				c ();
				d ("vhozeno 6 Kc");
				break;
			case VHOZENO_2:
				q = q7;
				c ();
				d ("vhozeno 7 Kc");
				break;
			case VHOZENO_5:
				q = q10;
				c ();
				d ("vhozeno 10 Kc");
				break;
			case VHOZENO_10:
				q = q15;
				c ();
				d ("vhozeno 15 Kc");
				break;
			case VHOZENO_20:
				q = q25;
				c ();
				d ("vhozeno 25 Kc");
				break;
			default:
				break;
			}
			break;
		case q6:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 6 Kc");
				break;
			case VHOZENO_1:
				q = q7;
				c ();
				d ("vhozeno 7 Kc");
				break;
			case VHOZENO_2:
				q = q8;
				c ();
				d ("vhozeno 8 Kc");
				break;
			case VHOZENO_5:
				q = q11;
				c ();
				d ("vhozeno 11 Kc");
				break;
			case VHOZENO_10:
				q = q16;
				c ();
				d ("vhozeno 16 Kc");
				break;
			case VHOZENO_20:
				q = q26;
				c ();
				d ("vhozeno 26 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				break;
			default:
				break;
			}
			break;
		case q7:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 7 Kc");
				break;
			case VHOZENO_1:
				q = q8;
				c ();
				d ("vhozeno 8 Kc");
				break;
			case VHOZENO_2:
				q = q9;
				c ();
				d ("vhozeno 9 Kc");
				break;
			case VHOZENO_5:
				q = q12;
				c ();
				d ("vhozeno 12 Kc");
				break;
			case VHOZENO_10:
				q = q17;
				c ();
				d ("vhozeno 17 Kc");
				break;
			case VHOZENO_20:
				q = q27;
				c ();
				d ("vhozeno 27 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 1 Kc");
				break;
			default:
				break;
			}
			break;
		case q8:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 8 Kc");
				break;
			case VHOZENO_1:
				q = q9;
				c ();
				d ("vhozeno 9 Kc");
				break;
			case VHOZENO_2:
				q = q10;
				c ();
				d ("vhozeno 10 Kc");
				break;
			case VHOZENO_5:
				q = q13;
				c ();
				d ("vhozeno 13 Kc");
				break;
			case VHOZENO_10:
				q = q18;
				c ();
				d ("vhozeno 18 Kc");
				break;
			case VHOZENO_20:
				q = q28;
				c ();
				d ("vhozeno 28 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 2 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				break;
			default:
				break;
			}
			break;
		case q9:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 9 Kc");
				break;
			case VHOZENO_1:
				q = q10;
				c ();
				d ("vhozeno 10 Kc");
				break;
			case VHOZENO_2:
				q = q11;
				c ();
				d ("vhozeno 11 Kc");
				break;
			case VHOZENO_5:
				q = q14;
				c ();
				d ("vhozeno 14 Kc");
				break;
			case VHOZENO_10:
				q = q19;
				c ();
				d ("vhozeno 19 Kc");
				break;
			case VHOZENO_20:
				q = q29;
				c ();
				d ("vhozeno 29 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 3 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 1 Kc");
				break;
			default:
				break;
			}
			break;
		case q10:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 10 Kc");
				break;
			case VHOZENO_1:
				q = q11;
				c ();
				d ("vhozeno 11 Kc");
				break;
			case VHOZENO_2:
				q = q12;
				c ();
				d ("vhozeno 12 Kc");
				break;
			case VHOZENO_5:
				q = q15;
				c ();
				d ("vhozeno 15 Kc");
				break;
			case VHOZENO_10:
				q = q20;
				c ();
				d ("vhozeno 20 Kc");
				break;
			case VHOZENO_20:
				q = q30;
				c ();
				d ("vhozeno 30 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 4 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 2 Kc");
				break;
			default:
				break;
			}
			break;
		case q11:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 11 Kc");
				break;
			case VHOZENO_1:
				q = q12;
				c ();
				d ("vhozeno 12 Kc");
				break;
			case VHOZENO_2:
				q = q13;
				c ();
				d ("vhozeno 13 Kc");
				break;
			case VHOZENO_5:
				q = q16;
				c ();
				d ("vhozeno 16 Kc");
				break;
			case VHOZENO_10:
				q = q21;
				c ();
				d ("vhozeno 21 Kc");
				break;
			case VHOZENO_20:
				q = q31;
				c ();
				d ("vhozeno 31 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 5 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 3 Kc");
				break;
			default:
				break;
			}
			break;
		case q12:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 12 Kc");
				break;
			case VHOZENO_1:
				q = q13;
				c ();
				d ("vhozeno 13 Kc");
				break;
			case VHOZENO_2:
				q = q14;
				c ();
				d ("vhozeno 14 Kc");
				break;
			case VHOZENO_5:
				q = q17;
				c ();
				d ("vhozeno 17 Kc");
				break;
			case VHOZENO_10:
				q = q22;
				c ();
				d ("vhozeno 22 Kc");
				break;
			case VHOZENO_20:
				q = q32;
				c ();
				d ("vhozeno 32 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 6 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 4 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				break;
			default:
				break;
			}
			break;
		case q13:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 13 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 7 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 5 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 1 Kc");
				break;
			default:
				break;
			}
			break;
		case q14:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 14 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 8 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 6 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 2 Kc");
				break;
			default:
				break;
			}
			break;
		case q15:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 15 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 9 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 7 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 3 Kc");
				break;
			default:
				break;
			}
			break;
		case q16:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 16 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 10 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 8 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 4 Kc");
				break;
			default:
				break;
			}
			break;
		case q17:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 17 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 11 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 9 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 5 Kc");
				break;
			default:
				break;
			}
			break;
		case q18:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 18 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 12 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 10 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 6 Kc");
				break;
			default:
				break;
			}
			break;
		case q19:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 19 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 13 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 11 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 7 Kc");
				break;
			default:
				break;
			}
			break;
		case q20:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 20 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 14 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 12 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 8 Kc");
				break;
			default:
				break;
			}
			break;
		case q21:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 21 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 15 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 13 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 9 Kc");
				break;
			default:
				break;
			}
			break;
		case q22:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 22 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 16 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 14 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 10 Kc");
				break;
			default:
				break;
			}
			break;
		case q23:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 23 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 17 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 15 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 11 Kc");
				break;
			default:
				break;
			}
			break;
		case q24:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 24 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 18 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 16 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 12 Kc");
				break;
			default:
				break;
			}
			break;
		case q25:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 25 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 19 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 17 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 13 Kc");
				break;
			default:
				break;
			}
			break;
		case q26:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 26 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 20 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 18 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 14 Kc");
				break;
			default:
				break;
			}
			break;
		case q27:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 27 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 21 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 19 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 15 Kc");
				break;
			default:
				break;
			}
			break;
		case q28:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 28 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 22 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 20 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 16 Kc");
				break;
			default:
				break;
			}
			break;
		case q29:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 29 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 23 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 21 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 17 Kc");
				break;
			default:
				break;
			}
			break;
		case q30:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 30 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 24 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 22 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 18 Kc");
				break;
			default:
				break;
			}
			break;
		case q31:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 31 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 25 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 23 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 19 Kc");
				break;
			default:
				break;
			}
			break;
		case q32:
			switch (i)
			{
			case STORNO:
				q = q0;
				c ();
				d ("vraceno 32 Kc");
				break;
			case JIZDENKA_6:
				q = q0;
				c ();
				d ("vydavam za 6 Kc");
				d ("vraceno 26 Kc");
				break;
			case JIZDENKA_8:
				q = q0;
				c ();
				d ("vydavam za 8 Kc");
				d ("vraceno 24 Kc");
				break;
			case JIZDENKA_12:
				q = q0;
				c ();
				d ("vydavam za 12 Kc");
				d ("vraceno 20 Kc");
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

