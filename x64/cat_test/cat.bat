@echo off
rgb332.exe -h
echo -----------------------------------------------
rgb332.exe -i cat.png -o cat_dither.h -d
echo -----------------------------------------------
rgb332.exe -i cat.png -o cat.h
echo -----------------------------------------------
check.exe cat_dither.h
echo -----------------------------------------------
check.exe cat.h
pause