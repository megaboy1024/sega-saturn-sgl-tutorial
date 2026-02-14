@ECHO Off
SET COMPILER_DIR=..\..\Compiler
SET JOENGINE_DIR=..\..\..\joengine\Compiler
SET PATH=%COMPILER_DIR%\TOOLS;%JOENGINE_DIR%\WINDOWS\bin;%JOENGINE_DIR%\WINDOWS\Other Utilities;%PATH%
make re
JoEngineCueMaker
PAUSE
