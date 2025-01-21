@echo off
rgb332.exe -h
echo -----------------------------------------------
rgb332.exe -i cat_solo.png -o cat_solo_dither.h -d
echo -----------------------------------------------
rgb332.exe -i cat_solo.png -o cat_solo.h
echo -----------------------------------------------
pause