# ==============================================================================
# Saturn LVGL Project Makefile (sh-elf-gcc + SGL ELF libs)
# ==============================================================================

# Ensure compiler DLLs (libgcc_s_dw2-1.dll etc.) are findable by cc1.exe
JOENGINE_DIR = ../../../joengine
export PATH := $(CURDIR)/../../Compiler/TOOLS:$(CURDIR)/$(JOENGINE_DIR)/Compiler/WINDOWS/bin:$(CURDIR)/$(JOENGINE_DIR)/Compiler/WINDOWS/Other Utilities:$(PATH)

INCLUDES = .

SRCS = main.c
SRCS += lv_port_disp.c
SRCS += lv_port_indev.c
SRCS += lv_port_tick.c
SRCS += libc_shims.c

# --- LVGL source files (minimal set for core + label + button) ---
include lvgl_srcs_minimal.mk

# --- SGL startup object (provides work area + calls main) ---
SYSOBJS = $(SGLLDR)/SGLAREA.O

OBJS = $(SRCS:.c=.o)

# --- SGL directories ---
# Headers: use SGL tutorial repo's clean headers (joengine's have broken comments)
COMPILER_DIR = ../../Compiler
SGLIDR = $(COMPILER_DIR)/SGL_302j/INC
# Libraries: use joengine's ELF-format libs (SGL tutorial only has COFF)
SGLLDR = $(JOENGINE_DIR)/Compiler/COMMON/SGL_302j/LIB_ELF

CMNDIR = $(JOENGINE_DIR)/Compiler/COMMON

LIBS +=  $(SGLLDR)/SEGA_SYS.A $(SGLLDR)/LIBCD.A $(SGLLDR)/LIBSGL.A $(SGLLDR)/LIBSND.A $(SGLLDR)/LIBMEM.A

CC = sh-elf-gcc
CONV = sh-elf-objcopy
RM = rm -f

CCFLAGS += -fkeep-inline-functions -W -Wall -Wshadow -Wbad-function-cast -Winline -Wcomment \
-Winline -Wsign-compare -Wextra -Wno-strict-aliasing \
-fno-common -ffast-math \
--param max-inline-insns-single=50 -fms-extensions -std=gnu99 \
-fmerge-all-constants -fno-ident -fno-unwind-tables -fno-asynchronous-unwind-tables \
-fomit-frame-pointer -fstrength-reduce -frerun-loop-opt -Os -nodefaultlibs -nostdlib -fno-builtin \
-m2 -c -I$(INCLUDES)

CCFLAGS += -I$(SGLIDR)

# --- LVGL include paths ---
CCFLAGS += -Ilvgl

# --- Suppress LVGL warnings ---
CCFLAGS += -Wno-unused-parameter
CCFLAGS += -Wno-sign-compare
CCFLAGS += -Wno-type-limits
CCFLAGS += -Wno-unused-function
CCFLAGS += -Wno-unused-variable
CCFLAGS += -Wno-missing-field-initializers
CCFLAGS += -Wno-cast-qual
CCFLAGS += -Wno-unused-but-set-variable
CCFLAGS += -Wno-implicit-fallthrough

# --- Define SATURN for SGL.H conditional includes ---
CCFLAGS += -DSATURN

LDFLAGS = -m2
LDFLAGS += -L$(SGLLDR)

LDFLAGS +=-Xlinker -T$(LDFILE) -Xlinker -Map \
          -Xlinker $(MPFILE) -Xlinker -e -Xlinker ___Start -nostartfiles
DFLAGS =

TARGET   = sl_coff.coff
TARGET1  = $(TARGET:.coff=.bin)
TARGET2  = $(TARGET:.coff=.iso)
MPFILE   = $(TARGET:.coff=.map)
IPFILE   = $(COMPILER_DIR)/COMMON/IP.BIN
LDFILE   = $(CMNDIR)/sgl.linker

MAKEFILE = makefile

all: $(TARGET) $(TARGET1) $(TARGET2)

$(TARGET) : $(SYSOBJS) $(OBJS) $(MAKEFILE) $(LDFILE)
	$(CC) $(LDFLAGS) $(SYSOBJS) $(OBJS) $(LIBS) -o $@

$(TARGET1) : $(TARGET) $(SYSOBJS) $(OBJS) $(MAKEFILE) $(LDFILE)
	$(CONV) -O binary $(TARGET) $(TARGET1)

$(TARGET2): $(TARGET1)
	$(CONV) -O binary $(TARGET) cd/0.bin
	mkisofs -quiet -sysid "SEGA SATURN" -volid "SATURN LVGL" -volset "SATURN LVGL" -publisher "LVGL ON SGL" -preparer "LVGL ON SGL" -appid "SATURN LVGL" -abstract "ABS.TXT" -copyright "CPY.TXT" -biblio "BIB.TXT" -generic-boot $(IPFILE) -full-iso9660-filenames -o $(TARGET2) cd

.c.o:
	$(CC) $< $(DFLAGS) $(CCFLAGS) $(_CCFLAGS) -o $@

clean:
	$(RM) $(OBJS) $(TARGET) $(TARGET1) $(TARGET2) $(MPFILE) cd/0.bin

re: clean all
