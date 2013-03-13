#define  GDK_VERSION_MIN_REQUIRED  GDK_VERSION_3_0
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_26

#include <gtk/gtk.h>
#include <math.h>

#include <libves/cpu.h>
#include <libves/delay.h>
#include <libves/display.h>
#include <libves/font.h>
#include <libves/generator.h>
#include <libves/incrsensor.h>
#include <libves/led.h>
#include <libves/pl2303.h>

#include "libves.h"

/* Program for the controller. */
extern int __main (void);

// ----- Support --------------------------------------------------------------

static GtkWidget *g_wm, *g_da;

static cairo_surface_t *g_surface;
static cairo_t *g_ct;

static int g_start_line = 0;
static int g_contrast = 0xff;
static int g_flipped = 0;
static int g_inverse = 0;

static gboolean
on_key_press (GtkWidget *w, GdkEventKey *e, GCallback cb)
{
	g_return_val_if_fail (cb != 0, FALSE);
	((void (*) (GdkEventKey *)) cb) (e);
	return FALSE;
}

static gboolean
on_key_release (GtkWidget *w, GdkEventKey *e, GCallback cb)
{
	g_return_val_if_fail (cb != 0, FALSE);
	((void (*) (GdkEventKey *)) cb) (e);
	return FALSE;
}

void
libves_register_key_handler (void (*handler) (GdkEventKey *))
{
	g_signal_connect (g_wm, "key-press-event",
		G_CALLBACK (on_key_press), (gpointer) handler);
	g_signal_connect (g_wm, "key-release-event",
		G_CALLBACK (on_key_release), (gpointer) handler);
}

static void
window_on_destroyed (GObject *object, gpointer user_data)
{
	gtk_main_quit ();
}

static gboolean
draw_callback (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	guint width, height;

	width = gtk_widget_get_allocated_width (widget);
	height = gtk_widget_get_allocated_height (widget);

	if (!g_flipped)
	{
		cairo_translate (cr, 0, height);
		cairo_scale (cr, 1, -1);
	}

	if (g_start_line == 0)
	{
		cairo_set_source_surface (cr, g_surface, 0, 0);
		cairo_rectangle (cr, 0, 0, width, height);
		cairo_fill (cr);
	}
	else
	{
		cairo_set_source_surface (cr, g_surface, 0, -g_start_line);
		cairo_rectangle (cr, 0, 0, width, height);
		cairo_fill (cr);

		cairo_set_source_surface (cr, g_surface,
			0, SCREEN_HEIGHT - g_start_line);
		cairo_rectangle (cr, 0, 0, width, height);
		cairo_fill (cr);
	}

	if (g_inverse)
	{
		cairo_set_operator (cr, CAIRO_OPERATOR_DIFFERENCE);
		cairo_set_source_rgb (cr, 1, 1, 1);
		cairo_paint (cr);
	}

	if (g_contrast != 255)
	{
		double c = 0.7 + 0.3 * g_contrast / 255.;
		cairo_set_operator (cr, CAIRO_OPERATOR_MULTIPLY);
		cairo_set_source_rgb (cr, c, c, c);
		cairo_paint (cr);
	}

	return FALSE;
}

static void
refresh (void)
{
	gdk_threads_enter ();
	gtk_widget_queue_draw (g_da);
	gdk_threads_leave ();
}

static void
put_pixel_internal (int x, int y, int color)
{
	if (x < 0 || y < 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
		return;

	cairo_surface_flush (g_surface);
	*(guint32 *) (cairo_image_surface_get_data (g_surface)
		+ cairo_image_surface_get_stride (g_surface) * y + 4 * x)
		= color * 0xffffff;
	cairo_surface_mark_dirty (g_surface);
}

static gpointer
painter (gpointer data)
{
	return GINT_TO_POINTER (__main ());
}

int
main (int argc, char *argv[])
{
	GThread *th;
	GModule *module;

	// Default values for configuration registers.
	int c1 = 0x7fff, c2 = 0xf7ff, c3 = 0xffff;

	module = g_module_open (NULL, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
	if (!module)
		fprintf (stderr, "Failed to open self for reading "
			"out _CONFIG[123] values.\n");
	else
	{
		// Procedures for reading out configuration values.
		int (*pc1) (void), (*pc2) (void), (*pc3) (void);

		if (g_module_symbol (module, "_return_CONFIG1", (gpointer) &pc1))
			fprintf (stderr, "_CONFIG1 set to 0x%04x\n", c1 = pc1 ());
		if (g_module_symbol (module, "_return_CONFIG2", (gpointer) &pc2))
			fprintf (stderr, "_CONFIG2 set to 0x%04x\n", c2 = pc2 ());
		if (g_module_symbol (module, "_return_CONFIG3", (gpointer) &pc3))
			fprintf (stderr, "_CONFIG3 set to 0x%04x\n", c3 = pc3 ());
		g_module_close (module);
	}

	// TODO: Do something with the values.
	(void) c1;
	(void) c2;
	(void) c3;

	g_thread_init (NULL);
	gdk_threads_init ();
	gtk_init (&argc, &argv);

	g_wm = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (g_wm, "destroy", G_CALLBACK (window_on_destroyed), NULL);

	gtk_widget_add_events (g_wm, GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

	g_surface = cairo_image_surface_create
		(CAIRO_FORMAT_RGB24, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_ct = cairo_create (g_surface);

	/* Create a drawing area. */
	g_da = gtk_drawing_area_new ();
	gtk_widget_set_size_request (GTK_WIDGET (g_da),
		SCREEN_WIDTH, SCREEN_HEIGHT);
	g_signal_connect (G_OBJECT (g_da), "draw",
		G_CALLBACK (draw_callback), NULL);

	gtk_window_set_position (GTK_WINDOW (g_wm), GTK_WIN_POS_CENTER);
	gtk_container_add (GTK_CONTAINER (g_wm), g_da);
	gtk_widget_show_all (g_wm);

	/* Create a thread that draws onto the canvas. */
	fprintf (stderr, "Running the main program in a separate thread...\n");
	th = g_thread_new ("painter", painter, NULL);

	fprintf (stderr, "Running the event loop...\n");
	gtk_main ();

	fprintf (stderr, "Main window closed, "
		"waiting for the main program to end...\n");
	return GPOINTER_TO_INT (g_thread_join (th));
}

// ----- Delay ----------------------------------------------------------------

void delay_loop_ms    (int ms) { g_usleep (1000 * ms); }
void delay_loop_100us (void)   { g_usleep (100); }
void delay_loop_10us  (void)   { g_usleep (10); }
void delay_loop_1us   (void)   { g_usleep (1); }

// ----- Display --------------------------------------------------------------

void
disp_init (void)
{
	static int initialized = 0;

	if (initialized)  return;
	initialized = 1;

	// Decompiled from the original libves.
	sh1101_reset ();
	sh1101_reset ();

	sh1101_write (COMMAND, SET_DISPLAY_OFF);
	sh1101_write (COMMAND, SET_VCOM_DESEL_LEVEL);

	sh1101_write (COMMAND, SET_DISPLAY_OFF);
	sh1101_write (COMMAND, SET_VCOM_DESEL_LEVEL);
	sh1101_write (COMMAND, 0x23);
	sh1101_write (COMMAND, SET_DIS_PRE_CHARGE_PERIOD);
	sh1101_write (COMMAND, 0x22);
	sh1101_write (COMMAND, SET_SEGMENT_REMAP_LEFT);
	sh1101_write (COMMAND, SET_COMMON_OUTPUT_SCAN_DIR_FLIPPED);
	sh1101_write (COMMAND, SET_COMMON_PADS_HW_CONFIG);
	sh1101_write (COMMAND, 0x12);
	sh1101_write (COMMAND, SET_MULTIPLEX_RATIO);
	sh1101_write (COMMAND, 0x3f);
	sh1101_write (COMMAND, SET_DISPLAY_CLOCK_DIVIDE_RATIO);
	sh1101_write (COMMAND, 0xa0);
	sh1101_write (COMMAND, SET_CONTRAST_CONTROL_REGISTER);
	sh1101_write (COMMAND, 0x60);
	sh1101_write (COMMAND, SET_DISPLAY_OFFSET);
	sh1101_write (COMMAND, 0x00);
	sh1101_write (COMMAND, SET_NORMAL_DISPLAY);
	sh1101_write (COMMAND, SET_DC_DC);
	sh1101_write (COMMAND, 0x8b);
	sh1101_write (COMMAND, SET_DISPLAY_ON);
	delay_loop_ms (200);
	sh1101_write (COMMAND, SET_ENTIRE_DISP_ON);
	sh1101_write (COMMAND, SET_DISPLAY_START_LINE);
	sh1101_write (COMMAND, SET_LOWER_COLUMN_ADDRESS | 0x02);
	sh1101_write (COMMAND, SET_HIGHER_COLUMN_ADDRESS);
	sh1101_write (COMMAND, SET_ENTIRE_DISP_NORMAL);

	disp_clear ();
	cairo_set_line_width (g_ct, 1);
}

void
disp_clear (void)
{
	cairo_set_source_rgb (g_ct, 0, 0, 0);
	cairo_paint (g_ct);
	refresh ();

	disp_home ();
}

// ----- Font -----------------------------------------------------------------

// Derived from http://leonbrooks.blogspot.com
// /2012/07/5x7-text-dot-matrix-in-five-minutes.html
static unsigned char g_font[][6] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //   0x20  32
	{ 0x00, 0x00, 0xf6, 0x00, 0x00, 0x00 }, // ! 0x21  33
	{ 0x00, 0xe0, 0x00, 0xe0, 0x00, 0x00 }, // " 0x22  34
	{ 0x28, 0xfe, 0x28, 0xfe, 0x28, 0x00 }, // # 0x23  35
	{ 0x74, 0x54, 0xff, 0x54, 0x5c, 0x00 }, // $ 0x24  36
	{ 0xc4, 0xc8, 0x10, 0x26, 0x46, 0x00 }, // % 0x25  37
	{ 0x6c, 0x92, 0x6a, 0x04, 0x0a, 0x00 }, // & 0x26  38
	{ 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00 }, // ' 0x27  39
	{ 0x00, 0x38, 0x44, 0x82, 0x00, 0x00 }, // ( 0x28  40
	{ 0x00, 0x82, 0x44, 0x38, 0x00, 0x00 }, // ) 0x29  41
	{ 0x28, 0x10, 0x7c, 0x10, 0x28, 0x00 }, // * 0x2a  42
	{ 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00 }, // + 0x2b  43
	{ 0x00, 0x0a, 0x0c, 0x00, 0x00, 0x00 }, // , 0x2c  44
	{ 0x10, 0x10, 0x10, 0x10, 0x10, 0x00 }, // - 0x2d  45
	{ 0x00, 0x06, 0x06, 0x00, 0x00, 0x00 }, // . 0x2e  46
	{ 0x04, 0x08, 0x10, 0x20, 0x40, 0x00 }, // / 0x2f  47
	{ 0x7c, 0x8a, 0x92, 0xa2, 0x7c, 0x00 }, // 0 0x30  48
	{ 0x00, 0x42, 0xfe, 0x02, 0x00, 0x00 }, // 1 0x31  49
	{ 0x42, 0x86, 0x8a, 0x92, 0x62, 0x00 }, // 2 0x32  50
	{ 0x84, 0x82, 0xa2, 0xd2, 0x8c, 0x00 }, // 3 0x33  51
	{ 0x18, 0x28, 0x48, 0xfe, 0x08, 0x00 }, // 4 0x34  52
	{ 0xe4, 0xa2, 0xa2, 0xa2, 0x9c, 0x00 }, // 5 0x35  53
	{ 0x3c, 0x52, 0x92, 0x92, 0x0c, 0x00 }, // 6 0x36  54
	{ 0x80, 0x8e, 0x90, 0xa0, 0xc0, 0x00 }, // 7 0x37  55
	{ 0x6c, 0x92, 0x92, 0x92, 0x6c, 0x00 }, // 8 0x38  56
	{ 0x60, 0x92, 0x92, 0x94, 0x78, 0x00 }, // 9 0x39  57
	{ 0x00, 0x6c, 0x6c, 0x00, 0x00, 0x00 }, // : 0x3a  58
	{ 0x00, 0x6a, 0x6c, 0x00, 0x00, 0x00 }, // ; 0x3b  59
	{ 0x10, 0x28, 0x44, 0x82, 0x00, 0x00 }, // < 0x3c  60
	{ 0x28, 0x28, 0x28, 0x28, 0x28, 0x00 }, // = 0x3d  61
	{ 0x00, 0x82, 0x44, 0x28, 0x10, 0x00 }, // > 0x3e  62
	{ 0x40, 0x80, 0x8a, 0x90, 0x60, 0x00 }, // ? 0x3f  63
	{ 0x7c, 0x82, 0xba, 0x92, 0x72, 0x00 }, // @ 0x40  64
	{ 0x7e, 0x90, 0x90, 0x90, 0x7e, 0x00 }, // A 0x41  65
	{ 0xfe, 0x92, 0x92, 0x92, 0x6c, 0x00 }, // B 0x42  66
	{ 0x7c, 0x82, 0x82, 0x82, 0x44, 0x00 }, // C 0x43  67
	{ 0xfe, 0x82, 0x82, 0x82, 0x7c, 0x00 }, // D 0x44  68
	{ 0xfe, 0x92, 0x92, 0x92, 0x82, 0x00 }, // E 0x45  69
	{ 0xfe, 0x90, 0x90, 0x90, 0x80, 0x00 }, // F 0x46  70
	{ 0x7c, 0x82, 0x92, 0x92, 0x5e, 0x00 }, // G 0x47  71
	{ 0xfe, 0x10, 0x10, 0x10, 0xfe, 0x00 }, // H 0x48  72
	{ 0x00, 0x82, 0xfe, 0x82, 0x00, 0x00 }, // I 0x49  73
	{ 0x04, 0x02, 0x82, 0xfc, 0x80, 0x00 }, // J 0x4a  74
	{ 0xfe, 0x10, 0x28, 0x44, 0x82, 0x00 }, // K 0x4b  75
	{ 0xfe, 0x02, 0x02, 0x02, 0x02, 0x00 }, // L 0x4c  76
	{ 0xfe, 0x40, 0x30, 0x40, 0xfe, 0x00 }, // M 0x4d  77
	{ 0xfe, 0x20, 0x10, 0x08, 0xfe, 0x00 }, // N 0x4e  78
	{ 0x7c, 0x82, 0x82, 0x82, 0x7c, 0x00 }, // O 0x4f  79
	{ 0xfe, 0x90, 0x90, 0x90, 0x60, 0x00 }, // P 0x50  80
	{ 0x7c, 0x82, 0x8a, 0x84, 0x7a, 0x00 }, // Q 0x51  81
	{ 0xfe, 0x90, 0x98, 0x94, 0x62, 0x00 }, // R 0x52  82
	{ 0x62, 0x92, 0x92, 0x92, 0x8c, 0x00 }, // S 0x53  83
	{ 0x80, 0x80, 0xfe, 0x80, 0x80, 0x00 }, // T 0x54  84
	{ 0xfc, 0x02, 0x02, 0x02, 0xfc, 0x00 }, // U 0x55  85
	{ 0xf0, 0x0c, 0x02, 0x0c, 0xf0, 0x00 }, // V 0x56  86
	{ 0xfc, 0x02, 0x0c, 0x02, 0xfc, 0x00 }, // W 0x57  87
	{ 0xc6, 0x28, 0x10, 0x28, 0xc6, 0x00 }, // X 0x58  88
	{ 0xe0, 0x10, 0x0e, 0x10, 0xe0, 0x00 }, // Y 0x59  89
	{ 0x86, 0x8a, 0x92, 0xa2, 0xc2, 0x00 }, // Z 0x5a  90
	{ 0x00, 0x00, 0xfe, 0x82, 0x00, 0x00 }, // [ 0x5b  91
	{ 0x40, 0x20, 0x10, 0x08, 0x04, 0x00 }, // \ 0x5c  92
	{ 0x00, 0x82, 0xfe, 0x00, 0x00, 0x00 }, // ] 0x5d  93
	{ 0x20, 0x40, 0x80, 0x40, 0x20, 0x00 }, // ^ 0x5e  94
	{ 0x02, 0x02, 0x02, 0x02, 0x02, 0x00 }, // _ 0x5f  95
	{ 0x00, 0x00, 0xc0, 0x20, 0x00, 0x00 }, // ` 0x60  96
	{ 0x04, 0x2a, 0x2a, 0x2a, 0x1e, 0x00 }, // a 0x61  97
	{ 0xfe, 0x12, 0x22, 0x22, 0x1c, 0x00 }, // b 0x62  98
	{ 0x1c, 0x22, 0x22, 0x22, 0x22, 0x00 }, // c 0x63  99
	{ 0x1c, 0x22, 0x22, 0x12, 0xfe, 0x00 }, // d 0x64 100
	{ 0x1c, 0x2a, 0x2a, 0x2a, 0x18, 0x00 }, // e 0x65 101
	{ 0x10, 0x7e, 0x90, 0x80, 0x40, 0x00 }, // f 0x66 102
	{ 0x30, 0x4a, 0x4a, 0x4a, 0x7c, 0x00 }, // g 0x67 103
	{ 0xfe, 0x10, 0x20, 0x20, 0x1e, 0x00 }, // h 0x68 104
	{ 0x00, 0x22, 0xbe, 0x02, 0x00, 0x00 }, // i 0x69 105
	{ 0x04, 0x02, 0x22, 0xbc, 0x00, 0x00 }, // j 0x6a 106
	{ 0x00, 0xfe, 0x08, 0x14, 0x22, 0x00 }, // k 0x6b 107
	{ 0x00, 0x82, 0xfe, 0x02, 0x00, 0x00 }, // l 0x6c 108
	{ 0x3e, 0x20, 0x18, 0x20, 0x1e, 0x00 }, // m 0x6d 109
	{ 0x3e, 0x10, 0x20, 0x20, 0x1e, 0x00 }, // n 0x6e 110
	{ 0x1c, 0x22, 0x22, 0x22, 0x1c, 0x00 }, // o 0x6f 111
	{ 0x3e, 0x28, 0x28, 0x28, 0x10, 0x00 }, // p 0x70 112
	{ 0x10, 0x28, 0x28, 0x28, 0x3e, 0x00 }, // q 0x71 113
	{ 0x3e, 0x10, 0x20, 0x20, 0x10, 0x00 }, // r 0x72 114
	{ 0x12, 0x2a, 0x2a, 0x2a, 0x04, 0x00 }, // s 0x73 115
	{ 0x20, 0xfc, 0x22, 0x02, 0x04, 0x00 }, // t 0x74 116
	{ 0x3c, 0x02, 0x02, 0x04, 0x3e, 0x00 }, // u 0x75 117
	{ 0x38, 0x04, 0x02, 0x04, 0x38, 0x00 }, // v 0x76 118
	{ 0x3c, 0x02, 0x0c, 0x02, 0x3c, 0x00 }, // w 0x77 119
	{ 0x22, 0x14, 0x08, 0x14, 0x22, 0x00 }, // x 0x78 120
	{ 0x30, 0x0a, 0x0a, 0x0a, 0x3c, 0x00 }, // y 0x79 121
	{ 0x22, 0x26, 0x2a, 0x32, 0x22, 0x00 }, // z 0x7a 122
	{ 0x00, 0x10, 0x6c, 0x82, 0x82, 0x00 }, // { 0x7b 123
	{ 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00 }, // | 0x7c 124
	{ 0x82, 0x82, 0x6c, 0x10, 0x00, 0x00 }, // } 0x7d 125
	{ 0x10, 0x30, 0x10, 0x18, 0x10, 0x00 }, // ~ 0x7e 126
};

unsigned char *
get_font_data_ptr (char c)
{
	return c >= 32 && c <= 126 ? g_font[c - ' '] : g_font['?' - ' '];
}

// ----- SH1101 ---------------------------------------------------------------

static int g_page_address;              //!< Page address.
static int g_lo_col_address;            //!< Lower 8 bits of column address.
static int g_hi_col_address;            //!< Upper 8 bits of column address.

static int g_read_modify_write;         //!< We're in read/modify/write mode.
static int g_read_modify_write_lo_col;  //!< Lower bits of saved col. addr.
static int g_read_modify_write_hi_col;  //!< Upper bits of saved col. addr.

void
sh1101_reset (void)
{
	// TODO: Set the display to off.

	g_lo_col_address = 0;
	g_hi_col_address = 0;
	g_flipped        = 0;
	g_start_line     = 0;
	g_contrast       = 0x80;

	refresh ();
}

void
sh1101_write (int addr, unsigned char v)
{
	if (addr == COMMAND)
	{
		// FIXME: There are more two-byte commands
		//        than just setting the contrast.
		static int setting_contrast = 0;

		if (setting_contrast)
		{
			g_contrast = v;
			setting_contrast = 0;
			refresh ();
		}
		else switch (v & ~0x0f)
		{
		case SET_PAGE_ADDRESS:
			g_page_address   = v & 0xf;
			break;
		case SET_LOWER_COLUMN_ADDRESS:
			g_lo_col_address = v & 0xf;
			break;
		case SET_HIGHER_COLUMN_ADDRESS:
			g_hi_col_address = v & 0xf;
			break;
		case SET_COMMON_OUTPUT_SCAN_DIR_NORM:
			g_flipped        = v & 0x4;
			break;

		case SET_DISPLAY_START_LINE:
		case SET_DISPLAY_START_LINE | 0x10:
		case SET_DISPLAY_START_LINE | 0x20:
		case SET_DISPLAY_START_LINE | 0x30:
			g_start_line = v & 0x3f;
			refresh ();
			break;

		default:
			switch (v)
			{
			case SET_CONTRAST_CONTROL_REGISTER:
				setting_contrast = 1;
				break;
			case SET_NORMAL_DISPLAY:
				g_inverse = 0;
				refresh ();
				break;
			case SET_REVERSE_DISPLAY:
				g_inverse = 1;
				refresh ();
				break;
			case SET_READ_MODIFY_WRITE_MODE:
				g_read_modify_write_lo_col = g_lo_col_address;
				g_read_modify_write_hi_col = g_hi_col_address;
				g_read_modify_write = 1;
				break;
			case END:
				g_lo_col_address = g_read_modify_write_lo_col;
				g_hi_col_address = g_read_modify_write_hi_col;
				g_read_modify_write = 0;
				break;
			}
		}
	}
	else if (addr == DATA)
	{
		int i, col = (g_lo_col_address + (g_hi_col_address << 4)) - 2,
			   row = SCREEN_HEIGHT - 1 - g_page_address * 8;
		for (i = 0; i < 8; i++)
			put_pixel_internal (col, row - i , (v >> i) & 1);
		refresh ();

		if (++g_lo_col_address == 0x10)
		{
			g_lo_col_address = 0;
			g_hi_col_address++;
		}
	}
}

unsigned char
sh1101_read (int addr)
{
	if (addr == COMMAND)
	{
		// TODO: Report display on/off
	}
	else if (addr == DATA)
	{
		int x = g_lo_col_address + (g_hi_col_address << 4) - 2;
		if (x < 0 || x >= SCREEN_WIDTH)
			return 0;

		cairo_surface_flush (g_surface);

		int y, y_base = SCREEN_HEIGHT - g_page_address * 8 - 1;
		unsigned char v = 0;
		for (y = 0; y < 8; y++)
			v = v << 1 | ((*(guint32 *)
				(cairo_image_surface_get_data (g_surface)
				+ cairo_image_surface_get_stride (g_surface) * (y_base + y)
				+ 4 * x)) != 0);

		if (!g_read_modify_write
		 && ++g_lo_col_address == 0x10)
		{
			g_lo_col_address = 0;
			g_hi_col_address++;
		}

		return v;
	}

	return 0;
}

// ----- Graphics -------------------------------------------------------------

int get_char_width  (char c) { return 6; }
int get_char_height (char c) { return 8; }

int
get_string_width (char *s)
{
	int w = 0, c;
	while ((c = *s++))
		w += get_char_width (c);
	return w;
}

int
get_pixel (int x, int y)
{
	if (x < 0 || y < 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
		return 0;

	cairo_surface_flush (g_surface);

	return (*(guint32 *) (cairo_image_surface_get_data (g_surface)
			+ cairo_image_surface_get_stride (g_surface) * y + 4 * x)) != 0;
}

void
put_pixel (int x, int y, int color)
{
	put_pixel_internal (x, y, color);
	refresh ();
}

// TODO: draw_line(), draw_rect(), draw_ellipse()

void
draw_char (int x, int y, char c)
{
	unsigned char col, row, *bits;
	bits = get_font_data_ptr (c);

	for (col = 0; col < 6; col++)
		for (row = 0; row < 8; row++)
			put_pixel_internal (x + col, y + row,
				(bits[col] >> (7 - row)) & 1);
	refresh ();
}

// ----- Terminal -------------------------------------------------------------

static int g_row, g_col;

int disp_get_col () { return g_col; }
int disp_get_row () { return g_row; }

void
disp_home (void)
{
	disp_at (0, 0);
}

void
disp_at (int row, int col)
{
	g_row = row;
	g_col = col;
}

void
disp_nl (void)
{
	g_row++;
	g_col = 0;
}

void
disp_line (char *c)
{
	disp_str (c);
	disp_nl ();
}

void
disp_str (char *c)
{
	char x;

	while ((x = *c++))
	{
		disp_char (x);
		if (++g_col == TERM_WIDTH)
			disp_nl ();
	}
}

void
disp_char (char c)
{
	draw_char (g_col * 6, g_row * 8, c);
}
