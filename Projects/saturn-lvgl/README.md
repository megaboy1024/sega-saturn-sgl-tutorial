# Saturn LVGL Widget Showcase (Pure SGL)

LVGL v9.2 running on the Sega Saturn using SGL directly — no Jo Engine dependency.

## What it does

- Renders LVGL UI to VDP2 NBG1 in bitmap mode (320x224, RGB555)
- D-pad moves cursor, A button clicks
- **Widget showcase**: cycles through 6 scenes demonstrating 19 LVGL widgets
- **RAM cart detection**: detects 1MB/4MB expansion cartridge if present
- L/R triggers: manual scene navigation
- Z button: pause/resume auto-advance
- X button: toggle performance/memory overlay

### Showcase scenes

| Scene | Widgets |
|-------|---------|
| **Controls** | button, checkbox, switch, slider |
| **Selectors** | dropdown, roller, spinbox, button matrix |
| **Text** | textarea + on-screen keyboard, span group, scrolling label |
| **Data** | bar (animated), LED, line, table |
| **Lists** | tabview with list items |
| **Containers** | window, tileview, message box |

Each scene runs for 8 seconds, then auto-advances (loops forever).

### Interactive features

- **Spinbox** (Scene 2): `-`/`+` buttons to increment/decrement
- **Number pad** (Scene 2): press buttons, result label shows which was pressed
- **Dropdown** (Scene 2): opens a styled list popup with dark background
- **Textarea** (Scene 3): focus to show on-screen keyboard, press OK or keyboard icon to close

### Enabled widgets (20)

label, button, bar, slider, switch, checkbox, led, line, table, tileview,
span, dropdown, roller, buttonmatrix, textarea, spinbox, keyboard, tabview,
list, msgbox, win, image (internal dependency for msgbox)

### Excluded widgets

| Widget | Reason |
|--------|--------|
| arc, spinner, scale | Require `LV_DRAW_SW_COMPLEX=1` (arc rendering) |
| chart, calendar | Too heavy for Saturn constraints |
| canvas, animimage, lottie | Require image/vector subsystems |
| menu | Hard-codes `lv_image_create()` for back-arrow icon |

### Performance overlay

- **Bottom-right**: FPS, CPU%, render/flush time (`LV_USE_PERF_MONITOR`)
- **Bottom-left**: Heap usage in KB, peak, fragmentation % (`LV_USE_MEM_MONITOR`)
- **X button**: toggles overlay visibility (uses `lv_obj_check_type` to hide only labels, preserving cursor)

## RAM expansion cartridge

The Saturn supports 1MB and 4MB RAM expansion cartridges. This project detects the cart at startup and displays its status on screen.

| Cart Type | ID at 0x24FFFFFF | Memory Range |
|-----------|-----------------|--------------|
| 4MB (32 Mbit) | 0x5c | 0x22400000 - 0x22600000 (2 MB) |
| 1MB (8 Mbit) | 0x5a | Two 512 KB zones at 0x22400000 and 0x22600000 |

To test with a 4MB cart in Mednafen, `Mednafen.bat` passes `-ss.cart extram4`.

## Prerequisites

This project requires the [joengine](https://github.com/johannes-fetz/joern) repository cloned as a sibling directory:

```
Documents/GitHub/
├── sega-saturn-sgl-tutorial/   (this repo)
└── joengine/                   (required for sh-elf-gcc 9.3 + ELF SGL libs)
```

The original `sh-coff-gcc` (GCC 4.0) in this repo's `Compiler/` is too old for LVGL v9.2 (no C99 support, missing `stdint.h`, can't handle UTF-8 BOM). The build uses joengine's `sh-elf-gcc` 9.3.0 instead.

## Toolchain details

| Component | Source | Why |
|-----------|--------|-----|
| Compiler (`sh-elf-gcc` 9.3) | `joengine/Compiler/WINDOWS/bin/` | GCC 4.0 can't compile LVGL v9.2 |
| SGL headers | `sega-saturn-sgl-tutorial/Compiler/SGL_302j/INC/` | Joengine's copy has broken nested comments in `SEGA_SYS.H` |
| SGL libraries (ELF) | `joengine/Compiler/COMMON/SGL_302j/LIB_ELF/` | This repo only has COFF-format libs |
| Startup (`SGLAREA.O`) | `joengine/Compiler/COMMON/SGL_302j/LIB_ELF/` | Provides work area + calls `main()` |
| Linker script (`sgl.linker`) | `joengine/Compiler/COMMON/` | `OUTPUT_FORMAT(coff-sh)` required for correct symbol naming |
| Tools (mkisofs, IP.BIN) | `sega-saturn-sgl-tutorial/Compiler/` | Native Windows builds (joengine's mkisofs is cygwin) |

## Build

1. Clone LVGL v9.2 into the project directory:
   ```
   cd Projects/saturn-lvgl
   git clone --branch release/v9.2 --depth 1 https://github.com/lvgl/lvgl.git lvgl
   ```

2. Build:
   ```
   compile.bat
   ```

3. Run in Mednafen (with 4MB RAM cart):
   ```
   Mednafen.bat
   ```

## Project structure

```
saturn-lvgl/
├── makefile              sh-elf-gcc build, links SGLAREA.O + SGL ELF libs + LVGL
├── compile.bat           Sets PATH for joengine compiler + SGL tools, runs make
├── clean.bat             Removes all build artifacts
├── Mednafen.bat          Launches emulator with sl_coff.cue + 4MB RAM cart
├── main.c                SGL init + LVGL init + 6 widget showcase scenes
├── saturn_ramcart.h      RAM expansion cartridge detection and initialization
├── lv_port_disp.c/h      VDP2 NBG1 bitmap flush, optimized RGB565 -> Saturn RGB555
├── lv_port_indev.c/h      Smpc_Peripheral D-pad + A button (active-low)
├── lv_port_tick.c/h       slIntFunction() vblank counter -> milliseconds
├── lv_conf.h             LVGL config: 128KB heap, RGB565, 19 widgets, sysmon
├── saturn_limits.h       Minimal limits.h for SH-2
├── libc_shims.c          memcpy/memset/strlen etc. (no libc linked)
├── lvgl_srcs_minimal.mk  LVGL source files (core + 19 widgets + sysmon)
├── common.h              SGL work area constants (unused — SGLAREA.O used instead)
├── ZTE/workarea.c        Custom work area (unused — SGLAREA.O used instead)
├── cd/ABS.TXT            ISO metadata
├── cd/BIB.TXT
├── cd/CPY.TXT
└── lvgl/                 (clone of LVGL v9.2 — not committed, see Build step 1)
```

## Flush callback optimization

The VDP2 flush (`lv_port_disp.c`) converts LVGL RGB565 to Saturn RGB555 per-pixel during the copy to VRAM. Key constraints and optimizations:

- **16-bit VRAM writes only** — VDP2 VRAM is on the B-bus (16-bit); 32-bit writes cause freezes
- **Row pointer increment** — `vrow += 512` avoids a `y * 512` multiply per row
- **Single-step green extraction** — `(px >> 1) & 0x03E0` converts G6 to G5 in one ALU op
- **Exact pixel stride** — source advances by `w` pixels per row (not `w >> 1` pairs), correct for any flush width including odd sub-regions

## Key differences from Jo Engine version

The [joengine saturn-lvgl sample](https://github.com/johannes-fetz/joern) (`joengine/Samples/saturn-lvgl/`) does the same thing using Jo Engine. This version replaces Jo Engine with direct SGL calls:

| Aspect | Jo Engine | Pure SGL |
|--------|-----------|----------|
| Entry point | `jo_main()` | `main()` |
| Init | `jo_core_init()` | `slInitSystem()` + `slBitMapNbg1()` + `slTVOn()` |
| Main loop | `jo_core_run()` + callbacks | `while(1) { lv_timer_handler(); slSynch(); }` |
| VBlank tick | `jo_core_add_vblank_callback()` | `slIntFunction()` |
| Input | `jo_is_pad1_key_pressed(JO_KEY_A)` | `!(Smpc_Peripheral[0].data & PER_DGT_TA)` |
| Display stride | `JO_VDP2_WIDTH` | `512` (hardcoded) |
| Resolution | 320x240 | 320x224 |
| Compiler | `sh-elf-gcc` (via jo_engine_makefile) | `sh-elf-gcc` (standalone makefile) |

## Saturn constraints

- **CPU**: SH-2 @ 28.6 MHz, no FPU
- **LVGL heap**: 128 KB (`LV_MEM_SIZE`)
- **Display**: 320x224, VDP2 NBG1 bitmap, 24-line double-buffered partial rendering
- **Drawing**: `LV_DRAW_SW_COMPLEX=0` (no rounded corners, shadows, or arcs)
- **Widgets**: 20 of 36 LVGL widgets enabled (flat-rendering compatible only)
- **Tick resolution**: ~16.67 ms (vblank-based, NTSC 60 Hz)
