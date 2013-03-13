#ifndef __TOUCH_H
#define __TOUCH_H

void touch_init (void);
unsigned touch_sample (int ch);
unsigned touch_pressed (int channel);
int touch_getkey (void);
int touch_readkey (void);

#endif

