@echo off
rgb332.exe -h
echo -----------------------------------------------
rgb332.exe -i rgb.png -o rgb_dither.h -d
echo -----------------------------------------------
rgb332.exe -i rgb.png -o rgb.h
echo -----------------------------------------------
check.exe rgb_dither.h
echo -----------------------------------------------
check.exe rgb.h
pause