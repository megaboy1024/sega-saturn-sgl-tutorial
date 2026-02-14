@ECHO Off
SET JOENGINE_DIR=..\..\..\joengine\Compiler
SET PATH=%JOENGINE_DIR%\WINDOWS\Other Utilities;%PATH%

rm -f ./cd/0.bin
rm -f *.o
rm -f ./sl_coff.bin
rm -f ./sl_coff.elf
rm -f ./sl_coff.map
rm -f ./sl_coff.iso
rm -f ./sl_coff.cue

REM Clean LVGL object files
find lvgl/src -name "*.o" -delete 2>NUL

ECHO Done.
