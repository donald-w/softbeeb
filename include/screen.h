#pragma once

// Video Output Control Functions / Data

extern void (*screen_byte_P)(ubyte, bbcuint);

extern void text_screen_byte(ubyte, bbcuint);

extern void teletext_init(void);

extern void pixel_vid_init(void);

extern void update_cursor(void);

extern void set_colour_bits(void);

extern void set_screen_start(void);

extern void (*update_screen)(void);

extern void flash(void);

extern bbcuint ram_screen_start;
extern ubyte teletext;
extern ubyte disp_chars;
extern bbcuint screen_start;
extern ubyte vidpal[16];

extern void mono_update(void);

extern void four_update(void);

extern void sixt_update(void);

extern ubyte pic_temp_ram[];