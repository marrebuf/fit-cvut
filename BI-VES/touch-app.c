#include <p24fxxxx.h>

#define _ADDED_C_LIB 1
#include <stdio.h>
#include <string.h>

#include "libves/display.h"
#include "libves/delay.h"

#include "touch.h"

// Set the clock to 16MHz etc.
_CONFIG1 (JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF \
     & FWDTEN_OFF & ICS_PGx2)
_CONFIG2 (0xF7FF & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_OFF \
     & POSCMOD_HS & FNOSC_PRIPLL & PLLDIV_DIV3 & IOL1WAY_ON)

int
main ()
{
	disp_init ();
	touch_init ();

	int x = 0;
	while (1)
	{
		char *keys[] = { "", "JEDNA", "DVA", "TRI", "CTYRI", "PET" };
		int key = touch_readkey ();

		disp_clear ();
		disp_at (x, 1);
		disp_str (keys[key]);

		x = ((x + 1) & 7);
	}

	return 0;
}

