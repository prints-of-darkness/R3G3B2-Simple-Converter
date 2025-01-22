@echo off
rgb332.exe -h
echo -----------------------------------------------
rgb332.exe -i parrots.png -o parrots_dither.h -d
echo -----------------------------------------------
rgb332.exe -i parrots.png -o parrots.h
echo -----------------------------------------------
check.exe parrots_dither.h
echo -----------------------------------------------
check.exe parrots.h
pause