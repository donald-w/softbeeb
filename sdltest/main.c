#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sdl_shim.h"

static void draw_demo(void) {
    // Simple gradient and a moving box.
    for (int y = 0; y < 200; y++) {
        for (int x = 0; x < 320; x++) {
            int color = (x / 20 + y / 20) % 16;
            sdl_putpixel((uint16_t) (x * 2), (uint16_t) (y * 2), color);
            sdl_putpixel((uint16_t) (x * 2 + 1), (uint16_t) (y * 2), color);
            sdl_putpixel((uint16_t) (x * 2), (uint16_t) (y * 2 + 1), color);
            sdl_putpixel((uint16_t) (x * 2 + 1), (uint16_t) (y * 2 + 1), color);
        }
    }
    sdl_setvisualpage(0);
}

int main(void) {
    int driver = VGA;
    int mode = VGAMED;

    sdl_initgraph(&driver, &mode, "");
    sdl_setgraphmode(VGAMED);
    sdl_cleardevice();
    draw_demo();

    printf("sdltest: press 's' to play tone, 'x' to stop, 'q' to quit.\n");

    int box_color = EGA_LIGHTRED;
    int pos = 0;
    uint16_t last_tick = (uint16_t) clock();

    while (!sdl_quit_requested()) {
        if (sdl_kbhit()) {
            int c = sdl_coniogetch();
            if (c == 'q') break;
            if (c == 's') sdl_sound(440.0);
            if (c == 'x') sdl_nosound();
        }

        // Animate a small square.
        uint16_t now = (uint16_t) clock();
        if (now - last_tick > CLOCKS_PER_SEC / 60) {
            last_tick = now;
            for (int y = 220; y < 240; y++) {
                for (int x = 0; x < 40; x++) {
                    sdl_putpixel((uint16_t) ((pos + x) % 640), (uint16_t) y, box_color);
                }
            }
            pos = (pos + 4) % 640;
            sdl_setvisualpage(0);
        }
        sdl_delay(1);
    }

    sdl_nosound();
    sdl_closegraph();
    return 0;
}
