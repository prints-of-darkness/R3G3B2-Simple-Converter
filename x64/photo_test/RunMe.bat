@echo off
rgb332.exe -h
echo -----------------------------------------------
rgb332.exe -i ParrotRedMacaw.jpg -o ParrotRedMacaw_dither.h -d
echo -----------------------------------------------
rgb332.exe -i ParrotRedMacaw.jpg -o ParrotRedMacaw.h
echo -----------------------------------------------
check.exe ParrotRedMacaw_dither.h
echo -----------------------------------------------
check.exe ParrotRedMacaw.h
pause