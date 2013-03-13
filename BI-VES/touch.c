#define  GDK_VERSION_MIN_REQUIRED  GDK_VERSION_3_0
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_26

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libves.h"
#include "touch.h"

// ----- libves ---------------------------------------------------------------

gboolean g_pressed[5];
GMutex g_pressed_mutex;
GCond g_pressed_cond;

static void
key_handler (GdkEventKey *e)
{
	g_mutex_lock (&g_pressed_mutex);

	switch (e->keyval)
	{
	case GDK_KEY_Up:
		g_pressed[0] = e->type == GDK_KEY_PRESS;
		break;
	case GDK_KEY_Right:
		g_pressed[1] = e->type == GDK_KEY_PRESS;
		break;
	case GDK_KEY_Down:
		g_pressed[2] = e->type == GDK_KEY_PRESS;
		break;
	case GDK_KEY_Left:
		g_pressed[3] = e->type == GDK_KEY_PRESS;
		break;
	case GDK_KEY_Return:
		g_pressed[4] = e->type == GDK_KEY_PRESS;
		break;
	default:
		break;
	}

	g_cond_signal (&g_pressed_cond);
	g_mutex_unlock (&g_pressed_mutex);
}

// ----- Application API ------------------------------------------------------

#define LOCK_API(r, n, p, pp)               \
	r n (p pp)                              \
	{                                       \
		g_mutex_lock (&g_pressed_mutex);    \
		r x = n##_unlocked (pp);            \
		g_mutex_unlock (&g_pressed_mutex);  \
		return x;                           \
	}

void
touch_init ()
{
	libves_register_key_handler (key_handler);
}

static unsigned
touch_sample_unlocked (int ch)
{
	g_return_val_if_fail (ch >= 8 && ch <= 12, 0);
	return g_pressed[ch - 8] ? 500 : 1000;
}

LOCK_API (unsigned, touch_sample, int, ch)

static unsigned
touch_pressed_unlocked (int ch)
{
	return touch_sample_unlocked (ch) < 750;
}

LOCK_API (unsigned, touch_pressed, int, ch)

static int
touch_getkey_unlocked (void)
{
	if (touch_pressed_unlocked (8))  return 1;
	if (touch_pressed_unlocked (9))  return 2;
	if (touch_pressed_unlocked (10)) return 3;
	if (touch_pressed_unlocked (11)) return 4;
	if (touch_pressed_unlocked (12)) return 5;
	return 0;
}

LOCK_API (int, touch_getkey, void, /* no arg */)

int
touch_readkey (void)
{
	g_mutex_lock (&g_pressed_mutex);
	int key = touch_getkey_unlocked ();
	while (key == 0)
	{
		g_cond_wait (&g_pressed_cond, &g_pressed_mutex);
		key = touch_getkey_unlocked ();
	}

	int tmp = touch_getkey_unlocked ();
	while (tmp != 0)
	{
		g_cond_wait (&g_pressed_cond, &g_pressed_mutex);
		tmp = touch_getkey_unlocked ();
	}
	g_mutex_unlock (&g_pressed_mutex);

	return key;
}

