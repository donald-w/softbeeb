# Softbeeb at a Glance
- BBC Model B emulator written in C (circa 1995), now built with CMake and C99. Entry point is `main.c`, which repeatedly fetches opcodes from `pc` (initially `0xD9CD`), dispatches via the `decode[256]` table in `decodes.c`, and generates periodic interrupts via `gen_irq()`.
- Memory map lives in `address.c`: 32K RAM (`RAM`), paged language ROM, and OS ROM loaded from `roms/basic.bin` and `roms/os.bin` (startup fails if these files are missing). `getbyte`/`putbyte` route accesses to RAM, ROM, or the Shelia I/O region and also mirror writes into the active screen handler.
- I/O plumbing is split between `io_read.c` (`rsheila` read handlers) and `io_write.c` (`wsheila` write handlers). These emulate the system VIA, video ULA control (`vid_con_reg`), palette registers, and ROM select (`romsel`). Keyboard state is funneled through `current_key/current_shift/current_control` and the VIA registers.
- Video: `screen.c` implements teletext text output (`text_screen_byte`) and pixel modes (`screen_byte_1/2/4`), mapping 1/2/4 bits-per-pixel into palette colours and wrapping screen addresses. `scroll.c` provides the `*_update` helpers used for vertical scrolling. All graphics functions ultimately call Turbo C BGI-style wrappers declared in `include/tc_graphics.h`.
- Sound: `sound.c` decodes the BBC sound chip bytes into per-channel frequency/volume (`freqbits[]`, `vol[]`) and computes a composite frequency passed to `sound()`/`nosound()` from `tc_dos.h`.
- Interrupts and keyboard: `irq.c` simulates 100 Hz and 50 Hz interrupts (tied to VIA `IFR/IER`) and converts PC scan codes from `_bios_keybrd` into BBC key codes. These BIOS calls, along with `peekb/pokeb`, live in `tc_bios.h`/`tc_dos.h` and are stubbed out in this tree.
- UI/utilities: `monitor.c` is the in-emulator menu (triggered by the HOME key) for quit/reset/sound toggling. `misc.c` performs startup (`system_init`), draws the title screen (`titlepic`), and offers debug helpers like `show_regs()`. `screen_byte_P` is set during `graphics_init` to the correct renderer for the current mode.
- Compatibility shims: `tc_*.c` files are placeholders for Turbo C DOS APIs (graphics, conio, BIOS, DOS, alloc). Nearly all functions are empty; a modern port must replace these with real implementations (e.g., SDL2 for video/input/audio).
- Assets and legacy bits: `data/title.bmp` and `data/logo.gif` are used for splash/logo art. The `turboc/` directory holds old project files and is not part of the CMake target. `changes.txt` documents historical tweaks.

## Building and Running
- Build: `cmake -S . -B build && cmake --build build` (C99, single `softbeeb` binary). No tests or install target are defined.
- Runtime prerequisites:
  - ROM images at `roms/os.bin` and `roms/basic.bin` (16 KB each). Without them `init_mem()` exits.
  - Implementations for the Turbo C APIs in `tc_graphics.c`, `tc_conio.c`, `tc_dos.c`, and `tc_bios.c`; the current stubs mean video, sound, and keyboard are non-functional.
  - The program writes `output.dat` when `show_regs()` is invoked; ensure the working directory is writable.
- Limitations: expects DOS-era hardware semantics (text/graphics modes, PC speaker, BIOS keyboard). Current state will compile but not emulate meaningfully until the shims are filled in.

## Key Behaviors and Gotchas
- CPU flags are split across globals (`result_f`, `ovr_f`, `carry_f`, `except`, etc.); many opcodes rely on `CLE` clearing the negative/zero pseudo-flag. Changes to flag handling ripple across decode implementations.
- `putbyte` mirrors screen writes via `screen_byte_P` when the address is within the active screen RAM window; ensure any new mode updates assign the correct handler and update `ram_screen_start`.
- Interrupt cadence is controlled by `CLKFREQ` in `main.c` (instructions between IRQ checks). Adjusting timing affects keyboard scanning and VIA flag behavior.
- `init_mem()` assumes little-endian host when reading ROM bytes and exits on first file error; wrap improvements with clear error messages and paths.
- Teletext vs pixel mode switches are driven by the `vid_ULA` write handler; mode flips reset the renderer and may trigger `flash()` palette cycling.
- Keyboard translation in `irq.c` is hard-coded scan-code mapping and synchronized with capslock via `peekb/pokeb`; changes should be tested against VIA input expectations.

## Recommended Starting Points for Agents
- Implement modern replacements for the Turbo C shims to get video/input/audio working (SDL/Allegro/ANSI console). Keep signatures intact or update call sites consistently.
- Add a small harness that loads ROMs from configurable paths and validates their size before launching the main loop.
- Introduce unit or integration tests around `getbyte/putbyte`, palette writes, and a subset of opcode implementations to guard against regressions while modernizing.
- Consider splitting rendering (text vs pixel) as hinted in `changes.txt` to simplify backend replacement.
