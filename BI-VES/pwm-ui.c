#include <p24fxxxx.h>

#define _ADDED_C_LIB 1
#include <stdio.h>
#include <string.h>

#include "libves/display.h"
#include "libves/delay.h"
#include "libves/led.h"
#include "libves/font.h"

#include "touch.h"

// Set the clock to 16MHz etc.
_CONFIG1 (JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF \
	 & FWDTEN_OFF & ICS_PGx2)
_CONFIG2 (0xF7FF & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_OFF \
	 & POSCMOD_HS & FNOSC_PRIPLL & PLLDIV_DIV3 & IOL1WAY_ON)

#undef RUNNING_ON_PIC

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

static int min (int a, int b) { return a < b ? a : b; }
static int max (int a, int b) { return a > b ? a : b; }

enum { PWMLED_R, PWMLED_G, PWMLED_B, PWMLED_BRI, PWMLED_COUNT };
static unsigned g_color_current;

#define PWMLED_STEP (1 << 8)

#ifndef RUNNING_ON_PIC
volatile unsigned OC1R, OC2R, OC3R;
#endif // ! RUNNING_ON_PIC

static void
pwmled_redraw (void)
{
	char x[20];
	disp_clear ();

	disp_at (2, 4);
	snprintf (x, sizeof x, " Red:   %3d%% ", (int) (OC1R / 4094. * 100));
	g_color_current == PWMLED_R ? disp_str_inverse (x) : disp_str (x);

	disp_at (3, 4);
	snprintf (x, sizeof x, " Green: %3d%% ", (int) (OC2R / 4094. * 100));
	g_color_current == PWMLED_G ? disp_str_inverse (x) : disp_str (x);

	disp_at (4, 4);
	snprintf (x, sizeof x, " Blue:  %3d%% ", (int) (OC3R / 4094. * 100));
	g_color_current == PWMLED_B ? disp_str_inverse (x) : disp_str (x);

	disp_at (6, 4);
	snprintf (x, sizeof x, "< Brightnes >");
	g_color_current == PWMLED_BRI ? disp_str_inverse (x) : disp_str (x);
}

static void
pwmled_init (void)
{
#ifdef RUNNING_ON_PIC
	OC1CON1 = OC2CON1 = OC3CON1 = 0;
	OC1CON2 = OC2CON2 = OC3CON2 = 0;
	OC1RS   = OC2RS   = OC3RS   = PR2 = 4095;
	OC1R    = OC2R    = OC3R    = 0;

	// Center-aligned PWM mode
	OC1CON1bits.OCM     = OC2CON1bits.OCM     = OC3CON1bits.OCM     = 0b111;
	// Synchronizace s Timer 2
	OC1CON2bits.SYNCSEL = OC2CON2bits.SYNCSEL = OC3CON2bits.SYNCSEL = 0b01100;

	RPOR5bits. RP10R = 18;  // OC1 na RP10 (R)
	RPOR8bits. RP17R = 18;  // OC1 na RP17 (R)
	RPOR9bits. RP19R = 19;  // OC2 na RP19 (G)
	RPOR13bits.RP27R = 19;  // OC2 na RP27 (G)
	RPOR10bits.RP21R = 20;  // OC3 na RP21 (B)
	RPOR13bits.RP26R = 20;  // OC3 na RP26 (B)

	T2CONbits.T32    = 0;   // 16bitovy rezim
	T2CONbits.TCKPS  = 0;   // Delici pomer 1:1
	T2CONbits.TCS    = 0;   // Interni hodiny
	T2CONbits.TGATE  = 0;   // Vypnout Gated Time Accumulation
	T2CONbits.TON    = 1;   // Zapnout casovac T2
#endif // RUNNING_ON_PIC

	pwmled_redraw ();
}

static void
pwmled_up (void)
{
	switch (g_color_current)
	{
	case PWMLED_R: OC1R = min (4094, OC1R + PWMLED_STEP); break;
	case PWMLED_G: OC2R = min (4094, OC2R + PWMLED_STEP); break;
	case PWMLED_B: OC3R = min (4094, OC3R + PWMLED_STEP); break;

	case PWMLED_BRI:
		OC1R = min (4094, (OC1R / 4094.) * 1.1 * 4094);
		OC2R = min (4094, (OC2R / 4094.) * 1.1 * 4094);
		OC3R = min (4094, (OC3R / 4094.) * 1.1 * 4094);
	}

	pwmled_redraw ();
}

static void
pwmled_down (void)
{
	switch (g_color_current)
	{
	case PWMLED_R: OC1R = max (0, (int) OC1R - PWMLED_STEP); break;
	case PWMLED_G: OC2R = max (0, (int) OC2R - PWMLED_STEP); break;
	case PWMLED_B: OC3R = max (0, (int) OC3R - PWMLED_STEP); break;

	case PWMLED_BRI:
		OC1R = min (4094, (OC1R / 4094.) * 0.9 * 4094);
		OC2R = min (4094, (OC2R / 4094.) * 0.9 * 4094);
		OC3R = min (4094, (OC3R / 4094.) * 0.9 * 4094);
	}

	pwmled_redraw ();
}

static void
pwmled_next (void)
{
	g_color_current = (g_color_current + 1) % PWMLED_COUNT;
	pwmled_redraw ();
}

static void
pwmled_prev (void)
{
	g_color_current = (g_color_current + PWMLED_COUNT - 1) % PWMLED_COUNT;
	pwmled_redraw ();
}

int
main ()
{
	disp_init ();
	touch_init ();
#ifdef RUNNING_ON_PIC
	led_init ();
#endif // RUNNING_ON_PIC

	pwmled_init ();

	while (1)
	switch (touch_readkey ())
	{
	case 2: pwmled_up   (); break;
	case 4: pwmled_down (); break;
	case 3: pwmled_next (); break;
	case 1: pwmled_prev (); break;
	}

	return 0;
}
