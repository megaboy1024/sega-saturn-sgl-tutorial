# Saturn LVGL (Pure SGL)

LVGL v9.2 running on the Sega Saturn using SGL directly — no Jo Engine dependency.

## What it does

- Renders LVGL UI to VDP2 NBG1 in bitmap mode (320x224, RGB555)
- D-pad moves a red 8x8 cursor, A button clicks
- **Benchmark mode**: cycles through 5 test scenes with real-time performance monitoring

### Benchmark scenes

| Scene | What it tests |
|-------|--------------|
| **Fill** | 16 color-cycling rectangles in a 4x4 grid |
| **Text** | 8 labels rendered simultaneously |
| **Bars** | 6 progress bars with staggered ping-pong animation |
| **Churn** | Rapid object create/delete every 250ms (memory stress) |
| **Mixed** | 4 animated rects + 4 labels + 3 animated bars |

Each scene runs for 6 seconds, then auto-advances (loops forever).

### Performance overlay

- **Bottom-right**: FPS, CPU%, render/flush time (`LV_USE_PERF_MONITOR`)
- **Bottom-left**: Heap usage in KB, peak, fragmentation % (`LV_USE_MEM_MONITOR`)

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

3. Run in Mednafen:
   ```
   Mednafen.bat
   ```

## Project structure

```
saturn-lvgl/
├── makefile              sh-elf-gcc build, links SGLAREA.O + SGL ELF libs + LVGL
├── compile.bat           Sets PATH for joengine compiler + SGL tools, runs make
├── clean.bat             Removes all build artifacts
├── Mednafen.bat          Launches emulator with sl_coff.cue
├── main.c                SGL init + LVGL init + benchmark (5 scenes + sysmon)
├── lv_port_disp.c/h      VDP2 NBG1 bitmap flush, optimized RGB565 -> Saturn RGB555
├── lv_port_indev.c/h      Smpc_Peripheral D-pad + A button (active-low)
├── lv_port_tick.c/h       slIntFunction() vblank counter -> milliseconds
├── lv_conf.h             LVGL config: 48KB heap, RGB565, no FPU, big-endian, sysmon
├── saturn_limits.h       Minimal limits.h for SH-2
├── libc_shims.c          memcpy/memset/strlen etc. (no libc linked)
├── lvgl_srcs_minimal.mk  126 LVGL source files (core + label + button + bar + sysmon)
├── common.h              SGL work area constants (unused — SGLAREA.O used instead)
├── ZTE/workarea.c        Custom work area (unused — SGLAREA.O used instead)
├── cd/ABS.TXT            ISO metadata
├── cd/BIB.TXT
├── cd/CPY.TXT
└── lvgl/                 (clone of LVGL v9.2 — not committed, see Build step 1)
```

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

## Flush callback optimization

The VDP2 flush (`lv_port_disp.c`) converts LVGL RGB565 to Saturn RGB555 per-pixel during the copy to VRAM. Key constraints and optimizations:

- **16-bit VRAM writes only** — VDP2 VRAM is on the B-bus (16-bit); 32-bit writes cause freezes
- **Row pointer increment** — `vrow += 512` avoids a `y * 512` multiply per row
- **Single-step green extraction** — `(px >> 1) & 0x03E0` converts G6 to G5 in one ALU op
- **Exact pixel stride** — source advances by `w` pixels per row (not `w >> 1` pairs), correct for any flush width including odd sub-regions

## Saturn constraints

- **CPU**: SH-2 @ 28.6 MHz, no FPU
- **LVGL heap**: 48 KB (`LV_MEM_SIZE`)
- **Display**: 320x224, VDP2 NBG1 bitmap, 24-line double-buffered partial rendering
- **Drawing**: `LV_DRAW_SW_COMPLEX=0` (no rounded corners, shadows, or arcs)
- **Widgets**: label, button, bar only (to fit in 48KB)
- **Tick resolution**: ~16.67 ms (vblank-based, NTSC 60 Hz)
