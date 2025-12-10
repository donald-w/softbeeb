#include <SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdl_shim.h"

// Basic SDL-backed shims to get keyboard, graphics, and sound working for tests.

static SDL_Window *g_window = NULL;
static SDL_Renderer *g_renderer = NULL;
static SDL_Texture *g_texture = NULL;
static uint32_t *g_framebuffer = NULL;
static int g_width = 640;
static int g_height = 512;
static ubyte g_palette[16];         // logical -> physical index
static uint32_t g_rgba[16];         // physical index -> ARGB8888
static int g_foreground = EGA_WHITE;
static int g_background = EGA_BLACK;
static SDL_AudioDeviceID g_audio = 0;
static double g_tone_freq = 0.0;
static double g_tone_phase = 0.0;
static int g_cursor_x = 0;
static int g_cursor_y = 0;
static int g_quit_requested = 0;
static int g_shift_down = 0;
static int g_ctrl_down = 0;

// Very small key queue to emulate sdl_kbhit/sdl_coniogetch/sdl_bios_keybrd.
#define KEY_QUEUE_SIZE 64
static int g_key_queue[KEY_QUEUE_SIZE];
static int g_key_head = 0;
static int g_key_tail = 0;

static void push_key(int code) {
    int next = (g_key_head + 1) % KEY_QUEUE_SIZE;
    if (next == g_key_tail) return; // drop on overflow
    g_key_queue[g_key_head] = code;
    g_key_head = next;
}

static int pop_key(void) {
    if (g_key_head == g_key_tail) return -1;
    int code = g_key_queue[g_key_tail];
    g_key_tail = (g_key_tail + 1) % KEY_QUEUE_SIZE;
    return code;
}

static int queue_size(void) {
    int size = g_key_head - g_key_tail;
    if (size < 0) size += KEY_QUEUE_SIZE;
    return size;
}

static uint32_t ega_color_to_rgba(int code) {
    // Simple 16-color palette (ARGB8888)
    static const uint32_t table[16] = {
            0xFF000000, // black
            0xFF00AA00, // green-ish
            0xFFAA00AA, // magenta
            0xFF0000AA, // blue
            0xFFAAAA00, // yellow-ish
            0xFF00AAAA, // cyan
            0xFFAA0000, // red
            0xFFAAAAAA, // white/grey
            0xFF555555, // light black
            0xFFFF5555, // light red
            0xFF55FF55, // light green
            0xFFFFFF55, // light yellow
            0xFF5555FF, // light blue
            0xFFFF55FF, // light magenta
            0xFF55FFFF, // light cyan
            0xFFFFFFFF  // bright white
    };
    if (code < 0) code = 0;
    if (code > 15) code = 15;
    return table[code];
}

static void update_texture(void) {
    if (!g_texture || !g_framebuffer) return;
    SDL_UpdateTexture(g_texture, NULL, g_framebuffer, g_width * sizeof(uint32_t));
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
    SDL_RenderPresent(g_renderer);
}

static void ensure_video(void) {
    if (g_window) return;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        exit(1);
    }

    g_window = SDL_CreateWindow("softbeeb SDL shim",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                g_width, g_height,
                                SDL_WINDOW_RESIZABLE);
    if (!g_window) {
        fprintf(stderr, "SDL window failed: %s\n", SDL_GetError());
        exit(1);
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888,
                                  SDL_TEXTUREACCESS_STREAMING, g_width, g_height);
    g_framebuffer = calloc((size_t) g_width * g_height, sizeof(uint32_t));

    for (int i = 0; i < 16; i++) {
        g_palette[i] = (ubyte) i;
        g_rgba[i] = ega_color_to_rgba(i);
    }
}

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    (void) userdata;
    memset(stream, 0, (size_t) len);
    if (g_tone_freq <= 0.0) return;

    int samples = len / sizeof(float);
    float *out = (float *) stream;
    double phase_inc = g_tone_freq / 48000.0;
    for (int i = 0; i < samples; i++) {
        float sample = (float) (sin(g_tone_phase * 2.0 * M_PI) * 0.2);
        g_tone_phase += phase_inc;
        if (g_tone_phase >= 1.0) g_tone_phase -= 1.0;
        out[i] = sample;
    }
}

static void ensure_audio(void) {
    if (g_audio) return;
    SDL_AudioSpec want = {0}, have = {0};
    want.freq = 48000;
    want.format = AUDIO_F32SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audio_callback;
    g_audio = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!g_audio) {
        fprintf(stderr, "SDL audio failed: %s\n", SDL_GetError());
        return;
    }
    SDL_PauseAudioDevice(g_audio, 0);
}

static void pump_events(void) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT:
                g_quit_requested = 1;
                break;
            case SDL_KEYDOWN:
                if (ev.key.keysym.sym == SDLK_LSHIFT || ev.key.keysym.sym == SDLK_RSHIFT) g_shift_down = 1;
                if (ev.key.keysym.sym == SDLK_LCTRL || ev.key.keysym.sym == SDLK_RCTRL) g_ctrl_down = 1;
                push_key((int) ev.key.keysym.sym);
                break;
            case SDL_KEYUP:
                if (ev.key.keysym.sym == SDLK_LSHIFT || ev.key.keysym.sym == SDLK_RSHIFT) g_shift_down = 0;
                if (ev.key.keysym.sym == SDLK_LCTRL || ev.key.keysym.sym == SDLK_RCTRL) g_ctrl_down = 0;
                break;
            default:
                break;
        }
    }
}

void sdl_getimage(int x1, int y1, int x2, int y2, ubyte ram[]) {
    ensure_video();
    if (!g_framebuffer) return;
    int w = x2 - x1 + 1;
    int h = y2 - y1 + 1;
    int idx = 0;
    for (int y = 0; y < h; y++) {
        int sy = y1 + y;
        if (sy < 0 || sy >= g_height) continue;
        for (int x = 0; x < w; x++) {
            int sx = x1 + x;
            if (sx < 0 || sx >= g_width) continue;
            uint32_t pixel = g_framebuffer[sy * g_width + sx];
            ram[idx++] = (ubyte) (pixel & 0xFF);
            ram[idx++] = (ubyte) ((pixel >> 8) & 0xFF);
            ram[idx++] = (ubyte) ((pixel >> 16) & 0xFF);
            ram[idx++] = (ubyte) ((pixel >> 24) & 0xFF);
        }
    }
}

void sdl_putimage(int x1, int y1, ubyte ram[], int mode) {
    (void) mode;
    ensure_video();
    if (!g_framebuffer) return;
    // Assumes buffer was filled by sdl_getimage: 4 bytes per pixel.
    int idx = 0;
    for (int y = y1; y < g_height && idx < g_width * g_height * 4; y++) {
        for (int x = x1; x < g_width && idx < g_width * g_height * 4; x++) {
            uint32_t rgba = (uint32_t) ram[idx] |
                            ((uint32_t) ram[idx + 1] << 8) |
                            ((uint32_t) ram[idx + 2] << 16) |
                            ((uint32_t) ram[idx + 3] << 24);
            g_framebuffer[y * g_width + x] = rgba;
            idx += 4;
        }
    }
    update_texture();
}

void sdl_putpixel(uint16_t x, uint16_t y, int color) {
    ensure_video();
    if (!g_framebuffer) return;
    if ((int) x < 0 || (int) y < 0 || x >= (uint) g_width || y >= (uint) g_height) return;
    int idx = (int) x + (int) y * g_width;
    ubyte logical = (ubyte) color;
    ubyte physical = g_palette[logical & 0x0F];
    g_framebuffer[idx] = g_rgba[physical & 0x0F];
    
    // Update screen after every few pixels to show drawing progress
    static int pixel_count = 0;
    if (++pixel_count >= 100) {
        update_texture();
        pixel_count = 0;
    }
}

void sdl_setgraphmode(int vgamed) {
    (void) vgamed;
    ensure_video();
    sdl_cleardevice();
}

void sdl_restorecrtmode() {
    // Nothing special; keep window open.
}

void sdl_clrscr() {
    sdl_cleardevice();
}

void sdl_textcolor(int c) {
    g_foreground = c & 0x0F;
}

void sdl_textbackground(int c) {
    g_background = c & 0x0F;
}

void sdl_textmode(int mode) {
    (void) mode;
}

int sdl_registerfarbgidriver(int farptr) {
    (void) farptr;
    return 0;
}

void sdl_setpalette(ubyte logical, ubyte physical) {
    if (logical > 15) return;
    g_palette[logical] = physical & 0x0F;
    g_rgba[logical] = ega_color_to_rgba(physical & 0x0F);
}

void sdl_setvisualpage(int page) {
    (void) page;
    update_texture();
}

void sdl_setactivepage(int page) {
    (void) page;
}

void sdl_cprintf(char *string) {
    printf("%s", string);
    fflush(stdout);
}

void sdl_closegraph() {
    if (g_audio) {
        SDL_CloseAudioDevice(g_audio);
        g_audio = 0;
    }
    free(g_framebuffer);
    g_framebuffer = NULL;
    if (g_texture) SDL_DestroyTexture(g_texture);
    if (g_renderer) SDL_DestroyRenderer(g_renderer);
    if (g_window) SDL_DestroyWindow(g_window);
    g_texture = NULL;
    g_renderer = NULL;
    g_window = NULL;
    SDL_Quit();
}

void sdl_initgraph(int *pInt, int *pInt1, char *string) {
    (void) pInt;
    (void) pInt1;
    (void) string;
    ensure_video();
    ensure_audio();
    sdl_cleardevice();
}

void sdl_setrgbpalette(int idx, int r, int g, int b) {
    if (idx < 0 || idx > 15) return;
    uint32_t rr = (uint32_t) (r & 0x3F) * 4;
    uint32_t gg = (uint32_t) (g & 0x3F) * 4;
    uint32_t bb = (uint32_t) (b & 0x3F) * 4;
    g_rgba[idx] = 0xFF000000 | (rr << 16) | (gg << 8) | bb;
}

bbcuint sdl_getcolor() {
    return (bbcuint) g_foreground;
}

void sdl_setbkcolor(int c) {
    g_background = c & 0x0F;
}

void sdl_cleardevice() {
    ensure_video();
    if (!g_framebuffer) return;
    ubyte physical = g_palette[g_background & 0x0F];
    uint32_t fill = g_rgba[physical & 0x0F];
    for (int i = 0; i < g_width * g_height; i++) {
        g_framebuffer[i] = fill;
    }
    update_texture();
}

void sdl_settextstyle(int font, int dir, int size) {
    (void) font;
    (void) dir;
    (void) size;
}

void sdl_setcolor(int c) {
    g_foreground = c & 0x0F;
}

void sdl_outtextxy(int x, int y, char *string) {
    (void) x;
    (void) y;
    sdl_cprintf(string);
    sdl_cprintf("\n");
}

// ---------- conio ----------

void sdl_gotoxy(uint16_t x, uint16_t y) {
    g_cursor_x = (int) x;
    g_cursor_y = (int) y;
}

void sdl_putch(ubyte ch) {
    // Basic cursor-less console output.
    (void) g_cursor_x;
    (void) g_cursor_y;
    fputc(ch, stdout);
    fflush(stdout);
}

int sdl_coniogetch() {
    pump_events();
    return pop_key();
}

int sdl_kbhit(void) {
    pump_events();
    return queue_size() > 0;
}

void sdl_setcursortype(int cursortype) {
    (void) cursortype;
}

// ---------- BIOS / DOS ----------

ubyte sdl_bios_keybrd(int shiftstatus) {
    pump_events();
    switch (shiftstatus) {
        case _KEYBRD_SHIFTSTATUS:
            return (ubyte) ((g_shift_down ? 1 : 0) | (g_ctrl_down ? 4 : 0));
        case _KEYBRD_READY:
            return (ubyte) (queue_size() > 0 ? 1 : 0);
        case _KEYBRD_READ:
        default:
            return (ubyte) (pop_key() & 0xFF);
    }
}

void sdl_nosound() {
    ensure_audio();
    g_tone_freq = 0.0;
}

void sdl_sound(double totfreq) {
    ensure_audio();
    g_tone_freq = totfreq;
}

void sdl_delay(int ms) {
    SDL_Delay((Uint32) ms);
}

char sdl_peekb(int segment, int offset) {
    (void) segment;
    (void) offset;
    return 0;
}

void sdl_pokeb(int segment, int offset, char value) {
    (void) segment;
    (void) offset;
    (void) value;
}

int sdl_quit_requested(void) {
    pump_events();
    return g_quit_requested;
}
