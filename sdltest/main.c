#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tc_graphics.h"
#include "tc_conio.h"
#include "tc_dos.h"
#include "tc_bios.h"

// Helper exported by the shim to let us exit on SDL_QUIT.
int sdl_quit_requested(void);

static void draw_demo(void) {
    // Simple gradient and a moving box.
    for (int y = 0; y < 200; y++) {
        for (int x = 0; x < 320; x++) {
            int color = (x / 20 + y / 20) % 16;
            putpixel((uint16_t) (x * 2), (uint16_t) (y * 2), color);
            putpixel((uint16_t) (x * 2 + 1), (uint16_t) (y * 2), color);
            putpixel((uint16_t) (x * 2), (uint16_t) (y * 2 + 1), color);
            putpixel((uint16_t) (x * 2 + 1), (uint16_t) (y * 2 + 1), color);
        }
    }
    setvisualpage(0);
}

int main(void) {
    int driver = VGA;
    int mode = VGAMED;

    initgraph(&driver, &mode, "");
    setgraphmode(VGAMED);
    cleardevice();
    draw_demo();

    printf("sdltest: press 's' to play tone, 'x' to stop, 'q' to quit.\n");

    int box_color = EGA_LIGHTRED;
    int pos = 0;
    uint16_t last_tick = (uint16_t) clock();

    while (!sdl_quit_requested()) {
        if (kbhit()) {
            int c = coniogetch();
            if (c == 'q') break;
            if (c == 's') sound(440.0);
            if (c == 'x') nosound();
        }

        // Animate a small square.
        uint16_t now = (uint16_t) clock();
        if (now - last_tick > CLOCKS_PER_SEC / 60) {
            last_tick = now;
            for (int y = 220; y < 240; y++) {
                for (int x = 0; x < 40; x++) {
                    putpixel((uint16_t) ((pos + x) % 640), (uint16_t) y, box_color);
                }
            }
            pos = (pos + 4) % 640;
            setvisualpage(0);
        }
        delay(1);
    }

    nosound();
    closegraph();
    return 0;
}
